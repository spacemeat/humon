#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "humon.internal.h"


void initTrove(huTrove * trove, huDeserializeOptions * deserializeOptions, huErrorResponse errorResponse)
{
    trove->dataString = NULL;
    trove->dataStringSize = 0;
    trove->allocator = deserializeOptions->allocator;

    initGrowableVector(& trove->tokens, sizeof(huToken), & trove->allocator);
    initGrowableVector(& trove->nodes, sizeof(huNode), & trove->allocator);
    initGrowableVector(& trove->errors, sizeof(huError), & trove->allocator);

    trove->errorResponse = errorResponse;
    trove->inputTabSize = deserializeOptions->tabSize;

    initGrowableVector(& trove->metatags, sizeof(huMetatag), & trove->allocator);
    initGrowableVector(& trove->comments, sizeof(huComment), & trove->allocator);

    trove->lastMetatagToken = NULL;
}


void printError(huErrorResponse errorResponse, char const * msg)
{
    // Depending on errorResponse, output something
    FILE * stream = stdout;
    if (errorResponse == HU_ERRORRESPONSE_STDERR ||
        errorResponse == HU_ERRORRESPONSE_STDERRANSICOLOR)
        { stream = stderr; }
    if (errorResponse == HU_ERRORRESPONSE_STDOUT ||
        errorResponse == HU_ERRORRESPONSE_STDERR)
        { fprintf(stream, "Error: %s\n", msg); }
    else if (errorResponse == HU_ERRORRESPONSE_STDOUTANSICOLOR ||
             errorResponse == HU_ERRORRESPONSE_STDERRANSICOLOR)
        { fprintf(stream, "%sError%s: %s\n", ansi_lightRed, ansi_off, msg); }
}


