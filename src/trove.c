#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


huTrove const * makeTrove(huStringView const * data, int inputTabSize)
{
    huTrove * t = malloc(sizeof(huTrove));
    if (t == NULL)
        { return & humon_nullTrove; }

    t->dataStringSize = data->size;
    // Pad by 4 nulls. This lets us look ahead three bytes for a 4-byte char match.
    char * newDataString = malloc(data->size + 4);
    if (newDataString == NULL)
    {
        free(t);
        return & humon_nullTrove;
    }
    memcpy(newDataString, data->str, data->size);
    newDataString[data->size] = '\0';
    newDataString[data->size + 1] = '\0';
    newDataString[data->size + 2] = '\0';
    newDataString[data->size + 3] = '\0';

    t->dataString = newDataString;

    huInitVector(& t->tokens, sizeof(huToken));
    huInitVector(& t->nodes, sizeof(huNode));
    huInitVector(& t->errors, sizeof(huError));

    t->inputTabSize = inputTabSize;

    huInitVector(& t->annotations, sizeof(huAnnotation));
    huInitVector(& t->comments, sizeof(huComment));

    huTokenizeTrove(t);
    huParseTrove(t);

    /*
    huStringView defaultColors[] = 
    {
        { darkGray, strlen(darkGray) },                 // NONE
        { "", 0 },                                      // END
        { lightGray, strlen(lightGray) },               // PUNCLIST
        { lightGray, strlen(lightGray) },               // PUNCDIST
        { lightGray, strlen(lightGray) },               // PUNCVALUESEP
        { darkGray, strlen(darkGray) },                 // PUNCANNOTATE
        { darkGray, strlen(darkGray) },                 // PUNCANNOTATEDICT
        { darkGray, strlen(darkGray) },                 // PUNCANNOTATEKEYVALUESEP
        { darkYellow, strlen(darkYellow) },             // KEY
        { lightYellow, strlen(lightYellow) },           // VALUE
        { darkGreen, strlen(darkGreen) },               // COMMENT
        { darkMagenta, strlen(darkMagenta) },           // ANNOKEY
        { lightMagenta, strlen(lightMagenta) },         // ANNOVALUE
        { darkGray, strlen(darkGray) },                 // WHITESPACE
    };

    huVector str;
    huInitVector(& str, sizeof(char));
    huTroveToString(& str, t, HU_OUTPUTFORMAT_MINIMAL, false, defaultColors);
    printf("Minimal:\n%s\n", (char *) str.buffer);

    huResetVector(& str);
    huTroveToString(& str, t, HU_OUTPUTFORMAT_PRESERVED, false, defaultColors);
    printf("Preserved:\n%s\n", (char *) str.buffer);

    huResetVector(& str);
    huTroveToString(& str, t, HU_OUTPUTFORMAT_PRETTY, false, defaultColors);
    printf("Pretty:\n%s\n", (char *) str.buffer);
    */
 
    return t;
}


huTrove const * huMakeTroveFromStringZ(char const * data, int inputTabSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (data == NULL)
        { return & humon_nullTrove; }
#endif

    return huMakeTroveFromStringN(data, strlen(data), inputTabSize);
}


huTrove const * huMakeTroveFromStringN(char const * data, int dataLen, int inputTabSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (data == NULL || dataLen < 0 ||
        inputTabSize < MIN_INPUT_TAB_SIZE || inputTabSize > MAX_INPUT_TAB_SIZE)
        { return & humon_nullTrove; }
#endif

    // pad by 4 nulls -- see above
    char * newData = malloc(dataLen + 4);
    if (newData == NULL)
        { return & humon_nullTrove; }

    memcpy(newData, data, dataLen);
    newData[dataLen] = '\0';
    newData[dataLen + 1] = '\0';
    newData[dataLen + 2] = '\0';
    newData[dataLen + 3] = '\0';

    huStringView dataView = { newData, dataLen };
    huTrove const * newTrove = makeTrove(& dataView, inputTabSize);
    if (newTrove == NULL)
    {
        free(newData);
        return & humon_nullTrove;
    }
    
    return newTrove;
}


huTrove const * huMakeTroveFromFileZ(char const * path, int inputTabSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return & humon_nullTrove; }
#endif

    return huMakeTroveFromFileN(path, strlen(path), inputTabSize);
}


huTrove const * huMakeTroveFromFileN(char const * path, int pathLen, int inputTabSize)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL || inputTabSize < MIN_INPUT_TAB_SIZE || inputTabSize > MAX_INPUT_TAB_SIZE)
        { return & humon_nullTrove; }
