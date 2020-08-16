#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include "ansiColors.h"
#include "utest.hpp"

using namespace std;


void printUsage()
{
    cout << R"(Usage: 
test <args>
 -cn                do not colorize output
 -v                 verbose output
 -lg                print a list of test group names
 -ln                print a list of group.test names
 -g <groupname>     include test groups with name containing 'groupname'
 -n <testname>      include tests with name containing 'testname'
 -xg <groupname>    exclude test groups with name containing 'groupname'
 -xn <testname>     exclude tests with name containing 'testname'

Multiple include/exclude (inex) arguments can be given. The first 
-g <groupname> argument sets all groups to be excluded, and 'groupname' is
then added. Subsequent -g arguments add groups to the run list. The same
logic applies to tests.

Returns 0 on successful test run with all tests passing.
Returns 1 when a bad parameter is given.
Returns 2 when tests fail.
)";
}


std::string toString(bool val)
{
    return std::to_string(val);
}

std::string toString(long val)
{
    return std::to_string(val);
}

std::string toString(double val)
{
    return std::to_string(val);
}

std::string toString(char const * val, long len)
{
    return std::string(val, val + std::min(len, 10L));
}

std::string toString(void * val)
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}


bool argMatches(char const * arg, char const * spec)
{
    int argLen = (int)strlen(arg);
    int specLen = (int)strlen(spec);
    return argLen == specLen && memcmp(arg, spec, specLen) == 0;
}


bool argStartsWith(char const * arg, char const * spec)
{
    int argLen = (int)strlen(arg);
    int specLen = (int)strlen(spec);
    return argLen >= specLen && memcmp(arg, spec, specLen) == 0;
}


enum class InexOperation
{
    excludeGroup,
    excludeTest,
    includeGroup,
    includeTest,
};


enum class ExpectedArgument
{
    cmdSwitch,
    name
};


enum class OperatingMode
{
    runTests,
    printGroups,
    printTests
};