huErrorCode huDeserializeTroveZ(huTrove ** trovePtr, char const * data, huDeserializeOptions * deserializeOptions, huErrorResponse errorResponse)
{
    if (trovePtr)
        { * trovePtr = HU_NULLTROVE; }

#ifdef HUMON_CHECK_PARAMS
    if (data == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    // always check this'n
    size_t dataLenC = strlen(data);
    if (dataLenC > maxOfType(huSize_t))
        { return HU_ERROR_BADPARAMETER; }

    return huDeserializeTroveN(trovePtr, data, (huSize_t) dataLenC, deserializeOptions, errorResponse);
}


huErrorCode huDeserializeTroveN(huTrove ** trovePtr, char const * data, huSize_t dataLen, huDeserializeOptions * deserializeOptions, huErrorResponse errorResponse)
{
    if (trovePtr)
        { * trovePtr = HU_NULLTROVE; }

#ifdef HUMON_CHECK_PARAMS
    if (trovePtr == NULL || data == NULL || isNegative(dataLen))
        { return HU_ERROR_BADPARAMETER; }
    if (deserializeOptions &&
        (isNegative(deserializeOptions->encoding) ||
         deserializeOptions->encoding > HU_ENCODING_UNKNOWN ||
         isNegative(deserializeOptions->tabSize)))
        { return HU_ERROR_BADPARAMETER; }
    if (isNegative(errorResponse) ||
        errorResponse >= HU_ERRORRESPONSE_NUMRESPONSES)
        { return HU_ERROR_BADPARAMETER; }
#endif

    huDeserializeOptions localDeserializeOptions;
    if (deserializeOptions == NULL)
    {
        huInitDeserializeOptions(& localDeserializeOptions, HU_ENCODING_UTF8, true, 4, NULL, HU_BUFFERMANAGEMENT_COPYANDOWN);
        deserializeOptions = & localDeserializeOptions;
    }

    if (deserializeOptions->allocator.memAlloc == NULL)
        { deserializeOptions->allocator.memAlloc = & sysAlloc; }
    if (deserializeOptions->allocator.memRealloc == NULL)
        { deserializeOptions->allocator.memRealloc = & sysRealloc; }
    if (deserializeOptions->allocator.memFree == NULL)
        { deserializeOptions->allocator.memFree = & sysFree; }

    huStringView inputDataView = { data, dataLen };

    if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
    {
        huSize_t numEncBytes = 0;    // not useful here
        deserializeOptions->encoding = swagEncodingFromString(& inputDataView, & numEncBytes, deserializeOptions);
        if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
        {
            printError(errorResponse, "Error: Could not determine Unicode encoding.");
            return HU_ERROR_BADENCODING;
        }
    }

    huTrove * trove = ourAlloc(& deserializeOptions->allocator, sizeof(huTrove));
    if (trove == HU_NULLTROVE)
    {
        printError(errorResponse, "Error: Out of memory.");
        return HU_ERROR_OUTOFMEMORY;
    }

    initTrove(trove, deserializeOptions, errorResponse);

    // We're guaranteed that UTF8 strings will be no longer than the transcoded UTF*
    // equivalent, as long as we reject unpaired surrogates. MS filenames
    // can contain unpaired surrogates, and Humon will accept them if strictUnicode
    // is clear. At that point, a UTF8 string can be longer than its UTF16, so we
    // have to double the size.

    huSize_t sizeFactor = deserializeOptions->allowUtf16UnmatchedSurrogates == false &&
                               (deserializeOptions->encoding == HU_ENCODING_UTF16_BE ||
                                deserializeOptions->encoding == HU_ENCODING_UTF16_LE) ? 2 : 1;

    char const * newConstData;
    huSize_t newConstDataLen = 0;

    // dso.bufferManagement may encourage us to move instead of copy.
    // This is only going to happen if the src is utf8, and we're ignoring unicode errors.
    // Otherwise, set the bufferManagement to copy and clone the input string.
    if (deserializeOptions->allowOutOfRangeCodePoints &&
        deserializeOptions->allowUtf16UnmatchedSurrogates &&
        deserializeOptions->encoding == HU_ENCODING_UTF8 &&
        deserializeOptions->bufferManagement != HU_BUFFERMANAGEMENT_COPYANDOWN)
    {
        newConstData = inputDataView.ptr;
        newConstDataLen = inputDataView.size;
    }
    else
    {
        char * newData;
        deserializeOptions->bufferManagement = HU_BUFFERMANAGEMENT_COPYANDOWN;
        newData = ourAlloc(& deserializeOptions->allocator, dataLen * sizeFactor);
        if (newData == NULL)
        {
            ourFree(& deserializeOptions->allocator, trove);
            printError(errorResponse, "Out of memory.");
            return HU_ERROR_OUTOFMEMORY;
        }

        huSize_t transcodedLen = 0;
        huErrorCode error = transcodeToUtf8FromString(newData, & transcodedLen, & inputDataView, deserializeOptions);
        if (error != HU_ERROR_NOERROR)
        {
            if (deserializeOptions->bufferManagement == HU_BUFFERMANAGEMENT_COPYANDOWN)
                { ourFree(& deserializeOptions->allocator, newData); }
            ourFree(& deserializeOptions->allocator, trove);
            printError(errorResponse, "Transcoding failed.");
            return error;
        }

        newConstData = newData;
        newConstDataLen = transcodedLen;
    }

    trove->dataString = newConstData;
    trove->dataStringSize = newConstDataLen;
    trove->bufferManagement = deserializeOptions->bufferManagement;

    // Errors here are recorded in the trove object.
    tokenizeTrove(trove);
    parseTrove(trove);

    * trovePtr = trove;

    return huGetNumErrors(trove) == 0 ? HU_ERROR_NOERROR : HU_ERROR_TROVEHASERRORS;
}


huErrorCode huDeserializeTroveFromFile(huTrove ** trovePtr, char const * path, huDeserializeOptions * deserializeOptions, huErrorResponse errorResponse)
{
    if (trovePtr != NULL)
        { * trovePtr = HU_NULLTROVE; }

#ifdef HUMON_CHECK_PARAMS
    if (trovePtr == NULL || path == NULL)
        { return HU_ERROR_BADPARAMETER; }
    if (deserializeOptions &&
        (isNegative(deserializeOptions->encoding) ||
         deserializeOptions->encoding > HU_ENCODING_UNKNOWN ||
         isNegative(deserializeOptions->tabSize)))
        { return HU_ERROR_BADPARAMETER; }
    if (isNegative(errorResponse) ||
        errorResponse >= HU_ERRORRESPONSE_NUMRESPONSES)
        { return HU_ERROR_BADPARAMETER; }
#endif

    huDeserializeOptions localDeserializeOptions;
    if (deserializeOptions == NULL)
    {
        huInitDeserializeOptions(& localDeserializeOptions, HU_ENCODING_UNKNOWN, true, 4, NULL, HU_BUFFERMANAGEMENT_COPYANDOWN);
        deserializeOptions = & localDeserializeOptions;
    }

    if (deserializeOptions->allocator.memAlloc == NULL)
        { deserializeOptions->allocator.memAlloc = & sysAlloc; }
    if (deserializeOptions->allocator.memRealloc == NULL)
        { deserializeOptions->allocator.memRealloc = & sysRealloc; }
    if (deserializeOptions->allocator.memFree == NULL)
        { deserializeOptions->allocator.memFree = & sysFree; }

    // This is obviously required for loading from file.
    deserializeOptions->bufferManagement = HU_BUFFERMANAGEMENT_COPYANDOWN;

	FILE * fp = openFile(path, "rb");
	if (fp == NULL)
	{
        printError(errorResponse, "Could not open file for reading.");
        return HU_ERROR_BADFILE;
    }

    huSize_t dataLen = 0;
    huErrorCode gfsError = getFileSize(fp, & dataLen, errorResponse);
    if (gfsError != HU_ERROR_NOERROR)
    {
        fclose(fp);
        return gfsError;
    }

    if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
    {
        huSize_t numEncBytes = 0;    // not useful here
        deserializeOptions->encoding = swagEncodingFromFile(fp, dataLen, & numEncBytes, deserializeOptions);
        if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
        {
            fclose(fp);
            printError(errorResponse, "Could not determine Unicode encoding.");
            return HU_ERROR_BADENCODING;
        }

        rewind(fp);
    }

    huTrove * trove = ourAlloc(& deserializeOptions->allocator, sizeof(huTrove));
    if (trove == HU_NULLTROVE)
    {
        fclose(fp);
        printError(errorResponse, "Out of memory.");
        return HU_ERROR_OUTOFMEMORY;
    }

    initTrove(trove, deserializeOptions, errorResponse);

    // We're guaranteed that UTF8 strings will be no longer than the transcoded UTF*
    // equivalent, as long as we reject unpaired surrogates. MS filenames
    // can contain unpaired surrogates, and Humon will accept them if strictUnicode
    // is clear. At that point, a UTF8 string can be longer than its UTF16, so we
    // have to double the size.
    huSize_t sizeFactor = deserializeOptions->allowUtf16UnmatchedSurrogates == false &&
                               (deserializeOptions->encoding == HU_ENCODING_UTF16_BE ||
                                deserializeOptions->encoding == HU_ENCODING_UTF16_LE) ? 2 : 1;
    char * newData = ourAlloc(& deserializeOptions->allocator,
        dataLen * sizeFactor + 4); // padding with 4 bytes of null at the end
    if (newData == NULL)
    {
        fclose(fp);
        printError(errorResponse, "Out of memory.");
        return HU_ERROR_OUTOFMEMORY;
    }

    huSize_t transcodedLen = 0;
    huErrorCode error = transcodeToUtf8FromFile(newData, & transcodedLen, fp, dataLen, deserializeOptions);
    fclose(fp);
    if (error != HU_ERROR_NOERROR)
    {
        ourFree(& deserializeOptions->allocator, newData);
        printError(errorResponse, "Transcoding failed.");
        return error;
    }

    // transcodedLen is guaranteed to be <= dataLen.
    newData[transcodedLen] = '\0';
    newData[transcodedLen + 1] = '\0';
    newData[transcodedLen + 2] = '\0';
    newData[transcodedLen + 3] = '\0';

    trove->dataString = newData;
    trove->dataStringSize = transcodedLen;
    trove->bufferManagement = HU_BUFFERMANAGEMENT_COPYANDOWN;

    tokenizeTrove(trove);
    parseTrove(trove);

    * trovePtr = trove;

    return huGetNumErrors(trove) == 0 ? HU_ERROR_NOERROR : HU_ERROR_TROVEHASERRORS;
}


void huDestroyTrove(huTrove * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return; }
#endif

    //huTrove * ncTrove = (huTrove *) trove;

    for (int i = 0; i < trove->nodes.numElements; ++i)
        { destroyNode(huGetNodeByIndex(trove, i)); }

    if (trove->dataString != NULL)
    {
        if (trove->bufferManagement == HU_BUFFERMANAGEMENT_COPYANDOWN ||
            trove->bufferManagement == HU_BUFFERMANAGEMENT_MOVEANDOWN)
            { ourFree(& trove->allocator, (char *) trove->dataString); }

        trove->dataString = NULL;
        trove->dataStringSize = 0;
    }

    destroyVector(& trove->tokens);
    destroyVector(& trove->nodes);
    destroyVector(& trove->errors);

    destroyVector(& trove->metatags);
    destroyVector(& trove->comments);

    ourFree(& trove->allocator, trove);
}


