#include "humon.internal.h"


void eatWs(char const ** ppCur, int tabSize, int * line, int * col)
{
  bool eating = true;
  while (eating)
  {
    if (** ppCur == ' ')
    {
      * ppCur += 1;
      * col += 1;
    }
    else if (** ppCur == '\t')
    {
      * ppCur += 1;
      * col += tabSize - (* col % tabSize);
    }
    else if (**ppCur == ',')
    {
      * ppCur += 1;
      * col += 1;
    }
    else if (**ppCur == '\r')
    {
      * ppCur += 1;
      * col = 0;
      * line += 1;
      if (** ppCur == '\n')
        { * ppCur += 1; }
    }
    else if (** ppCur == '\n')
    {
      * ppCur += 1;
      * col = 0;
      * line += 1;
      if (** ppCur == '\r')
        { * ppCur += 1; }
    }
    else
      { eating = false; }    
  }
}


void measureComment(char const * cur, int tabSize, int * len, int * line, int * col)
{
  * len = 1;
  * col += 1;
  char const * lookAhead = cur + 1;

  while (* lookAhead != '\0' && 
        * lookAhead != '\n' && 
        * lookAhead != '\r')
  {
    lookAhead += 1;
    * len += 1;
    if (* lookAhead == '\n' ||
        * lookAhead == '\r')
    {
      * col = 1;
      * line += 1;
    }
    else if (* lookAhead == '\t')
    {
      * col += (tabSize - * col % tabSize);
    }
    else
    {
      * col += 1;
    }
  }
}


void measureDoubleSlashComment(char const * cur, int tabSize, int * len, int * line, int * col)
{
  * len = 2;
  * col += 2;

  char const * lookAhead = cur + 2;

  while (* lookAhead != '\0' && 
         * lookAhead != '\n' && 
         * lookAhead != '\r')
  {
    lookAhead += 1;
    * len += 1;
    
    if (* lookAhead == '\n' ||
        * lookAhead == '\r')
    {
      * col = 1;
      * line += 1;
    }
    else if (* lookAhead == '\t')
    {
      * col += (tabSize - * col % tabSize);
    }
    else
    {
      * col += 1;
    }
  }
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
      * col += (tabSize - * col % tabSize);
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
        * col += tabSize - (* col % tabSize);
        * len += 1;
        break;
      
      case '\r':
        cur += 1;
        * col = 0;
        * line += 1;
        if (* cur == '\n')
          { cur += 1; }
        break;

      case '\n':
        cur += 1;
        * col = 0;
        * line += 1;
        if (*cur == '\r')
          { cur += 1; }
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
  int line = 0;
  int col = 0;
  while (*cur != '\0')
  {
    eatWs(& cur, trove->inputTabSize, & line, & col);
    int len = 0;
    int lineM = line;
    int colM = col;
    
    switch(*cur)
    {
    case '\0': break;
    case '{':
      allocNewToken(trove, HU_TOKENKIND_STARTDICT, cur, 1, line, col);
      cur += 1;
      col += 1;
      break;
    case '}':
      allocNewToken(trove, HU_TOKENKIND_ENDDICT, cur, 1, line, col);
      cur += 1;
      col += 1;
      break;
    case '[':
      allocNewToken(trove, HU_TOKENKIND_STARTLIST, cur, 1, line, col);
      cur += 1;
      col += 1;
      break;
    case ']':
      allocNewToken(trove, HU_TOKENKIND_ENDLIST, cur, 1, line, col);
      cur += 1;
      col += 1;
      break;
    case ':':
      allocNewToken(trove, HU_TOKENKIND_KEYVALUESEP, cur, 1, line, col);
      cur += 1;
      col += 1;
      break;
    case '@':
      allocNewToken(trove, HU_TOKENKIND_ANNOTATE, cur, 1, line, col);
      cur += 1;
      col += 1;
      break;
    case '#':
      measureComment(cur, trove->inputTabSize, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_COMMENT, cur, len, line, col);
      cur += len;
      line = lineM;
      col = colM;
      break;
    case '/':
      if (*(cur + 1) == '/')
      {
        measureDoubleSlashComment(cur, trove->inputTabSize, & len, & lineM, & colM);
        allocNewToken(trove, HU_TOKENKIND_COMMENT, cur, len, line, col);
        cur += len;;
        line = lineM;
        col = colM;
      }
      else if (*(cur + 1) == '*')
      {
        measureCstyleComment(cur, trove->inputTabSize, & len, & lineM, & colM);
        allocNewToken(trove, HU_TOKENKIND_COMMENT, cur, len, line, col);
        cur += len;
        line = lineM;
        col = colM;
      }
      else
      {
        // else treat like a word char
        measureWord(cur, & len, & lineM, & colM);
        allocNewToken(trove, HU_TOKENKIND_WORD, cur, len, line, col);
        cur += len;
        line = lineM;
        col = colM;
      }
      break;      
    case '"':
      measureDoubleQuotedWord(cur, trove->inputTabSize, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur, len, line, col);
      cur += len;
      line = lineM;
      col = colM;
      break;
    case '\'':
      measureSingleQuotedWord(cur, trove->inputTabSize, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur, len, line, col);
      cur += len;
      line = lineM;
      col = colM;
      break;
    default: // word char
      measureWord(cur, & len, & lineM, & colM);
      allocNewToken(trove, HU_TOKENKIND_WORD, cur, len, line, col);
      cur += len;
      line = lineM;
      col = colM;
      break;
    }
  }

  allocNewToken(trove, HU_TOKENKIND_EOF, cur, 1, line, col);
}

