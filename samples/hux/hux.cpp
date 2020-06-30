#include "humon.hpp"
#include <iostream>
#include <cstring>
#include <charconv>
#include <optional>

using namespace std;
using namespace hu;


void printUsage()
{
    cout << R"(Usage:
 hux <args>
  -pp print pretty  [default]
  -px print xerographic
  -pm print minimal

  -my do print comments  [default]
  -mn do not print comments
 
  -t <tabSize> use tab size  [default=4]

  -cn do not use colors  [default]
  -ca use ansi colors
 
  -o <outputFile> output file

  -- use stdin [default]
  <intputFile> use file

Returns 0 on successful operation.
Returns 1 on bad command line arguments.
Returns 2 on bad / erroneous input.

If -px is specified, the output is a byte-for-byte clone of the original token
stream. As such, the -m, -t, and -c arguments have no effect on the output.

If -pm is specified, the output is minimized. As such, the -t argument has no 
effect on the output.

If -pp is specified, the output is pretty. Marvel at its beauty.
)";
}

enum class UsingColors
{
    none, ansi, html
};


void eatWs(char *& arg)
{
    while (*arg == ' ' ||
        *arg == '\t' ||
        *arg == '\n' ||
        *arg == '\r')
        { arg += 1; }
}


Trove getInput(istream & input)
{
    return Trove::fromIstream(input);
}


enum class ExpectedArgument
{
    cmdSwitch,
    outputFile,
    tabSize
};


bool argMatches(char const * arg, char const * spec)
{
    int argLen = strlen(arg);
    int specLen = strlen(spec);
    return argLen == specLen && memcmp(arg, spec, specLen) == 0;
}


bool argStartsWith(char const * arg, char const * spec)
{
    int argLen = strlen(arg);
    int specLen = strlen(spec);
    return argLen >= specLen && memcmp(arg, spec, specLen) == 0;
}


int main(int argc, char ** argv)
{
    bool loadFromStdin = false;
    string inputFile;
    string outputFile;

    OutputFormat format = OutputFormat::pretty;
    bool printComments = true;
    int tabSize = 4;
    UsingColors usingColors = UsingColors::none;

    ExpectedArgument expectedArg = ExpectedArgument::cmdSwitch;
   
    for (int i = 1; i < argc; ++i)
    {
        char * arg = argv[i];
        int argLen = strlen(arg);

        switch (expectedArg)
        {
        case ExpectedArgument::cmdSwitch:
            if (argMatches(arg, "-h") ||
                argMatches(arg, "-?"))
                { 
                    printUsage();
                    return 0;
                }
            else if (argMatches(arg, "-pp"))
                { format = OutputFormat::pretty; }
            else if (argMatches(arg, "-px"))
                { format = OutputFormat::xerographic; }
            else if (argMatches(arg, "-pm"))
                { format = OutputFormat::minimal; }
            else if (argStartsWith(arg, "-p"))
            {
                cerr << "Invalid format argument for -p.\n";
                return 1;
            }

            else if (argMatches(arg, "-my"))
                { printComments = true; }
            else if (argMatches(arg, "-mn"))
                { printComments = false; }
            else if (argStartsWith(arg, "-m"))
            {
                cerr << "Invalid comment enablement argument for -m.\n";
                return 1;
            }


            else if (argMatches(arg, "-t"))
                { expectedArg = ExpectedArgument::tabSize; }
            else if (argStartsWith(arg, "-t"))
            {
                if(auto [p, ec] = std::from_chars(arg + 2, arg + argLen, tabSize, 10);
                    ec != std::errc())
                {
                    cerr << "Invalid tab size argument for -t.\n";
                    return 1;
                }
            }

            else if (argMatches(arg, "-cn"))
                { usingColors = UsingColors::none; }
            else if (argMatches(arg, "-ca"))
                { usingColors = UsingColors::ansi; }
            else if (argStartsWith(arg, "-c"))
            {
                cerr << "Invalid color argument for -c.\n";
                return 1;
            }
            
            else if (argMatches(arg, "-o"))
                { expectedArg = ExpectedArgument::outputFile; }
            else if (argStartsWith(arg, "-o"))
                { outputFile = arg + 2; }

            else if (argMatches(arg, "--"))
                { loadFromStdin = true; }

            else if (argStartsWith(arg, "-"))
                { cerr << "Invalid switch " << arg << ".\n"; return 1; }
            else
            {
                loadFromStdin = false;
                inputFile = arg;
            }
            break;
        case ExpectedArgument::outputFile:
            outputFile = arg;
            expectedArg = ExpectedArgument::cmdSwitch;
            break;
        case ExpectedArgument::tabSize:
            if(auto [p, ec] = std::from_chars(arg, arg + strlen(arg), tabSize, 10);
                ec != std::errc())
            {
                cerr << "Invalid tab size argument for -t.\n";
                return 1;
            }
            expectedArg = ExpectedArgument::cmdSwitch;
            break;
        }
    }

    if (inputFile.size() == 0)
        { loadFromStdin = true; }

    // set up us the input
    Trove trove;
    if (loadFromStdin)
        { trove = getInput(cin); }
    else
    {
        auto in = ifstream (inputFile);
        if (in.is_open() == false)
            { cerr << "Could not open file " << inputFile << " for input.\n"; return 1; }

        trove = getInput(in);
    }

    std::optional<ColorTable> colorTable;
    if (usingColors == UsingColors::ansi)
        { colorTable.emplace(std::move(getAnsiColorTable())); }

    std::string output;
    switch(format)
    {
    case OutputFormat::xerographic:
        output = trove.toPreservedString();
        break;
    case OutputFormat::minimal:
        output = trove.toMinimalString(colorTable, printComments, "\n");
        break;
    case OutputFormat::pretty:
        output = trove.toPrettyString(tabSize, colorTable, printComments, "\n");
        break;
    }

    if (outputFile == "")
    {
        cout << output;
    }
    else
    {
        auto out = ofstream(outputFile);
        out << output;
    }

    if (trove.numErrors() > 0)
    {
        cerr << "Trove has errors:\n";
        for (auto [ec, tok] : trove.errors())
        {
            cerr << inputFile << "(" << tok.line() << "): " << to_string(ec) << "\n";
        }
        return 2;
    }

    return 0;
}