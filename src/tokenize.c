#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


static void analyzeCharacter(huScanner * scanner)
{
    huCursor * cursor = scanner->nextCursor;
    if ((cursor->character[0] & 0b10000000) == 0)
    {
        cursor->charLength = 1;
        cursor->codePoint = cursor->character[0];
        // treating crlf as one newline
        if (cursor->character[0] == '\0')
            { cursor->isEof = true; }
        else if (cursor->character[0] == '\r' && cursor->character[1] == '\n')
            { cursor->charLength = 2; }
    }
    else if ((cursor->character[0] & 0b11100000) == 0b11000000)
    {
        cursor->charLength = 2;
        cursor->codePoint = (cursor->character[1] & 0b00111111) |
                            ((cursor->character[0] & 0b00011111) << 6);
    }
    else if ((cursor->character[0] & 0b11110000) == 0b11100000)
    {
        cursor->charLength = 3;
        cursor->codePoint = (cursor->character[2] & 0b00111111) |
                            ((cursor->character[1] & 0b00111111) << 6) |
                            ((cursor->character[0] & 0b00001111) << 12);
    }
    else if ((cursor->character[0] & 0b11111000) == 0b11110000)
    {
        cursor->charLength = 4;
        cursor->codePoint = (cursor->character[3] & 0b00111111) |
                            ((cursor->character[2] & 0b00111111) << 6) |
                            ((cursor->character[1] & 0b00111111) << 12) |
                            ((cursor->character[0] & 0b00000111) << 18);
    }
    else
    {
        cursor->charLength = 0; // Error! Yaaaaaay!
        cursor->codePoint = 0;
        cursor->isError = true;
        recordTokenizeError(scanner->trove, HU_ERROR_BADENCODING, 0, 0);
    }
}


static void analyzeWhitespace(huScanner * scanner)
{
    // NOTE: Opportunity to test perf here. Try breaking up the switch, 
    // to ensure we cover the most common single-byte cases first?
    huCursor * cursor = scanner->nextCursor;
    switch (cursor->codePoint)
    {
        case 0x09:      // '\t':
            cursor->isTab = true;
            break;
        case 0x20:      // ' ':
        case 0x2c:      // ',':
            cursor->isSpace = true;
            break;
        case 0x0a:    // '\n'
        case 0x0b:    // '\v'
        case 0x0c:    // form feed
        case 0x0d:    // '\r'
        case 0x0085:
        case 0x2028:
        case 0x2029:
            cursor->isNewline = true;
            break;
        case 0x00a0:
        case 0x1680:
        case 0x2000:
        case 0x2001:
        case 0x2002:
        case 0x2003:
        case 0x2004:
        case 0x2005:
        case 0x2006:
        case 0x2007:
        case 0x2008:
        case 0x2009:
        case 0x200a:
        case 0x202f:
        case 0x205f:
        case 0x3000:
            cursor->isSpace = true;
            break;
    }
}


void nextCharacter(huScanner * scanner)
{
    // swap the cursor buffers
    huCursor * tempCursor = scanner->nextCursor;
    scanner->nextCursor = scanner->curCursor;
    scanner->curCursor = tempCursor;

    // prevCursor starts with zero length, so this is fine.
    scanner->nextCursor->character = scanner->curCursor->character + scanner->curCursor->charLength;
    scanner->nextCursor->isEof = false;
    scanner->nextCursor->isSpace = false;
    scanner->nextCursor->isTab = false;
    scanner->nextCursor->isNewline = false;
    scanner->nextCursor->isError = false;

    analyzeCharacter(scanner);
    analyzeWhitespace(scanner);
}


void initScanner(huScanner * scanner, huTrove * trove, char const * str, huIndexSize_t strLen)
{
    * scanner = (huScanner) {
        .trove = trove, 
        .inputStr = str,
        .inputStrLen = strLen,
        .nextCursor = NULL,
        .curCursor = NULL,
        .cursors = {
            {
                .character = str,
                .charLength = 0,
                .codePoint = 0,
                .isEof = false,
                .isSpace = false,
                .isTab = false,
                .isNewline = false,
                .isError =false
            },
            {
                .character = str,
                .charLength = 0,
                .codePoint = 0,
                .isEof = false,
                .isSpace = false,
                .isTab = false,
                .isNewline = false,
                .isError =false
            }
        }
    };
    scanner->curCursor = & scanner->cursors[1];
    scanner->nextCursor = & scanner->cursors[0];

    char bom[3] = { 0xef, 0xbb, 0xbf };
    if (memcmp(str, bom, 3) == 0)
        { scanner->nextCursor->character += 3; }

    analyzeCharacter(scanner);
    if (scanner->curCursor->isError == false)
    {
        analyzeWhitespace(scanner);
        nextCharacter(scanner);
    }
}


