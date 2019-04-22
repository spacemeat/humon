#include "humon.h"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>
	
using namespace std;
using namespace humon;


string decolonize(string const & str)
{
  string ret;
  bool wasspace = false;
  bool inquotes = false;
  bool wasescape = false;
  for (auto ch : str)
  {
    if (isspace(ch))
    {
      if (inquotes)
      {
        ret.push_back(ch);
        wasescape = false;
        wasspace = false;
      }
      else
      {
        wasescape = false;
        if (wasspace)
          { continue; }
        else
        {
          wasspace = true;
          ret.push_back(' ');
        }
      }
    }
    else if (ch == '\\')
    {
      if (wasescape)
      {
        ret.push_back('\\');
        wasescape = false;
        wasspace = false;
      }
      else
      {
        wasescape = true;
        wasspace = false;
      }
    }
    else if (ch == '\"')
    {
      if (wasescape == false)
        { inquotes = ! inquotes; }
      
      wasescape = false;
      wasspace = false;
    }
    else
    {
      ret.push_back(ch);
      wasescape = false;
      wasspace = false;
    }    
  }

  return ret;
}


void testReport(string inp, string expect)
{
  cout << "Trying: " << endl << inp << endl;
  auto h = fromString(inp);
  auto rep = decolonize(h->getReport());
  bool pass = rep == expect;
  cout << "Results: " << endl << rep << endl << " ... " << (pass ? "PASS" : "FAIL") << endl;
}


int main(int argc, char ** argv)
{
  // These test reporting, but also general parsing and construction.
  testReport("x", "x ");
  testReport("[x]", "[ x ] ");
  testReport("{x:y}", "{ x: y } ");
  testReport("[x y z]", "[ x y z ] ");
  testReport("[x, y\\, z\\,, z]", "[ x y, z, z ] ");
  testReport("[x,y,z,z,]", "[ x y z z ] ");
  testReport("{x:y z:w}", "{ x: y z: w } ");
  testReport("[[x0 x1] [y0 y1] [z0 z1]]", "[ [ x0 x1 ] [ y0 y1 ] [ z0 z1 ] ] ");
  testReport("{x:{y:z} z:{w:foo}}", "{ x: { y: z } z: { w: foo } } ");
  testReport("[aaa, {bbb: ccc ddd:eee fff:[ggg, hhh, iii] jjj: kkk} 1234567890 ]", "[ aaa { bbb: ccc ddd: eee fff: [ ggg hhh iii ] jjj: kkk } 1234567890 ] ");

  testReport("\"xxx\"", "xxx ");
  testReport("[\"xxx\"]", "[ xxx ] ");
  testReport("{\"xxx\":'chocobo'}", "{ xxx: chocobo } ");

  testReport("\n{\n  cats: rule // meow\n  dogs: drool // roof\n}\n", "{ cats: rule dogs: drool } ");

  auto hup = fromString(R"(
  {
    voltron: {
      "lion force": [
        black
        red
        blue
        green
        yellow
      ]
      victory: assured
      score: 4.25
      "danger to humanity": false
      "puny human pilots": 5
    }
    devastator: {
      constructicons: [
        scrapper
        bonecrusher
        scavenger
        mixmaster
        hook
        "long haul"
      ]
      victory: assured
      score: 4.5
      "danger to humanity": true
      "puny human pilots": 0
    }
  }
  ))");

  auto const & hu = * hup.get();

  if (hu % "voltron")
    { cout << "hu%voltron ... PASS" << endl; }
  else
    { cout << "hu%voltron ... FAIL" << endl; }

  if (hu % "devastator")
    { cout << "hu%devastator ... PASS" << endl; }
  else
    { cout << "hu%devastator ... FAIL" << endl; }

  if (hu / "voltron" % "victory")
    { cout << "hu/voltron%victory ... PASS" << endl; }
  else
    { cout << "hu/voltron%victory ... FAIL" << endl; }

  if (hu / "voltron" % "vehicle force" == false)
    { cout << "hu/voltron%\"vehicle force\" ... PASS" << endl; }
  else
    { cout << "hu/voltron%\"vehicle force\" ... FAIL" << endl; }
  
  if ((hu % 0 && hu % 1 && !hu % 2))
    { cout << "hu%0, hu%1, ! hu%2 ... PASS" << endl; }
  else
    { cout << "hu%0, hu%1, ! hu%2 ... FAIL" << endl; }

  string leadLion = hu / "voltron" / "lion force" / 0;
  cout << "Lead lion: " << leadLion << " ... " << (leadLion == "black" ? "PASS" : "FAIL") << endl;

  long numPilots = hu / "voltron" / "puny human pilots";
  cout << "numPilots: " << numPilots << " ... " << (numPilots == 5 ? "PASS" : "FAIL") << endl;

  float howAwesome = hu / "devastator" / "score";
  cout << "howAwesome: " << howAwesome << " ... " << (howAwesome == 4.5 ? "PASS" : "FAIL") << endl;

  hu / "voltron" / "lion force" / 3 >> leadLion;
  cout << "Nerdy lion: " << leadLion << " ... " << (leadLion == "green" ? "PASS" : "FAIL") << endl;

  hu / "devastator" / "puny human pilots" >> numPilots;
  cout << "numPilots: " << numPilots << " ... " << (numPilots == 0 ? "PASS" : "FAIL") << endl;

  hu / "voltron" / "score" >> howAwesome;
  cout << "howAwesome: " << howAwesome << " ... " << (howAwesome == 4.25 ? "PASS" : "FAIL") << endl;

  return 0;
}