huAllocator const * huGetAllocator(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return NULL; }
#endif

	return & trove->allocator;
}


huSize_t huGetNumTokens(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return 0; }
#endif

    return trove->tokens.numElements;
}


huToken const * huGetToken(huTrove const * trove, huSize_t tokenIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return HU_NULLTOKEN; }
#endif

    if (tokenIdx < trove->tokens.numElements)
        { return (huToken *) trove->tokens.buffer + tokenIdx; }

    return HU_NULLTOKEN;
}


huToken * allocNewToken(huTrove * trove, huTokenKind kind,
    char const * str, huSize_t size,
    huLine_t line, huCol_t col, huLine_t endLine, huCol_t endCol,
    huSize_t offsetIn, huSize_t offsetOut,
    char quoteChar)
{
    huSize_t num = 1;
    huToken * newToken = growVector(& trove->tokens, & num);
    if (num == 0)
        { return (huToken *) HU_NULLTOKEN; }

    newToken->kind = kind;
    newToken->quoteChar = quoteChar;
    newToken->rawStr.ptr = str;
    newToken->rawStr.size = size;
    newToken->str.ptr = str + (huSize_t) offsetIn;
    newToken->str.size = size - (huSize_t) offsetIn - (huSize_t) offsetOut;
    newToken->line = line;
    newToken->col = col;
    newToken->endLine = endLine;
    newToken->endCol = endCol;

#ifdef HUMON_CAVEPERSON_DEBUGGING
    printf ("%stoken%s: line: %s%lld%s  col: %s%lld%s  len: %s%lld%s  %s%s%s  '%s%.*s%s'\n",
        ansi_darkYellow, ansi_off,
        ansi_white, (long long int) line, ansi_off,
        ansi_white, (long long int) col, ansi_off,
        ansi_white, (long long int) size, ansi_off,
        ansi_lightMagenta, huTokenKindToString(kind), ansi_off,
        ansi_white, (int) newToken->str.size, newToken->str.ptr, ansi_off);
#endif

    return newToken;
}


