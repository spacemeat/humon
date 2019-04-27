[[ ! -d obj ]] && mkdir obj
[[ ! -d bin ]] && mkdir bin

echo Building...
rm obj/*.o
g++ --std=c++17 -Wall -c -g -Og -Iinc -o obj/humon.o src/humon.cpp 

echo Archiving...
rm bin/*.a
ar cr -o bin/libhumon-d.a obj/humon.o

echo Done.
