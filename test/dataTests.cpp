#include <CppUTest/TestHarness.h>
#include <string.h>
#include "humon.h"
#include <string_view>

using namespace std::literals;


TEST_GROUP(emptyString)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"()"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"(//1
/*2*//*3.
0*///4)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
snerb // 2
// 3)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(singleValue, nodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(singleValue, value)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->valueToken != NULL, "value set");
  LONGS_EQUAL_TEXT(strlen("snerb"), node->valueToken->str.size, "value strlen");
  STRNCMP_EQUAL_TEXT("snerb", node->valueToken->str.ptr, node->valueToken->str.size, "value text");
}

TEST(singleValue, lastValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->valueToken != NULL, "first value set");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  POINTERS_EQUAL_TEXT(node->valueToken, node->lastValueToken, "first == last");
}


TEST_GROUP(singleEmptyList)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
[] // 2
// 3)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(singleEmptyList, nodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(singleEmptyList, numChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(singleEmptyList, value)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->valueToken != NULL, "first value set");
  STRNCMP_EQUAL_TEXT("[", node->valueToken->str.ptr, 1, "valueToken");
}

TEST(singleEmptyList, lastValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  STRNCMP_EQUAL_TEXT("]", node->lastValueToken->str.ptr, 1, "valueToken");
}


TEST_GROUP(singleEmptyDict)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
{} // 2
// 3)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(singleEmptyDict, nodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(singleEmptyDict, numChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(singleEmptyDict, value)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->valueToken != NULL, "first value set");
  STRNCMP_EQUAL_TEXT("{", node->valueToken->str.ptr, 1, "valueToken");
}

TEST(singleEmptyDict, lastValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  STRNCMP_EQUAL_TEXT("}", node->lastValueToken->str.ptr, 1, "valueToken");
}


TEST_GROUP(listWithOneValue)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
[ /*2*/ one /*3*/] // 4
// 5)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(3, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(3, huGetNumComments(node), "getNumComments()");
}

TEST(listWithOneValue, parentNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listWithOneValue, parentNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithOneValue, value)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "parent not null");
  CHECK_TEXT(node->valueToken != NULL, "first value set");
  STRNCMP_EQUAL_TEXT("[", node->valueToken->str.ptr, 1, "valueToken");
}

TEST(listWithOneValue, lastValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "parent not null");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  STRNCMP_EQUAL_TEXT("]", node->lastValueToken->str.ptr, 1, "valueToken");
}

TEST(listWithOneValue, childNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");

  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(listWithOneValue, childNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithOneValue, childNumNodeComments)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(1, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(1, huGetNumComments(node), "getNumComments()");
}

TEST(listWithOneValue, childValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->valueToken != NULL, "value set");
  CHECK_TEXT(node->valueToken->str.ptr != NULL, "value set");
  LONGS_EQUAL_TEXT(3, node->valueToken->str.size, "value.size");
  STRNCMP_EQUAL_TEXT("one", node->valueToken->str.ptr, 3, "valueToken");
}


TEST(listWithOneValue, childLastValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->valueToken != NULL, "first value set");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  POINTERS_EQUAL_TEXT(node->valueToken, node->lastValueToken, "value.size");
}


TEST_GROUP(dictWithOneValue)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1
{ /*2*/ one /*3*/ : /*4*/ two /*5*/ } // 6
// 7)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(3, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(3, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithOneValue, parentNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictWithOneValue, parentNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithOneValue, value)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "parent not null");
  CHECK_TEXT(node->valueToken != NULL, "first value set");
  STRNCMP_EQUAL_TEXT("{", node->valueToken->str.ptr, 1, "valueToken");
}

TEST(dictWithOneValue, lastValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "parent not null");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  STRNCMP_EQUAL_TEXT("}", node->lastValueToken->str.ptr, 1, "valueToken");
}

TEST(dictWithOneValue, childNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(dictWithOneValue, childNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithOneValue, childNumNodeComments)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(3, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(3, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithOneValue, childKey)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  huNode const * nodeByKey = huGetChildByKeyZ(node, "one");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  POINTERS_EQUAL_TEXT(node, nodeByKey, "foo by key");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->str.ptr != NULL, "key set");
  LONGS_EQUAL_TEXT(3, node->keyToken->str.size, "key.size");
  STRNCMP_EQUAL_TEXT("one", node->keyToken->str.ptr, 3, "keyToken");
}

TEST(dictWithOneValue, childValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->valueToken != NULL, "value set");
  CHECK_TEXT(node->valueToken->str.ptr != NULL, "value set");
  LONGS_EQUAL_TEXT(3, node->valueToken->str.size, "value.size");
  STRNCMP_EQUAL_TEXT("two", node->valueToken->str.ptr, 3, "valueToken");
}

