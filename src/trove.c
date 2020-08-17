#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "humon.internal.h"


void initTrove(huTrove * trove, huDeserializeOptions * deserializeOptions, huEnumType_t errorResponse)
{
    trove->dataString = NULL;
    trove->dataStringSize = 0;
    trove->encoding = deserializeOptions->encoding;

    initGrowableVector(& trove->tokens, sizeof(huToken));
    initGrowableVector(& trove->nodes, sizeof(huNode));
    initGrowableVector(& trove->errors, sizeof(huError));

    trove->errorResponse = errorResponse;
    trove->inputTabSize = deserializeOptions->tabSize;

    initGrowableVector(& trove->annotations, sizeof(huAnnotation));
    initGrowableVector(& trove->comments, sizeof(huComment));

    trove->lastAnnoToken = NULL;
}


void printError(huEnumType_t errorResponse, char const * msg)
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


huEnumType_t huDeserializeTroveZ(huTrove const ** trovePtr, char const * data, huDeserializeOptions * deserializeOptions, huEnumType_t errorResponse)
{
    if (trovePtr)
        { * trovePtr = HU_NULLTROVE; }

#ifdef HUMON_CHECK_PARAMS
    if (data == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    // always check this'n
    size_t dataLenC = strlen(data);
    if (dataLenC > maxOfType(huIndexSize_t))
        { return HU_ERROR_BADPARAMETER; }

    return huDeserializeTroveN(trovePtr, data, (huIndexSize_t) dataLenC, deserializeOptions, errorResponse);
}


huEnumType_t huDeserializeTroveN(huTrove const ** trovePtr, char const * data, huIndexSize_t dataLen, huDeserializeOptions * deserializeOptions, huEnumType_t errorResponse)
{
    if (trovePtr)
        { * trovePtr = HU_NULLTROVE; }

#ifdef HUMON_CHECK_PARAMS
    if (trovePtr == NULL || data == NULL || dataLen < 0)
        { return HU_ERROR_BADPARAMETER; }
    if (deserializeOptions &&
        (deserializeOptions->encoding < 0 ||
         deserializeOptions->encoding > HU_ENCODING_UNKNOWN ||
         deserializeOptions->tabSize < 0))
        { return HU_ERROR_BADPARAMETER; }
    if (errorResponse < 0 ||
        errorResponse >= HU_ERRORRESPONSE_NUMRESPONSES)
        { return HU_ERROR_BADPARAMETER; }
#endif

    huDeserializeOptions localDeserializeOptions;
    if (deserializeOptions == NULL)
    {
        huInitDeserializeOptions(& localDeserializeOptions, HU_ENCODING_UTF8, true, 4);
        deserializeOptions = & localDeserializeOptions;
    }

    huStringView inputDataView = { data, dataLen };

    if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
    {
        huIndexSize_t numEncBytes = 0;    // not useful here
        deserializeOptions->encoding = swagEncodingFromString(& inputDataView, & numEncBytes, deserializeOptions);
        if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
        {
            printError(errorResponse, "Error: Could not determine Unicode encoding.");
            return HU_ERROR_BADENCODING;
        }
    }
    
    huTrove * trove = malloc(sizeof(huTrove));
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
    huIndexSize_t sizeFactor = deserializeOptions->allowUtf16UnmatchedSurrogates == false && 
                               (deserializeOptions->encoding == HU_ENCODING_UTF16_BE ||
                                deserializeOptions->encoding == HU_ENCODING_UTF16_LE) ? 2 : 1;
    char * newData = malloc(dataLen * sizeFactor + 4);  // padding with 4 bytes of null at the end.
    if (newData == NULL)
    {
        free(trove);
        printError(errorResponse, "Out of memory.");
        return HU_ERROR_OUTOFMEMORY;
    }

    huIndexSize_t transcodedLen = 0;
    huEnumType_t error = transcodeToUtf8FromString(newData, & transcodedLen, & inputDataView, deserializeOptions);
    if (error != HU_ERROR_NOERROR)
    {
        free(newData);
        free(trove);
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

    // Errors here are recorded in the trove object.
    tokenizeTrove(trove);
    parseTrove(trove);

    * trovePtr = trove;

    return huGetNumErrors(trove) == 0 ? HU_ERROR_NOERROR : HU_ERROR_TROVEHASERRORS;
}


huEnumType_t huDeserializeTroveFromFileZ(huTrove const ** trovePtr, char const * path, huDeserializeOptions * deserializeOptions, huEnumType_t errorResponse)
{
    if (trovePtr != NULL)
        { * trovePtr = HU_NULLTROVE; }

#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    // always check this'n
    size_t pathLenC = strlen(path);
    if (pathLenC > maxOfType(huIndexSize_t))
        { return HU_ERROR_BADPARAMETER; }

    return huDeserializeTroveFromFileN(trovePtr, path, (huIndexSize_t) pathLenC, deserializeOptions, errorResponse);
}


huEnumType_t huDeserializeTroveFromFileN(huTrove const ** trovePtr, char const * path, huIndexSize_t pathLen, huDeserializeOptions * deserializeOptions, huEnumType_t errorResponse)
{
    if (trovePtr != NULL)
        { * trovePtr = HU_NULLTROVE; }

#ifdef HUMON_CHECK_PARAMS
    if (path == NULL || pathLen < 1)
        { return HU_ERROR_BADPARAMETER; }
    if (deserializeOptions &&
        (deserializeOptions->encoding < 0 ||
         deserializeOptions->encoding > HU_ENCODING_UNKNOWN ||
         deserializeOptions->tabSize < 0))
        { return HU_ERROR_BADPARAMETER; }
    if (errorResponse < 0 ||
        errorResponse >= HU_ERRORRESPONSE_NUMRESPONSES)
        { return HU_ERROR_BADPARAMETER; }
#endif

    huDeserializeOptions localDeserializeOptions;
    if (deserializeOptions == NULL)
    {
        huInitDeserializeOptions(& localDeserializeOptions, HU_ENCODING_UNKNOWN, true, 4);
        deserializeOptions = & localDeserializeOptions;
    }

	FILE * fp = openFile(path, "rb");
	if (fp == NULL)
	{
        printError(errorResponse, "Could not open file for reading.");
        return HU_ERROR_BADFILE;
    }

    huIndexSize_t dataLen = 0;
    huEnumType_t gfsError = getFileSize(fp, & dataLen, errorResponse);
    if (gfsError != HU_ERROR_NOERROR)
    {
        fclose(fp);
        return gfsError;
    }

    if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
    {
        huIndexSize_t numEncBytes = 0;    // not useful here
        deserializeOptions->encoding = swagEncodingFromFile(fp, dataLen, & numEncBytes, deserializeOptions);
        if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
        {
            fclose(fp);
            printError(errorResponse, "Could not determine Unicode encoding.");
            return HU_ERROR_BADENCODING;
        }
    
        rewind(fp);
    }
    
    huTrove * trove = malloc(sizeof(huTrove));
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
    huIndexSize_t sizeFactor = deserializeOptions->allowUtf16UnmatchedSurrogates == false &&
                               (deserializeOptions->encoding == HU_ENCODING_UTF16_BE ||
                                deserializeOptions->encoding == HU_ENCODING_UTF16_LE) ? 2 : 1;
    char * newData = malloc(dataLen * sizeFactor + 4); // padding with 4 bytes of null at the end
    if (newData == NULL)
    {
        fclose(fp);
        printError(errorResponse, "Out of memory.");
        return HU_ERROR_OUTOFMEMORY;
    }

    huIndexSize_t transcodedLen = 0;
    huEnumType_t error = transcodeToUtf8FromFile(newData, & transcodedLen, fp, dataLen, deserializeOptions);
    fclose(fp);
    if (error != HU_ERROR_NOERROR)
    {
        free(newData);
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

    tokenizeTrove(trove);
    parseTrove(trove);
 
    * trovePtr = trove;

    return huGetNumErrors(trove) == 0 ? HU_ERROR_NOERROR : HU_ERROR_TROVEHASERRORS;
}


void huDestroyTrove(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return; }
#endif

    huTrove * ncTrove = (huTrove *) trove;

    if (ncTrove->dataString != NULL)
    {
        free((char *) ncTrove->dataString);
        ncTrove->dataString = NULL;
        ncTrove->dataStringSize = 0;
    }

    destroyVector(& ncTrove->tokens);
    destroyVector(& ncTrove->nodes);
    destroyVector(& ncTrove->errors);

    destroyVector(& ncTrove->annotations);
    destroyVector(& ncTrove->comments);

    free(ncTrove);
}


huIndexSize_t huGetNumTokens(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return 0; }
#endif

    return trove->tokens.numElements;
}


huToken const * huGetToken(huTrove const * trove, huIndexSize_t tokenIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return HU_NULLTOKEN; }
#endif

    if (tokenIdx < trove->tokens.numElements)
        { return (huToken *) trove->tokens.buffer + tokenIdx; }

    return HU_NULLTOKEN;
}