void eatWs(huScanner * scanner, huCol_t tabSize, huLine_t * line, huCol_t * col)
{
    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isError)
        {
            eating = false;
        }
        else if (scanner->curCursor->isSpace)
        {
            * col += 1;
            nextCharacter(scanner);
        }
        else if(scanner->curCursor->isTab)
        {
            * col += tabSize - ((* col - 1) % tabSize);
            nextCharacter(scanner);
        }
        else if (scanner->curCursor->isNewline)
        {
            * col = 1;
            * line += 1;
            nextCharacter(scanner);
        }
        else
            { eating = false; }
    }
}


static void eatDoubleSlashComment(huScanner * scanner, huCol_t tabSize, huIndexSize_t * len, huCol_t * col)
{
    // The first two characters are already confirmed //, so, next please.
    * len += scanner->curCursor->charLength;
    * col += 1;
    nextCharacter(scanner);
    * len += scanner->curCursor->charLength;
    * col += 1;
    nextCharacter(scanner);

    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isNewline == false && 
            scanner->curCursor->isError == false &&
            scanner->curCursor->isEof == false)
        {
            * len += scanner->curCursor->charLength;
            if (scanner->curCursor->isTab)
                { * col += tabSize - ((* col - 1) % tabSize); }
            else
                { * col += 1; }
            nextCharacter(scanner);
        }
        else
            { eating = false; }
    }
}


static void eatCStyleComment(huScanner * scanner, huCol_t tabSize, huIndexSize_t * len, huLine_t * line, huCol_t * col)
{
    // record the location for error reporting
    huLine_t tokenStartLine = * line;
    huCol_t tokenStartCol = * col;

    // The first two characters are already confirmed /*, so, next please.    
    * len += scanner->curCursor->charLength;
    * col += 1;
    nextCharacter(scanner);
    * len += scanner->curCursor->charLength;
    * col += 1;
    nextCharacter(scanner);

    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isError)
        {
            eating = false;
        }
        else if (scanner->curCursor->isEof == true)
        {
            eating = false;
            recordTokenizeError(scanner->trove, HU_ERROR_UNFINISHEDCSTYLECOMMENT, 
                tokenStartLine, tokenStartCol);
        }
        else if (scanner->curCursor->isNewline)
        {
            * len += scanner->curCursor->charLength;
            * col = 1;
            * line += 1;
            nextCharacter(scanner);
        }
        else if (scanner->curCursor->codePoint == '*')
        {
            * len += scanner->curCursor->charLength;
            * col += 1;
            nextCharacter(scanner);

            // look ahead
            if (scanner->curCursor->codePoint == '/')
            {
                * len += scanner->curCursor->charLength;
                * col += 1;
                nextCharacter(scanner);
                eating = false;
            }
        }
        else
        {
            * len += scanner->curCursor->charLength;
            if (scanner->curCursor->isTab)
                { * col += tabSize - ((* col - 1) % tabSize); }
            else
                { * col += 1; }
            nextCharacter(scanner);
        }
    }
}


static void eatWord(huScanner * scanner, huIndexSize_t * len, huCol_t * col)
{
    // The first character is already confirmed a word char, so, next please.
    * len += scanner->curCursor->charLength;
    * col += 1;
    nextCharacter(scanner);

    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isEof ||
            scanner->curCursor->isSpace ||
            scanner->curCursor->isTab ||
            scanner->curCursor->isNewline ||
            scanner->curCursor->isError)
            { eating = false; }

        else if (scanner->curCursor->codePoint == '\\')
        {
            * len += scanner->curCursor->charLength;
            * col += 1;
            nextCharacter(scanner);
            if (scanner->curCursor->isEof == false &&
                scanner->curCursor->codePoint != '\n')
            {
                * len += scanner->curCursor->charLength;
                * col += 1;
                nextCharacter(scanner);
            }
        }
        else if (scanner->curCursor->codePoint == '/')
        {
            // A comment abutting an unquoted string should still delimit.
            if (scanner->nextCursor->codePoint == '/' ||
                scanner->nextCursor->codePoint == '*')
                { eating = false; }
            else
            {
                * len += scanner->curCursor->charLength;
                * col += 1;
                nextCharacter(scanner);
            }
        }
        else
        {
            switch(scanner->curCursor->codePoint)
            {
            case '{': case '}': case '[': case ']':
            case ':': case '@': case '#':
                eating = false;
                break;
            default:
                * len += scanner->curCursor->charLength;
                * col += 1;
                nextCharacter(scanner);
                break;
            }
        }
    }    
}