#endif

    FILE * fp = fopen(path, "r");
    if (fp == NULL)
        { return & humon_nullTrove; }

    if (fseek(fp, 0, SEEK_END) != 0)
        { return & humon_nullTrove; }

    int newDataSize = ftell(fp);
    if (newDataSize == -1L)
        { return & humon_nullTrove; }

    fseek(fp, 0, SEEK_SET);
    
    char * newData = malloc(newDataSize + 4);
    if (newData == NULL)
        { return & humon_nullTrove; }

    int freadRet = fread(newData, 1, newDataSize, fp);
    if (freadRet != newDataSize)
    {
        free(newData);
        return & humon_nullTrove;
    }
    newData[newDataSize] = '\0';
    newData[newDataSize + 1] = '\0';
    newData[newDataSize + 2] = '\0';
    newData[newDataSize + 3] = '\0';

    huStringView dataView = { newData, newDataSize };
    huTrove const * newTrove = makeTrove(& dataView, inputTabSize);
    if (newTrove == NULL)
    {
        free(newData);
        return & humon_nullTrove;
    }
    
    return newTrove;
}


void huDestroyTrove(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove)
        { return; }
#endif

    huDestroyVector(& trove->tokens);
    huDestroyVector(& trove->nodes);
    huDestroyVector(& trove->errors);
    
    if (trove->dataString != NULL)
        { free((void *) trove->dataString); }

    free((void *) trove);
}


int huGetNumTokens(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove)
        { return 0; }
#endif

    return trove->tokens.numElements;
}


huToken const * huGetToken(huTrove const * trove, int tokenIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove)
        { return & humon_nullToken; }
#endif

    if (tokenIdx < trove->tokens.numElements)
        { return (huToken *) trove->tokens.buffer + tokenIdx; }

    return & humon_nullToken;
}


huToken * allocNewToken(huTrove * trove, int tokenKind, 
    char const * str, int size, int line, int col, int endLine, int endCol)
{
    huToken * newToken = huGrowVector(& trove->tokens, 1);
    if (newToken == NULL)
        { return (huToken *) & humon_nullToken; }

    newToken->tokenKind = tokenKind;
    newToken->value.str = str;
    newToken->value.size = size;
    newToken->line = line;
    newToken->col = col;
    newToken->endLine = endLine;
    newToken->endCol = endCol;

    //printf ("%stoken: line: %d  col: %d  len: %d  %s%s\n",
    //    darkYellow, line, col, size, huTokenKindToString(tokenKind), off);
    
    return newToken;
}


int huGetNumNodes(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove)
        { return 0; }
#endif

    return trove->nodes.numElements;
}


huNode const * huGetRootNode(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove)
        { return & humon_nullNode; }
#endif

    return trove->nodes.buffer;
}


huNode const * huGetNode(huTrove const * trove, int nodeIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove)
        { return & humon_nullNode; }
#endif

    if (nodeIdx >= 0 && nodeIdx < trove->nodes.numElements)
        { return (huNode *) trove->nodes.buffer + nodeIdx; }

    return & humon_nullNode;
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
    if (trove == NULL || trove == & humon_nullTrove || errorIdx < 0)
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
    if (trove == NULL || trove == & humon_nullTrove || annotationIdx < 0)
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
    if (trove == NULL || trove == & humon_nullTrove || key  == NULL || keyLen < 0)
        { return false; }
#endif

    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->key->value.size == keyLen && 
            strncmp(anno->key->value.str, key, keyLen) == 0)
            { return true; }
    }

    return false;
}


huToken const * huGetTroveAnnotationWithKeyZ(huTrove const * trove, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key  == NULL)
        { return & humon_nullToken; }
#endif

    return huGetTroveAnnotationWithKeyN(trove, key, strlen(key));
}


huToken const * huGetTroveAnnotationWithKeyN(huTrove const * trove, char const * key, int keyLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || key  == NULL || keyLen < 0)
        { return & humon_nullToken; }
#endif

    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->key->value.size == keyLen && 
            strncmp(anno->key->value.str, key, keyLen) == 0)
            { return anno->value; }
    }

    return & humon_nullToken;
}


int huGetNumTroveAnnotationsByValueZ(huTrove const * trove, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
        { return 0; }
#endif

    return huGetNumTroveAnnotationsByValueN(trove, value, strlen(value));
}


int huGetNumTroveAnnotationsByValueN(huTrove const * trove, char const * value, int valueLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || value  == NULL || valueLen < 0)
        { return 0; }
