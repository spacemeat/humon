#include <string.h>
#include "humon.internal.h"


bool stringInString(char const * haystack, int haystackLen, char const * needle, int needleLen)
{
    // I'm unconcerned about O(m*n).
    for (int i = 0; i < haystackLen - needleLen + 1; ++i)
    {
        if (strncmp(haystack + i, needle, needleLen) == 0)
            { return true; }
    }

    return false;
}


char const * huEncodingToString(int rhs)
{
    switch(rhs)
    {
    case HU_ENCODING_UTF8: return "UTF8";
    case HU_ENCODING_UTF16_BE: return "UTF16-BE";
    case HU_ENCODING_UTF16_LE: return "UTF16-LE";
    case HU_ENCODING_UTF32_BE: return "UTF32-BE";
    case HU_ENCODING_UTF32_LE: return "UTF32-LE";
    case HU_ENCODING_UNKNOWN: return "unknown";
    default: return "!!unknown!!";
    }
}


char const * huTokenKindToString(int rhs)
{
    switch(rhs)
    {
    case HU_TOKENKIND_NULL: return "null";
    case HU_TOKENKIND_EOF: return "eof";
    case HU_TOKENKIND_STARTLIST: return "startList";
    case HU_TOKENKIND_ENDLIST: return "endList";
    case HU_TOKENKIND_STARTDICT: return "startDict";
    case HU_TOKENKIND_ENDDICT: return "endDict";
    case HU_TOKENKIND_KEYVALUESEP: return "keyValueSep";
    case HU_TOKENKIND_ANNOTATE: return "annotate";
    case HU_TOKENKIND_WORD: return "word";
    case HU_TOKENKIND_COMMENT: return "comment";
    default: return "!!unknown!!";
    }
}


char const * huNodeKindToString(int rhs)
{
    switch(rhs)
    {
    case HU_NODEKIND_NULL: return "null";
    case HU_NODEKIND_LIST: return "list";
    case HU_NODEKIND_DICT: return "dict";
    case HU_NODEKIND_VALUE: return "value";
    default: return "!!unknown!!";
    }
}


char const * huWhitespaceFormatToString(int rhs)
{
    switch(rhs)
    {
    case HU_WHITESPACEFORMAT_CLONED: return "cloned";
    case HU_WHITESPACEFORMAT_MINIMAL: return "minimal";
    case HU_WHITESPACEFORMAT_PRETTY: return "pretty";
    default: return "!!unknown!!";
    }
}


char const * huOutputErrorToString(int rhs)
{
    switch(rhs)
    {
    case HU_ERROR_NOERROR: return "no error";
    case HU_ERROR_BADENCODING: return "bad encoding";
    case HU_ERROR_UNFINISHEDQUOTE: return "unfinished quote";
    case HU_ERROR_UNFINISHEDCSTYLECOMMENT: return "unfinished C-style comment";
    case HU_ERROR_UNEXPECTEDEOF: return "unexpected EOF";
    case HU_ERROR_TOOMANYROOTS: return "too many roots";
    case HU_ERROR_NONUNIQUEKEY: return "non-unique key";
    case HU_ERROR_SYNTAXERROR: return "syntax error";
    case HU_ERROR_NOTFOUND: return "not found";    
    case HU_ERROR_ILLEGAL: return "illegal operation";
    case HU_ERROR_BADPARAMETER: return "bad parameter";
    case HU_ERROR_BADFILE: return "bad file";
    case HU_ERROR_OUTOFMEMORY: return "out of memory";
    case HU_ERROR_TROVEHASERRORS: return "trove has errors";
    default: return "!!unknown!!";
    }
}


int min(int a, int b) { if (a < b) { return a; } else { return b; } }
int max(int a, int b) { if (a >= b) { return a; } else { return b; } }


bool isMachineBigEndian()
{
    uint16_t i = 0xff;
    return ((uint8_t*)(& i))[1] == 0xff;
}


void huInitDeserializeOptions(huDeserializeOptions * params, int encoding, bool strictUnicode, int tabSize)
{
    params->encoding = encoding;
    params->allowOutOfRangeCodePoints = ! strictUnicode;
    params->allowUtf16UnmatchedSurrogates = ! strictUnicode;
    params->tabSize = tabSize;
}


void huInitSerializeOptionsZ(huSerializeOptions * params, int WhitespaceFormat, int indentSize, 
    bool indentWithTabs, bool usingColors, huStringView const * colorTable,  bool printComments, 
    char const * newline, bool printBom)
{
    huInitSerializeOptionsN(params, WhitespaceFormat, indentSize, indentWithTabs, usingColors, colorTable, 
        printComments, newline, strlen(newline), printBom);
}


void huInitSerializeOptionsN(huSerializeOptions * params, int WhitespaceFormat, int indentSize, 
    bool indentWithTabs, bool usingColors, huStringView const * colorTable,  bool printComments, 
    char const * newline, int newlineSize, bool printBom)
{
    params->WhitespaceFormat = WhitespaceFormat;
    params->indentSize = indentSize;
    params->indentWithTabs = indentWithTabs;
    params->usingColors = usingColors;
    params->colorTable = colorTable;
    params->printComments = printComments;
    params->newline = (huStringView) { newline, newlineSize };
    params->printBom = printBom;
}
