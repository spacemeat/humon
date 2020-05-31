#include <stdlib.h>
#include <stdint.h>
#include "humon.internal.h"


int getCodepointLength(char const * cur)
{
  if ((*cur & 0x80) == 0)
      { return 1; }
  else if ((*cur & 0xc0) == 0xc0)
  {
    cur += 1;
    if ((*cur & 0xc0) == 0x80)
      { return 2; }
  }
  else if ((*cur & 0xe0) == 0xe0)
  {
    cur += 1;
    if ((*cur & 0xc0) == 0x80)
    {
      cur += 1;
      if ((*cur & 0xc0) == 0x80)
        { return 3; }
    }
  }
  else if ((*cur & 0xf0) == 0xf0)
  {
    cur += 1;
    if ((*cur & 0xc0) == 0x80)
    {
      cur += 1;
      if ((*cur & 0xc0) == 0x80)
      {
        cur += 1;
        if ((*cur & 0xc0) == 0x80)
            { return 4; }
      }
    }
  }

  return 0;
}


typedef struct cursor
{
    char const * character;
    uint8_t codepointLength;
    uint8_t ws_col;                // set if space-like whitespace
    uint8_t ws_line;               // set if newline-like whitespace
} cursor_t;


void nextCharacter(cursor_t * cursor)
{
    cursor->character += cursor->codepointLength;
    cursor->codepointLength = getCodepointLength(cursor->character);
    cursor->ws_col = 0;
    cursor->ws_line = 0;
}


void analyzeWhitespace(cursor_t * cursor)
{
    char ca = cursor->character[0];
    char cb = cursor->character[1];
    char cc = cursor->character[2];

    // split up the col check to cover the most common whitespace chars first.
    cursor->ws_col  = (ca == ' ' ||
                       ca == ',' ||
                       ca == '\t');
    cursor->ws_line = cursor->ws_col 
                    ? 0
                    : (ca == '\n' ||
                       ca == '\r' ||
                       (ca == 0xc2 && cb == 0x85) ||
                       (ca == 0xe2 && cb == 0x80 && cc == 0xa8) ||
                       (ca == 0xe2 && cb == 0x80 && cc == 0xa9));
    cursor->ws_col  = cursor->ws_line
                    ? 0
                    : (cursor->ws_col ||
                       (ca == 0xc2 && cb == 0xa0) ||
                       (ca == 0xe1 && cb == 0x9a && cc == 0x80) ||
                       (ca == 0xe2 && cb == 0x80 && 
                        cc >= 0x80 && cc <= 0x8a) ||
                       (ca == 0xe2 && cb == 0x80 && cc == 0xaf) ||
                       (ca == 0xe2 && cb == 0x81 && cc == 0x9f) ||
                       (ca == 0xe3 && cb == 0x80 && cc == 0x80));
    // Combine cr+lf into one 'codepoint'.
    if (ca == '\r' && cb == '\n')
    {
        cursor->codepointLength = 2;
    }
}


void eatWs(cursor_t * cursor, int tabSize, int * line, int * col)
{
    bool eating = true;
    while (eating)
    {
        analyzeWhitespace(cursor);
        if (cursor->ws_col)
        {
            if (cursor->character[0] == '\t')
                { * col += tabSize - ((* col - 1) % tabSize); }
            else
                { * col += 1; }
            nextCharacter(cursor);
        }
        else if (cursor->ws_line)
        {
            * col = 1;
            * line += 1;
            nextCharacter(cursor);
        }
        else
            { eating = false; }
    }
}


void eatDoubleSlashComment(cursor_t * cursor, int tabSize, int * len, int * line, int * col)
{
    // The first two characters are already confirmed //, so, next please.
    * len += 2;
    * col += 2;
    nextCharacter(cursor);
    nextCharacter(cursor);

    bool eating = true;
    while (eating)
    {
        analyzeWhitespace(cursor);
        if (! cursor->ws_line && 
            cursor->character[0] != '\0')
        {
            * len += cursor->codepointLength;
            if (cursor->character[0] == '\t')
                { * col += tabSize - ((* col - 1) % tabSize); }
            else
                { * col += 1; }
            nextCharacter(cursor);
        }
        else
            { eating = false; }
    }
}


void eatCStyleComment(cursor_t * cursor, int tabSize, int * len, int * line, int * col)
{
    // The first two characters are already confirmed /*, so, next please.
    nextCharacter(cursor);
    nextCharacter(cursor);
    * len += 2;
    * col += 2;

    bool eating = true;
    while (eating)
    {
        analyzeWhitespace(cursor);
        if (cursor->character[0] == '\0')
            { eating = false; }
        else if (cursor->ws_line)
        {
            * len += cursor->codepointLength;
            * col = 1;
            * line += 1;
            nextCharacter(cursor);
        }
        else if (cursor->character[0] != '\0')
        {
            * len += cursor->codepointLength;
            if (cursor->character[0] == '\t')
                { * col += tabSize - ((* col - 1) % tabSize); }
            else
                { * col += 1; }

            if (cursor->character[0] == '*' && 
                cursor->character[1] == '/')
            {
                // skip the *
                nextCharacter(cursor);
                // skip the /
                * len += 1;
                * col += 1;
                nextCharacter(cursor);
                eating = false;
            }
            else
            {
                nextCharacter(cursor);
            }
        }
        else
            { eating = false; }
    }
}