huToken * allocNewToken(huTrove * trove, huEnumType_t kind, char const * str, 
    huIndexSize_t size, huLine_t line, huCol_t col, huLine_t endLine, 
    huCol_t endCol, char quoteChar)
{
    huIndexSize_t num = 1;
    huToken * newToken = growVector(& trove->tokens, & num);
    if (num == 0)
        { return (huToken *) HU_NULLTOKEN; }

    newToken->kind = kind;
    newToken->quoteChar = quoteChar;
    newToken->str.ptr = str;
    newToken->str.size = size;
    newToken->line = line;
    newToken->col = col;
    newToken->endLine = endLine;
    newToken->endCol = endCol;

#ifdef HUMON_CAVEPERSON_DEBUGGING
    printf ("%stoken: line: %d  col: %d  len: %d  %s        '%.*s'%s\n",
        darkYellow, line, col, size, huTokenKindToString(kind), 
        newToken->str.size, newToken->str.ptr, off);
#endif
    
    return newToken;
}


huIndexSize_t huGetNumNodes(huTrove const * trove)
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


huNode const * huGetNodeByIndex(huTrove const * trove, huIndexSize_t nodeIdx)
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
    if (addressLenC > maxOfType(huIndexSize_t))
        { return HU_NULLNODE; }

    return huGetNodeByAddressN(trove, address, (huIndexSize_t) addressLenC);
}


