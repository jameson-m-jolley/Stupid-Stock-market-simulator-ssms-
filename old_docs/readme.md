
dependencies;
    you will need make, and clang-formant
    https://www.gnu.org/software/make/manual/make.html
    https://clang.llvm.org/docs/ClangFormat.html

making:
    we are using strict compiler flags so that we avoid errors 
    the make options are as fallows if no options is given 
    the project will build.
    if you encounter this error: *No rule to make target 'wsServer/src/ws.o', needed by 'sim'*
    rerun *make sim* or *make* command.
    make [options]:

    options:
        test:
            builds and runs tests

        clean:
            removes all build files

        run: 
            this will compile and run

        sever:
            Creates a server with examplary options


command line args:
sim [options]:

    -help   prints help and exits
    -seed   sets the random seed
    -unsafe ignore runtime checks
    -d      debug infomation output
    -m      mode options [1 = text based output, 2 = websocket mode, 3 = monty carlo mode]
    -u      update speed options [0 = fastest possible, 1000 = default]
    -s      size this sets the size of the sim; default 100
    -t      sets the runtime (ms); default -1 = indefinite
    -p      sets the port; default 443 (mode 2 only)
    -f      path to JSON config file (creates one if missing)
    -c      clamp invalid values
    -tr     sets number of traders; default 10
    -st     sets number of stocks; default 5
    -sp     sets initial stock price (cents); default 1000
    -tw     sets initial wallet (dollars); default 500
    -so     sets number of seed orders; defualt 5

Example usage:
./sim -m 2 -p 8080 -u 1000 -t -1 -tr 10 -st 5 -sp 1000 -tw 500 -so 3

Once Server is running click on the folder titled frontEnd. Inside frontEnd click on index.html and open in a browser.
If you are on a 'Waiting for Server' screen and the server is open and running, click on the hamburger button on the top right and press connect. 
If the server is made correctly, you should see the main screen.
___
# install

the way the makefile is set up requiers you to have clangformat so you must have gcc and g++ and clangfromant installed as well as make

<pre>
    git clone https://github.com/NMSU-CS-CS371/team-project-stock-market-simulator.git
    cd team-project-stock-market-simulator
    make
</pre>

or for ssh 

<pre>
    git clone git@github.com:NMSU-CS-CS371/team-project-stock-market-simulator.git
    cd team-project-stock-market-simulator
    make
</pre>
