#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


huTrove_t * huMakeTroveFromString(char const * name, char const * data, int inputTabSize, int outputTabSize)
{
  huTrove_t * t = malloc(sizeof(huTrove_t));
  if (t == NULL)
    { return NULL; }

  t->nameSize = strlen(name);
  t->name = malloc(t->nameSize);
  if (t->name == NULL)
  {
    free(t);
    return NULL;
  }
  strncpy(t->name, name, t->nameSize);

  t->dataStringSize = strlen(data);
  t->dataString = malloc(t->dataStringSize);
  if (t->dataString == NULL)
  {
    free(t->name);
    free(t);
    return NULL;
  }
  strncpy(t->dataString, data, t->dataStringSize);

  huInitVector(& t->tokens, sizeof(huToken_t));
  huInitVector(& t->nodes, sizeof(huNode_t));
  huInitVector(& t->errors, sizeof(huError_t));

  t->inputTabSize = inputTabSize;
  t->outputTabSize = outputTabSize;

  huTokenizeTrove(t);
  huParseTrove(t);

  return t;
}


huTrove_t * huMakeTroveFromFile(char const * name, FILE * fp, int inputTabSize, int outputTabSize)
{
   huTrove_t * t = malloc(sizeof(huTrove_t));
  if (t == NULL)
    { return NULL; }

  t->nameSize = strlen(name);
  t->name = malloc(t->nameSize);
  if (t->name == NULL)
  {
    free(t);
    return NULL;
  }
  strncpy(t->name, name, t->nameSize);

  if (fseek(fp, 0, SEEK_END) != 0)
  {
    free(t->name);
    free(t);
    return NULL;
  }

  t->dataStringSize = ftell(fp);
  if (t->dataStringSize == -1L)
  {
    free(t->name);
    free(t);
    return NULL;
  }

  t->dataString = malloc(t->dataStringSize);
  if (t->dataString == NULL)
  {
    free(t->name);
    free(t);
    return NULL;
  }

  fseek(fp, 0, SEEK_SET);
  if (fread(t->dataString, t->dataStringSize, 1, fp) != 
    t->dataStringSize)
  {
    free(t->name);
    free(t->dataString);
    free(t);
    return NULL;
  }

  huInitVector(& t->tokens, sizeof(huToken_t));
  huInitVector(& t->nodes, sizeof(huNode_t));
  huInitVector(& t->errors, sizeof(huError_t));

  t->inputTabSize = inputTabSize;
  t->outputTabSize = outputTabSize;

  huTokenizeTrove(t);
  huParseTrove(t);

  return t;
}


void huDestroyTrove(huTrove_t * trove)
{
  huDestroyVector(& trove->tokens);
  huDestroyVector(& trove->nodes);
  huDestroyVector(& trove->errors);
  
  if (trove->dataString != NULL)
    { free(trove->dataString); }

  if (trove->name != NULL)
    { free(trove->name); }

  free(trove);
}


int huGetNumTokens(huTrove_t * trove)
{
  return trove->tokens.numElements;
}


huToken_t * huGetToken(huTrove_t * trove, int tokenIdx)
{
  if (tokenIdx < trove->tokens.numElements)
    { return (huToken_t *) trove->tokens.buffer + tokenIdx; }

  return NULL;
}


huToken_t * huAllocNewToken(huTrove_t * trove, int tokenKind, 
  char const * str, int size, int line, int col)
{
  huToken_t * newToken = huGrowVector(& trove->tokens, 1);

  newToken->tokenKind = tokenKind;
  newToken->value.str = str;
  newToken->value.size = size;
  newToken->line = line;
  newToken->col = col;

  return newToken;
}


int huGetNumNodes(huTrove_t * trove)
{
  return trove->nodes.numElements;
}


huNode_t * huGetRootNode(huTrove_t * trove)
{
  return trove->nodes.buffer;
}


huNode_t * huGetNode(huTrove_t * trove, int nodeIdx)
{
  if (nodeIdx < trove->nodes.numElements)
    { return (huNode_t *) trove->nodes.buffer + nodeIdx; }

  return NULL;
}


huNode_t * huAllocNewNode(huTrove_t * trove, huToken_t * firstToken)
{
  huNode_t * newNode = huGrowVector(& trove->nodes, 1);

  int newNodeIdx = newNode - (huNode_t *) trove->nodes.buffer;
  newNode->nodeIdx = newNodeIdx;
  newNode->firstToken = firstToken;

  return newNode;
}


// User must free(*serializedTrove);
int huTroveToString(huTrove_t * trove, int outputFormat, bool includeComments, char ** serializedTrove)
{
  return -1;
}


int huTroveToFile(huTrove_t * trove, int outputFormat, bool includeComments, FILE * fp)
{
  return -1;
}
