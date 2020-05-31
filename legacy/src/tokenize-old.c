#include "humon.internal.h"

/*
  11000010 10100000

  UTF8 whitespace:
    for columns:
    code point        utf8        
      09              09            \t      -- tab, count columns until modulus
      20              20            ' '     -- space
      a0              c2 a0
      1680            e1 9a 80
      2000            e2 80 80
      2001            e2 80 81
      2002            e2 80 82
      2003            e2 80 83
      2004            e2 80 84
      2005            e2 80 85
      2006            e2 80 86
      2007            e2 80 87
      2008            e2 80 88
      2009            e2 80 89
      200a            e2 80 8a
      202f            e2 80 af
      205f            e2 81 9f
      3000            e3 80 80

    for rows:
      0a              0a            \n -- lf
      0b              0b            \v -- vt
      0c              0c               -- ff
      0d              0d            \r -- cr, and don't count the next lf if there is one
      85              c2 85
      2028            e2 80 a8
      2029            e2 80 a9
*/

void eatWs(char const ** ppCur, int tabSize, int * line, int * col)
{
  bool eating = true;
  while (eating)
  {
    // we're free to look ahead because of the eof padding.
    char ca = **ppCur;
    char cb = *(*ppCur + 1);
    char cc = *(*ppCur + 2);

    // check the more likely ones first
    if (ca == ' ' ||
        ca == ',')
    {
      * ppCur += 1;
      * col += 1;
    }
    else if (ca == '\t')
    {
      * ppCur += 1;
      * col += tabSize - ((* col - 1) % tabSize);
    }
    else if (ca == '\n')
    {
      * ppCur += 1;
      * col = 1;
      * line += 1;
    }
    else if (ca == '\r')
    {
      * ppCur += 1;
      * col = 1;
      * line += 1;
      if (** ppCur == '\n')
        { * ppCur += 1; }
    }

    // less likely
    else if ((ca == 0xc2 && *(*ppCur + 1) == 0xa0))
    {
      * ppCur += 2;
      * col += 1;
    }
    else if ((ca == 0xe1 && cb == 0x9a && cc == 0x80) ||
             (ca == 0xe2 && cb == 0x80 && 
              cc >= 0x80 && cc <= 0x8a) ||
             (ca == 0xe2 && cb == 0x80 && cc == 0xaf) ||
             (ca == 0xe2 && cb == 0x81 && cc == 0x9f) ||
             (ca == 0xe3 && cb == 0x80 && cc == 0x80))
    {
      * ppCur += 3;
      * col += 1;
    }
    else if ((ca == 0xc2 && cb == 0x85))
    {
      * ppCur += 2;
      * col = 1;
      * line += 1;
    }
    else if ((ca == 0xe2 && cb == 0x80 && cc == 0xa8) ||
             (ca == 0xe2 && cb == 0x80 && cc == 0xa9))
    {
      * ppCur += 3;
      * col = 1;
      * line += 1;
    }
    else
      { eating = false; }    
  }
}


void measureWhitespace(char const * cur, int tabSize, int * len, int * line, int * col, bool eatOnlyOneNewline)
{
  bool eating = true;
  while (eating)
  {
    // we're free to look ahead because of the eof padding.
    char ca = *cur;
    char cb = *(cur + 1);
    char cc = *(cur + 2);

    // check the more likely ones first
    if (ca == ' ' ||
        ca == ',')
    {
      * len += 1;
      * col += 1;
    }
    else if (ca == '\t')
    {
      * len += 1;
      * col += tabSize - ((* col - 1) % tabSize);
    }
    else if (ca == '\n')
    {
      * len += 1;
      * col = 1;
      * line += 1;
      if (eatOnlyOneNewline)
        { eating = false; }
    }
    else if (ca == '\r')
    {
      * len += 1;
      * col = 1;
      * line += 1;
      if (cb == '\n')
        { * len += 1; }
      if (eatOnlyOneNewline)
        { eating = false; }
    }

    // less likely
    else if (ca == 0xc2 && cb == 0xa0)
    {
      * len += 2;
      * col += 1;
    }
    else if ((ca == 0xe1 && cb == 0x9a && cc == 0x80) ||
             (ca == 0xe2 && cb == 0x80 && 
              cc >= 0x80 && cc <= 0x8a) ||
             (ca == 0xe2 && cb == 0x80 && cc == 0xaf) ||
             (ca == 0xe2 && cb == 0x81 && cc == 0x9f) ||
             (ca == 0xe3 && cb == 0x80 && cc == 0x80))
    {
      * len += 3;
      * col += 1;
    }
    else if (ca == 0xc2 && cb == 0x85)
    {
      * len += 2;
      * col = 1;
      * line += 1;
      if (eatOnlyOneNewline)
        { eating = false; }
    }
    else if ((ca == 0xe2 && cb == 0x80 && cc == 0xa8) ||
             (ca == 0xe2 && cb == 0x80 && cc == 0xa9))
    {
      * len += 3;
      * col = 1;
      * line += 1;
      if (eatOnlyOneNewline)
        { eating = false; }
    }
    else
      { eating = false; }    
  }
}