TEST(dictWithOneValue, childLastValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->valueToken != NULL, "first value set");
  CHECK_TEXT(node->lastValueToken != NULL, "last value set");
  POINTERS_EQUAL_TEXT(node->valueToken, node->lastValueToken, "value.size");
}


TEST_GROUP(listWithTwoValues)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"(// 1.1
[ /*1.2*/ 
  /*2.1*/
  two /*2.2*/
  /*3.1*/ three /*3.2*/ 
  // 1.3
  ] // 1.4
// t)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(4, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(4, huGetNumComments(node), "getNumComments()");
}

TEST(listWithTwoValues, parentNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listWithTwoValues, parentNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(2, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithTwoValues, twoNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(listWithTwoValues, twoNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithTwoValues, twoNumNodeComments)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(listWithTwoValues, twoValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->valueToken != NULL, "value set");
  CHECK_TEXT(node->valueToken->str.ptr != NULL, "value set");
  LONGS_EQUAL_TEXT(3, node->valueToken->str.size, "value.size");
  STRNCMP_EQUAL_TEXT("two", node->valueToken->str.ptr, 3, "valueToken");
}

TEST(listWithTwoValues, threeNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(listWithTwoValues, threeNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(listWithTwoValues, threeNumNodeComments)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(listWithTwoValues, threeValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->valueToken != NULL, "value set");
  CHECK_TEXT(node->valueToken->str.ptr != NULL, "value set");
  LONGS_EQUAL_TEXT(5, node->valueToken->str.size, "value.size");
  STRNCMP_EQUAL_TEXT("three", node->valueToken->str.ptr, 5, "valueToken");
}

TEST_GROUP(dictWithTwoValues)
{
  huTrove const * trove = NULL;

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
// t)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(4, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(4, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithTwoValues, parentNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictWithTwoValues, parentNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(2, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(2, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithTwoValues, twoNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(dictWithTwoValues, twoNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithTwoValues, twoNumNodeComments)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(6, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(6, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithTwoValues, twoKey)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  huNode const * nodeByKey = huGetChildByKeyZ(node, "two");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  POINTERS_EQUAL_TEXT(node, nodeByKey, "foo by key");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->str.ptr != NULL, "key set");
  LONGS_EQUAL_TEXT(3, node->keyToken->str.size, "key.size");
  STRNCMP_EQUAL_TEXT("two", node->keyToken->str.ptr, 3, "keyToken");
}

TEST(dictWithTwoValues, twoValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->valueToken != NULL, "value set");
  CHECK_TEXT(node->valueToken->str.ptr != NULL, "value set");
  LONGS_EQUAL_TEXT(3, node->valueToken->str.size, "value.size");
  STRNCMP_EQUAL_TEXT("red", node->valueToken->str.ptr, 3, "valueToken");
}

TEST(dictWithTwoValues, threeNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_VALUE, node->kind, "node kind");
}

TEST(dictWithTwoValues, threeNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictWithTwoValues, threeNumNodeComments)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(2, node->comments.numElements, "comments.num");
  LONGS_EQUAL_TEXT(2, huGetNumComments(node), "getNumComments()");
}

TEST(dictWithTwoValues, threeKey)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  huNode const * nodeByKey = huGetChildByKeyZ(node, "three");
  node = huGetChildByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  POINTERS_EQUAL_TEXT(node, nodeByKey, "foo by key");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->str.ptr != NULL, "key set");
  LONGS_EQUAL_TEXT(5, node->keyToken->str.size, "key.size");
  STRNCMP_EQUAL_TEXT("three", node->keyToken->str.ptr, 5, "keyToken");
}

TEST(dictWithTwoValues, threeValue)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 1);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->valueToken != NULL, "value set");
  CHECK_TEXT(node->valueToken->str.ptr != NULL, "value set");
  LONGS_EQUAL_TEXT(4, node->valueToken->str.size, "value.size");
  STRNCMP_EQUAL_TEXT("blue", node->valueToken->str.ptr, 4, "valueToken");
}


TEST_GROUP(listInList)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"([[]])"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listInList, numChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(listInList, childNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listInList, childNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}


TEST_GROUP(dictInList)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"([{}])"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(dictInList, numChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictInList, childNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictInList, childNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}

TEST_GROUP(listInDict)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"({foo:[]})"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(listInDict, numChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(listInDict, key)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  huNode const * nodeByKey = huGetChildByKeyZ(node, "foo");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  POINTERS_EQUAL_TEXT(node, nodeByKey, "foo by key");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->str.ptr != NULL, "key set");
  LONGS_EQUAL_TEXT(3, node->keyToken->str.size, "key.size");
  STRNCMP_EQUAL_TEXT("foo", node->keyToken->str.ptr, 3, "keyToken");
}

TEST(listInDict, childNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_LIST, node->kind, "node kind");
}

