#include <iostream>
#include <cstring>
#include <charconv>
#include <optional>
#include <fmt/core.h>

#include "humon.hpp"

using namespace std;
using namespace hu;


void printUsage()
{
    cout << R"(Usage:
 hux <args>
  -px print xero
  -pm print minimal
  -pp print pretty  [default]

  -my do print comments  [default]
  -mn do not print comments
 
  -i <indentSize> use indentSize spaces for indentation  [default=4]
  -it use \t for indentation

  -cn do not use colors  [default]
  -ca use ansi colors

  -by do print utf8 BOM
  -bn do not print utf8 BOM [default]
 
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


[[nodiscard]] DeserializeResult getInput(istream & input)
{
    return Trove::fromIstream(input);
}


enum class ExpectedArgument
{
    cmdSwitch,
    outputFile,
    indentSize
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
    int indentSize = 4;
    bool indentWithTabs = false;
    UsingColors usingColors = UsingColors::none;
    bool printBom = false;

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
                { format = OutputFormat::xero; }
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

            else if (argMatches(arg, "-it"))
                { indentWithTabs = true; }
            else if (argMatches(arg, "-i"))
                { expectedArg = ExpectedArgument::indentSize; }
            else if (argStartsWith(arg, "-i"))
            {
                if(auto [p, ec] = std::from_chars(arg + 2, arg + argLen, indentSize, 10);
                    ec != std::errc())
                {
                    cerr << "Invalid indent size argument for -i.\n";
                    return 1;
                }
                indentWithTabs = false;
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

            else if (argMatches(arg, "-by"))
                { printBom = true; }
            else if (argMatches(arg, "-bn"))
                { printBom = false; }
            
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
        case ExpectedArgument::indentSize:
            if(auto [p, ec] = std::from_chars(arg, arg + strlen(arg), indentSize, 10);
                ec != std::errc())
            {
                cerr << "Invalid indentSize size argument for -i.\n";
                return 1;
            }
            expectedArg = ExpectedArgument::cmdSwitch;
            break;
        }
    }

    if (inputFile.size() == 0)
        { loadFromStdin = true; }

    // set up us the input
    DeserializeResult desRes;
    if (loadFromStdin)
    {
        desRes = getInput(cin);
    }
    else
    {
        auto in = ifstream (inputFile);
        if (in.is_open() == false)
            { cerr << "Could not open file " << inputFile << " for input.\n"; return 1; }

        desRes = getInput(in);
    }

    if (auto error = get_if<ErrorCode>(& desRes))
    {
        cerr << "Error loading the token stream: " << to_string(* error) << "\n";
        return 2;
    }

    Trove trove = move(get<Trove>(desRes));
    if (trove.numErrors() > 0)
    {
        cerr << "Trove has errors:\n";
        for (auto [ec, tok] : trove.errors())
        {
            cerr << inputFile << "(" << tok.line() << "): " << to_string(ec) << "\n";
        }
        return 2;
    }

    std::optional<ColorTable> colorTable;
    if (usingColors == UsingColors::ansi)
        { colorTable.emplace(std::move(getAnsiColorTable())); }

    std::string output;
    ErrorCode error = ErrorCode::noError;

    switch(format)
    {
    case OutputFormat::xero:
        {
            auto ret = trove.toXeroString(printBom);
            if (auto str = get_if<std::string>(& ret))
                { output = std::move(* str); }
            else
                { error = get<ErrorCode>(ret); }
        }
        break;
    case OutputFormat::minimal:
        {
            auto ret = trove.toMinimalString(colorTable, printComments, "\n", printBom);
            if (auto str = get_if<std::string>(& ret))
                { output = std::move(* str); }
            else
                { error = get<ErrorCode>(ret); }
        }
        break;
    case OutputFormat::pretty:
        {
            auto ret = trove.toPrettyString(indentSize, indentWithTabs, colorTable, printComments, "\n", printBom);
            if (auto && str = get_if<std::string>(& ret))
                { output = std::move(* str); }
            else
                { error = get<ErrorCode>(ret); }
        }
        break;
    }

    if (error != ErrorCode::noError)
    {
        cerr << "Error encountered during printing: " << to_string(error) << "\n";
    }
    else
    {
        if (outputFile == "")
        {
            cout << output;
        }
        else
        {
            auto out = ofstream(outputFile);
            out << output;
        }
    }
    
    return 0;
}
