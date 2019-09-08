#include <iostream>
#include <algorithm>
#include <cctype>
#include <string>
#include "humon2.h"
#include "../../ansiTermCpp/inc/ansiTerm.h"
#include "../../../balls/fmt/fmt-5.3.0/include/fmt/core.h"


using namespace std;
using namespace humon;


vector<vector<string>> tests = {
  // input    minimal                     pretty
  { "x #y", 
    "x#y", "x", 
    "x #y", "x" },
  { "[x]", 
    "[x]", "[x]", 
    "[\n  x\n]", "[\n  x\n]" },
  { "#a\n[#b\nx#c\n]#d", 
    "#a\n[#b\nx#c\n]#d", "[x]", 
    "#a\n[ #b\n  x #c\n] #d", "[\n  x\n]" },
  { "{x:y}", 
    "{x:y}", "{x:y}", 
    "{\n  x: y\n}", "{\n  x: y\n}" },
  { "[x, y, z,, z]", 
    "[x y z z]", "[x y z z]",   
    "[\n  x\n  y\n  z\n  z\n]", "[\n  x\n  y\n  z\n  z\n]" },
  { "[[a b]]",
    "[[a b]]", "[[a b]]",
    "[[\n    a\n    b\n  ]\n]", "[[\n    a\n    b\n  ]\n]" },
  { "[a[b c]]",
    "[a[b c]]", "[a[b c]]", 
    "[\n  a\n  [\n    b\n    c\n  ]\n]", "[\n  a\n  [\n    b\n    c\n  ]\n]" },   
  { "[[b c]d]",
    "[[b c]d]", "[[b c]d]", 
    "[[\n    b\n    c\n  ]\n  d\n]", "[[\n    b\n    c\n  ]\n  d\n]" },
  { "[a[b c]d]",
    "[a[b c]d]", "[a[b c]d]", 
    "[\n  a\n  [\n    b\n    c\n  ]\n  d\n]", "[\n  a\n  [\n    b\n    c\n  ]\n  d\n]" },
  { "{a:[b c]d[e f]}",
    "{a:[b c]d[e f]}", "{a:[b c]d[e f]}",
    "{\n  a: [\n    b\n    c\n  ]\n  d\n  [\n    e\n    f\n  ]\n}", "{\n  a: [\n    b\n    c\n  ]\n  d\n  [\n    e\n    f\n  ]\n}" },
  { "{a:{b:c}d:{e:f}}",
    "{a:{b:c}d:{e:f}}", "{a:{b:c}d:{e:f}}", 
    "{\n  a: {\n    b: c\n  }\n  d: {\n    e: f\n  }\n}", "{\n  a: {\n    b: c\n  }\n  d: {\n    e: f\n  }\n}" },
  { "#x\n{#x\na#x\n:#x\n{#x\nb#x\n:#x\nc#x\n}#x\nd#x\n:#x\n{#x\ne#x\n:#x\nf#x\n}#x\n}#x\n",
    "#x\n{#x\na#x\n:#x\n{#x\nb#x\n:#x\nc#x\n}#x\nd#x\n:#x\n{#x\ne#x\n:#x\nf#x\n}#x\n}#x", "{a:{b:c}d:{e:f}}", 
    "#x\n{ #x\n  a #x\n  : #x\n  { #x\n    b #x\n    : #x\n    c #x\n  } #x\n  d #x\n  : #x\n  { #x\n    e #x\n    : #x\n    f #x\n  } #x\n} #x", "{\n  a: {\n    b: c\n  }\n  d: {\n    e: f\n  }\n}" },
  { R"({
  a: {
    b: c @ { d: e f: g }
    h: i @ j: k
    l: m
  }
})", 
    R"({a:{b:c@{d:e f:g}h:i@j:k l:m}})", 
    R"({a:{b:c@{d:e f:g}h:i@j:k l:m}})",
    R"({
  a: {
    b: c @{d: e f: g}
    h: i @j: k
    l: m
  }
})",
R"({
  a: {
    b: c @{d: e f: g}
    h: i @j: k
    l: m
  }
})"},
  { R"({
  a: {
    b: c #x
      @ #x
      { #x
        d #x
        : #x
        e #x
        f: g #x
      } #x
    h: i #x
    @ #x
    j #x
    : #x
    k #x
    l: m
  }
})", 
    R"({a:{b:c#x
