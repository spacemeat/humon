#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


void initTrove(huTrove * trove, huLoadParams * loadParams, int errorResponse)
{
    trove->dataString = NULL;
    trove->dataStringSize = 0;
    trove->encoding = loadParams->encoding;

    initGrowableVector(& trove->tokens, sizeof(huToken));
    initGrowableVector(& trove->nodes, sizeof(huNode));
    initGrowableVector(& trove->errors, sizeof(huError));

    trove->errorResponse = errorResponse;
    trove->inputTabSize = loadParams->tabSize;

    initGrowableVector(& trove->annotations, sizeof(huAnnotation));
    initGrowableVector(& trove->comments, sizeof(huComment));

    trove->lastAnnoToken = NULL;
}


int huMakeTroveFromStringZ(huTrove const ** trovePtr, char const * data, huLoadParams * loadParams, int errorResponse)
{
    if (trovePtr)
        { * trovePtr = hu_nullTrove; }

#ifdef HUMON_CHECK_PARAMS
    if (data == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    return huMakeTroveFromStringN(trovePtr, data, strlen(data), loadParams, errorResponse);
}


int huMakeTroveFromStringN(huTrove const ** trovePtr, char const * data, int dataLen, huLoadParams * loadParams, int errorResponse)
{
    if (trovePtr)
        { * trovePtr = hu_nullTrove; }

#ifdef HUMON_CHECK_PARAMS
    if (trovePtr == NULL || data == NULL || dataLen < 0)
        { return HU_ERROR_BADPARAMETER; }
    if (loadParams &&
        (loadParams->encoding < 0 ||
         loadParams->encoding > HU_ENCODING_UNKNOWN ||
         loadParams->tabSize < 0))
        { return HU_ERROR_BADPARAMETER; }
    if (errorResponse < 0 ||
        errorResponse >= HU_ERRORRESPONSE_NUMRESPONSES)
        { return HU_ERROR_BADPARAMETER; }
#endif

    huLoadParams localLoadParams;
    if (loadParams == NULL)
    {
        huInitLoadParams(& localLoadParams, HU_ENCODING_UTF8, true, 4);
        loadParams = & localLoadParams;
    }

    huStringView inputDataView = { data, dataLen };

    if (loadParams->encoding == HU_ENCODING_UNKNOWN)
    {
        size_t numEncBytes = 0;    // not useful here
        loadParams->encoding = swagEncodingFromString(& inputDataView, & numEncBytes, loadParams);
        if (loadParams->encoding == HU_ENCODING_UNKNOWN)
        {
            // TODO: Depending on errorResponse, output something
            return HU_ERROR_BADENCODING;
        }
    }
    
    huTrove * trove = malloc(sizeof(huTrove));
    if (trove == hu_nullTrove)
    {
        // TODO: Depending on errorResponse, output something
        return HU_ERROR_OUTOFMEMORY;
    }
        
    initTrove(trove, loadParams, errorResponse);

    // TODO: Padding with 4 nulls for now; let's see if we actually need to.
    // We're guaranteed that UTF8 strings will be no longer than the transcoded UTF* 
    // equivalent, as long as we reject unpaired surrogates. MS filenames
    // can contain unpaired surrogates, and Humon will accept them if strictUnicode
    // is clear. At that point, a UTF8 string can be longer than its UTF16, so we
    // have to double the size.
    int sizeFactor = loadParams->allowUtf16UnmatchedSurrogates == false && 
                       (loadParams->encoding == HU_ENCODING_UTF16_BE ||
                        loadParams->encoding == HU_ENCODING_UTF16_LE) ? 2 : 1;
    char * newData = malloc(dataLen * sizeFactor + 4);
    if (newData == NULL)
    {
        free(trove);
        // TODO: Depending on errorResponse, output something
        return HU_ERROR_OUTOFMEMORY;
    }

    size_t transcodedLen = 0;
    int error = transcodeToUtf8FromString(newData, & transcodedLen, & inputDataView, loadParams);
    if (error != HU_ERROR_NOERROR)
    {
        free(newData);
        free(trove);
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


int huMakeTroveFromFileZ(huTrove const ** trovePtr, char const * path, huLoadParams * loadParams, int errorResponse)
{
    if (trovePtr != NULL)
        { * trovePtr = hu_nullTrove; }

#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    return huMakeTroveFromFileN(trovePtr, path, strlen(path), loadParams, errorResponse);
}


int huMakeTroveFromFileN(huTrove const ** trovePtr, char const * path, int pathLen, huLoadParams * loadParams, int errorResponse)
{
    if (trovePtr != NULL)
        { * trovePtr = hu_nullTrove; }

#ifdef HUMON_CHECK_PARAMS
    if (path == NULL || pathLen < 1)
        { return HU_ERROR_BADPARAMETER; }
    if (loadParams &&
        (loadParams->encoding < 0 ||
         loadParams->encoding > HU_ENCODING_UNKNOWN ||
         loadParams->tabSize < 0))
        { return HU_ERROR_BADPARAMETER; }
    if (errorResponse < 0 ||
        errorResponse >= HU_ERRORRESPONSE_NUMRESPONSES)
        { return HU_ERROR_BADPARAMETER; }
#endif

    huLoadParams localLoadParams;
    if (loadParams == NULL)
    {
        huInitLoadParams(& localLoadParams, HU_ENCODING_UNKNOWN, true, 4);
        loadParams = & localLoadParams;
    }

    FILE * fp = fopen(path, "rb");
    if (fp == NULL)
    {
        // TODO: Depending on errorResponse, output something
        return HU_ERROR_BADFILE;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        // TODO: Depending on errorResponse, output something
        return HU_ERROR_BADFILE;
    }

    int dataLen = ftell(fp);
    if (dataLen == -1L)
    {
        fclose(fp);
        // TODO: Depending on errorResponse, output something
        return HU_ERROR_BADFILE;
    }

    rewind(fp);

    if (loadParams->encoding == HU_ENCODING_UNKNOWN)
    {
        size_t numEncBytes = 0;    // not useful here
        loadParams->encoding = swagEncodingFromFile(fp, dataLen, & numEncBytes, loadParams);
        if (loadParams->encoding == HU_ENCODING_UNKNOWN)
        {
            fclose(fp);
            // TODO: Depending on errorResponse, output something
            return HU_ERROR_BADENCODING;
        }
    
        rewind(fp);
    }
    
    huTrove * trove = malloc(sizeof(huTrove));
    if (trove == hu_nullTrove)
    {
        fclose(fp);
        // TODO: Depending on errorResponse, output something
        return HU_ERROR_OUTOFMEMORY;
    }

    initTrove(trove, loadParams, errorResponse);

    // TODO: Padding with 4 nulls for now; let's see if we actually need to.
    // We're guaranteed that UTF8 strings will be no longer than the transcoded UTF* 
    // equivalent, as long as we reject unpaired surrogates. MS filenames
    // can contain unpaired surrogates, and Humon will accept them if strictUnicode
    // is clear. At that point, a UTF8 string can be longer than its UTF16, so we
    // have to double the size.
    int sizeFactor = loadParams->allowUtf16UnmatchedSurrogates == false &&
                       (loadParams->encoding == HU_ENCODING_UTF16_BE ||
                        loadParams->encoding == HU_ENCODING_UTF16_LE) ? 2 : 1;
    char * newData = malloc(dataLen * sizeFactor + 4);
    if (newData == NULL)
    {
        fclose(fp);
        // TODO: Depending on errorResponse, output something
        return HU_ERROR_OUTOFMEMORY;
    }

    size_t transcodedLen = 0;
    int error = transcodeToUtf8FromFile(newData, & transcodedLen, fp, dataLen, loadParams);
    fclose(fp);
    if (error != HU_ERROR_NOERROR)
    {
        free(newData);
        // TODO: Depending on errorResponse, output something
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
    if (trove == hu_nullTrove)
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


int huGetNumTokens(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->tokens.numElements;
}


huToken const * huGetToken(huTrove const * trove, int tokenIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove)
        { return hu_nullToken; }
#endif

    if (tokenIdx < trove->tokens.numElements)
        { return (huToken *) trove->tokens.buffer + tokenIdx; }

    return hu_nullToken;
}


huToken * allocNewToken(huTrove * trove, int kind, 
    char const * str, int size, int line, int col, int endLine, 
    int endCol, char quoteChar)
{
    int num = 1;
    huToken * newToken = growVector(& trove->tokens, & num);
    if (num == 0)
        { return (huToken *) hu_nullToken; }

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


int huGetNumNodes(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->nodes.numElements;
}


huNode const * huGetRootNode(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove)
        { return hu_nullNode; }
#endif

    return (huNode *) trove->nodes.buffer;
}


huNode const * huGetNodeByIndex(huTrove const * trove, int nodeIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove)
        { return hu_nullNode; }
#endif

    if (nodeIdx >= 0 && nodeIdx < trove->nodes.numElements)
        { return (huNode *) trove->nodes.buffer + nodeIdx; }

    return hu_nullNode;
}


huNode const * huGetNodeByAddressZ(huTrove const * trove, char const * address)
{
#ifdef HUMON_CHECK_PARAMS
    if (address == NULL)
        { return hu_nullNode; }
#endif

    return huGetNodeByAddressN(trove, address, strlen(address));
}


huNode const * huGetNodeByAddressN(huTrove const * trove, char const * address, int addressLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || address == NULL || addressLen < 0)
        { return hu_nullNode; }
#endif

    // parse address; must start with '/' to start at root
    if (addressLen <= 0)
        { return hu_nullNode; }

    huScanner scanner;
    initScanner(& scanner, NULL, address, addressLen);
    int line = 0;  // unused
    int col = 0;

    eatWs(& scanner, 1, & line, & col);
    char const * wordStart = scanner.curCursor->character;

    if (scanner.curCursor->codePoint != '/')
        { return hu_nullNode; }

    huNode const * root = huGetRootNode(trove);
    if (root->kind == HU_NODEKIND_NULL)
        { return hu_nullNode; }

    return huGetRelativeN(root, wordStart + 1, addressLen - col - 1);
}


int huGetNumErrors(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->errors.numElements;
}


huError const * huGetError(huTrove const * trove, int errorIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || errorIdx < 0)
        { return NULL; }
#endif

    if (errorIdx < huGetNumErrors(trove))
    {
        return (huError *) trove->errors.buffer + errorIdx;
    }

    return NULL;
}


int huGetNumTroveAnnotations(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->annotations.numElements;
}


huAnnotation const * huGetTroveAnnotation(huTrove const * trove, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || annotationIdx < 0)
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

    return huTroveHasAnnotationWithKeyN(trove, key, strlen(key));
}


