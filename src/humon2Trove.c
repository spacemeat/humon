#include "humon2c.h"


int huMakeTroveFromString(char const * name, char const * data, int inputTabSize, int outputTabSize, huTrove_t ** dst)
{
  * dst = malloc(sizeof(huTrove_t));
  huTrove_t * t = * dst;

  t->nameSize = strlen(name);
  t->name = malloc(t->nameSize);
  strncpy(t->name, name, t->nameSize);

  t->dataStringSize = strlen(data);
  t->dataString = malloc(t->dataStringSize);
  strncpy(t->dataString, data, t->dataStringSize);

  huInitVector(& t->tokens);
  huInitVector(& t->nodes);
  huInitVector(& t->errors);

  t->inputTabSize = inputTabSize;
  t->outputTabSize = outputTabSize;

  huTokenizeTrove(t);
}


int huMakeTroveFromFile(char const * name, FILE * fp, int inputTabSize, int outputTabSize, huTrove_t ** dst)
{
  * dst = malloc(sizeof(huTrove_t));
  huTrove_t * t = * dst;

  t->nameSize = strlen(name);
  t->name = malloc(t->nameSize);
  strncpy(t->name, name, t->nameSize);

  fseek(fp, 0, SEEK_END);
  t->dataStringSize = ftell(fp);
  t->dataString = malloc(t->dataStringSize);
  fseek(fp, 0, SEEK_SET);
  fread(t->dataString, t->dataStringSize, 1, fp);

  huInitVector(& t->tokens);
  huInitVector(& t->nodes);
  huInitVector(& t->errors);

  t->inputTabSize = inputTabSize;
  t->outputTabSize = outputTabSize;

  huTokenizeTrove(t);
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


huToken_t * huGetToken(huTrove_t * trove, int tokenIdx)
{
  if (tokenIdx < trove->tokens.size / sizeof(huToken_t))
    { return trove->tokens.buffer + tokenIdx * sizeof(huToken_t); }

  return NULL;
}


huToken_t * huAllocNewToken(huTrove_t * trove, int tokenKind, 
  char const * str, int size, int line, int col)
{
  huGrowVector(& trove->tokens, sizeof(huToken_t));
  huToken_t * newToken = trove->tokens.buffer + trove->tokens.size - sizeof(huToken_t);

  newToken->tokenKind = tokenKind;
  newToken->value.str = str;
  newToken->value.size = size;
  newToken->line = line;
  newToken->col = col;

  return newToken;
}


huNode_t * huGetRootNode(huTrove_t * trove)
{
  return & trove->nodes.buffer[0];
}


huNode_t * huGetNode(huTrove_t * trove, int nodeIdx)
{
  if (nodeIdx < trove->nodes.size / sizeof(huNode_t))
    { return & ((huNode_t *) trove->nodes.buffer)[nodeIdx]; }

  return NULL;
}


huNode_t * huAllocNewToken(huTrove_t * trove)
{
  huGrowVector(& trove->nodes, sizeof(huNode_t));
  return trove->nodes.buffer + trove->nodes.size - sizeof(huNode_t);
}


// User must free(*serializedTrove);
int huTroveToString(huTrove_t * trove, int outputFormat, bool includeComments, char ** serializedTrove)
{

}


int huTroveToFile(huTrove_t * trove, int outputFormat, bool includeComments, FILE * fp)
{

}