huSize_t huGetNumNodes(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return 0; }
#endif

    return trove->nodes.numElements;
}


huNode const * huGetRootNode(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return HU_NULLNODE; }
#endif

    return (huNode *) trove->nodes.buffer;
}


huNode const * huGetNodeByIndex(huTrove const * trove, huSize_t nodeIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return HU_NULLNODE; }
#endif

    if (nodeIdx >= 0 && nodeIdx < trove->nodes.numElements)
        { return (huNode *) trove->nodes.buffer + nodeIdx; }

    return HU_NULLNODE;
}


huNode const * huGetNodeByAddressZ(huTrove const * trove, char const * address)
{
#ifdef HUMON_CHECK_PARAMS
    if (address == NULL)
        { return HU_NULLNODE; }
#endif

    size_t addressLenC = strlen(address);
    if (addressLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huGetNodeByAddressN(trove, address, (huSize_t) addressLenC);
}


huNode const * huGetNodeByAddressN(huTrove const * trove, char const * address, huSize_t addressLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || address == NULL || addressLen < 0)
        { return HU_NULLNODE; }
#endif

    // parse address; must start with '/' to start at root
    if (addressLen <= 0)
        { return HU_NULLNODE; }

    huScanner scanner;
    initScanner(& scanner, NULL, 1, address, addressLen);

    eatWs(& scanner);

    if (scanner.curCursor->codePoint != '/')
        { return HU_NULLNODE; }

    nextCharacter(& scanner);

    char const * wordStart = scanner.curCursor->character;

    huNode const * root = huGetRootNode(trove);
    if (root == HU_NULLNODE || root->kind == HU_NODEKIND_NULL)
        { return HU_NULLNODE; }

    return huGetNodeByRelativeAddressN(root, wordStart, addressLen - scanner.len);
}


