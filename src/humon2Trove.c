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

  huInitVector(& t->annotations, sizeof(huAnnotation_t));
  huInitVector(& t->comments, sizeof(huComment_t));

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


huToken_t * allocNewToken(huTrove_t * trove, int tokenKind, 
  char const * str, int size, int line, int col)
{
  huToken_t * newToken = huGrowVector(& trove->tokens, 1);

  newToken->tokenKind = tokenKind;
  newToken->value.str = str;
  newToken->value.size = size;
  newToken->line = line;
  newToken->col = col;

  printf ("token: line: %d  col: %d  len: %d  %s\n",
    line, col, size, huTokenKindToString(tokenKind));
  
  printf ("  (numTokens: %d)\n", trove->tokens.numElements);

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


int huGetNumErrors(huTrove_t * trove)
{
  return trove->errors.numElements;
}


huError_t * huGetError(huTrove_t * trove, int errorIdx)
{
  if (errorIdx < huGetNumErrors(trove))
  {
    return (huError_t *) trove->errors.buffer + errorIdx;
  }

  return NULL;
}


int huGetNumTroveComments(huTrove_t * trove)
{
  return trove->comments.numElements;
}


huError_t * huGetTroveComment(huTrove_t * trove, int commentIdx)
{
  if (commentIdx < huGetNumTroveComments(trove))
  {
    return (huError_t *) trove->comments.buffer + commentIdx;
  }

  return NULL;
}


huNode_t * allocNewNode(huTrove_t * trove, int nodeKind, huToken_t * firstToken)
{
  huNode_t * newNode = huGrowVector(& trove->nodes, 1);
  huInitNode(newNode, trove);
  int newNodeIdx = newNode - (huNode_t *) trove->nodes.buffer;
  newNode->nodeIdx = newNodeIdx;
  newNode->kind = nodeKind;
  newNode->firstToken = firstToken;

  printf ("node: nodeIdx: %d  firstToken: %d  %s\n",
    newNodeIdx, (int)(firstToken - (huToken_t *) trove->tokens.buffer), huTokenKindToString(nodeKind));

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