huNode const * huGetNodeByAddressN(huTrove const * trove, char const * address, huIndexSize_t addressLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || address == NULL || addressLen < 0)
        { return HU_NULLNODE; }
#endif

    // parse address; must start with '/' to start at root
    if (addressLen <= 0)
        { return HU_NULLNODE; }

    huScanner scanner;
    initScanner(& scanner, NULL, address, addressLen);
    huLine_t line = 0;  // unused
    huCol_t col = 0;

    eatWs(& scanner, 1, & line, & col);
    char const * wordStart = scanner.curCursor->character;

    if (scanner.curCursor->codePoint != '/')
        { return HU_NULLNODE; }

    huNode const * root = huGetRootNode(trove);
    if (root->kind == HU_NODEKIND_NULL)
        { return HU_NULLNODE; }

    return huGetRelativeN(root, wordStart + 1, addressLen - col - 1);
}


huIndexSize_t huGetNumErrors(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return 0; }
#endif

    return trove->errors.numElements;
}


huError const * huGetError(huTrove const * trove, huIndexSize_t errorIdx)
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


huIndexSize_t huGetNumTroveAnnotations(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return 0; }
#endif

    return trove->annotations.numElements;
}


huAnnotation const * huGetTroveAnnotation(huTrove const * trove, huIndexSize_t annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || annotationIdx < 0)
        { return NULL; }
#endif

    if (annotationIdx < trove->annotations.numElements)
    {
        return (huAnnotation *) trove->annotations.buffer + annotationIdx;
    }

    return NULL;
}


bool huTroveHasAnnotationWithKeyZ(huTrove const * trove, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key  == NULL)
        { return false; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huIndexSize_t))
        { return false; }

    return huTroveHasAnnotationWithKeyN(trove, key, (huIndexSize_t) keyLenC);
}


bool huTroveHasAnnotationWithKeyN(huTrove const * trove, char const * key, huIndexSize_t keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || key  == NULL || keyLen < 0)
        { return false; }
#endif

    for (huIndexSize_t i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->key->str.size == keyLen && 
            strncmp(anno->key->str.ptr, key, keyLen) == 0)
            { return true; }
    }

    return false;
}