huSize_t huGetNumErrors(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return 0; }
#endif

    return trove->errors.numElements;
}


huError const * huGetError(huTrove const * trove, huSize_t errorIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || errorIdx < 0)
        { return NULL; }
#endif

    if (errorIdx < huGetNumErrors(trove))
    {
        return (huError *) trove->errors.buffer + errorIdx;
    }

    return NULL;
}


huSize_t huGetNumTroveMetatags(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return 0; }
#endif

    return trove->metatags.numElements;
}


huMetatag const * huGetTroveMetatag(huTrove const * trove, huSize_t metatagIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || metatagIdx < 0)
        { return NULL; }
#endif

    if (metatagIdx < trove->metatags.numElements)
    {
        return (huMetatag *) trove->metatags.buffer + metatagIdx;
    }

    return NULL;
}


huSize_t huGetNumTroveMetatagsWithKeyZ(huTrove const * trove, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
        { return 0; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return 0; }

    return huGetNumTroveMetatagsWithKeyN(trove, key, (huSize_t) keyLenC);
}


huSize_t huGetNumTroveMetatagsWithKeyN(huTrove const * trove, char const * key, huSize_t keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || key  == NULL || keyLen < 0)
        { return 0; }
#endif

    huSize_t matches = 0;
    for (huSize_t i = 0; i < trove->metatags.numElements; ++i)
    {
        huMetatag * metatatg = (huMetatag *) trove->metatags.buffer + i;
        if (metatatg->key->str.size == keyLen &&
            strncmp(metatatg->key->str.ptr, key, keyLen) == 0)
            { matches += 1; }
    }

    return matches;
}

huToken const * huGetTroveMetatagWithKeyZ(huTrove const * trove, char const * key, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key  == NULL)
        { return HU_NULLTOKEN; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return HU_NULLTOKEN; }

    return huGetTroveMetatagWithKeyN(trove, key, (huSize_t) keyLenC, cursor);
}


huToken const * huGetTroveMetatagWithKeyN(huTrove const * trove, char const * key, huSize_t keyLen, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || key  == NULL || keyLen < 0 || cursor == NULL || * cursor < 0)
        { return HU_NULLTOKEN; }
#endif

    huToken const * token = HU_NULLTOKEN;
    for (; * cursor < trove->metatags.numElements; ++ * cursor)
    {
        huMetatag const * metatatg = (huMetatag *) trove->metatags.buffer + * cursor;
        if (metatatg->key->str.size == keyLen &&
            strncmp(metatatg->key->str.ptr, key, keyLen) == 0)
            { token = metatatg->value; break; }
    }

    * cursor += 1;
	return token;
}


huSize_t huGetNumTroveMetatagsWithValueZ(huTrove const * trove, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return 0; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huSize_t))
        { return 0; }

    return huGetNumTroveMetatagsWithValueN(trove, value, (huSize_t) valueLenC);
}


huSize_t huGetNumTroveMetatagsWithValueN(huTrove const * trove, char const * value, huSize_t valueLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || value  == NULL || valueLen < 0)
        { return 0; }
#endif

    huSize_t matches = 0;
    for (huSize_t i = 0; i < trove->metatags.numElements; ++i)
    {
        huMetatag * metatatg = (huMetatag *) trove->metatags.buffer + i;
        if (metatatg->value->str.size == valueLen &&
            strncmp(metatatg->value->str.ptr, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetTroveMetatagWithValueZ(huTrove const * trove, char const * value, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (value  == NULL)
        { return HU_NULLTOKEN; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huSize_t))
        { return HU_NULLTOKEN; }

    return huGetTroveMetatagWithValueN(trove, value, (huSize_t) valueLenC, cursor);
}


huToken const * huGetTroveMetatagWithValueN(huTrove const * trove, char const * value, huSize_t valueLen, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || value  == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
        { return HU_NULLTOKEN; }
#endif

    huToken const * token = HU_NULLTOKEN;
    for (; * cursor < trove->metatags.numElements; ++ * cursor)
    {
        huMetatag const * metatatg = (huMetatag *) trove->metatags.buffer + * cursor;
        if (metatatg->value->str.size == valueLen &&
            strncmp(metatatg->value->str.ptr, value, valueLen) == 0)
            { token = metatatg->key; break; }
    }

    * cursor += 1;
    return token;
}


huSize_t huGetNumTroveComments(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return 0; }
#endif

    return trove->comments.numElements;
}


huToken const * huGetTroveComment(huTrove const * trove, huSize_t commentIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || commentIdx < 0)
        { return HU_NULLTOKEN; }
#endif

    if (commentIdx < trove->comments.numElements)
    {
        return ((huComment *) trove->comments.buffer + commentIdx)->token;
    }

    return HU_NULLTOKEN;
}


