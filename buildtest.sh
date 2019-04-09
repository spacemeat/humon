[[ ! -d bin ]] && mkdir bin

echo Building...
g++ --std=c++17 -Wall -g -Og src/humon.cpp src/test.cpp -Iinc -o bin/test

echo Done.