huToken const * huGetTroveAnnotationWithKeyZ(huTrove const * trove, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key  == NULL)
        { return HU_NULLTOKEN; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huIndexSize_t))
        { return HU_NULLTOKEN; }

    return huGetTroveAnnotationWithKeyN(trove, key, (huIndexSize_t) keyLenC);
}


huToken const * huGetTroveAnnotationWithKeyN(huTrove const * trove, char const * key, huIndexSize_t keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || key  == NULL || keyLen < 0)
        { return HU_NULLTOKEN; }
#endif

    for (huIndexSize_t i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->key->str.size == keyLen && 
            strncmp(anno->key->str.ptr, key, keyLen) == 0)
            { return anno->value; }
    }

    return HU_NULLTOKEN;
}


huIndexSize_t huGetNumTroveAnnotationsWithValueZ(huTrove const * trove, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return 0; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huIndexSize_t))
        { return 0; }

    return huGetNumTroveAnnotationsWithValueN(trove, value, (huIndexSize_t) valueLenC);
}


huIndexSize_t huGetNumTroveAnnotationsWithValueN(huTrove const * trove, char const * value, huIndexSize_t valueLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || value  == NULL || valueLen < 0)
        { return 0; }
#endif

    huIndexSize_t matches = 0;
    for (huIndexSize_t i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->value->str.size == valueLen && 
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetTroveAnnotationWithValueZ(huTrove const * trove, char const * value, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (value  == NULL)
        { return HU_NULLTOKEN; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huIndexSize_t))
        { return HU_NULLTOKEN; }

    return huGetTroveAnnotationWithValueN(trove, value, (huIndexSize_t) valueLenC, cursor);
}


huToken const * huGetTroveAnnotationWithValueN(huTrove const * trove, char const * value, huIndexSize_t valueLen, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || value  == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
        { return HU_NULLTOKEN; }
#endif

    huToken const * token = HU_NULLTOKEN;
    for (; * cursor < trove->annotations.numElements; ++ * cursor)
    { 
        huAnnotation const * anno = (huAnnotation *) trove->annotations.buffer + * cursor;
        if (anno->value->str.size == valueLen && 
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
            { token = anno->key; break; }
    }

    * cursor += 1;
    return token;
}


huIndexSize_t huGetNumTroveComments(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE)
        { return 0; }
#endif

    return trove->comments.numElements;
}


huToken const * huGetTroveComment(huTrove const * trove, huIndexSize_t commentIdx)
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


huNode const * huFindNodesWithAnnotationKeyZ(huTrove const * trove, char const * key, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
       { return HU_NULLNODE; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huIndexSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithAnnotationKeyN(trove, key, (huIndexSize_t) keyLenC, cursor);
}


huNode const * huFindNodesWithAnnotationKeyN(huTrove const * trove, char const * key, huIndexSize_t keyLen, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || key == NULL || keyLen < 0 || cursor == NULL || * cursor < 0)
       { return HU_NULLNODE; }
#endif

    huIndexSize_t numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
        bool hasOne = huHasAnnotationWithKeyN(node, key, keyLen);
        if (hasOne)
        {
            * cursor += 1;
            return node;
        }
    }

    return HU_NULLNODE;
}


huNode const * huFindNodesWithAnnotationValueZ(huTrove const * trove, char const * value, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
       { return HU_NULLNODE; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huIndexSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithAnnotationValueN(trove, value, (huIndexSize_t) valueLenC, cursor);
}


huNode const * huFindNodesWithAnnotationValueN(huTrove const * trove, char const * value, huIndexSize_t valueLen, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || value == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
       { return HU_NULLNODE; }
#endif

    huIndexSize_t numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
		huIndexSize_t na = huGetNumAnnotationsWithValueN(node, value, valueLen);
        if (na > 0)
        {
            * cursor += 1;
            return node;
        }
    }

    return HU_NULLNODE;
}


huNode const * huFindNodesWithAnnotationKeyValueZZ(huTrove const * trove, char const * key, char const * value, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL)
       { return HU_NULLNODE; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huIndexSize_t))
        { return HU_NULLNODE; }

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huIndexSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithAnnotationKeyValueNN(trove, key, (huIndexSize_t) keyLenC, value, (huIndexSize_t) valueLenC, cursor);
}


