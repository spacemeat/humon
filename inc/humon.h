#pragma once

/*
    TODO:
        P1 CMake, vcpkg
        P1 doxygen
        P2 custom memory allocator fns
        P2 insertion ops (later priority)
        P2 string value transformers (getDecodedValue())
        P2 exceptional C++ version?
        P2 hashed dictionary access in C?
*/

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct huVector_tag
    {
        void * buffer;
        int elementSize;
        int numElements;
        int elementCapacity;
    } huVector;

    int huGetVectorSize(huVector * vector);
    void * huGetVectorElement(huVector * vector, int idx);

    enum huTokenKind
    {
        HU_TOKENKIND_NULL,
        HU_TOKENKIND_EOF,
        HU_TOKENKIND_STARTLIST,
        HU_TOKENKIND_ENDLIST,
        HU_TOKENKIND_STARTDICT,
        HU_TOKENKIND_ENDDICT,
        HU_TOKENKIND_KEYVALUESEP,
        HU_TOKENKIND_ANNOTATE,
        HU_TOKENKIND_WORD,
        HU_TOKENKIND_COMMENT
    };

    char const * huTokenKindToString(int rhs);


    enum huNodeKind
    {
        HU_NODEKIND_NULL,
        HU_NODEKIND_LIST,
        HU_NODEKIND_DICT,
        HU_NODEKIND_VALUE
    };

    char const * huNodeKindToString(int rhs);


    enum huOutputFormat
    {
        HU_OUTPUTFORMAT_PRESERVED,
        HU_OUTPUTFORMAT_MINIMAL,
        HU_OUTPUTFORMAT_PRETTY
    };

    char const * huOutputFormatToString(int rhs);


    enum huErrorCode
    {
        HU_ERROR_NO_ERROR,
        HU_ERROR_UNEXPECTED_EOF,
        HU_ERROR_UNFINISHED_QUOTE,
        HU_ERROR_UNFINISHED_CSTYLECOMMENT,
        HU_ERROR_SYNTAX_ERROR,
        HU_ERROR_START_END_MISMATCH,
        HU_ERROR_NOTFOUND
    };

    char const * huOutputErrorToString(int rhs);


    typedef struct huStringView_tag
    {
        char const * str;
        int size;
    } huStringView;


    typedef struct huToken_tag
    {
        int tokenKind;
        huStringView value;
        int line;
        int col;
        int endLine;
        int endCol;
    } huToken;


    typedef struct huDictEntry_tag
    {
        huToken * key;
        int idx;
    } huDictEntry;


    typedef struct huAnnotation_tag
    {
        huToken * key;
        huToken * value;
    } huAnnotation;


    typedef struct huNode_tag huNode;

    typedef struct huComment_tag
    {
        huToken * commentToken;
        huNode * owner; // NULL = trove
    } huComment;


    typedef struct huTrove_tag huTrove;

    typedef struct huNode_tag
    {
        huTrove * trove;

        int nodeIdx;
        int kind;

        huToken * firstToken;
        huToken * keyToken;
        huToken * firstValueToken;
        huToken * lastValueToken;
        huToken * lastToken;

    // TODO: change following to childOrdinal
        int childIdx;     // Among its siblings, to its parent.

        int parentNodeIdx;

        huVector childNodeIdxs;     // int []
        huVector childDictKeys;     // huDictEntry []
        huVector annotations;         // huAnnotation []
        huVector comments;     // huComment []
    } huNode;

    huNode * huGetParentNode(huNode * node);
    int huGetNumChildren(huNode * node);
    huNode * huGetChildNodeByIndex(huNode * node, int childIdx);
    huNode * huGetChildNodeByKeyZ(huNode * node, char const * key);
    huNode * huGetChildNodeByKeyN(huNode * node, char const * key, int keyLen);

    bool huHasKey(huNode * node);
    huToken * huGetKey(huNode * node);

    bool huHasValue(huNode * node);
    huToken * huGetValue(huNode * node);

    huToken * huGetStartToken(huNode * node);
    huToken * huGetEndToken(huNode * node);

    huNode * huNextSibling(huNode * node);

    int huGetNumAnnotations(huNode * node);
    huAnnotation * huGetAnnotation(huNode * node, int annotationIdx);

    int huGetNumAnnotationsByKeyZ(huNode * node, char const * key);
    int huGetNumAnnotationsByKeyN(huNode * node, char const * key, int keyLen);
    huAnnotation * huGetAnnotationByKeyZ(huNode * node, char const * key, int annotationIdx);
    huAnnotation * huGetAnnotationByKeyN(huNode * node, char const * key, int keyLen, int annotationIdx);

    int huGetNumAnnotationsByValueZ(huNode * node, char const * value);
    int huGetNumAnnotationsByValueN(huNode * node, char const * value, int valueLen);
    huAnnotation * huGetAnnotationByValueZ(huNode * node, char const * value, int annotationIdx);
    huAnnotation * huGetAnnotationByValueN(huNode * node, char const * value, int valueLen, int annotationIdx);

    int huGetNumComments(huNode * node);
    huComment * huGetComment(huNode * node, int commentIdx);
    
    huNode * huGetNodeByRelativeAddressZ(huNode * node, char const * address, int * error);
    huNode * huGetNodeByRelativeAddressN(huNode * node, char const * address, int addressLen, int * error);

    // user must free(retval.str)
    huStringView huGetNodeAddress(huNode * node);

    typedef struct huError_tag
    {
        int errorCode;
        huToken * errorToken;
    } huError;


    enum huColorKind
    {
        HU_COLORKIND_NONE = 0,
        HU_COLORKIND_END,
        HU_COLORKIND_PUNCLIST,
        HU_COLORKIND_PUNCDICT,
        HU_COLORKIND_PUNCKEYVALUESEP,
        HU_COLORKIND_PUNCANNOTATE,
        HU_COLORKIND_PUNCANNOTATEDICT,
        HU_COLORKIND_PUNCANNOTATEKEYVALUESEP,
        HU_COLORKIND_KEY,
        HU_COLORKIND_VALUE,
        HU_COLORKIND_COMMENT,
        HU_COLORKIND_ANNOKEY,
        HU_COLORKIND_ANNOVALUE,
        HU_COLORKIND_WHITESPACE,

        HU_COLORKIND_NUMCOLORKINDS
    };


    typedef struct huTrove_tag
    {
        int nameSize;
        char * name;
        int dataStringSize;
        char * dataString;
        huVector tokens;
        huVector nodes;
        huVector errors;
        int inputTabSize; // for error column #s mostly
        int outputTabSize;
        huVector annotations;         // huAnnotation []
        huVector comments;     // huComment[]
    } huTrove;


    huTrove * huMakeTroveFromStringZ(char const * name, char const * data, int inputTabSize, int outputTabSize);
    huTrove * huMakeTroveFromStringN(char const * name, char const * data, int dataLen, int inputTabSize, int outputTabSize);
    huTrove * huMakeTroveFromFile(char const * name, char const * path, int inputTabSize, int outputTabSize);

    void huDestroyTrove(huTrove * trove);

    int huGetNumTokens(huTrove * trove);
    huToken * huGetToken(huTrove * trove, int tokenIdx);

    int huGetNumNodes(huTrove * trove);
    huNode * huGetRootNode(huTrove * trove);
    huNode * huGetNode(huTrove * trove, int nodeIdx);

    int huGetNumErrors(huTrove * trove);
    huError * huGetError(huTrove * trove, int errorIdx);

    int huGetNumTroveAnnotations(huTrove * trove);
    huAnnotation * huGetTroveAnnotation(huTrove * trove, int errorIdx);

    int huGetNumTroveAnnotationsByKeyZ(huTrove * trove, char const * key);
    int huGetNumTroveAnnotationsByKeyN(huTrove * trove, char const * key, int keyLen);
    huAnnotation * huGetTroveAnnotationByKeyZ(huTrove * trove, char const * key, int annotationIdx);
    huAnnotation * huGetTroveAnnotationByKeyN(huTrove * trove, char const * key, int keyLen, int annotationIdx);

    int huGetNumTroveAnnotationsByValueZ(huTrove * trove, char const * value);
    int huGetNumTroveAnnotationsByValueN(huTrove * trove, char const * value, int valueLen);
    huAnnotation * huGetTroveAnnotationByValueZ(huTrove * trove, char const * value, int annotationIdx);
    huAnnotation * huGetTroveAnnotationByValueN(huTrove * trove, char const * value, int valueLen, int annotationIdx);

    int huGetNumTroveComments(huTrove * trove);
    huComment * huGetTroveComment(huTrove * trove, int errorIdx);

    huNode * huGetNodeByFullAddressZ(huTrove * trove, char const * address, int * error);
    huNode * huGetNodeByFullAddressN(huTrove * trove, char const * address, int addressLen, int * error);
    
    // User must free(retval.buffer);
    huVector huFindNodesByAnnotationKeyZ(huTrove * trove, char const * key);
    huVector huFindNodesByAnnotationKeyN(huTrove * trove, char const * key, int keyLen);
    huVector huFindNodesByAnnotationValueZ(huTrove * trove, char const * value);
    huVector huFindNodesByAnnotationValueN(huTrove * trove, char const * value, int valueLen);
    huVector huFindNodesByAnnotationKeyValueZZ(huTrove * trove, char const * key, char const * value);
    huVector huFindNodesByAnnotationKeyValueNZ(huTrove * trove, char const * key, int keyLen, char const * value);
    huVector huFindNodesByAnnotationKeyValueZN(huTrove * trove, char const * key, char const * value, int valueLen);
    huVector huFindNodesByAnnotationKeyValueNN(huTrove * trove, char const * key, int keyLen, char const * value, int valueLen);

    // User must free(retval.buffer);
    huVector huFindNodesByCommentZ(huTrove * trove, char const * text);
    huVector huFindNodesByCommentN(huTrove * trove, char const * text, int textLen);    

    // User must free(retval.str);
    huStringView huTroveToString(huTrove * trove, int outputFormat, bool excludeComments, huStringView * colorTable);

    size_t huTroveToFile(char const * path, huTrove * trove, int outputFormat, bool excludeComments, huStringView * colorTable);


    // Globals that represent error objects. All calls on these objects are legal and won't raise signals.
    extern huToken humon_nullToken;
    extern huNode humon_nullNode;
    extern huTrove humon_nullTrove;

#ifdef __cplusplus
} // extern "C"
#endif
