#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


huTrove_t * makeTrove(char const * name, huStringView_t * data, int inputTabSize, int outputTabSize)
{
  huTrove_t * t = malloc(sizeof(huTrove_t));
  if (t == NULL)
    { return NULL; }

  t->nameSize = strlen(name);
  t->name = malloc(t->nameSize + 1);
  if (t->name == NULL)
  {
    free(t);
    return NULL;
  }
  strncpy(t->name, name, t->nameSize);
  t->name[t->nameSize] = '\0';

  t->dataStringSize = data->size;
  t->dataString = malloc(data->size + 1);
  if (t->dataString == NULL)
  {
    free(t->name);
    free(t);
    return NULL;
  }
  strncpy(t->dataString, data->str, data->size);
  t->dataString[data->size] = '\0';

  printf("\n%snew trove: %s%s\n%s%s%s\n", darkGreen, lightGreen, t->name, lightYellow, t->dataString, off);

  huInitVector(& t->tokens, sizeof(huToken_t));
  huInitVector(& t->nodes, sizeof(huNode_t));
  huInitVector(& t->errors, sizeof(huError_t));

  t->inputTabSize = inputTabSize;
  t->outputTabSize = outputTabSize;

  huInitVector(& t->annotations, sizeof(huAnnotation_t));
  huInitVector(& t->comments, sizeof(huComment_t));

  huTokenizeTrove(t);
  huParseTrove(t);

  huStringView_t defaultColors[] = 
  {
    { darkGray, strlen(darkGray) },           // NONE
    { lightGray, strlen(lightGray) },           // PUNCLIST
    { lightGray, strlen(lightGray) },           // PUNCDIST
    { lightGray, strlen(lightGray) },           // PUNCVALUESEP
    { darkGray, strlen(darkGray) },         // PUNCANNOTATE
    { darkGray, strlen(darkGray) },         // PUNCANNOTATEDICT
    { darkGray, strlen(darkGray) },         // PUNCANNOTATEKEYVALUESEP
    { darkYellow, strlen(darkYellow) },       // KEY
    { lightYellow, strlen(lightYellow) },     // VALUE
    { darkGreen, strlen(darkGreen) },         // COMMENT
    { darkMagenta, strlen(darkMagenta) },     // ANNOKEY
    { lightMagenta, strlen(lightMagenta) },   // ANNOVALUE
    { darkGray, strlen(darkGray) },           // WHITESPACE
  };

  huStringView_t sv = huTroveToString(t, HU_OUTPUTFORMAT_MINIMAL, false, defaultColors);
  printf("%s", sv.str);

  return t;
}


huTrove_t * huMakeTroveFromString(char const * name, char const * data, int inputTabSize, int outputTabSize)
{
  int newDataSize = strlen(data);
  char * newData = malloc(newDataSize + 1);
  if (newData == NULL)
    { return NULL; }

  strncpy(newData, data, newDataSize);
  newData[newDataSize] = '\0';

  huStringView_t dataView = { newData, newDataSize };
  huTrove_t * newTrove = makeTrove(name, & dataView, inputTabSize, outputTabSize);
  if (newTrove == NULL)
    { free(newData); }
  
  return newTrove;
}


huTrove_t * huMakeTroveFromFile(char const * name, FILE * fp, int inputTabSize, int outputTabSize)
{
  if (fseek(fp, 0, SEEK_END) != 0)
    { return NULL; }

  int newDataSize = ftell(fp);
  if (newDataSize == -1L)
    { return NULL; }

  char * newData = malloc(newDataSize + 1);
  if (newData == NULL)
    { return NULL; }

  fseek(fp, 0, SEEK_SET);
  if (fread(newData, newDataSize, 1, fp) != 
    newDataSize)
  {
    free(newData);
    return NULL;
  }
  newData[newDataSize] = '\0';

  huStringView_t dataView = { newData, newDataSize };
  huTrove_t * newTrove = makeTrove(name, & dataView, inputTabSize, outputTabSize);
  if (newTrove == NULL)
    { free(newData); }
  
  return newTrove;
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

  printf ("%stoken: line: %d  col: %d  len: %d  %s%s\n",
    darkYellow, line, col, size, huTokenKindToString(tokenKind), off);
  
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
  if (nodeIdx >= 0 && nodeIdx < trove->nodes.numElements)
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
  newNode->lastToken = firstToken;

  printf ("%snode: nodeIdx: %d  firstToken: %d  %s%s\n",
    lightCyan, newNodeIdx, (int)(firstToken - (huToken_t *) trove->tokens.buffer), 
    huNodeKindToString(nodeKind), off);

  return newNode;
}