#endif

    int matches = 0;
    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->value->value.size == valueLen && 
            strncmp(anno->value->value.str, value, valueLen) == 0)
            { matches += 1; }
    }

    return matches;
}


huToken const * huGetTroveAnnotationByValueZ(huTrove const * trove, char const * value, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (value  == NULL)
        { return & humon_nullToken; }
#endif

    return huGetTroveAnnotationByValueN(trove, value, strlen(value), annotationIdx);
}


huToken const * huGetTroveAnnotationByValueN(huTrove const * trove, char const * value, int valueLen, int annotationIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || value  == NULL || valueLen < 0 || annotationIdx < 0)
        { return & humon_nullToken; }
#endif

    int matches = 0;
    for (int i = 0; i < trove->annotations.numElements; ++i)
    { 
        huAnnotation * anno = (huAnnotation *) trove->annotations.buffer + i;
        if (anno->value->value.size == valueLen && 
            strncmp(anno->value->value.str, value, valueLen) == 0)
        {
            if (matches == annotationIdx)
                { return anno->key; }

            matches += 1;
        }
    }

    return & humon_nullToken;
}


int huGetNumTroveComments(huTrove const * trove)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove)
        { return 0; }
#endif

    return trove->comments.numElements;
}


huToken const * huGetTroveComment(huTrove const * trove, int commentIdx)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || commentIdx < 0)
        { return & humon_nullToken; }
#endif

    if (commentIdx < trove->comments.numElements)
    {
        return ((huComment *) trove->comments.buffer + commentIdx)->commentToken;
    }

    return & humon_nullToken;
}


huNode const * huGetNodeByFullAddressZ(huTrove const * trove, char const * address, int * error)
{
#ifdef HUMON_CHECK_PARAMS
    if (address == NULL)
    {
        if (error) { * error = HU_ERROR_ILLEGAL; }
        return & humon_nullNode;
    }
#endif

    return huGetNodeByFullAddressN(trove, address, strlen(address), error);
}


huNode const * huGetNodeByFullAddressN(huTrove const * trove, char const * address, int addressLen, int * error)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || address == NULL || addressLen < 0)
    {
        if (error) { * error = HU_ERROR_ILLEGAL; }
        return & humon_nullNode;
    }
#endif

    if (error) { * error = HU_ERROR_NO_ERROR; }

    // parse address; must start with '/' to start at root
    if (addressLen <= 0)
    {
        if (error) 
            { * error = HU_ERROR_UNEXPECTED_EOF; }
        return & humon_nullNode;
    }

    huCursor cur = 
        { .trove = NULL, 
          .character = address, 
          .charLength = getcharLength(address) };
    int line = 0;  // unused
    int col = 0;

    eatWs(& cur, 1, & line, & col);
    char const * wordStart = address + col;

    if (* wordStart != '/')
    {
        if (error) 
            { * error = HU_ERROR_SYNTAX_ERROR; }
        return & humon_nullNode;
    }

    huNode const * root = huGetRootNode(trove);
    if (root->kind == HU_NODEKIND_NULL)
    {
        if (error) 
            { * error = HU_ERROR_NOTFOUND; }
        return & humon_nullNode;
    }

    return huGetNodeByRelativeAddressN(root, wordStart + 1, addressLen - col - 1, error);
}


// User must free(retval.buffer);
huVector huFindNodesByAnnotationKeyZ(huTrove const * trove, char const * key)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
    {
        huVector nodesVect;
        huInitVector(& nodesVect, sizeof(huNode *));
        return nodesVect;
    }
#endif

    return huFindNodesByAnnotationKeyN(trove, key, strlen(key));
}


// User must free(retval.buffer);
huVector huFindNodesByAnnotationKeyN(huTrove const * trove, char const * key, int keyLen)
{
    // grow a vector by looking at every node annotation for match
    huVector nodesVect;
    huInitVector(& nodesVect, sizeof(huNode *));

#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || key == NULL || keyLen < 0)
        { return nodesVect; }
#endif

    for (int i = 0; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);
        bool hasOne = huHasAnnotationWithKeyN(node, key, keyLen);
        if (hasOne)
        {
            huNode const ** pn = huGrowVector(& nodesVect, 1);
            pn[0] = node;
        }
    }

    return nodesVect;
}


// User must free(retval.buffer);
huVector huFindNodesByAnnotationValueZ(huTrove const * trove, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
    {
        huVector nodesVect;
        huInitVector(& nodesVect, sizeof(huNode *));
        return nodesVect;
    }
#endif

    return huFindNodesByAnnotationValueN(trove, value, strlen(value));
}


