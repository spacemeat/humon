#include "humon.h"
#include <iostream>

using namespace std;
using namespace humon;

void test(string inp)
{
  cout << "Trying: " << inp << endl;
  auto h = fromString(inp);
  cout << "Results: " << endl << *h << endl << endl;
}

int main(int argc, char ** argv)
{
  test("x");
  test("[x]");
  test("{x:y}");
  test("[x y z]");
  test("[x, y, z]");
  test("{x:y z:w}");
  test("[[x0 x1] [y0 y1] [z0 z1]]");
  test("{x:{y:z} z:{w:foo}}");
  test("[aaa, {bbb: ccc ddd:eee fff:[ggg, hhh, iii] jjj: kkk} 1234567890 ]");

  test("\"xxx\"");
  test("[\"xxx\"]");
  test("{\"xxx\":\'chocobo'}");

  test("\n{\n  cats: rule // meow\n  dogs: drool // roof\n}\n");


  return 0;
}