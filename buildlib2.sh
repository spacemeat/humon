[[ ! -d obj ]] && mkdir obj
[[ ! -d bin ]] && mkdir bin

echo Building...
rm obj/*.o
gcc -std=c99 -Wall -c -g -Og -Iinc -o obj/humon2Trove.o src/humon2Trove.c 
gcc -std=c99 -Wall -c -g -Og -Iinc -o obj/humon2Tokenize.o src/humon2Tokenize.c 
gcc -std=c99 -Wall -c -g -Og -Iinc -o obj/humon2Parse.o src/humon2Parse.c 
gcc -std=c99 -Wall -c -g -Og -Iinc -o obj/humon2Node.o src/humon2Node.c 
gcc -std=c99 -Wall -c -g -Og -Iinc -o obj/humon2Utils.o src/humon2Utils.c 

echo Archiving...
rm bin/*.a
ar cr -o bin/libhumon2c-d.a obj/humon2Trove.o obj/humon2Parse.o obj/humon2Tokenize.o

echo Done.
