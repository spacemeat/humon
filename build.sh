[[ -d obj ]] || mkdir obj
[[ -d bin ]] || mkdir bin
[[ -d test/bin ]] || mkdir test/bin
[[ -d samples/bin ]] || mkdir samples/bin

build_obj() {
    gcc -std=c99 -Wall -c -ggdb3 -O0 -DDEBUG -Iinc -o obj/$1-d.o src/$1.c
    gcc -std=c99 -Wall -c        -O3         -Iinc -o obj/$1.o   src/$1.c

    gcc -std=c99 -Wall -c -fPIC -ggdb3 -O0 -DDEBUG -Iinc -o obj/$1-rd.o src/$1.c
    gcc -std=c99 -Wall -c -fPIC        -O3         -Iinc -o obj/$1-r.o  src/$1.c
}

echo Building C objects...
rm obj/*.o
build_obj trove
build_obj vector
build_obj encoding
build_obj tokenize
build_obj parse
build_obj node
build_obj printing
build_obj utils
build_obj ansiColors

echo Done.

echo Building C library...
rm bin/*.a

echo -- static debug...
ar cr -o bin/libhumon-d.a obj/trove-d.o obj/vector-d.o obj/encoding-d.o obj/tokenize-d.o obj/parse-d.o obj/node-d.o obj/printing-d.o obj/utils-d.o obj/ansiColors-d.o
echo -- static release...
ar cr -o bin/libhumon.a   obj/trove.o   obj/vector.o   obj/encoding.o   obj/tokenize.o   obj/parse.o   obj/node.o   obj/printing.o   obj/utils.o   obj/ansiColors.o

echo -- shared debug...
g++ -std=c++17 -Wall -shared -Wl,-soname,libhumon-rd.so.0 -o bin/libhumon-rd.so.0.0.0 obj/trove-rd.o obj/vector-rd.o obj/encoding-rd.o obj/tokenize-rd.o obj/parse-rd.o obj/node-rd.o obj/printing-rd.o obj/utils-rd.o obj/ansiColors-rd.o
echo -- shared release...
g++ -std=c++17 -Wall -shared -Wl,-soname,libhumon-r.so.0  -o bin/libhumon-r.so.0.0.0  obj/trove-r.o  obj/vector-r.o  obj/encoding-r.o  obj/tokenize-r.o  obj/parse-r.o  obj/node-r.o  obj/printing-r.o  obj/utils-r.o  obj/ansiColors-r.o

echo Done.

echo Building samples...

echo "   ...sample-usage-c-d..."
gcc -std=c99 -Wall -ggdb3 -O0 -DDEBUG -Iinc -o samples/bin/sample-usage-c-d samples/usage-c/usage.c -Lbin -lhumon-d
echo "   ...sample-usage-cpp-d..."
g++ -std=c++17 -Wall -ggdb3 -O0 -DDEBUG -Iinc -o samples/bin/sample-usage-cpp-d samples/usage-cpp/usage.cpp -Lbin -lhumon-d -Wno-unused-value -Wno-unused-variable -Wno-unused-but-set-variable
echo "   ...hux..."
g++ -std=c++17 -Wall -ggdb3 -O0 -DDEBUG -Iinc -o samples/bin/hux samples/hux/hux.cpp -Lbin -lhumon-d

echo Building tests...
echo d
g++ -std=c++17 -Wall -o test/bin/test-d -ggdb3 -O0 -DDEBUG -Iinc test/testMain.cpp test/vectorTests.cpp test/apiTests.cpp test/utf8Tests.cpp test/dataTests.cpp test/cppTests.cpp test/commentTests.cpp test/errorTests.cpp -Lbin -lhumon-d -lCppUTest -lCppUTestExt
echo r
g++ -std=c++17 -Wall -o test/bin/test-r -O3 -Iinc test/testMain.cpp test/vectorTests.cpp test/apiTests.cpp test/utf8Tests.cpp test/dataTests.cpp test/cppTests.cpp test/commentTests.cpp test/errorTests.cpp -Lbin -lhumon -lCppUTest -lCppUTestExt

echo Done.