static void eatQuotedWord(huScanner * scanner, huCol_t tabSize, huIndexSize_t * len, huLine_t * line, huCol_t * col)
{
    // record the location for error reporting
    huLine_t tokenStartLine = * line;
    huCol_t tokenStartCol = * col;

    uint32_t quoteChar = scanner->curCursor->codePoint;

    // The first character is already confirmed quoteChar, so, next please.
    * col += 1;
    nextCharacter(scanner);

    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isError)
        {
            eating = false;
        }
        else if (scanner->curCursor->isEof)
        {
          eating = false;
          recordTokenizeError(scanner->trove, HU_ERROR_UNFINISHEDQUOTE, 
            tokenStartLine, tokenStartCol);
        }
        else if (scanner->curCursor->isNewline)
        {
            * len += scanner->curCursor->charLength;
            * col = 1;
            * line += 1;
            nextCharacter(scanner);
        }
        else
        {
            if (scanner->curCursor->isTab)
                { * col += tabSize - ((* col - 1) % tabSize); }
            else
                { * col += 1; }

            if (scanner->curCursor->codePoint == '\\')
            {
                // always eat the '\'
                * len += scanner->curCursor->charLength;
                nextCharacter(scanner);

                // skip over the escaped quote so we don't encounter it and end the string
                if (scanner->curCursor->codePoint == quoteChar)
                {
                    * len += scanner->curCursor->charLength;
                    nextCharacter(scanner);
                }
            }
            else if (scanner->curCursor->codePoint == quoteChar)
            {
                nextCharacter(scanner);
                eating = false;
            }
            else
            {
                * len += scanner->curCursor->charLength;
                nextCharacter(scanner);
            }
        }
    }
}


void tokenizeTrove(huTrove * trove)
{
    resetVector(& trove->tokens);

    char const * beg = trove->dataString;

    huScanner scanner;
    initScanner(& scanner, trove, beg, trove->dataStringSize);

    huLine_t line = 1;
    huCol_t col = 1;
    bool scanning = true;

    // lexi scan
    while (scanning && scanner.curCursor->isError == false)
    {
        eatWs(& scanner, trove->inputTabSize, & line, & col);
        huIndexSize_t len = 0;
        huLine_t lineM = line;
        huCol_t colM = col;

        huCursor * cur = scanner.curCursor;
        char const * tokenStart = cur->character;
        
        switch(scanner.curCursor->codePoint)
        {
        case '\0':
            allocNewToken(trove, HU_TOKENKIND_EOF, tokenStart, 0, line, col, line, col, '\0');
            scanning = false;
            break;
        case '{':
            allocNewToken(trove, HU_TOKENKIND_STARTDICT, tokenStart, cur->charLength, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& scanner);
            break;
        case '}':
            allocNewToken(trove, HU_TOKENKIND_ENDDICT, tokenStart, cur->charLength, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& scanner);
            break;
        case '[':
            allocNewToken(trove, HU_TOKENKIND_STARTLIST, tokenStart, cur->charLength, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& scanner);
            break;
        case ']':
            allocNewToken(trove, HU_TOKENKIND_ENDLIST, tokenStart, cur->charLength, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& scanner);
            break;
        case ':':
            allocNewToken(trove, HU_TOKENKIND_KEYVALUESEP, tokenStart, cur->charLength, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& scanner);
            break;
        case '@':
            allocNewToken(trove, HU_TOKENKIND_ANNOTATE, tokenStart, cur->charLength, line, col, line, col + 1, '\0');
            col += 1;
            nextCharacter(& scanner);
            break;
        case '/':
            if (scanner.nextCursor->codePoint == '/')
            {
                eatDoubleSlashComment(& scanner, trove->inputTabSize, & len, & colM);
                allocNewToken(trove, HU_TOKENKIND_COMMENT, tokenStart, len, line, col, line, colM, '\0');
                col = colM;
            }
            else if (scanner.nextCursor->codePoint == '*')
            {
                eatCStyleComment(& scanner, trove->inputTabSize, & len, & lineM, & colM);
                allocNewToken(trove, HU_TOKENKIND_COMMENT, tokenStart, len, line, col, lineM, colM, '\0');
                line = lineM;
                col = colM;
            }
            else
            {
                // else treat like a word char
                eatWord(& scanner, & len, & colM);
                allocNewToken(trove, HU_TOKENKIND_WORD, tokenStart, len, line, col, line, colM, '\0');
                col = colM;
            }
            break;
        case '\'':
        case '"':
        case '`':
            {
                char quoteChar = cur->codePoint;
                eatQuotedWord(& scanner, trove->inputTabSize, & len, & lineM, & colM);
                allocNewToken(trove, HU_TOKENKIND_WORD, tokenStart + 1, len, line, col, lineM, colM, quoteChar);
                line = lineM;
                col = colM;
            }
            break;
        default: // word char
            eatWord(& scanner, & len, & colM);
            allocNewToken(trove, HU_TOKENKIND_WORD, tokenStart, len, line, col, line, colM, '\0');
            col = colM;
            break;
        }
    }
}
