[[ ! -d obj ]] && mkdir obj
[[ ! -d bin ]] && mkdir bin

echo Building...
g++ --std=c++17 -Wall -c -fPIC -g -Og -Iinc -o obj/humon.o src/humon.cpp
g++ --std=c++17 -Wall -c -fPIC -g -Og -Iinc -o obj/humon2.o src/humon2.cpp 
g++ --std=c++17 -Wall -c -fPIC -g -Og -Iinc -o obj/parse.o src/parse.cpp 
g++ --std=c++17 -Wall -c -fPIC -g -Og -Iinc -o obj/tokenize.o src/tokenize.cpp 

echo Linking...
g++ --std=c++17 -Wall -shared -Wl,-soname,libhumon-d.so.0 -o bin/libhumon-d.so.0.0.0 obj/humon.o 
g++ --std=c++17 -Wall -shared -Wl,-soname,libhumon2-d.so.0 -o bin/libhumon2-d.so.0.0.0 obj/humon2.o obj/parse.o obj/tokenize.o

echo Done.