int main(int argc, char *argv[])
{
    bool useColor = true;
    bool verbose = false;
    vector<pair<InexOperation, string>> inexOps;
    InexOperation inexOp;
    ExpectedArgument expectedArg = ExpectedArgument::cmdSwitch;
    OperatingMode mode = OperatingMode::runTests;

    int i = 1;
    for (; i < argc; ++i)
    {
        char * arg = argv[i];

        switch (expectedArg)
        {
        case ExpectedArgument::cmdSwitch:
            if (argMatches(arg, "-cn"))
                { useColor = false; }
            else if (argMatches(arg, "-v"))
                { verbose = true; }
            else if (argMatches(arg, "-lg"))
                { mode = OperatingMode::printGroups; }
            else if (argMatches(arg, "-ln"))
                { mode = OperatingMode::printTests; }
            else if (argMatches(arg, "-g"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::includeGroup;
            }
            else if (argMatches(arg, "-n"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::includeTest;
            }
            else if (argMatches(arg, "-xg"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::excludeGroup;
            }
            else if (argMatches(arg, "-xn"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::excludeTest;
            }
            else
            {
                cerr << "Bad parameter: " << arg << "\n";
                return 1;
            }
            break;

        case ExpectedArgument::name:
            inexOps.push_back({inexOp, arg});
            expectedArg = ExpectedArgument::cmdSwitch;
            break;
        }
    }

    switch(mode)
    {
    case OperatingMode::printGroups:
        {
            auto tests = getAllTests();
            bool first = true;
            for (auto & [file, groups] : tests)
            {
                for (auto & [groupName, group] : groups)
                {
                    // if g fails filters, continue;
                    cout << (first ? "" : " ") << groupName;
                    first = false;
                }
            }
        }
        break;
    case OperatingMode::printTests:
        {
            auto tests = getAllTests();
            bool first = true;
            for (auto & [file, groups] : tests)
            {
                for (auto & [groupName, group] : groups)
                {
                    for (auto & testName : group)
                    {
                        // if g fails filters, continue;
                        // if t fails filters, continue;
                        cout << (first ? "" : " ") << groupName << "." << testName;
                        first = false;
                    }
                }
            }
        }
        break;
    case OperatingMode::runTests:
        {
            int numFilesRun = 0;
            int numGroupsRun = 0;
            int numTestsRun = 0;
            int numTestsPassed = 0;
            int numTestsFailed = 0;
            int numChecksRun = 0;

            auto tests = getAllTests();
            for (auto & [file, groups] : tests)
            {
                // if file fails filters, continue;
                numFilesRun += 1;
                for (auto & [groupName, group] : groups)
                {
                    // if g fails filters, continue;
                    numGroupsRun += 1;
                    for (auto & testName : group)
                    {
                        // if t fails filters, continue;
                        numTestsRun += 1;
                        auto test = runTest(file, groupName, testName);
                        if (test.passed)
                        {
                            if (verbose)
                            {
                                if (useColor)
                                {
                                    cout << "[" << ansi_lightGreen << "PASS" 
                                         << ansi_off << "]: " << ansi_lightCyan << file 
                                         << ansi_off << " - " << ansi_lightBlue << groupName 
                                         << ansi_off << " - " << ansi_lightMagenta << testName 
                                         << ansi_off << "\n";
                                }
                                else
                                {
                                    cout << "[PASS]: " << file << " - " << groupName
                                         << " - " << testName << "\n";
                                }
                            }
                            else
                                { cout << "."; }
                            numTestsPassed += 1;
                        }
                        else
                        {
                            if (verbose == false)
                                { cout << "\n"; }
                            if (useColor)
                            {
                                cout << "[" << ansi_lightRed << "FAIL"  
                                        << ansi_off << "]: " << ansi_lightCyan << file 
                                        << ansi_off << " - " << ansi_lightBlue << groupName 
                                        << ansi_off << " - " << ansi_lightMagenta << testName;
                                if (test.checkThatFailed.size() > 0)
                                {
                                    cout << ansi_off << " - " << ansi_darkYellow << test.checkThatFailed;
                                }
                                cout << ansi_off << "\n";
                                if (test.expected.size() > 0)
                                {
                                    cout << "Expected " << test.expected << ", got " << test.got << "\n";
                                }
                                cout << test.file << ": (" << test.line << ")\n";
                            }
                            else
                            {
                                cout << "[FAIL]: " << file << " - " << groupName << " - " << testName;
                                if (test.checkThatFailed.size() > 0)
                                    { cout << " - " << test.checkThatFailed; }
                                cout << ansi_off << "\n";
                                if (test.expected.size() > 0)
                                    { cout << "Expected " << test.expected << ", got " << test.got << "\n"; }
                                cout << test.file << ": (" << test.line << ")\n";
                            }
                            numTestsFailed += 1;
                        }
                        numChecksRun += test.checksRun;
                    }
                }
            }

            if (useColor)
            {
                cout << ansi_off << "\nFiles run: " << ansi_lightCyan << numFilesRun 
                     << ansi_off << "    Groups run: " << ansi_lightBlue << numGroupsRun
                     << ansi_off << "    Tests run: " << ansi_lightMagenta << numTestsRun
                     << ansi_off << "    Checks run: " << ansi_darkYellow << numChecksRun
                     << ansi_off << "\n    Tests passed: " << ansi_lightGreen << std::setw(7) << numTestsPassed << std::setw(0) << ansi_off << " / " << numTestsRun
                     << ansi_off << "\n    Tests failed: " << (numTestsFailed > 0 ? ansi_lightRed : ansi_lightGreen) << std::setw(7) << numTestsFailed << std::setw(0) << ansi_off << " / " << numTestsRun
                     << ansi_off << "\n";
            }
            else
            {
                cout << "\nFiles run: " << numFilesRun 
                     << "    Groups run: " << numGroupsRun
                     << "    Tests run: " << numTestsRun
                     << "    Checks run: " << numChecksRun
                     << "\n    Tests passed: " << std::setw(7) << numTestsPassed << std::setw(0) << " / " << numTestsRun
                     << "\n    Tests failed: " << std::setw(7) << numTestsFailed << std::setw(0) << " / " << numTestsRun
                     << "\n";
            }
        }
        break;
    }
}