// User must free(retval.buffer);
huVector huFindNodesByAnnotationValueN(huTrove const * trove, char const * value, int valueLen)
{
    // grow a vector by looking at every node annotation for match
    huVector nodesVect;
    huInitVector(& nodesVect, sizeof(huNode *));

#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || value == NULL || valueLen < 0)
        { return nodesVect; }
#endif

    for (int i = 0; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);
        int na = huGetNumAnnotationsByValueN(node, value, valueLen);
        if (na > 0)
        {
            huNode const ** pn = huGrowVector(& nodesVect, 1);
            pn[0] = node;
        }
    }

    return nodesVect;
}


// User must free(retval.buffer);
huVector huFindNodesByAnnotationKeyValueZZ(huTrove const * trove, char const * key, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL || value == NULL)
    {
        huVector nodesVect;
        huInitVector(& nodesVect, sizeof(huNode *));
        return nodesVect;
    }
#endif

    return huFindNodesByAnnotationKeyValueNN(trove, key, strlen(key), value, strlen(value));
}


// User must free(retval.buffer);
huVector huFindNodesByAnnotationKeyValueNZ(huTrove const * trove, char const * key, int keyLen, char const * value)
{
#ifdef HUMON_CHECK_PARAMS
    if (value == NULL)
    {
        huVector nodesVect;
        huInitVector(& nodesVect, sizeof(huNode *));
        return nodesVect;
    }
#endif

    return huFindNodesByAnnotationKeyValueNN(trove, key, keyLen, value, strlen(value));
}


// User must free(retval.buffer);
huVector huFindNodesByAnnotationKeyValueZN(huTrove const * trove, char const * key, char const * value, int valueLen)
{
#ifdef HUMON_CHECK_PARAMS
    if (key == NULL)
    {
        huVector nodesVect;
        huInitVector(& nodesVect, sizeof(huNode *));
        return nodesVect;
    }
#endif

    return huFindNodesByAnnotationKeyValueNN(trove, key, strlen(key), value, valueLen);
}


// User must free(retval.buffer);
huVector huFindNodesByAnnotationKeyValueNN(huTrove const * trove, char const * key, int keyLen, char const * value, int valueLen)
{
    // grow a vector by looking at every node annotation for match
    huVector nodesVect;
    huInitVector(& nodesVect, sizeof(huNode *));

#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || key == NULL || keyLen < 0 || value == NULL || valueLen < 0)
        { return nodesVect; }
#endif

    for (int i = 0; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);
        huToken const * anno = huGetAnnotationByKeyN(node, key, keyLen);
        if (anno->tokenKind != HU_NODEKIND_NULL)
        {
            if (anno->value.size == valueLen &&
                strncmp(anno->value.str, value, valueLen) == 0)
            {
                huNode const ** pn = huGrowVector(& nodesVect, 1);                
                * pn = node;
            }
        }
    }

    return nodesVect;
}



// User must free(retval.buffer);
huVector huFindNodesByCommentContainingZ(huTrove const * trove, char const * containedText)
{
#ifdef HUMON_CHECK_PARAMS
    if (containedText == NULL)
    {
        huVector nodesVect;
        huInitVector(& nodesVect, sizeof(huNode *));
        return nodesVect;
    }
#endif

    return huFindNodesByCommentContainingN(trove, containedText, strlen(containedText));
}


// User must free(retval.buffer);
huVector huFindNodesByCommentContainingN(huTrove const * trove, char const * containedText, int containedTextLen)
{
    // grow a vector by looking at every node annotation for match
    huVector nodesVect;
    huInitVector(& nodesVect, sizeof(huNode *));

#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || containedText == NULL || containedTextLen < 0)
        { return nodesVect; }
#endif

    for (int i = 0; i < huGetNumNodes(trove); ++i)
    {
        huNode const * node = huGetNode(trove, i);

        int na = huGetNumComments(node);
        for (int j = 0; j < na; ++j)
        {
            huToken const * comm = huGetComment(node, j);
            if (stringInString(comm->value.str, comm->value.size, containedText, containedTextLen))
            {
                huNode const ** pn = huGrowVector(& nodesVect, 1);                
                * pn = node;
                break;  // just one comment per node, so we don't return multiple refs to the same node
            }
        }
    }

    return nodesVect;
}


