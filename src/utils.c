#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "humon.internal.h"


FILE * openFile(char const * path, char const * mode)
{
#ifdef _MSC_VER
	struct stat fstatData;
	if (stat(path, & fstatData) < 0)
        { return NULL; }
    
    if ((_S_IFREG & fstatData.st_mode) == 0)
        { return NULL; }
#else
	struct stat fstatData;
	if (stat(path, & fstatData) < 0)
        { return NULL; }
    
    if (S_ISREG(fstatData.st_mode) == false)
        { return NULL; }
#endif

    FILE * fp = NULL;
#ifdef _MSC_VER
    errno_t err = fopen_s(& fp, path, mode);
#else
    fp = fopen(path, mode);
#endif
    return fp;
}


huEnumType_t getFileSize(FILE * fp, huIndexSize_t * fileLen, huEnumType_t errorResponse)
{
#ifdef _MSC_VER
	if (_fseeki64(fp, 0, SEEK_END) != 0)
#else
	if (fseeko(fp, 0, SEEK_END) != 0)
#endif
	{
		printError(errorResponse, "Could not read file.");
		return HU_ERROR_BADFILE;
	}

#ifdef _MSC_VER
	__int64 dataLen = _ftelli64(fp);
#else
    off_t dataLen = ftello(fp);
#endif
    if (dataLen < 0)
    {
        printError(errorResponse, "Could not read file.");
        return HU_ERROR_BADFILE;
    }

    if ((long long unsigned) dataLen > (long long unsigned) maxOfType(huIndexSize_t))
    {
        printError(errorResponse, "File is too large. Consider setting HU_STRLEN_TYPE to a 64-bit integer type.");
        return HU_ERROR_BADFILE;
    }

    rewind(fp);

    * fileLen = (huIndexSize_t) dataLen;

    return HU_ERROR_NOERROR;
}


bool stringInString(char const * haystack, huIndexSize_t haystackLen, char const * needle, huIndexSize_t needleLen)
{
    // I'm unconcerned about O(m*n).
    if (haystackLen < needleLen)
        { return false; }

    for (huIndexSize_t i = 0; i < haystackLen - needleLen + 1; ++i)
    {
        if (strncmp(haystack + i, needle, needleLen) == 0)
            { return true; }
    }

    return false;
}


char const * huEncodingToString(huEnumType_t rhs)
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


char const * huTokenKindToString(huEnumType_t rhs)
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


char const * huNodeKindToString(huEnumType_t rhs)
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


char const * huWhitespaceFormatToString(huEnumType_t rhs)
{
    switch(rhs)
    {
    case HU_WHITESPACEFORMAT_CLONED: return "cloned";
    case HU_WHITESPACEFORMAT_MINIMAL: return "minimal";
    case HU_WHITESPACEFORMAT_PRETTY: return "pretty";
    default: return "!!unknown!!";
    }
}


char const * huOutputErrorToString(huEnumType_t rhs)
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


huIndexSize_t min(huIndexSize_t a, huIndexSize_t b)
{
	if (a < b)
		{ return a; }
	else
		{ return b; }
}

huIndexSize_t max(huIndexSize_t a, huIndexSize_t b)
{
	if (a >= b)
		{ return a; }
	else
		{ return b; }
}


bool isMachineBigEndian()
{
    uint16_t i = 0xff;
    return ((uint8_t*)(& i))[1] == 0xff;
}


void huInitDeserializeOptions(huDeserializeOptions * params, huEnumType_t encoding, bool strictUnicode, huCol_t tabSize)
{
    params->encoding = encoding;
    params->allowOutOfRangeCodePoints = ! strictUnicode;
    params->allowUtf16UnmatchedSurrogates = ! strictUnicode;
    params->tabSize = tabSize;
}


void huInitSerializeOptionsZ(huSerializeOptions * params, huEnumType_t WhitespaceFormat, huCol_t indentSize,
    bool indentWithTabs, bool usingColors, huStringView const * colorTable,  bool printComments, 
    char const * newline, bool printBom)
{
    size_t newlineLenC = strlen(newline);
    if (newlineLenC > maxOfType(huIndexSize_t))
        { newlineLenC = maxOfType(huIndexSize_t); }

    huInitSerializeOptionsN(params, WhitespaceFormat, indentSize, indentWithTabs, usingColors, colorTable, 
        printComments, newline, (huIndexSize_t) newlineLenC, printBom);
}


void huInitSerializeOptionsN(huSerializeOptions * params, huEnumType_t WhitespaceFormat, huCol_t indentSize,
    bool indentWithTabs, bool usingColors, huStringView const * colorTable,  bool printComments, 
    char const * newline, huIndexSize_t newlineSize, bool printBom)
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
