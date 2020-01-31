#pragma once

#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct huVector
  {
    void * buffer;
    int elementSize;
    int numElements;
    int elementCapacity;
  } huVector_t;

  int huGetVectorSize(huVector_t * vector);
  void * huGetElement(huVector_t * vector, int idx);

  enum huTokenKind
  {
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
    HU_ERROR_UNEXPECTED_EOF,
    HU_ERROR_SYNTAX_ERROR,
    HU_ERROR_START_END_MISMATCH
  };

  char const * huOutputErrorToString(int rhs);


  typedef struct huSubstring
  {
    int loc;
    int size;
  } huSubstring_t;


  typedef struct huStringView
  {
    char const * str;
    int size;
  } huStringView_t;


  typedef struct huToken
  {
    int tokenKind;
    huStringView_t value;
    int line;
    int col;
  } huToken_t;


  typedef struct huDictEntry
  {
    huToken_t * key;
    int idx;
  } huDictEntry_t;


  typedef struct huAnnotation
  {
    huToken_t * key;
    huToken_t * value;
  } huAnnotation_t;


  typedef struct huNode huNode_t;

  typedef struct huComment
  {
    huToken_t * commentToken;
    huNode_t * owner; // NULL = trove
  } huComment_t;


  typedef struct huTrove huTrove_t;

  typedef struct huNode
  {
    huTrove_t * trove;

    int nodeIdx;
    int kind;

    huToken_t * firstToken;
    huToken_t * keyToken;
    huToken_t * firstValueToken;
    huToken_t * lastValueToken;
    huToken_t * lastToken;

    int childIdx;   // Among its siblings, to its parent.

    int parentNodeIdx;

    huVector_t childNodeIdxs;   // int []
    huVector_t childDictKeys;   // huDictEntry_t []
    huVector_t annotations;     // huAnnotation_t []
    huVector_t comments;   // huComment_t []
  } huNode_t;

  huNode_t * huGetParentNode(huNode_t * node);
  int huGetNumChildren(huNode_t * node);
  huNode_t * huGetChildNodeByIndex(huNode_t * node, int childIdx);
  huNode_t * huGetChildNodeByKey(huNode_t * node, char const * key);

  bool huHasKey(huNode_t * node);
  huToken_t * huGetKey(huNode_t * node);

  bool huHasValue(huNode_t * node);
  huToken_t * huGetValue(huNode_t * node);

  huToken_t * huGetStartToken(huNode_t * node);
  huToken_t * huGetEndToken(huNode_t * node);

  huNode_t * huNextSibling(huNode_t * node);

  int huGetNumAnnotations(huNode_t * node);
  huAnnotation_t * huGetAnnotation(huNode_t * node, int annotationIdx);
  
  int huGetNumComments(huNode_t * node);
  huComment_t * huGetComment(huNode_t * node, int commentIdx);
  

  typedef struct huError
  {
    int errorCode;
    huToken_t * errorToken;
  } huError_t;


  enum colorKind
  {
    HU_COLORKIND_NONE = 0,
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

  typedef struct {
    int tokenKind;
    char const * colorCode;
  } huColorFormatEntry_t;


  typedef struct huTrove
  {
    int nameSize;
    char * name;
    int dataStringSize;
    char * dataString;
    huVector_t tokens;
    huVector_t nodes;
    huVector_t errors;
    int inputTabSize; // for error column #s mostly
    int outputTabSize;
    huVector_t annotations;     // huAnnotation_t []
    huVector_t comments;   // huComment_t[]
  } huTrove_t;


  huTrove_t * huMakeTroveFromString(char const * name, char const * data, int inputTabSize, int outputTabSize);

  huTrove_t * huMakeTroveFromFile(char const * name, FILE * fp, int inputTabSize, int outputTabSize);

  void huDestroyTrove(huTrove_t * trove);

  int huGetNumTokens(huTrove_t * trove);
  huToken_t * huGetToken(huTrove_t * trove, int tokenIdx);

  int huGetNumNodes(huTrove_t * trove);
  huNode_t * huGetRootNode(huTrove_t * trove);
  huNode_t * huGetNode(huTrove_t * trove, int nodeIdx);

  int huGetNumErrors(huTrove_t * trove);
  huError_t * huGetError(huTrove_t * trove, int errorIdx);

  int huGetNumTroveComments(huTrove_t * trove);
  huComment_t * huGetTroveComment(huTrove_t * trove, int errorIdx);
  
  // User must free(*serializedTrove);
  void huTroveToString(huVector_t * str, huTrove_t * trove, int outputFormat, bool excludeComments, huStringView_t * colorTable);

  int huTroveToFile(huTrove_t * trove, int outputFormat, bool includeComments, FILE * fp);


#ifdef __cplusplus
} // extern "C"
#endif