huNode const * huFindNodesWithMetatagKeyZ(huTrove const * trove, char const * key, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
       { return HU_NULLNODE; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithMetatagKeyN(trove, key, (huSize_t) keyLenC, cursor);
}


huNode const * huFindNodesWithMetatagKeyN(huTrove const * trove, char const * key, huSize_t keyLen, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || key == NULL || keyLen < 0 || cursor == NULL || * cursor < 0)
       { return HU_NULLNODE; }
#endif

    huSize_t numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
        huSize_t numTokens = huGetNumMetatagsWithKeyN(node, key, keyLen);
        if (numTokens != 0)
        {
            * cursor += 1;
            return node;
        }
    }

    return HU_NULLNODE;
}


huNode const * huFindNodesWithMetatagValueZ(huTrove const * trove, char const * value, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
       { return HU_NULLNODE; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithMetatagValueN(trove, value, (huSize_t) valueLenC, cursor);
}


huNode const * huFindNodesWithMetatagValueN(huTrove const * trove, char const * value, huSize_t valueLen, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || value == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
       { return HU_NULLNODE; }
#endif

    huSize_t numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
		huSize_t na = huGetNumMetatagsWithValueN(node, value, valueLen);
        if (na > 0)
        {
            * cursor += 1;
            return node;
        }
    }

    return HU_NULLNODE;
}


huNode const * huFindNodesWithMetatagKeyValueZZ(huTrove const * trove, char const * key, char const * value, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL)
       { return HU_NULLNODE; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithMetatagKeyValueNN(trove, key, (huSize_t) keyLenC, value, (huSize_t) valueLenC, cursor);
}


huNode const * huFindNodesWithMetatagKeyValueNZ(huTrove const * trove, char const * key, huSize_t keyLen, char const * value, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || keyLen < 0 || value == NULL)
       { return HU_NULLNODE; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithMetatagKeyValueNN(trove, key, keyLen, value, (huSize_t) valueLenC, cursor);
}


huNode const * huFindNodesWithMetatagKeyValueZN(huTrove const * trove, char const * key, char const * value, huSize_t valueLen, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL || valueLen < 0)
       { return HU_NULLNODE; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithMetatagKeyValueNN(trove, key, (huSize_t) keyLenC, value, valueLen, cursor);
}


huNode const * huFindNodesWithMetatagKeyValueNN(huTrove const * trove, char const * key, huSize_t keyLen, char const * value, huSize_t valueLen, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || key == NULL || keyLen < 0 || value == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
       { return HU_NULLNODE; }
#endif

    huSize_t numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
		huSize_t metatatgCursor = 0;
        huToken const * metatatg = huGetMetatagWithKeyN(node, key, keyLen, & metatatgCursor);
        if (metatatg != NULL)
        {
            if (metatatg->str.size == valueLen &&
                strncmp(metatatg->str.ptr, value, valueLen) == 0)
            {
                * cursor += 1;
                return node;
            }
        }
    }

    return HU_NULLNODE;
}


huNode const * huFindNodesByCommentContainingZ(huTrove const * trove, char const * containedText, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
       { return HU_NULLNODE; }
#endif

    size_t containedTextLenC = strlen(containedText);
    if (containedTextLenC > maxOfType(huSize_t))
        { return HU_NULLNODE; }

    return huFindNodesByCommentContainingN(trove, containedText, (huSize_t) containedTextLenC, cursor);
}


huNode const * huFindNodesByCommentContainingN(huTrove const * trove, char const * containedText, huSize_t containedTextLen, huSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || containedText == NULL || containedTextLen < 0 || cursor == NULL || * cursor < 0)
       { return HU_NULLNODE; }
#endif

    huSize_t numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
        if (huHasCommentsContainingN(node, containedText, containedTextLen))
        {
            * cursor += 1;
            return node;
        }
    }

    return HU_NULLNODE;
}