huNode const * huFindNodesWithAnnotationKeyValueNZ(huTrove const * trove, char const * key, huIndexSize_t keyLen, char const * value, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || keyLen < 0 || value == NULL)
       { return HU_NULLNODE; }
#endif

    size_t valueLenC = strlen(value);
    if (valueLenC > maxOfType(huIndexSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithAnnotationKeyValueNN(trove, key, keyLen, value, (huIndexSize_t) valueLenC, cursor);
}


huNode const * huFindNodesWithAnnotationKeyValueZN(huTrove const * trove, char const * key, char const * value, huIndexSize_t valueLen, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL || valueLen < 0)
       { return HU_NULLNODE; }
#endif

    size_t keyLenC = strlen(key);
    if (keyLenC > maxOfType(huIndexSize_t))
        { return HU_NULLNODE; }

    return huFindNodesWithAnnotationKeyValueNN(trove, key, (huIndexSize_t) keyLenC, value, valueLen, cursor);
}


huNode const * huFindNodesWithAnnotationKeyValueNN(huTrove const * trove, char const * key, huIndexSize_t keyLen, char const * value, huIndexSize_t valueLen, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || key == NULL || keyLen < 0 || value == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
       { return HU_NULLNODE; }
#endif

    huIndexSize_t numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
        huToken const * anno = huGetAnnotationWithKeyN(node, key, keyLen);
        if (anno != HU_NULLTOKEN)
        {
            if (anno->str.size == valueLen &&
                strncmp(anno->str.ptr, value, valueLen) == 0)
            {
                * cursor += 1;
                return node;
            }
        }
    }

    return HU_NULLNODE;
}


huNode const * huFindNodesByCommentContainingZ(huTrove const * trove, char const * containedText, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
       { return HU_NULLNODE; }
#endif

    size_t containedTextLenC = strlen(containedText);
    if (containedTextLenC > maxOfType(huIndexSize_t))
        { return HU_NULLNODE; }

    return huFindNodesByCommentContainingN(trove, containedText, (huIndexSize_t) containedTextLenC, cursor);
}


huNode const * huFindNodesByCommentContainingN(huTrove const * trove, char const * containedText, huIndexSize_t containedTextLen, huIndexSize_t * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || containedText == NULL || containedTextLen < 0 || cursor == NULL || * cursor < 0)
       { return HU_NULLNODE; }
#endif

    huIndexSize_t numNodes = huGetNumNodes(trove);
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


huNode * allocNewNode(huTrove * trove, huEnumType_t nodeKind, huToken const * firstToken)
{
    huIndexSize_t num = 1;

    huNode * newNode = growVector(& trove->nodes, & num);
    if (num == 0)
        { return (huNode *) HU_NULLNODE; }

    initNode(newNode, trove);
    huIndexSize_t newNodeIdx = (huIndexSize_t)(newNode - (huNode *) trove->nodes.buffer);
    newNode->nodeIdx = newNodeIdx;
    newNode->kind = nodeKind;
    newNode->firstToken = firstToken;
    newNode->lastToken = firstToken;

#ifdef HUMON_CAVEPERSON_DEBUGGING
    printf ("%snode: nodeIdx: %d    firstToken: %d    %s%s\n",
        lightCyan, newNodeIdx, (huIndexSize_t)(firstToken - (huToken *) trove->tokens.buffer), 
        huNodeKindToString(nodeKind), off);
#endif

    return newNode;
}


void recordTokenizeError(huTrove * trove, huEnumType_t errorCode, huLine_t line, huCol_t col)
{
	if (trove == NULL)
        { return; }

	huIndexSize_t num = 1;
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


void recordParseError(huTrove * trove, huEnumType_t errorCode, huToken const * pCur)
{
    // Let's not worry about unexptected EOF if we've encountered other errors.
    // Yeah, maybe.
//    if (errorCode == HU_ERROR_UNEXPECTEDEOF &&
//        huGetNumErrors(trove) > 0)
//        { return; }

    huIndexSize_t num = 1;
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
        fprintf (stream, "Error: line: %llu    col: %llu    %s\n", 
            (unsigned long long) pCur->line, (unsigned long long) pCur->col, huOutputErrorToString(errorCode));
    }
    else
    {
        fprintf (stream, "%sError%s: line: %llu    col: %llu    %s\n", ansi_lightRed, ansi_off, 
            (unsigned long long) pCur->line, (unsigned long long) pCur->col, huOutputErrorToString(errorCode));
    }
}