TEST(listInDict, childNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}


TEST_GROUP(dictInDict)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"({foo:{}})"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
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
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictInDict, numChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  LONGS_EQUAL_TEXT(1, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(1, huGetNumChildren(node), "getNumChildren()");
}

TEST(dictInDict, key)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  CHECK_TEXT(node->keyToken != NULL, "key set");
  CHECK_TEXT(node->keyToken->str.ptr != NULL, "key string set");
  LONGS_EQUAL_TEXT(3, node->keyToken->str.size, "key.size");
  STRNCMP_EQUAL_TEXT("foo", node->keyToken->str.ptr, 3, "keyToken");
}

TEST(dictInDict, childNodeKind)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(HU_NODEKIND_DICT, node->kind, "node kind");
}

TEST(dictInDict, childNumChildren)
{
  huNode const * node = huGetRootNode(trove);
  CHECK_TEXT(node != NULL, "node not null");
  node = huGetChildByIndex(node, 0);
  CHECK_TEXT(node != NULL, "child node not null");
  LONGS_EQUAL_TEXT(0, node->childNodeIdxs.numElements, "node.numChildren");
  LONGS_EQUAL_TEXT(0, huGetNumChildren(node), "getNumChildren()");
}


TEST_GROUP(multipleNestedLists)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = R"([
  [[a b c] [d e f]]
  [[g h i] [j k l]]
])"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(multipleNestedLists, values)
{
  huNode const * node = huGetRootNode(trove);
  LONGS_EQUAL_TEXT(2, huGetNumChildren(node), "t0 ch");

  char c[] = "a";
  for (int i = 0; i < 2; ++i)
  {
    huNode const * ch0 = huGetChildByIndex(node, i);
    LONGS_EQUAL_TEXT(2, huGetNumChildren(ch0), "t1 ch");
    for (int j = 0; j < 2; ++j)
    {
      huNode const * ch1 = huGetChildByIndex(ch0, j);
      LONGS_EQUAL_TEXT(3, huGetNumChildren(ch1), "t2 ch");
      for (int k = 0; k < 3; ++k)
      {
        huNode const * ch2 = huGetChildByIndex(ch1, k);
        CHECK_TEXT(ch2->valueToken != NULL, "value set");
        LONGS_EQUAL_TEXT(1, ch2->valueToken->str.size, "value.size");
        STRNCMP_EQUAL_TEXT(c, ch2->valueToken->str.ptr, 1, "t3 ch");
        c[0] += 1;
      }
    }
  }
}

TEST_GROUP(oneAnnoOnly)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"(@a:b)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneAnnoOnly, values)
{
  LONGS_EQUAL_TEXT(0, huGetNumErrors(trove), "GetNumErrors()");
  LONGS_EQUAL_TEXT(1, huGetNumTroveAnnotations(trove), "getNumTAs()");
  huAnnotation const * anno = huGetTroveAnnotation(trove, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "val val");
}


TEST_GROUP(oneValueAnno)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"(foo   @ a : b)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneValueAnno, values)
{
  huNode const * node = huGetRootNode(trove);
  LONGS_EQUAL_TEXT(1, huGetNumAnnotations(node), "num anno");
  huAnnotation const * anno = huGetAnnotation(node, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "val val");
}


TEST_GROUP(oneValueTwoAnno)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"(foo   @ a : b
@ c : d)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneValueTwoAnno, values)
{
  huNode const * node = huGetRootNode(trove);
  LONGS_EQUAL_TEXT(2, huGetNumAnnotations(node), "num anno");
  huAnnotation const * anno = huGetAnnotation(node, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "a key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "a key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "b val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "b val val");
  anno = huGetAnnotation(node, 1);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "c key len");
  STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, 1, "c key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "d val len");
  STRNCMP_EQUAL_TEXT("d", anno->value->str.ptr, 1, "d val val");
}


TEST_GROUP(oneValueTwoAnnoGroup)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"(foo   @ {a : b
c : d} )"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneValueTwoAnnoGroup, values)
{
  huNode const * node = huGetRootNode(trove);
  LONGS_EQUAL_TEXT(2, huGetNumAnnotations(node), "num anno");
  huAnnotation const * anno = huGetAnnotation(node, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "a key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "a key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "b val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "b val val");
  anno = huGetAnnotation(node, 1);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "c key len");
  STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, 1, "c key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "d val len");
  STRNCMP_EQUAL_TEXT("d", anno->value->str.ptr, 1, "d val val");
}


TEST_GROUP(oneEmptyListTwoAnnoGroup)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"([   @ {a : b
c : d} ])"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneEmptyListTwoAnnoGroup, values)
{
  huNode const * node = huGetRootNode(trove);
  LONGS_EQUAL_TEXT(2, huGetNumAnnotations(node), "num anno");
  huAnnotation const * anno = huGetAnnotation(node, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "a key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "a key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "b val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "b val val");
  anno = huGetAnnotation(node, 1);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "c key len");
  STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, 1, "c key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "d val len");
  STRNCMP_EQUAL_TEXT("d", anno->value->str.ptr, 1, "d val val");
}


