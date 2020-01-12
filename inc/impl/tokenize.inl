#pragma once

// This #include won't resolve because this file is actually inclued from humon2.h, but it's nice for the editor.
#include "humon2cpp.h"
#include <numeric>

namespace humon
{
  template <typename idx_t>
  inline void eatWs(char const *& cur, idx_t tabSize, idx_t & line, idx_t & col) noexcept
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
  }


  template <typename idx_t>
  void measureComment(char const * cur, idx_t & len, idx_t & line, idx_t & col) noexcept
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


  template <typename idx_t>
  void measureWord(char const * cur, idx_t & len, idx_t & line, idx_t & col) noexcept
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


  template <typename idx_t>
  void measureQuotedWord(char const * cur, char quoteChar, idx_t tabSize, idx_t & len, idx_t & line, idx_t & col) noexcept
  {
    len = 1;
    col += 1;
    char const * lookAhead = cur + 1;
    bool scanning = true;
    while (scanning)
    {
      if (*lookAhead == quoteChar)
      {
        cur += 1;
        len += 1;
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


  template <typename idx_t>
  inline void measureSingleQuotedWord(char const * cur, idx_t tabSize, idx_t & len, idx_t & line, idx_t & col) noexcept
  {
    measureQuotedWord(cur, '\'', tabSize, len, line, col);
  }


  template <typename idx_t>
  inline void measureDoubleQuotedWord(char const * cur, idx_t tabSize, idx_t & len, idx_t & line, idx_t & col) noexcept
  {
    measureQuotedWord(cur, '"', tabSize, len, line, col);
  }


  template <typename T>
  void Trove<T>::tokenize()
  {
    tokens.clear();
    char const * beg = data.data();
    char const * cur = beg;
    idx_t line = 0;
    idx_t col = 0;
    while (*cur != '\0')
    {
      eatWs(cur, inputTabSize, line, col);
      idx_t len = 0;
      idx_t lineM = line;
      idx_t colM = col;
      
      switch(*cur)
      {
      case '\0': break;
      case '{':
        tokens.emplace_back(TokenKind::startDict, data, std::string_view{cur, 1}, line, col);
        cur += 1;
        col += 1;
        break;
      case '}':
        tokens.emplace_back(TokenKind::endDict, data, std::string_view{cur, 1}, line, col); 
        cur += 1;
        col += 1;
        break;
      case '[':
        tokens.emplace_back(TokenKind::startList, data, std::string_view{cur, 1}, line, col);
        cur += 1;
        col += 1;
        break;
      case ']':
        tokens.emplace_back(TokenKind::endList, data, std::string_view{cur, 1}, line, col);
        cur += 1;
        col += 1;
        break;
      case ':':
        tokens.emplace_back(TokenKind::keyValueSep, data, std::string_view {cur, 1}, line, col);
        cur += 1;
        col += 1;
        break;
      case '@':
        tokens.emplace_back(TokenKind::annotate, data, std::string_view {cur, 1}, line, col);
        cur += 1;
        col += 1;
        break;
      case '#':
        measureComment(cur, len, lineM, colM);
        tokens.emplace_back(TokenKind::comment, data, std::string_view {cur, len}, line, col);
        cur += len;
        line = lineM;
        col = colM;
        break;
      case '"':
        measureDoubleQuotedWord(cur, inputTabSize, len, lineM, colM);
        tokens.emplace_back(TokenKind::word, data, std::string_view {cur, len}, line, col);
        cur += len;
        line = lineM;
        col = colM;
        break;
      case '\'':
        measureSingleQuotedWord(cur, inputTabSize, len, lineM, colM);
        tokens.emplace_back(TokenKind::word, data, std::string_view {cur, len}, line, col);
        cur += len;
        line = lineM;
        col = colM;
        break;
      default: // word char
        measureWord(cur, len, lineM, colM);
        tokens.emplace_back(TokenKind::word, data, std::string_view {cur, len}, line, col);
        cur += len;
        line = lineM;
        col = colM;
        break;
      }
    }

    tokens.emplace_back(TokenKind::eof, data, std::string_view {cur, 1}, line, col);
  }
}