void eatWord(cursor_t * cursor, int * len, int * line, int * col)
{
    // The first character is already confirmed a word char, so, next please.
    * len += cursor->codepointLength;
    * col += 1;
    nextCharacter(cursor);

    bool eating = true;
    while (eating)
    {
        analyzeWhitespace(cursor);
        if (cursor->character[0] == '\0')
            { eating = false; }
        else if (cursor->ws_line || cursor->ws_col)
            { eating = false; }
        else
        {
            switch(cursor->character[0])
            {
            case '{': case '}': case '[': case ']':
            case ':': case '@': case '#':
                eating = false;
                break;
            default:
                * len += cursor->codepointLength;
                * col += 1;
                nextCharacter(cursor);
                break;
            }
        }
    }    
}


void eatQuotedWord(cursor_t * cursor, char quoteChar, int tabSize, int * len, int * line, int * col)
{
    // The first character is already confirmed quoteChar, so, next please.
    * len += 1;
    * col += 1;
    nextCharacter(cursor);

    bool eating = true;
    while (eating)
    {
        analyzeWhitespace(cursor);
        if (cursor->character[0] == '\0')
            { eating = false; /*TODO: error: Unfinished quoted string before EOF */}
        else if (cursor->ws_line)
        {
            * len += cursor->codepointLength;
            * col = 1;
            * line += 1;
            nextCharacter(cursor);
        }
        else
        {
            * len += cursor->codepointLength;
            if (cursor->character[0] == '\t')
                { * col += tabSize - ((* col - 1) % tabSize); }
            else
                { * col += 1; }

            if (cursor->character[0] == quoteChar)
            {
                nextCharacter(cursor);
                eating = false;
            }
            else if (cursor->character[0] != '\\' && 
                cursor->character[1] == quoteChar)
            {
                nextCharacter(cursor);
                * len += 1;
                * col += 1;
                nextCharacter(cursor);
                eating = false;
            }
            else
            {
                nextCharacter(cursor);
            }
        }
    }
}


void eatSingleQuotedWord(cursor_t * cur, int tabSize, int * len, int * line, int * col)
{
  eatQuotedWord(cur, '\'', tabSize, len, line, col);
}


void eatDoubleQuotedWord(cursor_t * cur, int tabSize, int * len, int * line, int * col)
{
  eatQuotedWord(cur, '"', tabSize, len, line, col);
}


void eatBackQuotedWord(cursor_t * cur, int tabSize, int * len, int * line, int * col)
{
  eatQuotedWord(cur, '`', tabSize, len, line, col);
}


void huTokenizeTrove(struct huTrove * trove)
{
  huResetVector(& trove->tokens);

  char const * beg = trove->dataString;
  cursor_t cur = 
    { .character = beg, .codepointLength = getCodepointLength(beg) };
  int line = 1;
  int col = 1;
  bool scanning = true;

  // lexi scan
  while (scanning)
  {
    eatWs(& cur, trove->inputTabSize, & line, & col);
    int len = 0;
    int lineM = line;
    int colM = col;
    
    switch(* cur.character)
    {
    case '\0':
      allocNewToken(trove, HU_TOKENKIND_EOF, cur.character, 0, line, col, line, col);
      scanning = false;
      break;
    case '{':
      allocNewToken(trove, HU_TOKENKIND_STARTDICT, cur.character, 1, line, col, line, col + 1);
      col += 1;
      nextCharacter(& cur);
      break;
    case '}':
      allocNewToken(trove, HU_TOKENKIND_ENDDICT, cur.character, 1, line, col, line, col + 1);
      col += 1;
      nextCharacter(& cur);
      break;
    case '[':
      allocNewToken(trove, HU_TOKENKIND_STARTLIST, cur.character, 1, line, col, line, col + 1);
      col += 1;
      nextCharacter(& cur);
      break;
    case ']':
      allocNewToken(trove, HU_TOKENKIND_ENDLIST, cur.character, 1, line, col, line, col + 1);
      col += 1;
      nextCharacter(& cur);
      break;
    case ':':
      allocNewToken(trove, HU_TOKENKIND_KEYVALUESEP, cur.character, 1, line, col, line, col + 1);
      col += 1;
      nextCharacter(& cur);
      break;
    case '@':
      allocNewToken(trove, HU_TOKENKIND_ANNOTATE, cur.character, 1, line, col, line, col + 1);
      col += 1;
      nextCharacter(& cur);
      break;
    case '/':
      if (*(cur.character + 1) == '/')
      {
        eatDoubleSlashComment(& cur, trove->inputTabSize, & len, & lineM, & colM);
        allocNewToken(trove, HU_TOKENKIND_COMMENT, cur.character - len, len, line, col, lineM, colM);
        line = lineM;
        col = colM;
      }
      else if (*(cur.character + 1) == '*')
      {
        eatCStyleComment(& cur, trove->inputTabSize, & len, & lineM, & colM);
        allocNewToken(trove, HU_TOKENKIND_COMMENT, cur.character - len, len, line, col, lineM, colM);
        line = lineM;
        col = colM;
      }
      else
      {
        // else treat like a word char
        eatWord(& cur, & len, & lineM, & colM);
        allocNewToken(trove, HU_TOKENKIND_WORD, cur.character - len, len, line, col, lineM, colM);
        line = lineM;
        col = colM;
      }
      break;      
    case '"':
      eatDoubleQuotedWord(& cur, trove->inputTabSize, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur.character - len, len, line, col, lineM, colM);
      line = lineM;
      col = colM;
      break;
    case '\'':
      eatSingleQuotedWord(& cur, trove->inputTabSize, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur.character - len, len, line, col, lineM, colM);
      line = lineM;
      col = colM;
      break;
    case '`':
      eatBackQuotedWord(& cur, trove->inputTabSize, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur.character - len, len, line, col, lineM, colM);
      line = lineM;
      col = colM;
      break;
    default: // word char
      eatWord(& cur, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur.character - len, len, line, col, lineM, colM);
      line = lineM;
      col = colM;
      break;
    }
  }
}
