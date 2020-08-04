#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


void initTrove(huTrove * trove, huDeserializeOptions * deserializeOptions, int errorResponse)
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


int huDeserializeTroveZ(huTrove const ** trovePtr, char const * data, huDeserializeOptions * deserializeOptions, int errorResponse)
{
    if (trovePtr)
        { * trovePtr = hu_nullTrove; }

#ifdef HUMON_CHECK_PARAMS
    if (data == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    return huDeserializeTroveN(trovePtr, data, strlen(data), deserializeOptions, errorResponse);
}


static void printError(int errorResponse, char const * msg)
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


int huDeserializeTroveN(huTrove const ** trovePtr, char const * data, int dataLen, huDeserializeOptions * deserializeOptions, int errorResponse)
{
    if (trovePtr)
        { * trovePtr = hu_nullTrove; }

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
        size_t numEncBytes = 0;    // not useful here
        deserializeOptions->encoding = swagEncodingFromString(& inputDataView, & numEncBytes, deserializeOptions);
        if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
        {
            printError(errorResponse, "Error: Could not determine Unicode encoding.");
            return HU_ERROR_BADENCODING;
        }
    }
    
    huTrove * trove = malloc(sizeof(huTrove));
    if (trove == hu_nullTrove)
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
    int sizeFactor = deserializeOptions->allowUtf16UnmatchedSurrogates == false && 
                       (deserializeOptions->encoding == HU_ENCODING_UTF16_BE ||
                        deserializeOptions->encoding == HU_ENCODING_UTF16_LE) ? 2 : 1;
    char * newData = malloc(dataLen * sizeFactor + 4);  // padding with 4 bytes of null at the end.
    if (newData == NULL)
    {
        free(trove);
        printError(errorResponse, "Out of memory.");
        return HU_ERROR_OUTOFMEMORY;
    }

    size_t transcodedLen = 0;
    int error = transcodeToUtf8FromString(newData, & transcodedLen, & inputDataView, deserializeOptions);
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


int huDeserializeTroveFromFileZ(huTrove const ** trovePtr, char const * path, huDeserializeOptions * deserializeOptions, int errorResponse)
{
    if (trovePtr != NULL)
        { * trovePtr = hu_nullTrove; }

#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    return huDeserializeTroveFromFileN(trovePtr, path, strlen(path), deserializeOptions, errorResponse);
}


int huDeserializeTroveFromFileN(huTrove const ** trovePtr, char const * path, int pathLen, huDeserializeOptions * deserializeOptions, int errorResponse)
{
    if (trovePtr != NULL)
        { * trovePtr = hu_nullTrove; }

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

    FILE * fp = fopen(path, "rb");
    if (fp == NULL)
    {
        printError(errorResponse, "Could not open file for reading.");
        return HU_ERROR_BADFILE;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        printError(errorResponse, "Could not read file.");
        return HU_ERROR_BADFILE;
    }

    int dataLen = ftell(fp);
    if (dataLen == -1L)
    {
        fclose(fp);
        printError(errorResponse, "Could not read file.");
        return HU_ERROR_BADFILE;
    }

    rewind(fp);

    if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
    {
        size_t numEncBytes = 0;    // not useful here
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
    if (trove == hu_nullTrove)
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
    int sizeFactor = deserializeOptions->allowUtf16UnmatchedSurrogates == false &&
                       (deserializeOptions->encoding == HU_ENCODING_UTF16_BE ||
                        deserializeOptions->encoding == HU_ENCODING_UTF16_LE) ? 2 : 1;
    char * newData = malloc(dataLen * sizeFactor + 4); // padding with 4 bytes of null at the end
    if (newData == NULL)
    {
        fclose(fp);
        printError(errorResponse, "Out of memory.");
        return HU_ERROR_OUTOFMEMORY;
    }

    size_t transcodedLen = 0;
    int error = transcodeToUtf8FromFile(newData, & transcodedLen, fp, dataLen, deserializeOptions);
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


huToken const * huGetTroveAnnotationWithValueZ(huTrove const * trove, char const * value, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (value  == NULL)
        { return hu_nullToken; }
#endif

    return huGetTroveAnnotationWithValueN(trove, value, strlen(value), cursor);
}


huToken const * huGetTroveAnnotationWithValueN(huTrove const * trove, char const * value, int valueLen, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || value  == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
        { return hu_nullToken; }
#endif

    huToken const * token = hu_nullToken;
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


huNode const * huFindNodesWithAnnotationKeyZ(huTrove const * trove, char const * key, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyN(trove, key, strlen(key), cursor);
}


huNode const * huFindNodesWithAnnotationKeyN(huTrove const * trove, char const * key, int keyLen, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || key == NULL || keyLen < 0 || cursor == NULL || * cursor < 0)
       { return hu_nullNode; }
#endif

    int numNodes = huGetNumNodes(trove);
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

    return hu_nullNode;
}


huNode const * huFindNodesWithAnnotationValueZ(huTrove const * trove, char const * value, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationValueN(trove, value, strlen(value), cursor);
}


huNode const * huFindNodesWithAnnotationValueN(huTrove const * trove, char const * value, int valueLen, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || value == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
       { return hu_nullNode; }
#endif

    int numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
        int na = huGetNumAnnotationsWithValueN(node, value, valueLen);
        if (na > 0)
        {
            * cursor += 1;
            return node;
        }
    }

    return hu_nullNode;
}


huNode const * huFindNodesWithAnnotationKeyValueZZ(huTrove const * trove, char const * key, char const * value, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, strlen(key), value, strlen(value), cursor);
}


huNode const * huFindNodesWithAnnotationKeyValueNZ(huTrove const * trove, char const * key, int keyLen, char const * value, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || keyLen < 0 || value == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, keyLen, value, strlen(value), cursor);
}