void measureWhitespaceThroughOneNewline(char const * cur, int tabSize, int * len, int * line, int * col)
{
  measureWhitespace(cur, tabSize, len, line, col, true);
}


bool isNonNewlineWhitespace(char const * cur)
{
  // we're free to look ahead because of the eof padding.
  char ca = *cur;
  char cb = *(cur + 1);
  char cc = *(cur + 2);

  return (ca == ' ' ||
          ca == ',' ||
          ca == '\t' ||
          (ca == 0xc2 && cb == 0xa0) ||
          (ca == 0xe1 && cb == 0x9a && cc == 0x80) ||
          (ca == 0xe2 && cb == 0x80 && 
           cc >= 0x80 && cc <= 0x8a) ||
          (ca == 0xe2 && cb == 0x80 && cc == 0xaf) ||
          (ca == 0xe2 && cb == 0x81 && cc == 0x9f) ||
          (ca == 0xe3 && cb == 0x80 && cc == 0x80));
}


bool isNewline(char const * cur)
{
  // we're free to look ahead because of the eof padding.
  char ca = *cur;
  char cb = *(cur + 1);
  char cc = *(cur + 2);

  return (ca == '\n' ||
          ca == '\r' ||
          (ca == 0xc2 && cb == 0x85) ||
          (ca == 0xe2 && cb == 0x80 && cc == 0xa8) ||
          (ca == 0xe2 && cb == 0x80 && cc == 0xa9));
}


void measureUtf8Codepoint(char const * cur, int tabSize, int * len, int * line, int * col)
{    
  if (*cur & 0x80 == 0)
  {
    cur += 1;
    * len += 1;
    * col += 1;
  }
  else if (*cur & 0xc0 == 0xc0)
  {
    cur += 1;
    if (*cur & 0xc0 == 0x80)
    {
      * len += 2;
      * col += 1;
    }
  }
  else if (*cur & 0xe0 == 0xe0)
  {
    cur += 1;
    if (*cur & 0xc0 == 0x80)
    {
      cur != 1;
      if (*cur & 0xc0 == 0x80)
      {
        * len += 3;
        * col += 1;
      }
    }
  }
  else if (*cur & 0xf0 == 0xf0)
  {
    cur += 1;
    if (*cur & 0xc0 == 0x80)
    {
      cur != 1;
      if (*cur & 0xc0 == 0x80)
      {
        cur != 1;
        if (*cur & 0xc0 == 0x80)
        {
          * len += 4;
          * col += 1;
        }
      }
    }
  }
}


void measureDoubleSlashComment(char const * cur, int tabSize, int * len, int * line, int * col)
{
  * len = 2;
  * col += 2;

  char const * lookAhead = cur + 2;

  bool eating = true;
  while (eating)
  {
    if (*cur == '\0')
      { eating = false; }

    else if (isNewline(cur, len, NULL, NULL))
      { eating = false; }

    else
    {

    }

    measureNonNewlineWhitespace(cur, tabSize, len, line, col);

    measureNonWhitespace(cur, len, len, col);
  }

  /*
  while (* lookAhead != '\0' && 
         * lookAhead != '\n' && 
         * lookAhead != '\r')
  {
    lookAhead += 1;
    * len += 1;
    
    if (* lookAhead == '\n' ||
        * lookAhead == '\r')
    {
//      * col = 1;
    }
    else if (* lookAhead == '\t')
    {
      * col += (tabSize - (* col - 1) % tabSize);
    }
    else
    {
      * col += 1;
    }
  }
  */
}


void measureCstyleComment(char const * cur, int tabSize, int * len, int * line, int * col)
{
  * len = 2;
  * col += 2;

  char const * lookAhead1 = cur + 2;
  char const * lookAhead2 = cur + 3;

  if (* lookAhead1 == '\0')
    { return; }

  while ((* lookAhead1 != '*' ||
          * lookAhead2 != '/') && 
          * lookAhead2 != '\0')
  {
    lookAhead1 += 1;
    lookAhead2 += 1;
    * len += 1;
    if (* lookAhead1 == '\n' ||
        * lookAhead1 == '\r')
    {
      * col = 1;
      * line += 1;
    }
    else if (* lookAhead1 == '\t')
    {
      * col += (tabSize - (* col - 1) % tabSize);
    }
    else
    {
      * col += 1;
    }
  }

  if (* lookAhead2 == '\0')
  {
    * col += 1;
    * len += 1;
  }
  else
  {
    * col += 2;
    * len += 2;
  }
}


