

INCLUDES := -I doctest/doctest
INCLUDES += -I../third_party/spdlog/include
INCLUDES += -I../wsServer
INCLUDES += -I../wsServer/include
INCLUDES += -I../

STATIC_LINK := 
COMPILER = g++
STD_FLAG = -std=c++17
WERROR   = -Werror

SOURCES  := $(wildcard *.cpp)
EXECUTABLES := $(patsubst %.cpp,%,$(SOURCES))
linked_srcs = ../wallet.o ../logger.o ../config.o ../tools.o ../socket.o ../trader.o  ../strategies.o ../orderbook.o ../order.o
linked_srcs += ../wsServer/src/*.o


doctest/README.md:
	git clone https://github.com/doctest/doctest.git

all: $(EXECUTABLES) run 
	$(info The value of EXECUTABLES is: $(EXECUTABLES))

%: %.cpp
	$(COMPILER) $(STD_FLAG) $(INCLUDES) $^  $(linked_srcs) -o $@ 
	
	
memtest: $(EXECUTABLES) doctest/README.md
	$(foreach item,$(EXECUTABLES),valgrind ./$(item) -m;)


run: $(EXECUTABLES) doctest/README.md 
	$(foreach item,$(EXECUTABLES),./$(item);)


clean:
	rm -f $(EXECUTABLES)
