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
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(singleValue, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(singleValue, value)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  CHECK_TEXT(node->valueToken != NULL, "value set");
  LONGS_EQUAL_TEXT(strlen("snerb"), node->valueToken->value.size, "value strlen");
  STRNCMP_EQUAL_TEXT("snerb", node->valueToken->value.str, node->valueToken->value.size, "value text");
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
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(singleEmptyList, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(singleEmptyList, numChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "node.numChildren");
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
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(singleEmptyDict, nodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(singleEmptyDict, numChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "node.numChildren");
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
  LONGS_EQUAL_TEXT(3, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(3, huGetNumComments(node), "getNumComments()");
}

TEST(listWithOneValue, parentNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listWithOneValue, parentNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "node.numChildren");
}

TEST(listWithOneValue, childNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);

  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(listWithOneValue, childNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "node.numChildren");
}

TEST(listWithOneValue, childNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);
  LONGS_EQUAL_TEXT(1, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(1, huGetNumComments(node), "getNumComments()");
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
  LONGS_EQUAL_TEXT(3, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(3, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithOneValue, parentNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictWithOneValue, parentNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "node.numChildren");
}

TEST(dictWithOneValue, childNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);

  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(dictWithOneValue, childNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "node.numChildren");
}

TEST(dictWithOneValue, childNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);
  LONGS_EQUAL_TEXT(3, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(3, huGetNumComments(node), "getNumComments()");
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
  LONGS_EQUAL_TEXT(4, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(4, huGetNumComments(node), "getNumComments()");
}

TEST(listWithTwoValues, parentNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listWithTwoValues, parentNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  LONGS_EQUAL_TEXT(2, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(2, huGetNumChildren(node), "node.numChildren");
}

TEST(listWithTwoValues, twoNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);

  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(listWithTwoValues, twoNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "node.numChildren");
}

TEST(listWithTwoValues, twoNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}


TEST(listWithTwoValues, threeNodeKind)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);

  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(listWithTwoValues, threeNumChildren)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "node.numChildren");
}

TEST(listWithTwoValues, threeNumNodeComments)
{
  huNode_t * node = (huNode_t *) huGetElement(& trove->nodes, 0);
  node = huGetChildNodeByIndex(node, 0);
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}
