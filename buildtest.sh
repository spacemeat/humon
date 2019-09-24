[[ ! -d bin ]] && mkdir bin

echo Building...
g++ --std=c++17 -Wall -g -Og src/humon.cpp src/test.cpp -Iinc -o bin/test
g++ --std=c++17 -Wall -g -O0 src/test2.cpp -Iinc -I../../balls/fmt/fmt-5.3.0/include -I../ansiTermCpp/inc -o bin/test2 -L../../balls/fmt/build-5.3.0 -lfmt

echo Done.