huNode * allocNewNode(huTrove * trove, int nodeKind, huToken * firstToken)
{
    huNode * newNode = huGrowVector(& trove->nodes, 1);
    if (newNode == NULL)
        { return (huNode *) & humon_nullNode; }

    huInitNode(newNode, trove);
    int newNodeIdx = newNode - (huNode *) trove->nodes.buffer;
    newNode->nodeIdx = newNodeIdx;
    newNode->kind = nodeKind;
    newNode->firstToken = firstToken;
    newNode->lastToken = firstToken;

    // printf ("%snode: nodeIdx: %d    firstToken: %d    %s%s\n",
    //    lightCyan, newNodeIdx, (int)(firstToken - (huToken *) trove->tokens.buffer), 
    //    huNodeKindToString(nodeKind), off);

    return newNode;
}


void recordError(huTrove * trove, int errorCode, huToken const * pCur)
{
    //fprintf (stderr, "%sError%s: line: %d    col: %d    %s\n", lightRed, off, 
    //    pCur->line, pCur->col, huOutputErrorToString(errorCode));
    huError * error = huGrowVector(& trove->errors, 1);
    error->errorCode = errorCode;
    error->errorToken = pCur;
}


void appendString(huVector * str, char const * addend, int size)
{
    char * dest = huGrowVector(str, size);
    memcpy(dest, addend, size);
}


void appendWs(huVector * str, int numChars)
{
    char const spaces[] = "                                "; // 16 spaces
    while (numChars > 16)
    {
        appendString(str, spaces, 16);
        numChars -= 16;
    }
    appendString(str, spaces, numChars);
}


void appendColor(huVector * str, huStringView const * colorTable, int colorCode)
{
    if (colorTable == NULL)
        { return; }
    huStringView const * color = colorTable + colorCode;
    appendString(str, color->str, color->size);
}


void endColor(huVector * str, huStringView const * colorTable)
{
    appendColor(str, colorTable, HU_COLORKIND_END);
}


void appendColoredString(huVector * str, char const * addend, int size, huStringView const * colorTable, int colorCode)
{
    appendColor(str, colorTable, colorCode);
    appendString(str, addend, size);
    appendColor(str, colorTable, HU_COLORKIND_END);
}


void printComment(huToken const * comment, huVector * str, huStringView const * colorTable)
{
    huStringView const * comstr = & comment->value;
    appendColoredString(str, comstr->str, comstr->size, 
        colorTable, HU_COLORKIND_COMMENT);
}


int printSameLineComments(huNode const * node, bool firstToken, int startingCommentIdx, huVector * str, huStringView const* colorTable)
{
    int iCom = startingCommentIdx;
    for (; iCom < huGetNumComments(node); ++iCom)
    {
        huToken const * comment = huGetComment(node, iCom);
        // If not firstToken, forego the position check. Handles enqueued comments before the end token.
        if (firstToken == false ||
                (comment->line == node->firstToken->line &&
                 comment->col < node->lastValueToken->col))
        {
            appendWs(str, 1);
            printComment(comment, str, colorTable);
        }
        else
            { break; }
    }

    return iCom;
}


void printAnnotations(huAnnotation const * annos, int numAnno, bool troveOwned, huVector * str, huStringView const * colorTable)
{
    if (numAnno == 0)
        { return; }
    
    if (troveOwned == false)
        { appendString(str, " ", 1); }

    appendColoredString(str, "@", 1, 
        colorTable, HU_COLORKIND_PUNCANNOTATE);

    if (numAnno > 1)
    {
        appendColoredString(str, "{", 1, 
            colorTable, HU_COLORKIND_PUNCANNOTATEDICT);
    }

    for (int iAnno = 0; iAnno < numAnno; ++iAnno)
    {
        huAnnotation const * anno = annos + iAnno;
        if (iAnno > 0)
            { appendWs(str, 1); }

        appendColoredString(str, anno->key->value.str, anno->key->value.size,
            colorTable, HU_COLORKIND_ANNOKEY);
        appendColoredString(str, ": ", 2,
            colorTable, HU_COLORKIND_PUNCANNOTATEKEYVALUESEP);
        appendColoredString(str, anno->value->value.str, anno->value->value.size,
            colorTable, HU_COLORKIND_ANNOVALUE);
    }

    if (numAnno > 1)
    {
        appendColoredString(str, "}", 1,
            colorTable, HU_COLORKIND_PUNCANNOTATEDICT);
    }
}


void printTroveAnnotations(huTrove const * trove, huVector * str, huStringView const * colorTable)
{
    if (huGetNumTroveAnnotations(trove) == 0)
        { return; }

    huAnnotation const * annos = huGetTroveAnnotation(trove, 0);
    printAnnotations(annos, huGetNumTroveAnnotations(trove), true, str, colorTable);
}