TEST_GROUP(oneEmptyListTwoAnnoLast)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"([   @ a : b
]@c : d )"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneEmptyListTwoAnnoLast, values)
{
  huNode const * node = huGetRootNode(trove);
  LONGS_EQUAL_TEXT(2, huGetNumAnnotations(node), "num anno");
  huAnnotation const * anno = huGetAnnotation(node, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "a key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "a key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "b val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "b val val");
  anno = huGetAnnotation(node, 1);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "c key len");
  STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, 1, "c key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "d val len");
  STRNCMP_EQUAL_TEXT("d", anno->value->str.ptr, 1, "d val val");
}


TEST_GROUP(oneEmptyDictTwoAnnoGroup)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"({   @ {a : b
c : d} })"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneEmptyDictTwoAnnoGroup, values)
{
  huNode const * node = huGetRootNode(trove);
  LONGS_EQUAL_TEXT(2, huGetNumAnnotations(node), "num anno");
  huAnnotation const * anno = huGetAnnotation(node, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "a key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "a key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "b val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "b val val");
  anno = huGetAnnotation(node, 1);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "c key len");
  STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, 1, "c key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "d val len");
  STRNCMP_EQUAL_TEXT("d", anno->value->str.ptr, 1, "d val val");
}


TEST_GROUP(oneEmptyDictTwoAnnoLast)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"({   @ a : b}@
c : d)"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneEmptyDictTwoAnnoLast, values)
{
  huNode const * node = huGetRootNode(trove);
  LONGS_EQUAL_TEXT(2, huGetNumAnnotations(node), "num anno");
  huAnnotation const * anno = huGetAnnotation(node, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "a key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "a key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "b val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "b val val");
  anno = huGetAnnotation(node, 1);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "c key len");
  STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, 1, "c key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "d val len");
  STRNCMP_EQUAL_TEXT("d", anno->value->str.ptr, 1, "d val val");
}


TEST_GROUP(oneValueListTwoAnnoGroup)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"([  foo @ {a : b
c : d} ])"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneValueListTwoAnnoGroup, values)
{
  huNode const * node = huGetRootNode(trove);
  node = huGetChildByIndex(node, 0);
  LONGS_EQUAL_TEXT(2, huGetNumAnnotations(node), "num anno");
  huAnnotation const * anno = huGetAnnotation(node, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "a key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "a key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "b val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "b val val");
  anno = huGetAnnotation(node, 1);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "c key len");
  STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, 1, "c key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "d val len");
  STRNCMP_EQUAL_TEXT("d", anno->value->str.ptr, 1, "d val val");
}


TEST_GROUP(oneValueDictFourAnno)
{
  huTrove const * trove = NULL;

  void setup()
  {
    auto humon = 
R"({  foo @ a : b :
  @ c : d @e:f bar @g:h })"sv;
    huMakeTroveFromStringN(& trove, humon.data(), humon.size(), NULL, HU_ERRORRESPONSE_STDERRANSICOLOR);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};

TEST(oneValueDictFourAnno, values)
{
  huNode const * node = huGetRootNode(trove);
  node = huGetChildByIndex(node, 0);
  LONGS_EQUAL_TEXT(4, huGetNumAnnotations(node), "num anno");
  huAnnotation const * anno = huGetAnnotation(node, 0);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "a key len");
  STRNCMP_EQUAL_TEXT("a", anno->key->str.ptr, 1, "a key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "b val len");
  STRNCMP_EQUAL_TEXT("b", anno->value->str.ptr, 1, "b val val");
  anno = huGetAnnotation(node, 1);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "c key len");
  STRNCMP_EQUAL_TEXT("c", anno->key->str.ptr, 1, "c key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "d val len");
  STRNCMP_EQUAL_TEXT("d", anno->value->str.ptr, 1, "d val val");
  anno = huGetAnnotation(node, 2);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "e key len");
  STRNCMP_EQUAL_TEXT("e", anno->key->str.ptr, 1, "e key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "f val len");
  STRNCMP_EQUAL_TEXT("f", anno->value->str.ptr, 1, "f val val");
  anno = huGetAnnotation(node, 3);
  LONGS_EQUAL_TEXT(1, anno->key->str.size, "g key len");
  STRNCMP_EQUAL_TEXT("g", anno->key->str.ptr, 1, "g key val");
  LONGS_EQUAL_TEXT(1, anno->value->str.size, "h val len");
  STRNCMP_EQUAL_TEXT("h", anno->value->str.ptr, 1, "h val val");
}
