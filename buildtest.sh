[[ ! -d bin ]] && mkdir bin

echo Building...
g++ --std=c++17 -Wall -g -Og src/humon.cpp src/test.cpp -Iinc -o bin/test
g++ --std=c++17 -Wall -g -Og src/humon2.cpp src/parse.cpp src/tokenize.cpp src/test2.cpp -Iinc -o bin/test2 -L../../balls/fmt/build-5.3.0 -lfmt

echo Done.
