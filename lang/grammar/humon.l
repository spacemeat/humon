%{
#include "humon2c.h"
#include "humon.tab.h"
int lineNum = 1;
char * qstringBegin, * qstringEnd;
%}

%x SQSTRING
%x DQSTRING
%option noyywrap
%option nounput

%%

[ \t]                               ;
\n                                  { lineNum += 1;  }
[^[\]{}():'",]+                     { yylval.node = new Node(NodeKind::Identifier, yytext); return IDENTIFIER; }
\'                                  { BEGIN SQSTRING; qstringBegin = yytext; }
<SQSTRING>\\\'                      { }
<SQSTRING>\'                        { BEGIN 0; /*buf has the string*/ }
<SQSTRING>.                         { }
\"                                  { BEGIN DQSTRING; /*make buf*/ }
<DQSTRING>\\\"                      { }
<DQSTRING>\"                        { BEGIN 0; qstringEnd = yytext; }
<DQSTRING>.                         { }
@                                   { return ANNOTATION; }
\(                                  { return LPAREN; }
\)                                  { return RPAREN; }
\[                                  { return LBRACKET; }
\]                                  { return RBRACKET; }
\{                                  { return LBRACE; }
\}                                  { return RBRACE; }
:                                   { return KEYVALUESEP; }
