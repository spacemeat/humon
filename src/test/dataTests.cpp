#include <CppUTest/TestHarness.h>
#include <string.h>
#include "humon2c.h"
#include "ansiColors.h"


TEST_GROUP(emptyString)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"()";
    trove = huMakeTroveFromString("emptyString", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(emptyString, numTokens)
{
  LONGS_EQUAL_TEXT(1, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(1, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(emptyString, numNodes)
{
  LONGS_EQUAL_TEXT(0, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(0, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(emptyString, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(emptyString, numTroveComments)
{
  LONGS_EQUAL_TEXT(0, trove->comments.numElements, "troveComments.num");
  LONGS_EQUAL_TEXT(0, huGetNumTroveComments(trove), "getNumTroveComments()");
}

TEST_GROUP(commentsOnly)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"(//1
/*2*//*3.
0*///4)";
    trove = huMakeTroveFromString("commentsOnly", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(commentsOnly, numTokens)
{
  LONGS_EQUAL_TEXT(5, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(5, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(commentsOnly, numNodes)
{
  LONGS_EQUAL_TEXT(0, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(0, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(commentsOnly, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(commentsOnly, numTroveComments)
{
  LONGS_EQUAL_TEXT(4, trove->comments.numElements, "troveComments.num");
  LONGS_EQUAL_TEXT(4, huGetNumTroveComments(trove), "getNumTroveComments()");
}


TEST_GROUP(singleValue)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
snerb // 2
// 3)";
    trove = huMakeTroveFromString("singleValue", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(singleValue, numTokens)
{
  LONGS_EQUAL_TEXT(5, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(5, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(singleValue, numNodes)
{
  LONGS_EQUAL_TEXT(1, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(1, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(singleValue, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(singleValue, numTroveComments)
{
  LONGS_EQUAL_TEXT(1, trove->comments.numElements, "troveComments.num");
  LONGS_EQUAL_TEXT(1, huGetNumTroveComments(trove), "getNumTroveComments()");
}

TEST(singleValue, numNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(singleValue, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(singleValue, value)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "value set");
  LONGS_EQUAL_TEXT(strlen("snerb"), node->firstValueToken->value.size, "value strlen");
  STRNCMP_EQUAL_TEXT("snerb", node->firstValueToken->value.str, node->firstValueToken->value.size, "value text");
}

TEST(singleValue, lastValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "first value set");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  POINTERS_EQUAL_TEXT(node->firstValueToken, node->lastValueToken, "first == last");
}


TEST_GROUP(singleEmptyList)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
[] // 2
// 3)";
    trove = huMakeTroveFromString("singleEmptyList", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(singleEmptyList, numTokens)
{
  LONGS_EQUAL_TEXT(6, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(6, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(singleEmptyList, numNodes)
{
  LONGS_EQUAL_TEXT(1, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(1, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(singleEmptyList, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(singleEmptyList, numTroveComments)
{
  LONGS_EQUAL_TEXT(1, trove->comments.numElements, "troveComments.num");
  LONGS_EQUAL_TEXT(1, huGetNumTroveComments(trove), "getNumTroveComments()");
}

TEST(singleEmptyList, numNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(singleEmptyList, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(singleEmptyList, numChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(singleEmptyList, value)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "first value set");
  STRNCMP_EQUAL_TEXT("[", node->firstValueToken->value.str, 1, "valueToken");
}

TEST(singleEmptyList, lastValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  STRNCMP_EQUAL_TEXT("]", node->lastValueToken->value.str, 1, "valueToken");
}


TEST_GROUP(singleEmptyDict)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
{} // 2
// 3)";
    trove = huMakeTroveFromString("singleEmptyDict", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(singleEmptyDict, numTokens)
{
  LONGS_EQUAL_TEXT(6, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(6, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(singleEmptyDict, numNodes)
{
  LONGS_EQUAL_TEXT(1, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(1, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(singleEmptyDict, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(singleEmptyDict, numTroveComments)
{
  LONGS_EQUAL_TEXT(1, trove->comments.numElements, "troveComments.num");
  LONGS_EQUAL_TEXT(1, huGetNumTroveComments(trove), "getNumTroveComments()");
}

TEST(singleEmptyDict, numNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(singleEmptyDict, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(singleEmptyDict, numChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(singleEmptyDict, value)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "first value set");
  STRNCMP_EQUAL_TEXT("{", node->firstValueToken->value.str, 1, "valueToken");
}

TEST(singleEmptyDict, lastValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  STRNCMP_EQUAL_TEXT("}", node->lastValueToken->value.str, 1, "valueToken");
}


TEST_GROUP(listWithOneValue)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
[ /*2*/ one /*3*/] // 4
// 5)";
    trove = huMakeTroveFromString("listWithOneValue", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(listWithOneValue, numTokens)
{
  LONGS_EQUAL_TEXT(9, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(9, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(listWithOneValue, numNodes)
{
  LONGS_EQUAL_TEXT(2, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(2, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(listWithOneValue, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(listWithOneValue, numTroveComments)
{
  LONGS_EQUAL_TEXT(1, trove->comments.numElements, "troveComments.num");
  LONGS_EQUAL_TEXT(1, huGetNumTroveComments(trove), "getNumTroveComments()");
}

TEST(listWithOneValue, numParentNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(3, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(3, huGetNumComments(node), "getNumComments()");
}

TEST(listWithOneValue, parentNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listWithOneValue, parentNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithOneValue, value)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "parent not null");
  CHECK_TEXT(node->firstValueToken != NULL, "first value set");
  STRNCMP_EQUAL_TEXT("[", node->firstValueToken->value.str, 1, "valueToken");
}

TEST(listWithOneValue, lastValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "parent not null");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  STRNCMP_EQUAL_TEXT("]", node->lastValueToken->value.str, 1, "valueToken");
}

TEST(listWithOneValue, childNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");

  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(listWithOneValue, childNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithOneValue, childNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(1, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(1, huGetNumComments(node), "getNumComments()");
}

TEST(listWithOneValue, childValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "value set");
  CHECK_TEXT(node->firstValueToken->value.str != NULL, "value set");
  LONGS_EQUAL_TEXT(3, node->firstValueToken->value.size, "value.size");
  STRNCMP_EQUAL_TEXT("one", node->firstValueToken->value.str, 3, "valueToken");
  STRNCMP_EQUAL_TEXT("one", huGetValue(node)->value.str, 3, "getValue()");
}


TEST(listWithOneValue, childLastValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "first value set");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  POINTERS_EQUAL_TEXT(node->firstValueToken, node->lastValueToken, "value.size");
}


TEST_GROUP(dictWithOneValue)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
{ /*2*/ one /*3*/ : /*4*/ two /*5*/ } // 6
// 7)";
    trove = huMakeTroveFromString("dictWithOneValue", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(dictWithOneValue, numTokens)
{
  LONGS_EQUAL_TEXT(13, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(13, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(dictWithOneValue, numNodes)
{
  LONGS_EQUAL_TEXT(2, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(2, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(dictWithOneValue, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(dictWithOneValue, numTroveComments)
{
  LONGS_EQUAL_TEXT(1, trove->comments.numElements, "troveComments.num");
  LONGS_EQUAL_TEXT(1, huGetNumTroveComments(trove), "getNumTroveComments()");
}

TEST(dictWithOneValue, numParentNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(3, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(3, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithOneValue, parentNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictWithOneValue, parentNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithOneValue, value)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "parent not null");
  CHECK_TEXT(node->firstValueToken != NULL, "first value set");
  STRNCMP_EQUAL_TEXT("{", node->firstValueToken->value.str, 1, "valueToken");
}

TEST(dictWithOneValue, lastValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "parent not null");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  STRNCMP_EQUAL_TEXT("}", node->lastValueToken->value.str, 1, "valueToken");
}

TEST(dictWithOneValue, childNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(dictWithOneValue, childNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithOneValue, childNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(3, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(3, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithOneValue, childKey)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  huNode_t * nodeByKey = huGetChildNodeByKey(node, "one");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  POINTERS_EQUAL_TEXT(node, nodeByKey, "foo by key");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->value.str != NULL, "key set");
  LONGS_EQUAL_TEXT(3, node->keyToken->value.size, "key.size");
  STRNCMP_EQUAL_TEXT("one", node->keyToken->value.str, 3, "keyToken");
  STRNCMP_EQUAL_TEXT("one", huGetKey(node)->value.str, 3, "getKey()");
}

TEST(dictWithOneValue, childValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "value set");
  CHECK_TEXT(node->firstValueToken->value.str != NULL, "value set");
  LONGS_EQUAL_TEXT(3, node->firstValueToken->value.size, "value.size");
  STRNCMP_EQUAL_TEXT("two", node->firstValueToken->value.str, 3, "valueToken");
  STRNCMP_EQUAL_TEXT("two", huGetValue(node)->value.str, 3, "getValue()");
}

TEST(dictWithOneValue, childLastValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "first value set");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  POINTERS_EQUAL_TEXT(node->firstValueToken, node->lastValueToken, "value.size");
}


TEST_GROUP(listWithTwoValues)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1.1
[ /*1.2*/ 
  /*2.1*/
  two /*2.2*/
  /*3.1*/ three /*3.2*/ 
  // 1.3
  ] // 1.4
// t)";
    trove = huMakeTroveFromString("listWithTwoValues", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(listWithTwoValues, numTokens)
{
  LONGS_EQUAL_TEXT(14, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(14, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(listWithTwoValues, numNodes)
{
  LONGS_EQUAL_TEXT(3, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(3, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(listWithTwoValues, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(listWithTwoValues, numTroveComments)
{
  LONGS_EQUAL_TEXT(1, trove->comments.numElements, "troveComments.num");
  LONGS_EQUAL_TEXT(1, huGetNumTroveComments(trove), "getNumTroveComments()");
}

TEST(listWithTwoValues, numParentNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(4, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(4, huGetNumComments(node), "getNumComments()");
}

TEST(listWithTwoValues, parentNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listWithTwoValues, parentNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(2, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithTwoValues, twoNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(listWithTwoValues, twoNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithTwoValues, twoNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(listWithTwoValues, twoValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "value set");
  CHECK_TEXT(node->firstValueToken->value.str != NULL, "value set");
  LONGS_EQUAL_TEXT(3, node->firstValueToken->value.size, "value.size");
  STRNCMP_EQUAL_TEXT("two", node->firstValueToken->value.str, 3, "valueToken");
  STRNCMP_EQUAL_TEXT("two", huGetValue(node)->value.str, 3, "getValue()");
}

TEST(listWithTwoValues, threeNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(listWithTwoValues, threeNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithTwoValues, threeNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(listWithTwoValues, threeValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "value set");
  CHECK_TEXT(node->firstValueToken->value.str != NULL, "value set");
  LONGS_EQUAL_TEXT(5, node->firstValueToken->value.size, "value.size");
  STRNCMP_EQUAL_TEXT("three", node->firstValueToken->value.str, 5, "valueToken");
  STRNCMP_EQUAL_TEXT("three", huGetValue(node)->value.str, 5, "getValue()");
}

TEST_GROUP(dictWithTwoValues)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1.1
{ /*1.2*/ 
  /*2.1*/
  two /*2.2*/
  //2.3
  : //2.4
  //2.5
  red //2.6
  /*3.1*/ three:blue /*3.2*/ 
  // 1.3
  } // 1.4
// t)";
    trove = huMakeTroveFromString("dictWithTwoValues", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(dictWithTwoValues, numTokens)
{
  LONGS_EQUAL_TEXT(22, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(22, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(dictWithTwoValues, numNodes)
{
  LONGS_EQUAL_TEXT(3, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(3, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(dictWithTwoValues, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(dictWithTwoValues, numTroveComments)
{
  LONGS_EQUAL_TEXT(1, trove->comments.numElements, "troveComments.num");
  LONGS_EQUAL_TEXT(1, huGetNumTroveComments(trove), "getNumTroveComments()");
}

TEST(dictWithTwoValues, numParentNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(4, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(4, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithTwoValues, parentNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictWithTwoValues, parentNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(2, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithTwoValues, twoNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(dictWithTwoValues, twoNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithTwoValues, twoNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(6, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(6, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithTwoValues, twoKey)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  huNode_t * nodeByKey = huGetChildNodeByKey(node, "two");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  POINTERS_EQUAL_TEXT(node, nodeByKey, "foo by key");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->value.str != NULL, "key set");
  LONGS_EQUAL_TEXT(3, node->keyToken->value.size, "key.size");
  STRNCMP_EQUAL_TEXT("two", node->keyToken->value.str, 3, "keyToken");
  STRNCMP_EQUAL_TEXT("two", huGetKey(node)->value.str, 3, "getKey()");
}

TEST(dictWithTwoValues, twoValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "value set");
  CHECK_TEXT(node->firstValueToken->value.str != NULL, "value set");
  LONGS_EQUAL_TEXT(3, node->firstValueToken->value.size, "value.size");
  STRNCMP_EQUAL_TEXT("red", node->firstValueToken->value.str, 3, "valueToken");
  STRNCMP_EQUAL_TEXT("red", huGetValue(node)->value.str, 3, "getValue()");
}

TEST(dictWithTwoValues, threeNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(dictWithTwoValues, threeNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithTwoValues, threeNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithTwoValues, threeKey)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  huNode_t * nodeByKey = huGetChildNodeByKey(node, "three");
  node = huGetChildNodeByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  POINTERS_EQUAL_TEXT(node, nodeByKey, "foo by key");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->value.str != NULL, "key set");
  LONGS_EQUAL_TEXT(5, node->keyToken->value.size, "key.size");
  STRNCMP_EQUAL_TEXT("three", node->keyToken->value.str, 5, "keyToken");
  STRNCMP_EQUAL_TEXT("three", huGetKey(node)->value.str, 5, "getKey()");
}

TEST(dictWithTwoValues, threeValue)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->firstValueToken != NULL, "value set");
  CHECK_TEXT(node->firstValueToken->value.str != NULL, "value set");
  LONGS_EQUAL_TEXT(4, node->firstValueToken->value.size, "value.size");
  STRNCMP_EQUAL_TEXT("blue", node->firstValueToken->value.str, 4, "valueToken");
  STRNCMP_EQUAL_TEXT("blue", huGetValue(node)->value.str, 4, "getValue()");
}


TEST_GROUP(listInList)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"([[]])";
    trove = huMakeTroveFromString("listInList", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(listInList, numTokens)
{
  LONGS_EQUAL_TEXT(5, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(5, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(listInList, numNodes)
{
  LONGS_EQUAL_TEXT(2, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(2, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(listInList, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(listInList, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listInList, numChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(listInList, childNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listInList, childNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}


TEST_GROUP(dictInList)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"([{}])";
    trove = huMakeTroveFromString("dictInList", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(dictInList, numTokens)
{
  LONGS_EQUAL_TEXT(5, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(5, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(dictInList, numNodes)
{
  LONGS_EQUAL_TEXT(2, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(2, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(dictInList, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(dictInList, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(dictInList, numChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictInList, childNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictInList, childNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST_GROUP(listInDict)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"({foo:[]})";
    trove = huMakeTroveFromString("listInDict", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(listInDict, numTokens)
{
  LONGS_EQUAL_TEXT(7, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(7, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(listInDict, numNodes)
{
  LONGS_EQUAL_TEXT(2, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(2, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(listInDict, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(listInDict, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(listInDict, numChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(listInDict, key)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  huNode_t * nodeByKey = huGetChildNodeByKey(node, "foo");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  POINTERS_EQUAL_TEXT(node, nodeByKey, "foo by key");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->value.str != NULL, "key set");
  LONGS_EQUAL_TEXT(3, node->keyToken->value.size, "key.size");
  STRNCMP_EQUAL_TEXT("foo", node->keyToken->value.str, 3, "keyToken");
  STRNCMP_EQUAL_TEXT("foo", huGetKey(node)->value.str, 3, "getKey()");
}

TEST(listInDict, childNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listInDict, childNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}


TEST_GROUP(dictInDict)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"({foo:{}})";
    trove = huMakeTroveFromString("dictInDict", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(dictInDict, numTokens)
{
  LONGS_EQUAL_TEXT(7, trove->tokens.numElements, "tokens.num");
  LONGS_EQUAL_TEXT(7, huGetNumTokens(trove), "GetNumTokens()");
}

TEST(dictInDict, numNodes)
{
  LONGS_EQUAL_TEXT(2, trove->nodes.numElements, "nodes.num");
  LONGS_EQUAL_TEXT(2, huGetNumNodes(trove), "GetNumNodes()");
}

TEST(dictInDict, numErrors)
{
  LONGS_EQUAL_TEXT(0, trove->errors.numElements, "errors.num");
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
}

TEST(dictInDict, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictInDict, numChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictInDict, key)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->value.str != NULL, "key string set");
  LONGS_EQUAL_TEXT(3, node->keyToken->value.size, "key.size");
  STRNCMP_EQUAL_TEXT("foo", node->keyToken->value.str, 3, "keyToken");
  STRNCMP_EQUAL_TEXT("foo", huGetKey(node)->value.str, 3, "getKey()");
}

TEST(dictInDict, childNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictInDict, childNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildNodeByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}


TEST_GROUP(multipleNestedLists)
{
  huTrove_t * trove = NULL;

  void setup()
  {
    auto humon = R"([
  [[a b c] [d e f]]
  [[g h i] [j k l]]
])";
    trove = huMakeTroveFromString("dictInDict", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(multipleNestedLists, values)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(2, huGetNumChildren(node), "t0 ch");

  char c[] = "a";
  for (int i = 0; i < 2; ++i)
  {
    huNode_t * ch0 = huGetChildNodeByIndex(node, i);
    LONGS_EQUAL_TEXT(2, huGetNumChildren(ch0), "t1 ch");
    for (int j = 0; j < 2; ++j)
    {
      huNode_t * ch1 = huGetChildNodeByIndex(ch0, j);
      LONGS_EQUAL_TEXT(3, huGetNumChildren(ch1), c);
      for (int k = 0; k < 3; ++k)
      {
        huNode_t * ch2 = huGetChildNodeByIndex(ch1, k);
        CHECK_TEXT(ch2->firstValueToken != NULL, "value set");
        LONGS_EQUAL_TEXT(1, ch2->firstValueToken->value.size, "value.size");
        STRNCMP_EQUAL_TEXT(c, huGetValue(ch2)->value.str, 1, c);
        c[0] += 1;
      }
    }
  }
}
