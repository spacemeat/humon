[[ ! -d obj ]] && mkdir obj
[[ ! -d bin ]] && mkdir bin

echo Building...
g++ --std=c++17 -Wall -c -fPIC -g -Og -Iinc -o obj/humon.o src/humon.cpp

echo Linking...
g++ --std=c++17 -Wall -shared -Wl,-soname,libhumon-d.so.0 -o bin/libhumon-d.so.0.0.0 obj/humon.o 

echo Done.
