#pragma once

#include "humon2c.h"

#ifdef __cplusplus
extern "C"
{
#endif

  void huInitVector(huVector_t * vector, int elementSize);
  void huDestroyVector(huVector_t * vector);
  void huResetVector(huVector_t * vector);
  void * huGrowVector(huVector_t * vector, int numElements);


  void huInitNode(huNode_t * node, huTrove_t * trove);
  void huResetNode(huNode_t * node);
  void huDestroyNode(huNode_t * node);

  huToken_t * allocNewToken(huTrove_t * trove, int tokenKind, char const * str, int size, int line, int col);

  huNode_t * allocNewNode(huTrove_t * trove, int nodeKind, huToken_t * firstToken);

  void huTokenizeTrove(huTrove_t * trove);
  void huParseTrove(huTrove_t * trove);
  

#ifdef __cplusplus
} // extern "C"
#endif
