#!/usr/bin/python3

import os
import re
from pprint import pprint

testGroupRe     = re.compile('\s*TEST_GROUP\((\w+)\)(.*)$')
testRe          = re.compile('\s*TEST\((\w+), (\w+)\)(.*)$')


def processFile(file):
    groups = {}
    fi = open(file, 'rt')
    lines = fi.readlines()
    for line in lines:
        tgm = testGroupRe.match(line)
        tm = testRe.match(line)
        if tgm:
            # test group
            groups[tgm.group(1)] = []
        elif tm:
            # test
            groups[tm.group(1)].append(tm.group(2))
    return groups
    

def processTestSrc(testDir):
    hppFiles = [f for f in os.listdir(testDir)
                  if os.path.isfile(f) and 
                     os.path.splitext(f)[1] == '.hpp' and
                     os.path.splitext(f)[0].endswith('Tests')]
    testData = {f: processFile(f) for f in hppFiles}
    #pprint (testData)
    return testData


def generateRunner(srcFile, testGroups):
    fi = open(f"ztest/gen-{srcFile[:-4]}.cpp", "wt")
    fi.write(f"""// AUTO-GENERATED FILE. Do not modify this file, or your changes
// will be lost and you will has a sad.

#include <vector>
#include <map>
#include <string>
#include "ztest.hpp"

#include "../{srcFile}"
""")

    srcFileBase = os.path.splitext(srcFile)[0]

    groups = []
    tests = {}
    groupNames = ""
    testNames = f'        {{ "{srcFile}", {{\n'
    runTestBody = ""

    for g, t in testGroups.items():
        tests[g] = t
        groups.append(g)
        groupNames += f'        "{g}",\n'
        
        testNames += f'            {{ "{g}", '
        qt = ', '.join([f'"{te}"' for te in t])
        testNames += f'{{ {qt} }}'
        testNames += f' }},\n'

        runTestBody += f'''    {"" if len(runTestBody) == 0 else "else "}if (groupName == "{g}")
    {{
'''
        for idx, test in enumerate(t):
            runTestBody += f'''        {"" if idx == 0 else "else "}if (testName == "{test}")
        {{
            Test_{g}_{test} test;
            test.setGroupAndTest("{srcFile}", groupName, testName);
            test.setup();
            test.runTest();
            test.teardown();
            return std::move(test);
        }}
'''

        runTestBody += f'''    }}
'''

    testNames += f'        }} }}'
    
    fi.write(f"""
SourceMap getAllTests_{srcFileBase}(InexOpsVector const & inexOps)
{{
    SourceMap filteredTests;
""")

    for g, t in testGroups.items():
        for tt in t:
            fi.write(f"""
    if (passesFilters(inexOps, "{srcFile}", "{g}", "{tt}"))
        {{ addToFilteredTests(filteredTests, "{srcFile}", "{g}", "{tt}"); }}
""")

    fi.write(f"""
    return filteredTests;
}}


TestGroup runTest_{srcFileBase}(std::string_view groupName, std::string_view testName)
{{
{runTestBody}
    return {{ }};
}}
""")



def generateRunnerHeader(testData):
    fi = open(f"ztest/gen-test-runners.h", "wt")
    fi.write(f"""// AUTO-GENERATED FILE. Do not modify this file, or your changes
// will be lost and you will think about it a lot.

#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <map>
#include <utility>
#include "ztest.hpp"

using namespace std;
""")

    for f, _ in testData.items():
        srcFileBase = os.path.splitext(f)[0]

        fi.write(f"""

SourceMap getAllTests_{srcFileBase}(
    InexOpsVector const & inexOps);
TestGroup runTest_{srcFileBase}(std::string_view groupName, std::string_view testName);
""")


def generateRunnerSource(testData):
    fi = open(f"ztest/gen-test-runners.cpp", "wt")
    fi.write(f"""// AUTO-GENERATED FILE. Do not modify this file, or your changes
// will be lost and you will feel things about it.

#include "gen-test-runners.h"


SourceMap getAllTests(
    InexOpsVector const & inexOps)
{{
    SourceMap uberMap;
""")
    first = True
    for f, _ in testData.items():
        srcFileBase = os.path.splitext(f)[0]
        fi.write(f"    {'auto ' if first else ''}map = getAllTests_{srcFileBase}(inexOps);\n    uberMap.insert(map.begin(), map.end());\n")
        first = False

    fi.write(f"""    return uberMap;
}}


TestGroup runTest(InexOpsVector const & inexOps, std::string_view file, std::string_view groupName, std::string_view testName)
{{
    if (passesFilters(inexOps, file, groupName, testName) == false)
        {{ return {{}}; }}

""")
    first = True
    for f, _ in testData.items():
        srcFileBase = os.path.splitext(f)[0]
        fi.write(f"""    {'' if first else 'else '}if (file == "{f}") {{ return runTest_{srcFileBase}(groupName, testName); }}
""")
        first = False

    fi.write(f"""    return {{}};
}}

""")


def generateRunners(testData):
    for f, _ in testData.items():
        generateRunner(f, testData[f])
    generateRunnerHeader(testData)
    generateRunnerSource(testData)


def main():
    testData = processTestSrc('.')
    generateRunners(testData)

if __name__ == "__main__":
    main()