huNode const * huFindNodesWithAnnotationKeyValueZN(huTrove const * trove, char const * key, char const * value, int valueLen, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL || valueLen < 0)
       { return hu_nullNode; }
#endif

    return huFindNodesWithAnnotationKeyValueNN(trove, key, strlen(key), value, valueLen, cursor);
}


huNode const * huFindNodesWithAnnotationKeyValueNN(huTrove const * trove, char const * key, int keyLen, char const * value, int valueLen, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || key == NULL || keyLen < 0 || value == NULL || valueLen < 0 || cursor == NULL || * cursor < 0)
       { return hu_nullNode; }
#endif

    int numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
        huToken const * anno = huGetAnnotationWithKeyN(node, key, keyLen);
        if (anno != hu_nullToken)
        {
            if (anno->str.size == valueLen &&
                strncmp(anno->str.ptr, value, valueLen) == 0)
            {
                * cursor += 1;
                return node;
            }
        }
    }

    return hu_nullNode;
}


huNode const * huFindNodesByCommentContainingZ(huTrove const * trove, char const * containedText, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
       { return hu_nullNode; }
#endif

    return huFindNodesByCommentContainingN(trove, containedText, strlen(containedText), cursor);
}


huNode const * huFindNodesByCommentContainingN(huTrove const * trove, char const * containedText, int containedTextLen, int * cursor)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || containedText == NULL || containedTextLen < 0 || cursor == NULL || * cursor < 0)
       { return hu_nullNode; }
#endif

    int numNodes = huGetNumNodes(trove);
    for (; * cursor < numNodes; ++ * cursor)
    {
        huNode const * node = huGetNodeByIndex(trove, * cursor);
        if (huHasCommentsContainingN(node, containedText, containedTextLen))
        {
            * cursor += 1;
            return node;
        }
    }

    return hu_nullNode;
}


huNode * allocNewNode(huTrove * trove, int nodeKind, huToken const * firstToken)
{
    int num = 1;

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
    // Let's not worry about unexptected EOF if we've encountered other errors.
//    if (errorCode == HU_ERROR_UNEXPECTEDEOF &&
//        huGetNumErrors(trove) > 0)
//        { return; }

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


int huSerializeTrove(huTrove const * trove, char * dest, int * destLength, huSerializeOptions * SerializeOptions)
{
    if (dest == NULL && destLength != NULL)
        { * destLength = 0; }

#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || destLength == NULL)
        { return HU_ERROR_BADPARAMETER; }
    if (SerializeOptions &&
        (SerializeOptions->WhitespaceFormat < 0 || SerializeOptions->WhitespaceFormat >= 3 || 
         SerializeOptions->indentSize < 0 || 
         (SerializeOptions->usingColors && SerializeOptions->colorTable == NULL) ||
         SerializeOptions->newline.ptr == NULL || SerializeOptions->newline.size < 0))
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

int huSerializeTroveToFileZ(huTrove const * trove, char const * path, int * destLength, huSerializeOptions * SerializeOptions)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    return huSerializeTroveToFileN(trove, path, strlen(path), destLength, SerializeOptions);
}

int huSerializeTroveToFileN(huTrove const * trove, char const * path, int pathLen, int * destLength, huSerializeOptions * SerializeOptions)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == hu_nullTrove || path == NULL)
        { return HU_ERROR_BADPARAMETER; }
    if (SerializeOptions &&
        (SerializeOptions->WhitespaceFormat < 0 || SerializeOptions->WhitespaceFormat >= 3 || 
         SerializeOptions->indentSize < 0 || 
         (SerializeOptions->usingColors && SerializeOptions->colorTable == NULL) ||
         SerializeOptions->newline.ptr == NULL || SerializeOptions->newline.size < 0))
        { return HU_ERROR_BADPARAMETER; }
#endif

    if (destLength)
        { * destLength = 0; }

    int strLength = 0;
    int error = huSerializeTrove(trove, NULL, & strLength, SerializeOptions);
    if (error != HU_ERROR_NOERROR)
        { return error; }

    char * str = malloc(strLength);
    if (str == NULL)
        { return HU_ERROR_OUTOFMEMORY; }

    error = huSerializeTrove(trove, str, & strLength, SerializeOptions);
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
