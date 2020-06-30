#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


int getcharLength(char const * cur)
{
    if ((*cur & 0x80) == 0)
        { return 1; }
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
    else if ((*cur & 0xc0) == 0xc0)
    {
        cur += 1;
        if ((*cur & 0xc0) == 0x80)
            { return 2; }
    }

    return 0;
}


void nextCharacter(huCursor * cursor)
{
    cursor->character += cursor->charLength;
    cursor->charLength = getcharLength(cursor->character);
    cursor->ws_col = 0;
    cursor->ws_line = 0;
}


void analyzeWhitespace(huCursor * cursor)
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
    // Combine cr+lf into one sort of 'codepoint'.
    if (ca == '\r' && cb == '\n')
    {
        cursor->charLength = 2;
    }
}


void eatWs(huCursor * cursor, int tabSize, int * line, int * col)
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


void eatDoubleSlashComment(huCursor * cursor, int tabSize, int * len, int * line, int * col)
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
            * len += cursor->charLength;
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


void eatCStyleComment(huCursor * cursor, int tabSize, int * len, int * line, int * col)
{
    // record the location for error reporting
    int tokenStartLine = * line;
    int tokenStartCol = * col;

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
            * len += cursor->charLength;
            * col = 1;
            * line += 1;
            nextCharacter(cursor);
        }
        else if (cursor->character[0] != '\0')
        {
            * len += cursor->charLength;
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
        {
          eating = false;
          recordTokenizeError(cursor->trove, HU_ERROR_UNFINISHEDCSTYLECOMMENT, 
            tokenStartLine, tokenStartCol);
        }
    }
}


void eatWord(huCursor * cursor, int * len, int * line, int * col)
{
    // The first character is already confirmed a word char, so, next please.
    * len += cursor->charLength;
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
        else if (cursor->character[0] == '\\')
        {
                // skip the '\'
            nextCharacter(cursor);
            analyzeWhitespace(cursor);
            // skip whatever is next, unless it's a newline.
            if (! cursor->ws_line)
            {
              * len += cursor->charLength;
              * col += 1;
            }
            nextCharacter(cursor);
        }
        else if (cursor->character[0] == '/' &&
                 (cursor->character[1] == '/' || cursor->character[2] == '*'))
        {
            // A comment abutting an unquoted string should still delimit.
            eating = false;
        }
        else
        {
            switch(cursor->character[0])
            {
            case '{': case '}': case '[': case ']':
            case ':': case '@': case '#':
                eating = false;
                break;
            default:
                * len += cursor->charLength;
                * col += 1;
                nextCharacter(cursor);
                break;
            }
        }
    }    
}


void eatQuotedWord(huCursor * cursor, int tabSize, int * len, int * line, int * col)
{
    // record the location for error reporting
    int tokenStartLine = * line;
    int tokenStartCol = * col;

    char quoteChar = * cursor->character;

    // The first character is already confirmed quoteChar, so, next please.
    * col += 1;
    nextCharacter(cursor);

    bool eating = true;
    while (eating)
    {
        analyzeWhitespace(cursor);
        if (cursor->character[0] == '\0')
        {
          eating = false;
          recordTokenizeError(cursor->trove, HU_ERROR_UNFINISHEDQUOTE, 
            tokenStartLine, tokenStartCol);
        }
        else if (cursor->ws_line)
        {
            * len += cursor->charLength;
            * col = 1;
            * line += 1;
            nextCharacter(cursor);
        }
        else
        {
            if (cursor->character[0] == '\t')
                { * col += tabSize - ((* col - 1) % tabSize); }
            else
                { * col += 1; }

            if (cursor->character[0] == quoteChar)
            {
                nextCharacter(cursor);
                eating = false;
            }
            else if (cursor->character[0] == '\\' && 
                cursor->character[1] == quoteChar)
            {
                nextCharacter(cursor);
                * len += 2;
                * col += 1;
                nextCharacter(cursor);
            }
            else
            {
                * len += cursor->charLength;
                nextCharacter(cursor);
            }
        }
    }
}


void huTokenizeTrove(huTrove * trove)
{
    huResetVector(& trove->tokens);

    char const * beg = trove->dataString;
    huCursor cur = 
        { .trove = trove, .character = beg, 
          .charLength = getcharLength(beg) };
    int line = 1;
    int col = 1;
    bool scanning = true;

    char bom[3] = { 0xef, 0xbb, 0xbf };
    if (memcmp(cur.character, bom, 3) == 0)
    {
        cur.character += 3;
        cur.charLength = getcharLength(cur.character);
    }

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
            allocNewToken(trove, HU_TOKENKIND_EOF, cur.character, 0, line, col, line, col, '\0');
            scanning = false;
            break;
        case '{':
            allocNewToken(trove, HU_TOKENKIND_STARTDICT, cur.character, 1, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& cur);
            break;
        case '}':
            allocNewToken(trove, HU_TOKENKIND_ENDDICT, cur.character, 1, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& cur);
            break;
        case '[':
            allocNewToken(trove, HU_TOKENKIND_STARTLIST, cur.character, 1, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& cur);
            break;
        case ']':
            allocNewToken(trove, HU_TOKENKIND_ENDLIST, cur.character, 1, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& cur);
            break;
        case ':':
            allocNewToken(trove, HU_TOKENKIND_KEYVALUESEP, cur.character, 1, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& cur);
            break;
        case '@':
            allocNewToken(trove, HU_TOKENKIND_ANNOTATE, cur.character, 1, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& cur);
            break;
        case '/':
            if (*(cur.character + 1) == '/')
            {
                eatDoubleSlashComment(& cur, trove->inputTabSize, & len, & lineM, & colM);
                allocNewToken(trove, HU_TOKENKIND_COMMENT, cur.character - len, len, line, col, lineM, colM, '\0');
                line = lineM;
                col = colM;
            }
            else if (*(cur.character + 1) == '*')
            {
                eatCStyleComment(& cur, trove->inputTabSize, & len, & lineM, & colM);
                allocNewToken(trove, HU_TOKENKIND_COMMENT, cur.character - len, len, line, col, lineM, colM, '\0');
                line = lineM;
                col = colM;
            }
            else
            {
                // else treat like a word char
                eatWord(& cur, & len, & lineM, & colM);
                allocNewToken(trove, HU_TOKENKIND_WORD, cur.character - len, len, line, col, lineM, colM, '\0');
                line = lineM;
                col = colM;
            }
            break;
        case '\'':
        case '"':
        case '`':
            {
                char quoteChar = cur.character[0];
                eatQuotedWord(& cur, trove->inputTabSize, & len, & lineM, & colM);
                allocNewToken(trove, HU_TOKENKIND_WORD, cur.character - len - 1, len, line, col, lineM, colM, quoteChar);
                line = lineM;
                col = colM;
            }
            break;
        default: // word char
            eatWord(& cur, & len, & lineM, & colM);
            allocNewToken(trove, HU_TOKENKIND_WORD, cur.character - len, len, line, col, lineM, colM, '\0');
            line = lineM;
            col = colM;
            break;
        }
    }
}
