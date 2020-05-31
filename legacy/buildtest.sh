[[ ! -d bin ]] && mkdir bin

echo Building...
g++ --std=c++17 -Wall -g -Og src/humon.cpp src/test.cpp -Iinc -o bin/test
g++ --std=c++17 -Wall -g -O0 src/test2.cpp -Iinc -I../../local/ansiTerm/inc -o bin/test2 -lfmt

echo Done.
