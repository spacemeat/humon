#include "humon.hpp"
#include <iostream>
#include <cstring>
#include <charconv>

using namespace std;
using namespace hu;


void printUsage()
{
    cout << R"(Usage:
 hux <args>
  -pp print pretty  [default]
  -pr print preserved
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

If -pm is specified, the -m, -t, and -c arguments have no effect on the output.
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


int main(int argc, char ** argv)
{
    bool loadFromStdin = false;
    string inputFile;
    string outputFile;

    OutputFormat format = OutputFormat::pretty;
    bool printComments = true;
    int tabSize = 4;
    UsingColors usingColors = UsingColors::none;

    for (int i = 1; i < argc; ++i)
    {
        char * arg = argv[i];
        if (memcmp(arg, "-h", 2) == 0 ||
            memcmp(arg, "-?", 2) == 0)
            { 
                printUsage();
                return 0;
            }
        else if (memcmp(arg, "-pp", 3) == 0)
            { format = OutputFormat::pretty; }
        else if (memcmp(arg, "-pr", 3) == 0)
            { format = OutputFormat::preserved; }
        else if (memcmp(arg, "-pm", 3) == 0)
            { format = OutputFormat::minimal; }
        else if (memcmp(arg, "-p", 2) == 0)
        {
            cerr << "Invalid format argument for -p.\n";
            return 1;
        }

        else if (memcmp(arg, "-my", 3) == 0)
            { printComments = true; }
        else if (memcmp(arg, "-mn", 3) == 0)
            { printComments = false; }
        else if (memcmp(arg, "-m", 2) == 0)
        {
            cerr << "Invalid comment enablement argument for -m.\n";
            return 1;
        }
        
        else if (memcmp(arg, "-t", 2) == 0)
        {
            eatWs(arg);
            if(auto [p, ec] = std::from_chars(arg + 2, arg + 2 + strlen(arg + 2), tabSize, 10);
                ec != std::errc())
            {
                cerr << "Invalid tab size argument for -t.\n";
                return 1;
            }
        }

        else if (memcmp(arg, "-cn", 3) == 0)
            { usingColors = UsingColors::none; }
        else if (memcmp(arg, "-ca", 3) == 0)
            { usingColors = UsingColors::ansi; }
        else if (memcmp(arg, "-c", 2) == 0)
        {
            cerr << "Invalid color argument for -c.\n";
            return 1;
        }
        
        else if (memcmp(arg, "-o", 2) == 0)
        {
            eatWs(arg);
            outputFile = arg;
        }

        else if (memcmp(arg, "--", 2) == 0)
            { loadFromStdin = true; }

        else if (memcmp(arg, "-", 1) == 0)
            { cerr << "Invalid switch " << arg << ".\n"; return 1; }

        else
        {
            loadFromStdin = false;
            inputFile = arg;
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

    hu::ColorTable colorTable;
    std::string_view * colors = nullptr;
    if (usingColors == UsingColors::ansi)
    {
        getAnsiColorTable(colorTable);
        colors = colorTable.data();
    }

    std::string output;
    if (format == OutputFormat::preserved)
        { output = trove.toPreservedString(); }
    else if (format == OutputFormat::minimal)
        { output = trove.toMinimalString(printComments, "\n", colors); }
    else if (format == OutputFormat::pretty)
        { output = trove.toPrettyString(printComments, tabSize, "\n", colors); }    

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