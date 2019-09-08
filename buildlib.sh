[[ ! -d obj ]] && mkdir obj
[[ ! -d bin ]] && mkdir bin

echo Building...
rm obj/*.o
g++ --std=c++17 -Wall -c -g -Og -Iinc -o obj/humon.o src/humon.cpp 
g++ --std=c++17 -Wall -c -g -Og -Iinc -o obj/humon2.o src/humon2.cpp 
g++ --std=c++17 -Wall -c -g -Og -Iinc -o obj/parse.o src/parse.cpp 
g++ --std=c++17 -Wall -c -g -Og -Iinc -o obj/tokenize.o src/tokenize.cpp 

echo Archiving...
rm bin/*.a
ar cr -o bin/libhumon-d.a obj/humon.o
ar cr -o bin/libhumon2-d.a obj/humon2.o obj/parse.o obj/tokenize.o

echo Done.
