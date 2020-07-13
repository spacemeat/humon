#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


void initTrove(huTrove * trove, huLoadParams * loadParams)
{
    trove->dataString = NULL;
    trove->dataStringSize = 0;
    trove->encoding = loadParams->encoding;

    huInitGrowableVector(& trove->tokens, sizeof(huToken));
    huInitGrowableVector(& trove->nodes, sizeof(huNode));
    huInitGrowableVector(& trove->errors, sizeof(huError));

    trove->inputTabSize = loadParams->tabSize;

    huInitGrowableVector(& trove->annotations, sizeof(huAnnotation));
    huInitGrowableVector(& trove->comments, sizeof(huComment));

    trove->lastAnnoToken = NULL;
}


huTrove const * huMakeTroveFromStringZ(char const * data, huLoadParams * loadParams)
{
#ifdef HUMON_CHECK_PARAMS
    if (data == NULL)
        { return hu_nullTrove; }
#endif

    return huMakeTroveFromStringN(data, strlen(data), loadParams);
}


huTrove const * huMakeTroveFromStringN(char const * data, int dataLen, huLoadParams * loadParams)
{
#ifdef HUMON_CHECK_PARAMS
    if (data == NULL || dataLen < 0)
        { return hu_nullTrove; }
    if (loadParams &&
        (loadParams->encoding < 0 || loadParams->encoding > HU_ENCODING_UNKNOWN ||
         loadParams->tabSize < MIN_INPUT_TAB_SIZE || loadParams->tabSize > MAX_INPUT_TAB_SIZE))
        { return hu_nullTrove; }
#endif

    huLoadParams localLoadParams;
    if (loadParams == NULL)
    {
        huInitLoadParams(& localLoadParams, HU_ENCODING_UTF8, 4, true);
        loadParams = & localLoadParams;
    }

    huStringView inputDataView = { data, dataLen };

    if (loadParams->encoding == HU_ENCODING_UNKNOWN)
    {
        size_t numEncBytes = 0;    // not useful here
        loadParams->encoding = swagEncodingFromString(& inputDataView, & numEncBytes, loadParams);
        if (loadParams->encoding == HU_ENCODING_UNKNOWN)
            { return hu_nullTrove; }
    }
    
    huTrove * trove = malloc(sizeof(huTrove));
    if (trove == NULL)
        { return hu_nullTrove; }
    initTrove(trove, loadParams);

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
        return hu_nullTrove;
    }

    size_t transcodedLen = 0;
    int error = transcodeToUtf8FromString(newData, & transcodedLen, & inputDataView, loadParams);
    if (error != HU_ERROR_NOERROR)
    {
        free(newData);
        recordError(trove, error, NULL);
        return trove;
    }

    // transcodedLen is guaranteed to be <= dataLen.
    newData[transcodedLen] = '\0';
    newData[transcodedLen + 1] = '\0';
    newData[transcodedLen + 2] = '\0';
    newData[transcodedLen + 3] = '\0';

    trove->dataString = newData;
    trove->dataStringSize = transcodedLen;

    huTokenizeTrove(trove);
    huParseTrove(trove);
 
    return trove;
}


huTrove const * huMakeTroveFromFileZ(char const * path, huLoadParams * loadParams)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return hu_nullTrove; }
#endif

    return huMakeTroveFromFileN(path, strlen(path), loadParams);
}


huTrove const * huMakeTroveFromFileN(char const * path, int pathLen, huLoadParams * loadParams)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL || pathLen < 1)
        { return hu_nullTrove; }
    if (loadParams &&
        (loadParams->encoding < 0 || loadParams->encoding > HU_ENCODING_UNKNOWN ||
         loadParams->tabSize < MIN_INPUT_TAB_SIZE || loadParams->tabSize > MAX_INPUT_TAB_SIZE))
        { return hu_nullTrove; }
#endif

    huLoadParams localLoadParams;
    if (loadParams == NULL)
    {
        huInitLoadParams(& localLoadParams, HU_ENCODING_UNKNOWN, 4, true);
        loadParams = & localLoadParams;
    }

    FILE * fp = fopen(path, "r");
    if (fp == NULL)
        { return hu_nullTrove; }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        return hu_nullTrove;
    }

    int dataLen = ftell(fp);
    if (dataLen == -1L)
    {
        fclose(fp);
        return hu_nullTrove;
    }

    rewind(fp);

    if (loadParams->encoding == HU_ENCODING_UNKNOWN)
    {
        size_t numEncBytes = 0;    // not useful here
        loadParams->encoding = swagEncodingFromFile(fp, dataLen, & numEncBytes, loadParams);
        if (loadParams->encoding == HU_ENCODING_UNKNOWN)
        {
            fclose(fp);
            return hu_nullTrove;
        }
    
        rewind(fp);
    }
    
    huTrove * trove = malloc(sizeof(huTrove));
    if (trove == NULL)
    {
        fclose(fp);
        return hu_nullTrove;
    }
    initTrove(trove, loadParams);

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
        return hu_nullTrove;
    }

    size_t transcodedLen = 0;
    int error = transcodeToUtf8FromFile(newData, & transcodedLen, fp, dataLen, loadParams);

    fclose(fp);

    if (error != HU_ERROR_NOERROR)
    {
        free(newData);
        recordError(trove, error, NULL);
        return trove;
    }

    // transcodedLen is guaranteed to be <= dataLen.
    newData[transcodedLen] = '\0';
    newData[transcodedLen + 1] = '\0';
    newData[transcodedLen + 2] = '\0';
    newData[transcodedLen + 3] = '\0';

    trove->dataString = newData;
    trove->dataStringSize = transcodedLen;

    huTokenizeTrove(trove);
    huParseTrove(trove);
 
    return trove;
}