bool huTroveHasAnnotationWithKeyN(huTrove const * trove, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || key  == NULL || keyLen < 0)
        { return false; }
#endif

    for (int i = 0; i < trove->annotations.numElements; ++i)
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
        { return hu_nullToken; }
#endif

    return huGetTroveAnnotationWithKeyN(trove, key, strlen(key));
}


huToken const * huGetTroveAnnotationWithKeyN(huTrove const * trove, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || key  == NULL || keyLen < 0)
        { return hu_nullToken; }
#endif

    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->key->str.size == keyLen && 
            strncmp(anno->key->str.ptr, key, keyLen) == 0)
            { return anno->value; }
    }

    return hu_nullToken;
}


int huGetNumTroveAnnotationsWithValueZ(huTrove const * trove, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return 0; }
#endif

    return huGetNumTroveAnnotationsWithValueN(trove, value, strlen(value));
}


int huGetNumTroveAnnotationsWithValueN(huTrove const * trove, char const * value, int valueLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || value  == NULL || valueLen < 0)
        { return 0; }
#endif

    int matches = 0;
    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->value->str.size == valueLen && 
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetTroveAnnotationWithValueZ(huTrove const * trove, char const * value, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (value  == NULL)
        { return hu_nullToken; }
#endif

    return huGetTroveAnnotationWithValueN(trove, value, strlen(value), annotationIdx);
}


