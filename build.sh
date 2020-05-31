[[ -d obj ]] || mkdir obj
[[ -d bin ]] || mkdir bin

build_obj() {
    gcc -std=c99 -Wall -c -g -O0 -Iinc -o obj/$1-d.o src/$1.c
    gcc -std=c99 -Wall -c    -O3 -Iinc -o obj/$1.o   src/$1.c

    gcc -std=c99 -Wall -c -fPIC -g -O0 -Iinc -o obj/$1-rd.o src/$1.c
    gcc -std=c99 -Wall -c -fPIC    -O3 -Iinc -o obj/$1-r.o  src/$1.c
}

echo Building C objects...
rm obj/*.o
build_obj trove
build_obj tokenize
build_obj parse
build_obj node
build_obj utils
build_obj ansiColors

echo Done.

echo Building C library...
rm bin/*.a

echo -- static debug...
ar cr -o bin/libhumon-d.a obj/trove-d.o obj/tokenize-d.o obj/parse-d.o obj/node-d.o obj/utils-d.o obj/ansiColors-d.o
echo -- static release...
ar cr -o bin/libhumon.a   obj/trove.o   obj/tokenize.o   obj/parse.o   obj/node.o   obj/utils.o   obj/ansiColors.o

echo -- shared debug...
g++ --std=c++17 -Wall -shared -Wl,-soname,libhumon-rd.so.0 -o bin/libhumon-rd.so.0.0.0 obj/trove-rd.o obj/tokenize-rd.o obj/parse-rd.o obj/node-rd.o obj/utils-rd.o obj/ansiColors-rd.o
echo -- shared release...
g++ --std=c++17 -Wall -shared -Wl,-soname,libhumon-r.so.0  -o bin/libhumon-r.so.0.0.0  obj/trove-r.o  obj/tokenize-r.o  obj/parse-r.o  obj/node-r.o  obj/utils-r.o  obj/ansiColors-r.o

echo Done.

echo Building tests...
g++ --std=c++17 -Wall -g -O0 src/test/testMain.cpp src/test/utf8Tests.cpp src/test/dataTests.cpp src/test/cppTests.cpp -Iinc -Lbin -lhumon-d -lCppUTest -lCppUTestExt -o bin/test

echo Done.

