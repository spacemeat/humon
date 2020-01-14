#include <CppUTest/TestHarness.h>
#include "humon2c.h"


TEST_GROUP(EmptyString)
{
  huTrove_t * trove;

  void setup()
  {
    auto humon = R"()";
    trove = huMakeTroveFromString("dataTests", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};


TEST(EmptyString, numTokens)
{
  LONGS_EQUAL(0, trove->tokens.numElements);
  LONGS_EQUAL(0, huGetNumTokens(trove));
}

TEST(EmptyString, numNodes)
{
  LONGS_EQUAL(0, trove->nodes.numElements);
  LONGS_EQUAL(0, huGetNumNodes(trove));
}

TEST(EmptyString, numErrors)
{
  LONGS_EQUAL(0, trove->errors.numElements);
  LONGS_EQUAL(0, huGetNumErrors(trove));
}

TEST(EmptyString, numComments)
{
  LONGS_EQUAL(0, trove->comments.numElements);
  LONGS_EQUAL(0, huGetNumTroveComments(trove));
}


TEST_GROUP(CommentsOnly)
{
  huTrove_t * trove;

  void setup()
  {
    auto humon = R"(// comment one
/* comment two */  /* comment
  three */ // comment four)";
    trove = huMakeTroveFromString("dataTests", humon, 2, 2);
  }

  void teardown()
  {
    if (trove)
      { huDestroyTrove(trove); }
  }
};


TEST(CommentsOnly, numTokens)
{
  LONGS_EQUAL(0, trove->tokens.numElements);
  LONGS_EQUAL(0, huGetNumTokens(trove));
}

TEST(CommentsOnly, numNodes)
{
  LONGS_EQUAL(0, trove->nodes.numElements);
  LONGS_EQUAL(0, huGetNumNodes(trove));
}

TEST(CommentsOnly, numErrors)
{
  LONGS_EQUAL(0, trove->errors.numElements);
  LONGS_EQUAL(0, huGetNumErrors(trove));
}

TEST(CommentsOnly, numComments)
{
  LONGS_EQUAL(4, trove->comments.numElements);
  LONGS_EQUAL(4, huGetNumTroveComments(trove));
}


