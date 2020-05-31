%{
#include "stdlib.h"
#include "string.h"
#include "intentNode.h"
#include <string_view>
#include <iostream>

// stuff from flex that bison needs to know about:
extern int yylex();
extern int yyparse();
extern FILE * yyin;
extern int lineNum;
void yyerror(char const * s);

class Node;
extern Node * topLevelStatement;

#define YYT std::cout << yystate << " "

%}

%union {
    Node * node;
}

%start topLevel

%%

topLevel:
        