huStringView huGetTroveTokenStream(huTrove const * trove)
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


huEnumType_t huSerializeTrove(huTrove const * trove, char * dest, huIndexSize_t * destLength, huSerializeOptions * SerializeOptions)
{
    if (dest == NULL && destLength != NULL)
        { * destLength = 0; }

#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || destLength == NULL)
        { return HU_ERROR_BADPARAMETER; }
    if (SerializeOptions &&
        (SerializeOptions->WhitespaceFormat < 0 || SerializeOptions->WhitespaceFormat >= 3 || 
         SerializeOptions->indentSize < 0 || 
         (SerializeOptions->usingColors && SerializeOptions->colorTable == NULL) ||
         SerializeOptions->newline.ptr == NULL || SerializeOptions->newline.size < 1))
        { return HU_ERROR_BADPARAMETER; }
#endif

    huSerializeOptions localSerializeOptions;
    if (SerializeOptions == NULL)
    {
        huInitSerializeOptionsN(& localSerializeOptions, HU_WHITESPACEFORMAT_PRETTY, 4, false, false, NULL, true, "\n", 1, false );
        SerializeOptions = & localSerializeOptions;
    }

    if (SerializeOptions->WhitespaceFormat == HU_WHITESPACEFORMAT_CLONED)
    {
        if (dest == NULL)
            { * destLength = trove->dataStringSize + SerializeOptions->printBom * 3; }
        else
        {
            char * destWithBom = dest;
            int bomLen = 0;
            if (SerializeOptions->printBom)
            {
                char utf8bom[] = { 0xef, 0xbb, 0xbf };
                memcpy(dest, utf8bom, 3);
                bomLen = 3;
                destWithBom += bomLen;
            }
            memcpy(destWithBom, trove->dataString, * destLength - bomLen);
        }
    }
    else if (SerializeOptions->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY ||
             SerializeOptions->WhitespaceFormat == HU_WHITESPACEFORMAT_MINIMAL)
    {
        // newline must be > 0; some things need a newline like // comments
        if (SerializeOptions->newline.size < 1)
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

        troveToPrettyString(trove, & str, SerializeOptions);
        if (dest == NULL)
            { * destLength = str.numElements; }
    }

    return HU_ERROR_NOERROR;
}

#pragma GCC diagnostic pop

huEnumType_t huSerializeTroveToFile(huTrove const * trove, char const * path, huIndexSize_t * destLength, huSerializeOptions * SerializeOptions)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == HU_NULLTROVE || path == NULL)
        { return HU_ERROR_BADPARAMETER; }
    if (SerializeOptions &&
        (SerializeOptions->WhitespaceFormat < 0 || SerializeOptions->WhitespaceFormat >= 3 || 
         SerializeOptions->indentSize < 0 || 
         (SerializeOptions->usingColors && SerializeOptions->colorTable == NULL) ||
         SerializeOptions->newline.ptr == NULL || SerializeOptions->newline.size < 1))
        { return HU_ERROR_BADPARAMETER; }
#endif

    if (destLength)
        { * destLength = 0; }

    huIndexSize_t strLength = 0;
    huEnumType_t error = huSerializeTrove(trove, NULL, & strLength, SerializeOptions);
    if (error != HU_ERROR_NOERROR)
        { return error; }

    char * str = malloc(strLength);
    if (str == NULL)
        { return HU_ERROR_OUTOFMEMORY; }

    error = huSerializeTrove(trove, str, & strLength, SerializeOptions);
    if (error != HU_ERROR_NOERROR)
        { free(str); return error; }

	FILE * fp = openFile(path, "wb");
	if (fp == NULL)
	    { free(str); return HU_ERROR_BADFILE; }

    huIndexSize_t writeLength = (huIndexSize_t) fwrite(str, sizeof(char), strLength, fp);
    free(str);
    if (writeLength != strLength)
        { return HU_ERROR_BADFILE; }

    if (destLength)
        { * destLength = strLength; }

    return HU_ERROR_NOERROR;
}
