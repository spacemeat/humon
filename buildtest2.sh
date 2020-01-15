[[ ! -d bin ]] && mkdir bin

source ./buildlib2.sh

echo Building...
g++ --std=c++17 -Wall -g -Og src/test/testMain.cpp src/test/dataTests.cpp -Iinc -Lbin -lhumon2c-d -lCppUTest -lCppUTestExt -o bin/test

echo Done.
