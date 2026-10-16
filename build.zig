const std = @import("std");

pub fn build(b: *std.Build) void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    // =========================================================
    // wsServer static library (C99)
    // =========================================================
    const ws_mod = b.createModule(.{
        .root_source_file = null,
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    ws_mod.addCSourceFiles(.{
        .files = &.{
            "third_party/wsServer/src/ws.c",
            "third_party/wsServer/src/utf8.c",
            "third_party/wsServer/src/sha1.c",
            "third_party/wsServer/src/handshake.c",
            "third_party/wsServer/src/base64.c",
        },
        .flags = &.{ "-Wall", "-Wextra", "-O2", "-std=c99", "-pedantic", "-DVALIDATE_UTF8" },
    });
    ws_mod.addIncludePath(b.path("third_party/wsServer/include"));

    const ws_lib = b.addLibrary(.{
        .name = "ws",
        .root_module = ws_mod,
        .linkage = .static,
    });
    b.installArtifact(ws_lib);

    // =========================================================
    // Main executable (C++17)
    // =========================================================
    const exe_mod = b.createModule(.{
        .root_source_file = null,
        .target = target,
        .optimize = optimize,
        .link_libc = true,
        .link_libcpp = true,
    });

    // C++ sources organized by layer
    exe_mod.addCSourceFiles(.{
        .files = &.{
            // core
            "src/core/order.cpp",
            "src/core/wallet.cpp",
            // util
            "src/util/config.cpp",
            "src/util/logger.cpp",
            "src/util/tools.cpp",
            // trading
            "src/trading/trader.cpp",
            "src/trading/orderbook.cpp",
            // net
            "src/net/socket.cpp",
            // top-level
            "src/main.cpp",
        },
        .flags = &.{ "-std=c++17", "-Wall", "-Wextra", "-Werror", "-g" },
    });

    // Include paths
    exe_mod.addIncludePath(b.path("src"));
    exe_mod.addIncludePath(b.path("src/core"));
    exe_mod.addIncludePath(b.path("src/util"));
    exe_mod.addIncludePath(b.path("src/trading"));
    exe_mod.addIncludePath(b.path("src/net"));
    exe_mod.addIncludePath(b.path("third_party/wsServer"));
    exe_mod.addIncludePath(b.path("third_party/wsServer/include"));
    exe_mod.addIncludePath(b.path("third_party/spdlog/include"));
    exe_mod.addIncludePath(b.path("third_party/nlohmann/include"));

    // Link dependencies
    exe_mod.linkLibrary(ws_lib);
    exe_mod.linkSystemLibrary("pthread", .{});
    exe_mod.linkSystemLibrary("m", .{});

    const exe = b.addExecutable(.{
        .name = "sim",
        .root_module = exe_mod,
    });
    b.installArtifact(exe);

    // =========================================================
    // Build steps
    // =========================================================

    // zig build run -- [args...]
    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(b.getInstallStep());
    if (b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the simulator");
    run_step.dependOn(&run_cmd.step);

    // zig build server
    const server_cmd = b.addRunArtifact(exe);
    server_cmd.step.dependOn(b.getInstallStep());
    server_cmd.addArgs(&.{ "-d", "-u", "1000", "-p", "8080", "-m", "2", "-t", "-1", "-st", "500", "-tr", "1000" });
    const server_step = b.step("server", "Run in WebSocket server mode");
    server_step.dependOn(&server_cmd.step);

    // zig build test
    const test_step = b.step("test", "Build and run all tests");
    const test_sources = [_][]const u8{
        "tests/CandlestickTest.cpp",
        "tests/ConfigTest.cpp",
        "tests/FlatHeepTests.cpp",
        "tests/LoggerTest.cpp",
        "tests/MarketTest.cpp",
        "tests/OrderBookTest.cpp",
        "tests/TraderTest.cpp",
        "tests/WalletTest.cpp",
        "tests/orderTest.cpp",
        "tests/simTest.cpp",
        "tests/socketTest.cpp",
        "tests/toolsTest.cpp",
    };

    for (test_sources) |src| {
        const name = std.fs.path.stem(src);

        const t_mod = b.createModule(.{
            .root_source_file = null,
            .target = target,
            .optimize = optimize,
            .link_libc = true,
            .link_libcpp = true,
        });

        // The test source file
        t_mod.addCSourceFile(.{
            .file = b.path(src),
            .flags = &.{"-std=c++17"},
        });

        // Re-use the main project's C++ sources
        t_mod.addCSourceFiles(.{
            .files = &.{
                "src/core/order.cpp",
                "src/core/wallet.cpp",
                "src/util/config.cpp",
                "src/util/logger.cpp",
                "src/util/tools.cpp",
                "src/trading/trader.cpp",
                "src/trading/orderbook.cpp",
                "src/net/socket.cpp",
            },
            .flags = &.{"-std=c++17"},
        });

        // Include paths for tests
        t_mod.addIncludePath(b.path("src"));
        t_mod.addIncludePath(b.path("src/core"));
        t_mod.addIncludePath(b.path("src/util"));
        t_mod.addIncludePath(b.path("src/trading"));
        t_mod.addIncludePath(b.path("src/net"));
        t_mod.addIncludePath(b.path("third_party/wsServer"));
        t_mod.addIncludePath(b.path("third_party/wsServer/include"));
        t_mod.addIncludePath(b.path("third_party/spdlog/include"));
        t_mod.addIncludePath(b.path("third_party/nlohmann/include"));
        t_mod.addIncludePath(b.path("tests/doctest"));

        t_mod.linkLibrary(ws_lib);
        t_mod.linkSystemLibrary("pthread", .{});
        t_mod.linkSystemLibrary("m", .{});

        const t_exe = b.addExecutable(.{
            .name = name,
            .root_module = t_mod,
        });
        b.installArtifact(t_exe);

        const run_t = b.addRunArtifact(t_exe);
        run_t.step.dependOn(b.getInstallStep());
        test_step.dependOn(&run_t.step);
    }

    // zig build clean
    const clean_cmd = b.addSystemCommand(&.{ "rm", "-rf", ".zig-cache", "zig-out" });
    const clean_step = b.step("clean", "Remove build artifacts");
    clean_step.dependOn(&clean_cmd.step);

    // zig build format
    const format_cmd = b.addSystemCommand(&.{ "clang-format", "--style=LLVM", "-i" });
    format_cmd.addArg("src");
    const format_step = b.step("format", "Run clang-format on source files");
    format_step.dependOn(&format_cmd.step);
}