void huDestroyTrove(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return; }
#endif

    huTrove * ncTrove = (huTrove *) trove;

    if (ncTrove->dataString != NULL)
    {
        free((char *) ncTrove->dataString);
        ncTrove->dataString = NULL;
        ncTrove->dataStringSize = 0;
    }

    huDestroyVector(& ncTrove->tokens);
    huDestroyVector(& ncTrove->nodes);
    huDestroyVector(& ncTrove->errors);

    huDestroyVector(& ncTrove->annotations);
    huDestroyVector(& ncTrove->comments);

    free(ncTrove);
}


int huGetNumTokens(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->tokens.numElements;
}


huToken const * huGetToken(huTrove const * trove, int tokenIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
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
    huToken * newToken = huGrowVector(& trove->tokens, & num);
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
    if (trove == NULL || trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->nodes.numElements;
}


huNode const * huGetRootNode(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return hu_nullNode; }
#endif

    return (huNode *) trove->nodes.buffer;
}


huNode const * huGetNode(huTrove const * trove, int nodeIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove)
        { return hu_nullNode; }
#endif

    if (nodeIdx >= 0 && nodeIdx < trove->nodes.numElements)
        { return (huNode *) trove->nodes.buffer + nodeIdx; }

    return hu_nullNode;
}


int huGetNumErrors(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL)
        { return 0; }
#endif

    return trove->errors.numElements;
}


huError const * huGetError(huTrove const * trove, int errorIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || errorIdx < 0)
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
    if (trove == NULL)
        { return 0; }
#endif

    return trove->annotations.numElements;
}


huAnnotation const * huGetTroveAnnotation(huTrove const * trove, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || annotationIdx < 0)
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
    if (trove == NULL || trove == hu_nullTrove || key  == NULL || keyLen < 0)
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
    if (trove == NULL || trove == hu_nullTrove || key  == NULL || keyLen < 0)
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
    if (trove == NULL || trove == hu_nullTrove || value  == NULL || valueLen < 0)
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
    if (trove == NULL || trove == hu_nullTrove || value  == NULL || valueLen < 0 || annotationIdx < 0)
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
    if (trove == NULL || trove == hu_nullTrove)
        { return 0; }
#endif

    return trove->comments.numElements;
}


huToken const * huGetTroveComment(huTrove const * trove, int commentIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || commentIdx < 0)
        { return hu_nullToken; }
#endif

    if (commentIdx < trove->comments.numElements)
    {
        return ((huComment *) trove->comments.buffer + commentIdx)->token;
    }

    return hu_nullToken;
}


huNode const * huGetNodeByFullAddressZ(huTrove const * trove, char const * address)
{
#ifdef HUMON_CHECK_PARAMS
    if (address == NULL)
        { return hu_nullNode; }
#endif

    return huGetNodeByFullAddressN(trove, address, strlen(address));
}


huNode const * huGetNodeByFullAddressN(huTrove const * trove, char const * address, int addressLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || address == NULL || addressLen < 0)
        { return hu_nullNode; }
#endif

    // parse address; must start with '/' to start at root
    if (addressLen <= 0)
        { return hu_nullNode; }

    huScanner scanner;
    huInitScanner(& scanner, NULL, address, addressLen);
    int line = 0;  // unused
    int col = 0;

    eatWs(& scanner, 1, & line, & col);
    char const * wordStart = scanner.curCursor->character;

    if (scanner.curCursor->codePoint != '/')
        { return hu_nullNode; }

    huNode const * root = huGetRootNode(trove);
    if (root->kind == HU_NODEKIND_NULL)
        { return hu_nullNode; }

    return huGetNodeByRelativeAddressN(root, wordStart + 1, addressLen - col - 1);
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
    if (trove == NULL || trove == hu_nullTrove || key == NULL || keyLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);
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
    if (trove == NULL || trove == hu_nullTrove || value == NULL || valueLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);
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
    if (value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, keyLen, value, strlen(value), startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueZN(huTrove const * trove, char const * key, char const * value, int valueLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, strlen(key), value, valueLen, startWith);
}