huNode * allocNewNode(huTrove * trove, huNodeKind nodeKind, huToken const * firstToken)
{
    huSize_t num = 1;

    huNode * newNode = growVector(& trove->nodes, & num);
    if (num == 0)
        { return (huNode *) HU_NULLNODE; }

    initNode(newNode, trove);
    huSize_t newNodeIdx = (huSize_t)(newNode - (huNode *) trove->nodes.buffer);
    newNode->nodeIdx = newNodeIdx;
    newNode->kind = nodeKind;
    newNode->firstToken = firstToken;
    newNode->lastToken = firstToken;

#ifdef HUMON_CAVEPERSON_DEBUGGING
    printf ("%snode%s: nodeIdx: %s%lld%s    firstToken: %s%lld%s    %s%s%s\n",
        ansi_lightCyan, ansi_off,
        ansi_lightBlue, (long long) newNodeIdx, ansi_off,
        ansi_darkYellow, (long long)(firstToken - (huToken *) trove->tokens.buffer), ansi_off,
        ansi_lightMagenta, huNodeKindToString(nodeKind), ansi_off);
#endif

    return newNode;
}


void recordTokenizeError(huTrove * trove, huErrorCode errorCode, huLine_t line, huCol_t col)
{
	if (trove == NULL)
        { return; }

	huSize_t num = 1;
    huError * error = growVector(& trove->errors, & num);
    if (num)
    {
        error->errorCode = errorCode;
        error->token = NULL;
        error->line = line;
        error->col = col;
    }

    if (trove->errorResponse == HU_ERRORRESPONSE_MUM)
        { return; }

    FILE * stream = stdout;
    if (trove->errorResponse == HU_ERRORRESPONSE_STDERR ||
        trove->errorResponse == HU_ERRORRESPONSE_STDERRANSICOLOR)
        { stream = stderr; }

    if (trove->errorResponse == HU_ERRORRESPONSE_STDERR ||
        trove->errorResponse == HU_ERRORRESPONSE_STDOUT)
    {
        fprintf (stream, "Error: line: %llu    col: %llu    %s\n",
            (unsigned long long) line, (unsigned long long) col, huOutputErrorToString(errorCode));
    }
    else
    {
        fprintf (stream, "%sError%s: line: %llu    col: %llu    %s\n", ansi_lightRed, ansi_off,
            (unsigned long long) line, (unsigned long long) col, huOutputErrorToString(errorCode));
    }
}


void recordParseError(huTrove * trove, huErrorCode errorCode, huToken const * pCur)
{
    // Let's not worry about unexptected EOF if we've encountered other errors.
    // Yeah, maybe.
//    if (errorCode == HU_ERROR_UNEXPECTEDEOF &&
//        huGetNumErrors(trove) > 0)
//        { return; }

    huSize_t num = 1;
    huError * error = growVector(& trove->errors, & num);
    if (num)
    {
        error->errorCode = errorCode;
        error->token = pCur;
        error->line = pCur->line;
        error->col = pCur->col;
    }

    if (trove->errorResponse == HU_ERRORRESPONSE_MUM)
        { return; }

    FILE * stream = stdout;
    if (trove->errorResponse == HU_ERRORRESPONSE_STDERR ||
        trove->errorResponse == HU_ERRORRESPONSE_STDERRANSICOLOR)
        { stream = stderr; }

    if (trove->errorResponse == HU_ERRORRESPONSE_STDERR ||
        trove->errorResponse == HU_ERRORRESPONSE_STDOUT)
    {
        fprintf(stream, "Error: line: %llu    col: %llu    %s\n",
            (unsigned long long) pCur->line, (unsigned long long) pCur->col, huOutputErrorToString(errorCode));
    }
    else
    {
        fprintf(stream, "%sError%s: line: %llu    col: %llu    %s\n", ansi_lightRed, ansi_off,
            (unsigned long long) pCur->line, (unsigned long long) pCur->col, huOutputErrorToString(errorCode));
    }
}