@#x
{#x
d#x
:#x
e#x
f:g#x
}#x
h:i#x
@#x
j#x
:#x
k#x
l:m}})", R"({a:{b:c@{d:e f:g}h:i@j:k l:m}})",
    R"({
  a: {
    b: c #x
      @ #x
      { #x
        d #x
        : #x
        e #x
        f: g #x
      } #x
    h: i #x
      @ #x
      j #x
      : #x
      k #x
    l: m
  }
})",
    R"({
  a: {
    b: c @{d: e f: g}
    h: i @j: k
    l: m
  }
})" }
};


void testReport(string inp, OutputFormat format, bool includeComments, string expect)
{
  cout << fmt::format(
    R"(Trying with format {}{}{}, comments {}{}{}.... )",
      ansi::lightYellow, to_string(format), ansi::off, 
      ansi::lightYellow, includeComments ? "on" : "off", ansi::off
   );

  auto h = HumonObj::fromString(inp, "test");
  auto rep = h.to_string(format, includeComments);
  bool pass = rep == expect;

  if (pass)
  {
    cout << fmt::format(
R"(... {}PASS{}
)", 
      ansi::lightGreen, ansi::off);
  }
  else
  {
    cout << fmt::format(
R"(... {}FAIL{}
Input: 
{}{}{}
Expected output:
{}{}{}
Actual output:
{}{}{}
)",
      ansi::lightRed, ansi::off,
      ansi::darkDarkBlueBg, inp, ansi::off, 
      ansi::darkDarkYellowBg, expect, ansi::off, 
      ansi::darkDarkMagentaBg, rep, ansi::off);
  }
}


int main(int argc, char ** argv)
{
  // These test reporting, but also general parsing and construction.

  for (auto & testCase : tests)
  {
    testReport(testCase[0], OutputFormat::preserved, true, testCase[0]);
    testReport(testCase[0], OutputFormat::minimal, true, testCase[1]);
    testReport(testCase[0], OutputFormat::minimal, false, testCase[2]);
    testReport(testCase[0], OutputFormat::pretty, true, testCase[3]);
    testReport(testCase[0], OutputFormat::pretty, false, testCase[4]);
  }

/*
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
    { cout << "hu/voltron%%\"vehicle force\" ... PASS" << endl; }
  else
    { cout << "hu/voltron%%\"vehicle force\" ... FAIL" << endl; }
  
  if ((hu % 0 && hu % 1 && hu % 2 == false))
    { cout << "hu%0, hu%1, ! hu%2 ... PASS" << endl; }
  else
    { cout << "hu%0, hu%1, ! hu%2 ... FAIL" << endl; }

  string leadLion = hu / "voltron" / "lion force" / 0;
  cout << "Lead lion: " << leadLion << " ... " << (leadLion == "black" ? "PASS" : "FAIL") << endl;

  int numPilots = hu / "voltron" / "puny human pilots";
  cout << "numPilots: " << numPilots << " ... " << (numPilots == 5 ? "PASS" : "FAIL") << endl;

  float howAwesome = hu / "devastator" / "score";
  cout << "howAwesome: " << howAwesome << " ... " << (howAwesome == 4.5 ? "PASS" : "FAIL") << endl;

  bool devastation = hu / "devastator" / "danger to humanity";
  cout << "devastation: " << devastation << " ... " << (devastation == true ? "PASS" : "FAIL") << endl;

  hu / "voltron" / "lion force" / 3 >> leadLion;
  cout << "Nerdy lion: " << leadLion << " ... " << (leadLion == "green" ? "PASS" : "FAIL") << endl;

  hu / "devastator" / "puny human pilots" >> numPilots;
  cout << "numPilots: " << numPilots << " ... " << (numPilots == 0 ? "PASS" : "FAIL") << endl;

  hu / "voltron" / "score" >> howAwesome;
  cout << "howAwesome: " << howAwesome << " ... " << (howAwesome == 4.25 ? "PASS" : "FAIL") << endl;
*/
  return 0;
}