// User must free(retval.buffer);
huNode const * huFindNodesWithAnnotationKeyValueNN(huTrove const * trove, char const * key, int keyLen, char const * value, int valueLen, huNode const * startWith)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || key == NULL || keyLen < 0 || value == NULL || valueLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);
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
    if (trove == NULL || trove == hu_nullTrove || containedText == NULL || containedTextLen < 0)
       { return hu_nullNode; }
#endif

    int beg = 0;
    if (startWith)
        { beg = startWith->nodeIdx + 1; }
    for (int i = beg; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);

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
    int foo = huGetVectorSize(& trove->nodes);
    if (foo > 15)
        { num = 1; }

    huNode * newNode = huGrowVector(& trove->nodes, & num);
    if (num == 0)
        { return (huNode *) hu_nullNode; }

    huInitNode(newNode, trove);
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
#ifdef HUMON_ERRORS_TO_STDERR
    fprintf (stderr, "%sError%s: line: %d    col: %d    %s\n", lightRed, off, 
        line, col, huOutputErrorToString(errorCode));
#endif

    int num = 1;
    huError * error = huGrowVector(& trove->errors, & num);
    if (num)
    {
        error->errorCode = errorCode;
        error->token = NULL;
        error->line = line;
        error->col = col;
    }
}


void recordError(huTrove * trove, int errorCode, huToken const * pCur)
{
#ifdef HUMON_ERRORS_TO_STDERR
    fprintf (stderr, "%sError%s: line: %d    col: %d    %s\n", lightRed, off, 
        pCur->line, pCur->col, huOutputErrorToString(errorCode));
#endif

    int num = 1;
    huError * error = huGrowVector(& trove->errors, & num);
    if (num)
    {
        error->errorCode = errorCode;
        error->token = pCur;
        error->line = pCur->line;
        error->col = pCur->col;
    }
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"


void huTroveToString(huTrove const * trove, char * dest, int * destLength, huStoreParams * storeParams)
{
    if (dest == NULL && destLength != NULL)
        { * destLength = 0; }

#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || destLength == NULL)
        { return; }
    if (storeParams &&
        (storeParams->outputFormat < 0 || storeParams->outputFormat >= 3 || 
         storeParams->tabSize < 0 || 
         (storeParams->usingColors && storeParams->colorTable == NULL) ||
         storeParams->newline.ptr == NULL || storeParams->newline.size < 0))
        { return; }
#endif

    huStoreParams localStoreParams;
    if (storeParams == NULL)
    {
        huInitStoreParamsN(& localStoreParams, HU_OUTPUTFORMAT_PRETTY, 4, false, NULL, true, "\n", 1, false );
        storeParams = & localStoreParams;
    }

    // newline must be > 0; some things need a newline like // comments
    if (storeParams->newline.size < 1)
        { return; }

    if (storeParams->outputFormat == HU_OUTPUTFORMAT_XERO)
    {
        if (dest == NULL)
            { * destLength = trove->dataStringSize; }
        else
            { memcpy(dest, trove->dataString, * destLength); }
    }
    else if (storeParams->outputFormat == HU_OUTPUTFORMAT_PRETTY ||
             storeParams->outputFormat == HU_OUTPUTFORMAT_MINIMAL)
    {
        huVector str;
        if (dest == NULL)
        {
            huInitVectorForCounting(& str); // counting only
        }
        else
        {
            huInitVectorPreallocated(& str, dest, sizeof(char), * destLength);
        }

        troveToPrettyString(trove, & str, storeParams);
        if (dest == NULL)
            { * destLength = str.numElements; }
    }
}

#pragma GCC diagnostic pop

size_t huTroveToFileZ(huTrove const * trove, char const * path, huStoreParams * storeParams)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return 0; }
#endif

    return huTroveToFileN(trove, path, strlen(path), storeParams);
}

size_t huTroveToFileN(huTrove const * trove, char const * path, int pathLen, huStoreParams * storeParams)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == hu_nullTrove || path == NULL)
        { return 0; }
    if (storeParams &&
        (storeParams->outputFormat < 0 || storeParams->outputFormat >= 3 || 
         storeParams->tabSize < 0 || 
         (storeParams->usingColors && storeParams->colorTable == NULL) ||
         storeParams->newline.ptr == NULL || storeParams->newline.size < 0))
        { return 0; }
#endif

    int strLength = 0;
    huTroveToString(trove, NULL, & strLength, storeParams);

    char * str = malloc(strLength + 1);
    if (str == NULL)
        { return 0; }

    huTroveToString(trove, str, & strLength, storeParams);

    FILE * fp = fopen(path, "w");
    if (fp == NULL)
        { free(str); return 0; }

    size_t ret = fwrite(str, sizeof(char), strLength, fp);

    free(str);
    return ret;
}