void appendString(huVector_t * str, char const * addend, int size)
{
  char * dest = huGrowVector(str, size);
  strncpy(dest, addend, size);
}


// User must free(returnval.str);
huStringView_t huTroveToString(huTrove_t * trove, int outputFormat, bool excludeComments, huStringView_t * colorTable)
{
  huVector_t str;
  huInitVector(& str, sizeof(char));

  switch (outputFormat)
  {
  case HU_OUTPUTFORMAT_PRESERVED:
    {
      if (excludeComments == false && colorTable == NULL)
      {
        char * newData = malloc(trove->dataStringSize + 1);
        strncpy(newData, trove->dataString, trove->dataStringSize + 1);
        huStringView_t sv = { newData, trove->dataStringSize };
        return sv;
      }
      int line = 1, col = 1;
      int lastColorTableIdx = HU_COLORKIND_NONE;
      bool inDict = false;
      bool inAnno = false;
      bool expectKey = false;
      bool isKey = false;

      huToken_t * tokens = (huToken_t *) trove->tokens.buffer;
      for (int i = 0; i < huGetNumTokens(trove); ++i)
      {
        huToken_t * tok = tokens + i;
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
            if (inAnno && ! inDict)
              { inAnno = false; }
          }
        }   

        // the space between tokens is filled with whitespace
        while (tok->line > line)
        {
          appendString(& str, "\n", 1);
          line += 1;
          col = 1;
        }
        while (tok->col > col)
        {
          appendString(& str, " ", 1);
          col += 1;
        }
        if (colorTable != NULL && tok->tokenKind != lastColorTableIdx)
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
          
          if (colorTableIdx != lastColorTableIdx)
          {
            huStringView_t * ce = colorTable + colorTableIdx;
            appendString(& str, ce->str, ce->size);
          }
        }
        appendString(& str, tok->value.str, tok->value.size);
      }
      // after token loop
      if (colorTable != NULL)
      {
        appendString(& str, off, strlen(off));
      }
      appendString(& str, "\n\0", 1);
    }
    break;

  case HU_OUTPUTFORMAT_MINIMAL:
    {
      int line = 1, col = 1;
      int lastColorTableIdx = HU_COLORKIND_NONE;
      bool inDict = false;
      bool inAnno = false;
      bool expectKey = false;
      bool isKey = false;

      huToken_t * tokens = (huToken_t *) trove->tokens.buffer;
      for (int i = 0; i < huGetNumTokens(trove); ++i)
      {
        huToken_t * tok = tokens + i;
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

        // the space between tokens is filled with whitespace
        if (tok->line != line)
        {
          appendString(& str, "\n", 1);
          line += 1;
          col = 1;
        }
        else if (tok->col > col)
        {
          appendString(& str, " ", 1);
          col += 1;
        }

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
            huStringView_t * ce = colorTable + colorTableIdx;
            appendString(& str, ce->str, ce->size);
            lastColorTableIdx = colorTableIdx;
          }
        }
        appendString(& str, tok->value.str, tok->value.size);
      }
      if (colorTable != NULL)
      {
        appendString(& str, off, strlen(off));
      }
      appendString(& str, "\n\0", 2);
    }
    break;

  case HU_OUTPUTFORMAT_PRETTY:
    {
      
    }
    break;
  }

  huStringView_t res = { str.buffer, str.numElements };
  return res;
}


int huTroveToFile(huTrove_t * trove, int outputFormat, bool includeComments, FILE * fp)
{
  return -1;
}