void printNodeAnnotations(huNode const * node, huVector * str, huStringView const * colorTable)
{
    if (huGetNumAnnotations(node) == 0)
        { return; }

    huAnnotation const * annos = huGetAnnotation(node, 0);
    printAnnotations(annos, huGetNumAnnotations(node), false, str, colorTable);
}


void printKey(huToken const * keyToken, huVector * str, huStringView const * colorTable)
{
    appendColoredString(str, keyToken->value.str, keyToken->value.size,
        colorTable, HU_COLORKIND_KEY);
    appendColoredString(str, ": ", 2,
        colorTable, HU_COLORKIND_PUNCKEYVALUESEP);
}


void printValue(huToken const * valueToken, huVector * str, huStringView const * colorTable)
{
    appendColoredString(str, valueToken->value.str, valueToken->value.size,
        colorTable, HU_COLORKIND_VALUE);
}


void troveToPrettyStringRec(huNode const * node, huVector * str, int depth, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable)
{
    bool lineIsDirty = false;

    // print preceeding comments
    int iCom = 0;
    for (; iCom < huGetNumComments(node); ++iCom)
    {
        huToken const * comment = huGetComment(node, iCom);
        if (comment->line < node->firstToken->line)
        {
            if (node->parentNodeIdx != -1)
                { appendString(str, "\n", 1); }
            appendWs(str, outputTabSize * depth);
            printComment(comment, str, colorTable);
            lineIsDirty = true;
        }
        else
            { break; }
    }
    
    huToken const * keyToken = huGetKey(node);

    // if node has a key, print key:
    if (keyToken != NULL)
    {
        appendString(str, "\n", 1);
        appendWs(str, outputTabSize * depth);

        printKey(keyToken, str, colorTable);
        lineIsDirty = true;
    }

    // print [ or {
    if (node->kind == HU_NODEKIND_LIST)
    {
        if (node->keyToken == NULL)
        {
            if (node->childIdx == -1)
            {
                if (lineIsDirty)
                    { appendString(str, "\n", 1); }
                else
                {
                    if (node->nodeIdx != 0)
                        { appendWs(str, 1); }
                }
            }
            else
            {
                if (lineIsDirty)
                    { appendString(str, "\n", 1); }
                else
                    { appendWs(str, 1); }
            }
        }
        appendColoredString(str, "[", 1, 
            colorTable, HU_COLORKIND_PUNCLIST);
    }
    else if (node->kind == HU_NODEKIND_DICT)
    {
        if (node->keyToken == NULL)
        {
            if (node->childIdx == -1)
            {
                if (lineIsDirty)
                    { appendString(str, "\n", 1); }
                else
                {
                    if (node->nodeIdx != 0)
                        { appendWs(str, 1); }
                }
            }
            else
            {
                if (lineIsDirty)
                    { appendString(str, "\n", 1); }
                else
                    { appendWs(str, 1); }
            }
        }
        appendColoredString(str, "{", 1, 
            colorTable, HU_COLORKIND_PUNCDICT);
    }

    if (node->kind == HU_NODEKIND_LIST ||
            node->kind == HU_NODEKIND_DICT)
    {
        // print annotations on one line
        printNodeAnnotations(node, str, colorTable);

        // print any same-line comments
        iCom = printSameLineComments(node, true, iCom, str, colorTable);

        // recursive calls
        int numCh = huGetNumChildren(node);
        for (int i = 0; i < numCh; ++i)
        {
            huNode const * chNode = huGetChildNodeByIndex(node, i);
            troveToPrettyStringRec(chNode, str, depth + 1, outputFormat, excludeComments, outputTabSize, colorTable);
        }

        // print ]
        if (node->kind == HU_NODEKIND_LIST)
        {
            appendString(str, "\n", 1);
            appendWs(str, outputTabSize * depth);
            appendColoredString(str, "]", 1, 
                colorTable, HU_COLORKIND_PUNCLIST);
        }
        else
        {
            appendString(str, "\n", 1);
            appendWs(str, outputTabSize * depth);
            appendColoredString(str, "}", 1, 
                colorTable, HU_COLORKIND_PUNCLIST);
        }
    }
    else if (node->kind == HU_NODEKIND_VALUE)
    {
        if (node->keyToken == NULL && 
                 (node->parentNodeIdx != -1 ||
                    node->firstToken != node->firstValueToken))
        {
            appendString(str, "\n", 1);
            appendWs(str, outputTabSize * depth);
        }
        printValue(node->firstValueToken, str, colorTable);
        printNodeAnnotations(node, str, colorTable);
    }

    // print post-object comments on same line
    printSameLineComments(node, false, iCom, str, colorTable);
}


