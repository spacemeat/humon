#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include "humon/ansiColors.h"
#include "ztest.hpp"

using namespace std;


void printUsage()
{
    cout << R"(Usage: 
test <args>
 -h|?               print this usage document
 -cn                do not colorize output
 -v                 verbose output
 -ls                print a list of test source files
 -lg                print a list of test source:group names
 -lt                print a list of source:group.test names
 -s <sourcename>    include test sources with name containing 'sourcename'
 -g <groupname>     include test groups with name containing 'groupname'
 -t <testname>      include tests with name containing 'testname'
 -xs <sourcename>   exclude test sources with name containing 'sourcename'
 -xg <groupname>    exclude test groups with name containing 'groupname'
 -xt <testname>     exclude tests with name containing 'testname'

Multiple include/exclude (inex) arguments can be given. The first 
-s <sourcename> argument sets all other sources to be excluded and then 
'sourcename' is included. Subsequent -s arguments add sources to the run list.
The same logic applies to groups and tests.

For another example, pass '-xt pathological' to skip all pathological tests in
all groups in all sources.

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

std::string toString(long long val)
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


enum class ExpectedArgument
{
    cmdSwitch,
    name
};


enum class OperatingMode
{
    runTests,
    printSources,
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
            if (argMatches(arg, "-h") ||
                argMatches(arg, "-?"))
                { printUsage(); return 0; }
            else if (argMatches(arg, "-cn"))
                { useColor = false; }
            else if (argMatches(arg, "-v"))
                { verbose = true; }
            else if (argMatches(arg, "-ls"))
                { mode = OperatingMode::printSources; }
            else if (argMatches(arg, "-lg"))
                { mode = OperatingMode::printGroups; }
            else if (argMatches(arg, "-lt"))
                { mode = OperatingMode::printTests; }
            else if (argMatches(arg, "-s"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::includeSource;
            }
            else if (argMatches(arg, "-g"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::includeGroup;
            }
            else if (argMatches(arg, "-t"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::includeTest;
            }
            else if (argMatches(arg, "-xs"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::excludeSource;
            }
            else if (argMatches(arg, "-xg"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::excludeGroup;
            }
            else if (argMatches(arg, "-xt"))
            {
                expectedArg = ExpectedArgument::name;
                inexOp = InexOperation::excludeTest;
            }
            else
            {
                cerr << "Bad switch: " << arg << "\n";
                printUsage();
                return 1;
            }
            break;

        case ExpectedArgument::name:
            inexOps.push_back({inexOp, arg});
            expectedArg = ExpectedArgument::cmdSwitch;
            break;
        }
    }

    if (expectedArg != ExpectedArgument::cmdSwitch)
    {
        cerr << "Missing switch argument.\n";
        printUsage();
        return 1;
    }

    auto tests = getAllTests(inexOps);

    switch(mode)
    {
    case OperatingMode::printSources:
        {
            bool first = true;
            for (auto & [file, groups] : tests)
            {
                cout << (first ? "" : " ") << file;
                first = false;
            }
        }
        return 0;

    case OperatingMode::printGroups:
        {
            bool first = true;
            for (auto & [file, groups] : tests)
            {
                for (auto & [groupName, group] : groups)
                {
                    cout << (first ? "" : " ") << file << ":" << groupName;
                    first = false;
                }
            }
        }
        return 0;

    case OperatingMode::printTests:
        {
            bool first = true;
            for (auto & [file, groups] : tests)
            {
                for (auto & [groupName, group] : groups)
                {
                    for (auto & testName : group)
                    {
                        cout << (first ? "" : " ") << file << ":" << groupName << "." << testName;
                        first = false;
                    }
                }
            }
        }
        return 0;

    case OperatingMode::runTests:
        {
            int numFilesRun = 0;
            int numGroupsRun = 0;
            int numTestsRun = 0;
            int numTestsPassed = 0;
            int numTestsFailed = 0;
            int numChecksRun = 0;

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
                        auto test = runTest(inexOps, file, groupName, testName);
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
            
            return numTestsFailed == 0 ? 0 : 2;
        }
    }

    return 3;
}