void measureWord(char const * cur, int * len, int * line, int * col)
{
  * len = 1;
  * col += 1;
  char const * lookAhead = cur + 1;

  bool scanning = true;
  while (scanning)
  {
    switch(*lookAhead)
    {
      case '\0':
      case ' ':
      case '\t':
      case ',':
      case '\r':
      case '\n':
      case '{':
      case '}':
      case '[':
      case ']':
      case ':':
      case '@':
      case '#':
        scanning = false;
        break;
      default:
        lookAhead += 1;
        * len += 1;
        * col += 1;
        break;
    }
  }
}


void measureQuotedWord(char const * cur, char quoteChar, int tabSize, int * len, int * line, int * col)
{
  * len = 1;
  * col += 1;
  char const * lookAhead = cur + 1;
  bool scanning = true;
  while (scanning)
  {
    if (*lookAhead == quoteChar)
    {
      cur += 1;
      * len += 1;
      scanning = false;
    }
    else
    {
      switch(*lookAhead)
      {
      case '\0':
        scanning = false;
        break;

      case '\\':
        cur += 1;
        lookAhead += 1;
        switch(*lookAhead)
        {
          case '\0':
            scanning = false;
            break;
          default:
            cur += 1;
            * len += 1;
            break;
        };
        break;
      
      case '\t':
        cur += 1;
        * col += tabSize - ((* col - 1) % tabSize);
        * len += 1;
        break;
      
      case '\r':
        cur += 1;
        * col = 1;
        * line += 1;
        if (* cur == '\n')
          { cur += 1; }
        break;

      case '\n':
        cur += 1;
        * col = 1;
        * line += 1;
        break;

      default:
        * len += 1;
      }
    }

    lookAhead += 1;
  }
}


void measureSingleQuotedWord(char const * cur, int tabSize, int * len, int * line, int * col)
{
  measureQuotedWord(cur, '\'', tabSize, len, line, col);
}


void measureDoubleQuotedWord(char const * cur, int tabSize, int * len, int * line, int * col)
{
  measureQuotedWord(cur, '"', tabSize, len, line, col);
}


void huTokenizeTrove(struct huTrove * trove)
{
  huResetVector(& trove->tokens);

  char const * beg = trove->dataString;
  char const * cur = beg;
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
    
    switch(* cur)
    {
    case '\0':
      allocNewToken(trove, HU_TOKENKIND_EOF, cur, 0, line, col, line, col);
      scanning = false;
      break;
    case '{':
      allocNewToken(trove, HU_TOKENKIND_STARTDICT, cur, 1, line, col, line, col + 1);
      cur += 1;
      col += 1;
      break;
    case '}':
      allocNewToken(trove, HU_TOKENKIND_ENDDICT, cur, 1, line, col, line, col + 1);
      cur += 1;
      col += 1;
      break;
    case '[':
      allocNewToken(trove, HU_TOKENKIND_STARTLIST, cur, 1, line, col, line, col + 1);
      cur += 1;
      col += 1;
      break;
    case ']':
      allocNewToken(trove, HU_TOKENKIND_ENDLIST, cur, 1, line, col, line, col + 1);
      cur += 1;
      col += 1;
      break;
    case ':':
      allocNewToken(trove, HU_TOKENKIND_KEYVALUESEP, cur, 1, line, col, line, col + 1);
      cur += 1;
      col += 1;
      break;
    case '@':
      allocNewToken(trove, HU_TOKENKIND_ANNOTATE, cur, 1, line, col, line, col + 1);
      cur += 1;
      col += 1;
      break;
    case '/':
      if (*(cur + 1) == '/')
      {
        measureDoubleSlashComment(cur, trove->inputTabSize, & len, & lineM, & colM);
        allocNewToken(trove, HU_TOKENKIND_COMMENT, cur, len, line, col, lineM, colM);
        cur += len;
        line = lineM;
        col = colM;
      }
      else if (*(cur + 1) == '*')
      {
        measureCstyleComment(cur, trove->inputTabSize, & len, & lineM, & colM);
        allocNewToken(trove, HU_TOKENKIND_COMMENT, cur, len, line, col, lineM, colM);
        cur += len;
        line = lineM;
        col = colM;
      }
      else
      {
        // else treat like a word char
        measureWord(cur, & len, & lineM, & colM);
        allocNewToken(trove, HU_TOKENKIND_WORD, cur, len, line, col, lineM, colM);
        cur += len;
        line = lineM;
        col = colM;
      }
      break;      
    case '"':
      measureDoubleQuotedWord(cur, trove->inputTabSize, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur, len, line, col, lineM, colM);
      cur += len;
      line = lineM;
      col = colM;
      break;
    case '\'':
      measureSingleQuotedWord(cur, trove->inputTabSize, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur, len, line, col, lineM, colM);
      cur += len;
      line = lineM;
      col = colM;
      break;
    default: // word char
      measureWord(cur, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur, len, line, col, lineM, colM);
      cur += len;
      line = lineM;
      col = colM;
      break;
    }
  }
}