huToken const * huGetTroveAnnotationWithValueN(huTrove const * trove, char const * value, int valueLen, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || value  == NULL || valueLen < 0 || annotationIdx < 0)
        { return hu_nullToken; }
#endif

    int matches = 0;
    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->value->str.size == valueLen && 
            strncmp(anno->value->str.ptr, value, valueLen) == 0)
        {
            if (matches == annotationIdx)
                { return anno->key; }

            matches += 1;
        }
    }

    return hu_nullToken;
}


int huGetNumTroveComments(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->comments.numElements;
}


huToken const * huGetTroveComment(huTrove const * trove, int commentIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || commentIdx < 0)
        { return hu_nullToken; }
#endif

    if (commentIdx < trove->comments.numElements)
    {
        return ((huComment *) trove->comments.buffer + commentIdx)->token;
    }

    return hu_nullToken;
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyZ(huTrove const * trove, char const * key, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyN(trove, key, strlen(key), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyN(huTrove const * trove, char const * key, int keyLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || key == NULL || keyLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNodeByIndex(trove, i);
        bool hasOne = huHasAnnotationWithKeyN(node, key, keyLen);
        if (hasOne)
            { return node; }
    }

    return hu_nullNode;
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationValueZ(huTrove const * trove, char const * value, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationValueN(trove, value, strlen(value), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationValueN(huTrove const * trove, char const * value, int valueLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || value == NULL || valueLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNodeByIndex(trove, i);
        int na = huGetNumAnnotationsWithValueN(node, value, valueLen);
        if (na > 0)
           { return node; }
    }

    return hu_nullNode;
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueZZ(huTrove const * trove, char const * key, char const * value, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, strlen(key), value, strlen(value), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueNZ(huTrove const * trove, char const * key, int keyLen, char const * value, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || keyLen < 0 || value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, keyLen, value, strlen(value), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueZN(huTrove const * trove, char const * key, char const * value, int valueLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL || valueLen < 0)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, strlen(key), value, valueLen, startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueNN(huTrove const * trove, char const * key, int keyLen, char const * value, int valueLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || key == NULL || keyLen < 0 || value == NULL || valueLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNodeByIndex(trove, i);
        huToken const * anno = huGetAnnotationWithKeyN(node, key, keyLen);
        if (anno != hu_nullToken)
        {
            if (anno->str.size == valueLen &&
                strncmp(anno->str.ptr, value, valueLen) == 0)
                { return node; }
        }
    }

    return hu_nullNode;
}


huNode const * huFindNodesByCommentContainingZ(huTrove const * trove, char const * containedText, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesByCommentContainingN(trove, containedText, strlen(containedText), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesByCommentContainingN(huTrove const * trove, char const * containedText, int containedTextLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || containedText == NULL || containedTextLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNodeByIndex(trove, i);

        int na = huGetNumComments(node);
        for (int j = 0; j < na; ++j)
        {
            huToken const * comm = huGetComment(node, j);
            if (stringInString(comm->str.ptr, comm->str.size, containedText, containedTextLen))
                { return node; }
        }
    }

    return hu_nullNode;
}


huNode * allocNewNode(huTrove * trove, int nodeKind, huToken const * firstToken)
{
    int num = 1;

    // TODO: Remove this debug hook
    int foo = getVectorSize(& trove->nodes);
    if (foo > 15)
        { num = 1; }

    huNode * newNode = growVector(& trove->nodes, & num);
    if (num == 0)
        { return (huNode *) hu_nullNode; }

    initNode(newNode, trove);
    int newNodeIdx = newNode - (huNode *) trove->nodes.buffer;
    newNode->nodeIdx = newNodeIdx;
    newNode->kind = nodeKind;
    newNode->firstToken = firstToken;
    newNode->lastToken = firstToken;

#ifdef HUMON_CAVEPERSON_DEBUGGING
     printf ("%snode: nodeIdx: %d    firstToken: %d    %s%s\n",
        lightCyan, newNodeIdx, (int)(firstToken - (huToken *) trove->tokens.buffer), 
        huNodeKindToString(nodeKind), off);
#endif

    return newNode;
}


void recordTokenizeError(huTrove * trove, int errorCode, int line, int col)
{
    int num = 1;
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
        fprintf (stream, "Error: line: %d    col: %d    %s\n", 
            line, col, huOutputErrorToString(errorCode));
    }
    else
    {
        fprintf (stream, "%sError%s: line: %d    col: %d    %s\n", ansi_lightRed, ansi_off, 
            line, col, huOutputErrorToString(errorCode));
    }
}


void recordParseError(huTrove * trove, int errorCode, huToken const * pCur)
{
    int num = 1;
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
        fprintf (stream, "Error: line: %d    col: %d    %s\n", 
            pCur->line, pCur->col, huOutputErrorToString(errorCode));
    }
    else
    {
        fprintf (stream, "%sError%s: line: %d    col: %d    %s\n", ansi_lightRed, ansi_off, 
            pCur->line, pCur->col, huOutputErrorToString(errorCode));
    }
}


huStringView huGetTroveTokenStream(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove)
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


int huTroveToString(huTrove const * trove, char * dest, int * destLength, huStoreParams * storeParams)
{
    if (dest == NULL && destLength != NULL)
        { * destLength = 0; }

#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || destLength == NULL)
        { return HU_ERROR_BADPARAMETER; }
    if (storeParams &&
        (storeParams->WhitespaceFormat < 0 || storeParams->WhitespaceFormat >= 3 || 
         storeParams->indentSize < 0 || 
         (storeParams->usingColors && storeParams->colorTable == NULL) ||
         storeParams->newline.ptr == NULL || storeParams->newline.size < 0))
        { return HU_ERROR_BADPARAMETER; }
#endif

    huStoreParams localStoreParams;
    if (storeParams == NULL)
    {
        huInitStoreParamsN(& localStoreParams, HU_WHITESPACEFORMAT_PRETTY, 4, false, false, NULL, true, "\n", 1, false );
        storeParams = & localStoreParams;
    }

    if (storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_XERO)
    {
        if (dest == NULL)
            { * destLength = trove->dataStringSize + storeParams->printBom * 3; }
        else
        {
            char * destWithBom = dest;
            int bomLen = 0;
            if (storeParams->printBom)
            {
                char utf8bom[] = { 0xef, 0xbb, 0xbf };
                memcpy(dest, utf8bom, 3);
                bomLen = 3;
                destWithBom += bomLen;
            }
            memcpy(destWithBom, trove->dataString, * destLength - bomLen);
        }
    }
    else if (storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY ||
             storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_MINIMAL)
    {
        // newline must be > 0; some things need a newline like // comments
        if (storeParams->newline.size < 1)
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

        troveToPrettyString(trove, & str, storeParams);
        if (dest == NULL)
            { * destLength = str.numElements; }
    }

    return HU_ERROR_NOERROR;
}

#pragma GCC diagnostic pop

int huTroveToFileZ(huTrove const * trove, char const * path, int * destLength, huStoreParams * storeParams)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    return huTroveToFileN(trove, path, strlen(path), destLength, storeParams);
}

int huTroveToFileN(huTrove const * trove, char const * path, int pathLen, int * destLength, huStoreParams * storeParams)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || path == NULL)
        { return HU_ERROR_BADPARAMETER; }
    if (storeParams &&
        (storeParams->WhitespaceFormat < 0 || storeParams->WhitespaceFormat >= 3 || 
         storeParams->indentSize < 0 || 
         (storeParams->usingColors && storeParams->colorTable == NULL) ||
         storeParams->newline.ptr == NULL || storeParams->newline.size < 0))
        { return HU_ERROR_BADPARAMETER; }
#endif

    if (destLength)
        { * destLength = 0; }

    int strLength = 0;
    int error = huTroveToString(trove, NULL, & strLength, storeParams);
    if (error != HU_ERROR_NOERROR)
        { return error; }

    char * str = malloc(strLength);
    if (str == NULL)
        { return HU_ERROR_OUTOFMEMORY; }

    error = huTroveToString(trove, str, & strLength, storeParams);
    if (error != HU_ERROR_NOERROR)
        { free(str); return error; }

    FILE * fp = fopen(path, "w");
    if (fp == NULL)
        { free(str); return HU_ERROR_BADFILE; }

    int writeLength = fwrite(str, sizeof(char), strLength, fp);
    free(str);
    if (writeLength != strLength)
        { return HU_ERROR_BADFILE; }

    if (destLength)
        { * destLength = strLength; }

    return HU_ERROR_NOERROR;
}
