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
    int size;
    int capacity;
  } huVector_t;

  void huInitVector(huVector_t * vector);

  void huDestroyVector(huVector_t * vector);

  void huResetVector(huVector_t * vector);

  void huGrowVector(huVector_t * vector, int numBytes);


  enum huTokenKind
  {
    HU_TOKENKIND_EOF,
    HU_TOKENKIND_STARTDICT,
    HU_TOKENKIND_ENDDICT,
    HU_TOKENKIND_STARTLIST,
    HU_TOKENKIND_ENDLIST,
    HU_TOKENKIND_KEYVALUESEP,
    HU_TOKENKIND_ANNOTATE,
    HU_TOKENKIND_WORD,
    HU_TOKENKIND_COMMENT
  };

  char const * huTokenKindToString(int rhs)
  {
    switch(rhs)
    {
    case HU_TOKENKIND_EOF: return "eof";
    case HU_TOKENKIND_STARTDICT: return "startDict";
    case HU_TOKENKIND_ENDDICT: return "endDict";
    case HU_TOKENKIND_STARTLIST: return "startList";
    case HU_TOKENKIND_ENDLIST: return "endList";
    case HU_TOKENKIND_KEYVALUESEP: return "keyValueSep";
    case HU_TOKENKIND_ANNOTATE: return "annotate";
    case HU_TOKENKIND_WORD: return "word";
    case HU_TOKENKIND_COMMENT: return "comment";
    default: return "!!unknown!!";
    }
  }


  enum huNodeKind
  {
    HU_NODEKIND_NULL,
    HU_NODEKIND_ERROR,
    HU_NODEKIND_LIST,
    HU_NODEKIND_DICT,
    HU_NODEKIND_VALUE,
    HU_NODEKIND_COMMENT
  };

  char const * huNodeKindToString(int rhs)
  {
    switch(rhs)
    {
    case HU_NODEKIND_NULL: return "null";
    case HU_NODEKIND_ERROR: return "error";
    case HU_NODEKIND_LIST: return "list";
    case HU_NODEKIND_DICT: return "dict";
    case HU_NODEKIND_VALUE: return "value";
    case HU_NODEKIND_COMMENT: return "comment";
    default: return "!!unknown!!";
    }
  }


  enum huOutputFormat
  {
    HU_OUTPUTFORMAT_PRESERVED,
    HU_OUTPUTFORMAT_MINIMAL,
    HU_OUTPUTFORMAT_PRETTY
  };

  char const * huOutputFormatToString(int rhs)
  {
    switch(rhs)
    {
    case HU_OUTPUTFORMAT_PRESERVED: return "preserved";
    case HU_OUTPUTFORMAT_MINIMAL: return "minimal";
    case HU_OUTPUTFORMAT_PRETTY: return "pretty";
    default: return "!!unknown!!";
    }
  }


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
    huStringView_t key;
    int idx;
  } huDictEntry_t;


  typedef struct huAnnotationEntry
  {
    int keyTokenIdx;
    int valueTokenIdx;
  } huAnnotationEntry_t;


  typedef struct huTrove huTrove_t;

  typedef struct huNode
  {
    huTrove_t * trove;

    int nodeIdx;
    int kind;

    int keyTokenIdx;

    int firstTokenIdx;
    int numTokens;

    int valueTokenIdx;

    int childIdx;
    int parentNodeIdx;
    int childAppendTokenIdx;
    int annotationAppendTokenIdx;

    int numChildren;
    int * childNodeIdxs;
    huDictEntry_t * childDictKeys;

    int numAnnotations;
    huAnnotationEntry_t * annotations;

    int numComments;
    huSubstring_t * comments;
  } huNode_t;

  void huInitNode(huNode_t * node);
  void huResetNode(huNode_t * node);
  void huDestroyNode(huNode_t * node);

  huNode_t * huGetParentNode(huNode_t * node);
  huNode_t * huGetChildNodeByIndex(huNode_t * node, int childIdx);
  huNode_t * huGetChildNodeByKey(huNode_t * node, char const * key);

  bool huHasKey(huNode_t * node);
  huToken_t * huGetKey(huNode_t * node);

  bool huHasValue(huNode_t * node);
  huToken_t * huGetValue(huNode_t * node);
  huNode_t * huNextSibling(huNode_t * node);

  typedef struct huError
  {
    char const * msg;
    int line;
    int col;
  } huError_t;


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
  } huTrove_t;


  int huMakeTroveFromString(char const * name, char const * data, int inputTabSize, int outputTabSize, huTrove_t ** dst);
  int huMakeTroveFromFile(char const * name, FILE * fp, int inputTabSize, int outputTabSize, huTrove_t ** dst);

  void huDestroyTrove(huTrove_t * trove);

  huToken_t * huGetToken(huTrove_t * trove, int tokenIdx);
  huToken_t * huAllocNewToken(huTrove_t * trove, int tokenKind, char const * str, int size, int line, int col);

  huNode_t * huGetRootNode(huTrove_t * trove);
  huNode_t * huGetNode(huTrove_t * trove, int nodeIdx);
  huNode_t * huAllocNewNode(huTrove_t * trove);
  
  // User must free(*serializedTrove);
  int huTroveToString(huTrove_t * trove, int outputFormat, bool includeComments, char ** serializedTrove);
  int huTroveToFile(huTrove_t * trove, int outputFormat, bool includeComments, FILE * fp);

  void huTokenizeTrove(huTrove_t * trove);
  void huParseTrove(huTrove_t * trove);
  


#ifdef __cplusplus
} // extern "C"
#endif
