#include "humon2.h"
#include <numeric>


using namespace std;
using namespace humon;


inline void eatWs(char const *& cur, size_t tabSize, size_t & line, size_t & col) noexcept
{
  bool eating = true;
  while (eating)
  {
    if (*cur == ' ')
    {
      cur += 1;
      col += 1;
    }
    else if (*cur == '\t')
    {
      cur += 1;
      col += tabSize - (col % tabSize);
    }
    else if (*cur == ',')
    {
      cur += 1;
      col += 1;
    }
    else if (*cur == '\r')
    {
      cur += 1;
      col = 0;
      line += 1;
      if (*cur == '\n')
        { cur += 1; }
    }
    else if (*cur == '\n')
    {
      cur += 1;
      col = 0;
      line += 1;
      if (*cur == '\r')
        { cur += 1; }
    }
    else
      { eating = false; }    
  }


  while (std::isspace(static_cast<unsigned char>(*cur)) ||
         *cur == ',')
  {

  }
}


void measureComment(char const * cur, size_t & len, size_t & line, size_t & col) noexcept
{
  len = 1;
  col += 1;
  char const * lookAhead = cur + 1;

  while (* lookAhead != '\0' && 
         * lookAhead != '\n' && 
         * lookAhead != '\r')
  {
    lookAhead += 1;
    len += 1;
    col += 1;
  }
}


void measureWord(char const * cur, size_t & len, size_t & line, size_t & col) noexcept
{
  len = 1;
  col += 1;
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
        len += 1;
        col += 1;
        break;
    }
  }
}


void measureQuotedWord(char const * cur, char quoteChar, size_t tabSize, size_t & len, size_t & line, size_t & col) noexcept
{
  len = 1;
  col += 1;
  char const * lookAhead = cur + 1;
  bool scanning = true;
  while (scanning)
  {
    if (*lookAhead == quoteChar)
      { scanning = false; }
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
            len += 1;
            break;
        };
        break;
      
      case '\t':
        cur += 1;
        col += tabSize - (col % tabSize);
        len += 1;
        break;
      
      case '\r':
        cur += 1;
        col = 0;
        line += 1;
        if (*cur == '\n')
          { cur += 1; }
        break;

      case '\n':
        cur += 1;
        col = 0;
        line += 1;
        if (*cur == '\r')
          { cur += 1; }
        break;

      default:
        len += 1;
      }
    }

    lookAhead += 1;
  }
}


inline void measureSingleQuotedWord(char const * cur, size_t tabSize, size_t & len, size_t & line, size_t & col) noexcept
{
  measureQuotedWord(cur, '\'', tabSize, len, line, col);
}


inline void measureDoubleQuotedWord(char const * cur, size_t tabSize, size_t & len, size_t & line, size_t & col) noexcept
{
  measureQuotedWord(cur, '"', tabSize, len, line, col);
}


void HumonObj::tokenize()
{
  tokens.clear();
  char const * cur = data.data();
  size_t line = 0;
  size_t col = 0;
  while (*cur != '\0')
  {
    eatWs(cur, inputTabSize, line, col);
    size_t len = 0;
    size_t lineM = line;
    size_t colM = col;
    
    switch(*cur)
    {
    case '\0': break;
    case '{':
      tokens.emplace_back(TokenType::startDict, string_view {cur, 1}, line, col);
      cur += 1;
      col += 1;
      break;
    case '}':
      tokens.emplace_back(TokenType::endDict, string_view {cur, 1}, line, col); 
      cur += 1;
      col += 1;
      break;
    case '[':
      tokens.emplace_back(TokenType::startList, string_view {cur, 1}, line, col);
      cur += 1;
      col += 1;
      break;
    case ']':
      tokens.emplace_back(TokenType::endList, string_view {cur, 1}, line, col);
      cur += 1;
      col += 1;
      break;
    case ':':
      tokens.emplace_back(TokenType::keyValueSep, string_view {cur, 1}, line, col);
      cur += 1;
      col += 1;
      break;
    case '@':
      tokens.emplace_back(TokenType::annotate, string_view {cur, 1}, line, col);
      cur += 1;
      col += 1;
      break;
    case '#':
      measureComment(cur, len, lineM, colM);
      tokens.emplace_back(TokenType::comment, string_view {cur, len}, line, col);
      cur += len;
      line = lineM;
      col = colM;
      break;
    case '"':
      measureDoubleQuotedWord(cur, inputTabSize, len, lineM, colM);
      tokens.emplace_back(TokenType::word, string_view {cur, len}, line, col);
      cur += len;
      line = lineM;
      col = colM;
      break;
    case '\'':
      measureSingleQuotedWord(cur, inputTabSize, len, lineM, colM);
      tokens.emplace_back(TokenType::word, string_view {cur, len}, line, col);
      cur += len;
      line = lineM;
      col = colM;
      break;
    default: // word char
      measureWord(cur, len, lineM, colM);
      tokens.emplace_back(TokenType::word, string_view {cur, len}, line, col);
      cur += len;
      line = lineM;
      col = colM;
      break;
    }
  }

  tokens.emplace_back(TokenType::eof, string_view {cur, 1}, line, col);
}