void troveToPrettyString(huTrove const * trove, huVector * str, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable)
{
    printTroveAnnotations(trove, str, colorTable);

    huNode const * nodes = (huNode const *) trove->nodes.buffer;

    if (nodes != NULL)
    {
        troveToPrettyStringRec(& nodes[0], str, 0, outputFormat, excludeComments, outputTabSize, colorTable);
        appendString(str, "\n", 1);
    }

    for (int iCom = 0; iCom < huGetNumTroveComments(trove); ++iCom)
    {
        huToken const * comment = huGetTroveComment(trove, iCom);
        printComment(comment, str, colorTable);
        appendString(str, "\n", 1);
    }

    appendString(str, "\0", 1);
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"

// User must free(retval.str);
huStringView huTroveToString(huTrove const * trove, 
    int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || outputFormat < 0 || outputFormat >= 3 || outputTabSize < 0)
    {
        huStringView sv = { .str = NULL, .size = 0 };
        return sv;
    }
#endif

    huVector str;
    huInitVector(& str, sizeof(char));

    if (outputFormat == HU_OUTPUTFORMAT_PRESERVED &&
        excludeComments == false && colorTable == NULL)
    {
        // raw output is raw
        char * newData = malloc(trove->dataStringSize + 1);
        strncpy(newData, trove->dataString, trove->dataStringSize + 1);
    }
    else if (outputFormat == HU_OUTPUTFORMAT_PRESERVED ||
                     outputFormat == HU_OUTPUTFORMAT_MINIMAL)
    {
        int line = 1, col = 1;
        int lastColorTableIdx = HU_COLORKIND_NONE;
        bool inDict = false;
        bool inAnno = false;
        bool expectKey = false;
        bool isKey = false;

        huToken const * tokens = (huToken const *) trove->tokens.buffer;
        huToken const * tok = NULL;
        huToken const * lastNonCommentToken = NULL;
        for (int i = 0; i < huGetNumTokens(trove); ++i)
        {
            huToken const * prevTok = tok;
            tok = tokens + i;
            if (tok->tokenKind != HU_TOKENKIND_COMMENT)
                { lastNonCommentToken = tok; }

#pragma region // Determine isDict, isAnno, expectKey, isKey.
            if (tok->tokenKind == HU_TOKENKIND_EOF)
                { break; }
            if (tok->tokenKind == HU_TOKENKIND_COMMENT && excludeComments)
                { continue; }
            if (tok->tokenKind == HU_TOKENKIND_STARTDICT)
                { inDict = true; expectKey = true; }
            else if (tok->tokenKind == HU_TOKENKIND_ENDDICT ||
                                tok->tokenKind == HU_TOKENKIND_STARTLIST)
            { 
                inDict = false;
                expectKey = false;
                inAnno = false;
            }
            else if (tok->tokenKind == HU_TOKENKIND_ANNOTATE)
                { inAnno = true; expectKey = true; }
            else if (tok->tokenKind == HU_TOKENKIND_WORD)
            {
                if (expectKey)
                    { isKey = true; expectKey = false; }
                else
                {
                    isKey = false;
                    if (inDict)
                        { expectKey = true; }
                    if (inAnno && ! inDict)
                        { inAnno = false; }
                }
            }
#pragma endregion

#pragma region // The space between tokens is filled with whitespace, according to the outputFormat.
            if (outputFormat == HU_OUTPUTFORMAT_PRESERVED)
            {
                while (line < tok->line)
                {
                    appendString(& str, "\n", 1);
                    line += 1;
                    col = 1;
                }
                while (col < tok->col)
                {
                    appendString(& str, " ", 1);
                    col += 1;
                }
            }
            else if (outputFormat == HU_OUTPUTFORMAT_MINIMAL)
            {
                if (prevTok != NULL)
                {
                    if (lastNonCommentToken != NULL &&
                            tok->line != lastNonCommentToken->line)
                    {
                        appendString(& str, "\n", 1);
                        line += 1;
                        col = 1;
                    }
                    else if (prevTok->tokenKind == HU_TOKENKIND_COMMENT && 
                            prevTok->value.str[0] == '/' && 
                            prevTok->value.str[1] == '/')
                    {
                        appendString(& str, "\n", 1);
                        line += 1;
                        col = 1;
                    }
                    else if (prevTok->tokenKind == HU_TOKENKIND_WORD &&
                                    tok->tokenKind == HU_TOKENKIND_WORD)
                    {
                        appendString(& str, " ", 1);
                        col += 1;
                    }
                }
            }
#pragma endregion

#pragma region // Colorize the output.
            if (colorTable != NULL)
            {
                int colorTableIdx = HU_COLORKIND_NONE;
                if (inAnno)
                {
                    if (tok->tokenKind == HU_TOKENKIND_WORD)
                    {
                        if (isKey)
                            { colorTableIdx = HU_COLORKIND_ANNOKEY; }
                        else
                            { colorTableIdx = HU_COLORKIND_ANNOVALUE; }
                    }
                    else if (tok->tokenKind == HU_TOKENKIND_STARTDICT ||
                                        tok->tokenKind == HU_TOKENKIND_ENDDICT)
                        { colorTableIdx = HU_COLORKIND_PUNCANNOTATEDICT; }
                    else if (tok->tokenKind == HU_TOKENKIND_KEYVALUESEP)
                        { colorTableIdx = HU_COLORKIND_PUNCANNOTATEKEYVALUESEP; }
                    else if (tok->tokenKind == HU_TOKENKIND_ANNOTATE)
                        { colorTableIdx = HU_COLORKIND_PUNCANNOTATE; }
                }
                else
                {
                    if (tok->tokenKind == HU_TOKENKIND_WORD)
                    {
                        if (isKey)
                            { colorTableIdx = HU_COLORKIND_KEY; }
                        else
                            { colorTableIdx = HU_COLORKIND_VALUE; }
                    }
                    else if (tok->tokenKind == HU_TOKENKIND_STARTLIST ||
                                        tok->tokenKind == HU_TOKENKIND_ENDLIST)
                        { colorTableIdx = HU_COLORKIND_PUNCLIST; }
                    else if (tok->tokenKind == HU_TOKENKIND_STARTDICT ||
                                        tok->tokenKind == HU_TOKENKIND_ENDDICT)
                        { colorTableIdx = HU_COLORKIND_PUNCDICT; }
                    else if (tok->tokenKind == HU_TOKENKIND_KEYVALUESEP)
                        { colorTableIdx = HU_COLORKIND_PUNCKEYVALUESEP; }
                }
                if (tok->tokenKind == HU_TOKENKIND_COMMENT)
                    { colorTableIdx = HU_COLORKIND_COMMENT; }
                
                if (colorTableIdx != lastColorTableIdx)
                {
                    huStringView const * ce = colorTable + colorTableIdx;
                    appendString(& str, ce->str, ce->size);
                    lastColorTableIdx = colorTableIdx;
                }
            }
#pragma endregion

            appendString(& str, tok->value.str, tok->value.size);

            int lineDelta = tok->endLine - tok->line;
            line += lineDelta;
            if (lineDelta > 0)
                { col = tok->endCol; }
            else
                { col += tok->endCol - tok->col; }
        }
        if (colorTable != NULL)
        {
            huStringView const * ce = colorTable + HU_COLORKIND_END;
            appendString(& str, ce->str, ce->size);
        }
        appendString(& str, "\n\0", 2);
    }
    else if (outputFormat == HU_OUTPUTFORMAT_PRETTY)
    {
        troveToPrettyString(trove, & str, outputFormat, excludeComments, outputTabSize, colorTable);
    }

    huStringView sv = { .str = str.buffer, .size = str.numElements };
    return sv;
}

#pragma GCC diagnostic pop

size_t huTroveToFileZ(huTrove const * trove, char const * path, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable)
{
#ifdef HUMON_CHECK_PARAMS
    if (path == NULL)
        { return 0; }
#endif

    return huTroveToFileN(trove, path, strlen(path), outputFormat, excludeComments, outputTabSize, colorTable);
}

size_t huTroveToFileN(huTrove const * trove, char const * path, int pathLen, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable)
{
#ifdef HUMON_CHECK_PARAMS
    if (trove == NULL || trove == & humon_nullTrove || path == NULL || outputFormat < 0 || outputFormat >= 3 || outputTabSize < 0)
        { return 0; }
#endif

    FILE * fp = fopen(path, "w");
    if (fp == NULL)
        { return 0; }

    huStringView str = huTroveToString(trove, outputFormat, excludeComments, outputTabSize, colorTable);
    size_t ret = fwrite(str.str, sizeof(char), str.size, fp);

    free((char *) str.str);
    return ret;
}
