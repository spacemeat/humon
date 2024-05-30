#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


static void analyzeCharacter(huScanner * scanner)
{
    huCursor * cursor = scanner->nextCursor;
    if (scanner->nextCursor->character >= scanner->inputStr + scanner->inputStrLen)
    {
        cursor->charLength = 1;
        cursor->codePoint = 0;
        cursor->isEof = true;
    }
    else if ((cursor->character[0] & 0b10000000) == 0)
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
    if (cursor->isEof)
        { return; }

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


void swapAndReadNext(huScanner * scanner)
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


void nextCharacter(huScanner * scanner)
{
    // track row/column/len
    if (scanner->curCursor->isNewline)
    {
        scanner->col = 1;
        scanner->line += 1;
    }
    else if (scanner->curCursor->isTab)
    {
        scanner->col += scanner->tabLen - ((scanner->col - 1) % scanner->tabLen);
    }
    else
    {
        scanner->col += 1;
    }

    scanner->len += scanner->curCursor->charLength;

    swapAndReadNext(scanner);
}


void initScanner(huScanner * scanner, huTrove * trove, huCol_t tabLen, char const * str, huSize_t strLen)
{
    * scanner = (huScanner) {
        .trove = trove,
        .tabLen = tabLen,
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

    scanner->line = 1;
    scanner->col = 1;
    scanner->len = 0;

    char bom[3] = { 0xef, 0xbb, 0xbf };
    if (memcmp(str, bom, 3) == 0)
    {
        scanner->nextCursor->character += 3;
        scanner->len += 3;
    }

    analyzeCharacter(scanner);
    analyzeWhitespace(scanner);
    swapAndReadNext(scanner);
}


void eatWs(huScanner * scanner)
{
    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isError)
            { eating = false; }
        else if (scanner->curCursor->isSpace ||
                 scanner->curCursor->isTab ||
                 scanner->curCursor->isNewline)
            { nextCharacter(scanner); }
        else
            { eating = false; }
    }
}


void eatLineWs(huScanner * scanner)
{
    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isError)
            { eating = false; }
        else if (scanner->curCursor->isSpace ||
                 scanner->curCursor->isTab)
            { nextCharacter(scanner); }
        else
            { eating = false; }
    }
}


static void eatDoubleSlashComment(huScanner * scanner, huSize_t * offsetIn)
{
    // The first two characters are already confirmed //, so, next please.
    nextCharacter(scanner);
    nextCharacter(scanner);

    eatLineWs(scanner);
    (void) offsetIn;

    bool eating = true;
    while (eating)
    {
        if (scanner->curCursor->isNewline == false &&
            scanner->curCursor->isError == false &&
            scanner->curCursor->isEof == false)
            { nextCharacter(scanner); }
        else
            { eating = false; }
    }
}


static void eatCStyleComment(huScanner * scanner)
{
    // record the location for error reporting
    huLine_t tokenStartLine = scanner->line;
    huCol_t tokenStartCol = scanner->col;

    // The first two characters are already confirmed /*, so, next please.
    nextCharacter(scanner);
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
            nextCharacter(scanner);
        }
        else if (scanner->curCursor->codePoint == '*')
        {
            nextCharacter(scanner);

            // look ahead
            if (scanner->curCursor->codePoint == '/')
            {
                nextCharacter(scanner);
                eating = false;
            }
        }
        else
        {
            nextCharacter(scanner);
        }
    }
}


static void eatWord(huScanner * scanner)
{
    // The first character is already confirmed a word char, so, next please.
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

        else if (scanner->curCursor->codePoint == '/')
        {
            // A comment abutting an unquoted string should still delimit.
            if (scanner->nextCursor->codePoint == '/' ||
                scanner->nextCursor->codePoint == '*')
                { eating = false; }
            else
            {
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
                nextCharacter(scanner);
                break;
            }
        }
    }
}


static void eatQuotedWord(huScanner * scanner)
{
    // record the location for error reporting
    huLine_t tokenStartLine = scanner->line;
    huCol_t tokenStartCol = scanner->col;

    uint32_t quoteChar = scanner->curCursor->codePoint;

    // The first character is already confirmed quoteChar, so, next please.
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
        else
        {
            if (scanner->curCursor->codePoint == quoteChar)
                { eating = false; }

            nextCharacter(scanner);
        }
    }
}


static void eatTagQuoteTag(huScanner * scanner, huSize_t * tagLen)
{
    // record the location for error reporting
    huLine_t tokenStartLine = scanner->line;
    huCol_t tokenStartCol = scanner->col;

    // The first character is already confirmed caret, so, next please.
    * tagLen += scanner->curCursor->charLength;
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
        else
        {
            * tagLen += scanner->curCursor->charLength;

            if (scanner->curCursor->codePoint == '^')
            {
                eating = false;
            }
            nextCharacter(scanner);
        }
    }
}


static void eatMatch(huScanner * scanner, huSize_t matchLen)
{
    huSize_t matchedLen = 0;
    bool eating = true;
    while (eating)
    {
        // One char at a time so we can keep column counts, etc.
        // Everything has to be nextCharacter()'d.
        matchedLen += scanner->curCursor->charLength;
        if (matchedLen == matchLen)
            { eating = false; }

        nextCharacter(scanner);
    }
}