huStringView huGetTroveSourceText(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
       { return (huStringView) { .ptr = "", .size = 0}; }
#endif

    huStringView sv = {
        .ptr = trove->dataString,
        .size = trove->dataStringSize
    };

    return sv;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"


huErrorCode huSerializeTrove(huTrove const * trove, char * dest, huSize_t * destLength, huSerializeOptions * serializeOptions)
{
    if (dest == NULL && destLength != NULL)
        { * destLength = 0; }

#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || destLength == NULL)
        { return HU_ERROR_BADPARAMETER; }
    if (serializeOptions &&
        (isNegative(serializeOptions->whitespaceFormat) || serializeOptions->whitespaceFormat >= 3 ||
         isNegative(serializeOptions->indentSize) ||
         (serializeOptions->usingColors && serializeOptions->colorTable == NULL)))
        { return HU_ERROR_BADPARAMETER; }

	// This is temporary until we add more unicode formats.
	if (serializeOptions &&
		serializeOptions->encoding != HU_ENCODING_UTF8)
		{ return HU_ERROR_BADENCODING; }
#endif

    huSerializeOptions localSerializeOptions;
    if (serializeOptions == NULL)
    {
        huInitSerializeOptionsN(& localSerializeOptions, HU_WHITESPACEFORMAT_PRETTY, 4, false, false, NULL, true, "\n", 1, HU_ENCODING_UTF8, false);
        serializeOptions = & localSerializeOptions;
    }

    if (serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_CLONED)
    {
        if (dest == NULL)
            { * destLength = trove->dataStringSize + serializeOptions->printBom * 3; }
        else
        {
            char * destWithBom = dest;
            int bomLen = 0;
            if (serializeOptions->printBom)
            {
                char utf8bom[] = { 0xef, 0xbb, 0xbf };
                memcpy(dest, utf8bom, 3);
                bomLen = 3;
                destWithBom += bomLen;
            }
            memcpy(destWithBom, trove->dataString, * destLength - bomLen);
        }
    }
    else if (serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY ||
             serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_MINIMAL)
    {
        // newline must be > 0; some things need a newline like // comments
        if (serializeOptions->newline.ptr == NULL || serializeOptions->newline.size < 1)
            { return HU_ERROR_BADPARAMETER; }

        huVector str;
        if (dest == NULL)
        {
            initVectorForCounting(& str); // counting only
        }
        else
        {
            initVectorPreallocated(& str, dest, sizeof(char), * destLength);
        }

        troveToPrettyString(trove, & str, serializeOptions);
        if (dest == NULL)
            { * destLength = str.numElements; }
    }

    return HU_ERROR_NOERROR;
}

#pragma GCC diagnostic pop

huErrorCode huSerializeTroveToFile(huTrove const * trove, char const * path, huSize_t * destLength, huSerializeOptions * serializeOptions)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || path == NULL)
        { return HU_ERROR_BADPARAMETER; }
    if (serializeOptions &&
        (isNegative(serializeOptions->whitespaceFormat) || serializeOptions->whitespaceFormat >= 3 ||
         isNegative(serializeOptions->indentSize) ||
         (serializeOptions->usingColors && serializeOptions->colorTable == NULL) ||
         serializeOptions->newline.ptr == NULL || serializeOptions->newline.size < 1))
        { return HU_ERROR_BADPARAMETER; }

	// This is temporary until we add more unicode formats.
	if (serializeOptions &&
		serializeOptions->encoding != HU_ENCODING_UTF8)
		{ return HU_ERROR_BADENCODING; }
#endif

    if (destLength)
        { * destLength = 0; }

    huSize_t strLength = 0;
    huErrorCode error = huSerializeTrove(trove, NULL, & strLength, serializeOptions);
    if (error != HU_ERROR_NOERROR)
        { return error; }

    char * str = ourAlloc(& trove->allocator, strLength);
    if (str == NULL)
        { return HU_ERROR_OUTOFMEMORY; }

    error = huSerializeTrove(trove, str, & strLength, serializeOptions);
    if (error != HU_ERROR_NOERROR)
        { ourFree(& trove->allocator, str); return error; }

	FILE * fp = openFile(path, "wb");
	if (fp == NULL)
        { ourFree(& trove->allocator, str); return HU_ERROR_BADFILE; }

    huSize_t writeLength = (huSize_t) fwrite(str, sizeof(char), strLength, fp);
	fclose(fp);
    ourFree(& trove->allocator, str);
    if (writeLength != strLength)
        { return HU_ERROR_BADFILE; }

    if (destLength)
        { * destLength = strLength; }

    return HU_ERROR_NOERROR;
}
