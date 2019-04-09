[[ ! -d obj ]] && mkdir obj
[[ ! -d bin ]] && mkdir bin

echo Building...
g++ --std=c++17 -Wall -c -g -Og -Iinc -o obj/humon.o src/humon.cpp 

echo Archiving...
ar cqv -o bin/libhumon-d.a obj/humon.o

echo Done.