static void eatTagQuotedWord(huScanner * scanner, huSize_t * offsetIn, huSize_t * offsetOut)
{
    // record the location for error reporting
    huLine_t tokenStartLine = scanner->line;
    huCol_t tokenStartCol = scanner->col;

    char const * tagStart = scanner->curCursor->character;
    huSize_t tagLen = 0;

    char const * tokenRawStart = scanner->curCursor->character;

    eatTagQuoteTag(scanner, & tagLen);

    char const * tokenStart = scanner->curCursor->character;

    bool seenNewline = false;
    bool seenOnlySpaces = true;

    bool eating = true;
    while (eating)
    {
        bool match = memcmp(scanner->curCursor->character, tagStart, tagLen) == 0;

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
        else if (match)
        {
            eatMatch(scanner, tagLen);
            * offsetIn = tokenStart - tokenRawStart;
            * offsetOut = tagLen;
            eating = false;
        }
        else if (scanner->curCursor->isNewline)
        {
            nextCharacter(scanner);
            if (seenNewline == false)
            {
                if (seenOnlySpaces == true)
                {
                    // start the token *after* the nextCharacter() call
                    tokenStart = scanner->curCursor->character;
                }

                seenNewline = true;
            }
        }
        else if (scanner->curCursor->isSpace)
        {
            nextCharacter(scanner);
        }
        else
        {
            seenOnlySpaces = false;
            nextCharacter(scanner);
        }
    }
}


void tokenizeTrove(huTrove * trove)
{
    resetVector(& trove->tokens);

    char const * beg = trove->dataString;

    huScanner scanner;
    initScanner(& scanner, trove, trove->inputTabSize, beg, trove->dataStringSize);

    bool scanning = true;

    // lexi scan
    while (scanning && scanner.curCursor->isError == false)
    {
        eatWs(& scanner);

        huLine_t line = scanner.line;
        huCol_t col = scanner.col;
        huSize_t len = scanner.len;

        huCursor * cur = scanner.curCursor;
        char const * tokenStart = cur->character;

        switch(scanner.curCursor->codePoint)
        {
        case '\0':
            allocNewToken(trove, HU_TOKENKIND_EOF, tokenStart, 0, line, col, line, col, 0, 0, '\0');
            scanning = false;
            break;
        case '{':
            allocNewToken(trove, HU_TOKENKIND_STARTDICT, tokenStart, cur->charLength, line, col, line, col + 1, 0, 0, '\0');
            nextCharacter(& scanner);
            break;
        case '}':
            allocNewToken(trove, HU_TOKENKIND_ENDDICT, tokenStart, cur->charLength, line, col, line, col + 1, 0, 0, '\0');
            nextCharacter(& scanner);
            break;
        case '[':
            allocNewToken(trove, HU_TOKENKIND_STARTLIST, tokenStart, cur->charLength, line, col, line, col + 1, 0, 0, '\0');
            nextCharacter(& scanner);
            break;
        case ']':
            allocNewToken(trove, HU_TOKENKIND_ENDLIST, tokenStart, cur->charLength, line, col, line, col + 1, 0, 0, '\0');
            nextCharacter(& scanner);
            break;
        case ':':
            allocNewToken(trove, HU_TOKENKIND_KEYVALUESEP, tokenStart, cur->charLength, line, col, line, col + 1, 0, 0, '\0');
            nextCharacter(& scanner);
            break;
        case '@':
            allocNewToken(trove, HU_TOKENKIND_METATAG, tokenStart, cur->charLength, line, col, line, col + 1, 0, 0, '\0');
            nextCharacter(& scanner);
            break;
        case '/':
            if (scanner.nextCursor->codePoint == '/')
            {
                huSize_t offsetIn = 0;
                eatDoubleSlashComment(& scanner, & offsetIn);
                allocNewToken(trove, HU_TOKENKIND_COMMENT, tokenStart,
                    scanner.len - len, line, col, line, scanner.col, offsetIn, 0, '\0');
            }
            else if (scanner.nextCursor->codePoint == '*')
            {
                eatCStyleComment(& scanner);
                allocNewToken(trove, HU_TOKENKIND_COMMENT, tokenStart, scanner.len - len, line, col, scanner.line, scanner.col, 2, 2, '\0');
            }
            else
            {
                // else treat like a word char
                eatWord(& scanner);
                allocNewToken(trove, HU_TOKENKIND_WORD, tokenStart, scanner.len - len, line, col, scanner.line, scanner.col, 0, 0, '\0');
            }
            break;
        case '\'':
        case '"':
        case '`':
            {
                char quoteChar = cur->codePoint;
                eatQuotedWord(& scanner);
                allocNewToken(trove, HU_TOKENKIND_WORD, tokenStart, scanner.len - len, line, col, scanner.line, scanner.col, 1, 1, quoteChar);
            }
            break;
        case '^':
            {
                huSize_t offsetIn = 0, offsetOut = 0;
                eatTagQuotedWord(& scanner, & offsetIn, & offsetOut);
                allocNewToken(trove, HU_TOKENKIND_WORD, tokenStart, scanner.len - len, line, col, scanner.line, scanner.col, offsetIn, offsetOut, '^');
            }
            break;
        default: // word char
            eatWord(& scanner);
            allocNewToken(trove, HU_TOKENKIND_WORD, tokenStart, scanner.len - len, line, col, scanner.line, scanner.col, 0, 0, '\0');
            break;
        }
    }
}
