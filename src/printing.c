#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"
#include "humon/ansiColors.h"


static void printUtf8Bom(PrintTracker * printer)
{
    char bom[] = { 0xef, 0xbb, 0xbf };
    appendToVector(printer->str, bom, 3);
}


void appendString(PrintTracker * printer, char const * addend, huSize_t size)
{
    appendToVector(printer->str, addend, size);
    printer->lastPrintWasIndent = false;
    printer->lastPrintWasNewline = false;
    printer->lastPrintWasWhitespace = false;
}


static void appendColor(PrintTracker * printer, huColorCode colorCode)
{
    if (printer->serializeOptions->usingColors == false)
        { return; }
    huStringView const * color = printer->serializeOptions->colorTable + colorCode;
    appendToVector(printer->str, color->ptr, color->size);
}


static void appendWs(PrintTracker * printer, huSize_t numChars)
{
    if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_MINIMAL)
        { numChars = max(numChars, 1); }

    char const spaces[] = "                "; // 16 spaces
    while (numChars > 16)
    {
        appendString(printer, spaces, 16);
        numChars -= 16;
    }
    appendString(printer, spaces, numChars);
    printer->lastPrintWasUnquotedWord = false;
    printer->lastPrintWasWhitespace = true;
}


static void ensureWs(PrintTracker * printer)
{
    if (printer->lastPrintWasWhitespace == false)
        { appendWs(printer, 1); }
}


static void appendTabs(PrintTracker * printer, huSize_t numTabs)
{
    char const tabs[] = "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"; // 16 tabs
    while (numTabs > 16)
    {
        appendString(printer, tabs, 16);
        numTabs -= 16;
    }
    appendString(printer, tabs, numTabs);
    printer->lastPrintWasUnquotedWord = false;
    printer->lastPrintWasWhitespace = true;
}


static void appendIndent(PrintTracker * printer)
{
    if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_MINIMAL || printer->lastPrintWasIndent)
        { return; }
    if (printer->serializeOptions->indentWithTabs)
        { appendTabs(printer, printer->currentDepth); }
    else
        { appendWs(printer, printer->serializeOptions->indentSize * printer->currentDepth); }
    printer->lastPrintWasIndent = true;
    printer->lastPrintWasUnquotedWord = false;
    printer->lastPrintWasWhitespace = true;
}


static void appendNewline(PrintTracker * printer)
{
    if (printer->lastPrintWasNewline)
        { return; }
    appendString(printer, printer->serializeOptions->newline.ptr, printer->serializeOptions->newline.size);
    printer->lastPrintWasNewline = true;
    printer->lastPrintWasUnquotedWord = false;
    printer->lastPrintWasWhitespace = true;
}


static void appendColoredString(PrintTracker * printer, char const * addend, huSize_t size, huColorCode colorCode)
{
    appendColor(printer, colorCode);
    appendString(printer, addend, size);
    appendColor(printer, HU_COLORCODE_TOKENEND);
    printer->lastPrintWasUnquotedWord = false;
}


static void appendColoredToken(PrintTracker * printer, huToken const * tok, huColorCode colorCode)
{
    // prevent adjacent unquoted words from abutting
    if (printer->lastPrintWasUnquotedWord && tok->kind != HU_TOKENKIND_COMMENT)
        { appendWs(printer, 1); }
    appendColor(printer, colorCode);
    appendString(printer, tok->rawStr.ptr, tok->rawStr.size);
    appendColor(printer, HU_COLORCODE_TOKENEND);
    printer->lastPrintWasUnquotedWord = tok->quoteChar == '\0';
}


static void printForwardComment(PrintTracker * printer, huToken const * tok)
{
    if (printer->serializeOptions->printComments == false)
        { return; }
    appendIndent(printer);
    appendColoredToken(printer, tok, HU_COLORCODE_COMMENT);
    printer->lastPrintWasUnquotedWord = false;
    if (tok->rawStr.ptr[1] == '/' || printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
        { appendNewline(printer); }
}


static void printTrailingComment(PrintTracker * printer, huToken const * tok)
{
    if (printer->serializeOptions->printComments == false)
        { return; }
    appendColoredToken(printer, tok, HU_COLORCODE_COMMENT);
    printer->lastPrintWasUnquotedWord = false;
    if (tok->rawStr.ptr[1] == '/')
        { appendNewline(printer); }
}


static huSize_t printAllPrecedingComments(PrintTracker * printer, huNode const * node, huToken const * tok, huSize_t startingWith)
{
    huSize_t commentIdx = startingWith;
    huSize_t numComments = huGetNumComments(node);

    for (; commentIdx < numComments; ++commentIdx)
    {
        huToken const * comm = huGetComment(node, commentIdx);
        if (comm->line < tok->line ||
            (comm->line == tok->line && comm->col < tok->col))
        {
            if (commentIdx == startingWith && printer->serializeOptions->printComments)
                { appendNewline(printer); }
            printForwardComment(printer, comm);
        }
        else
            { break; } // commentIdx remains our comment cursor
    }

    return commentIdx;
}


static huSize_t printAllTrailingComments(PrintTracker * printer, huNode const * node, huToken const * tok, huSize_t startingWith)
{
    huSize_t commentIdx = startingWith;
    huSize_t numComments = huGetNumComments(node);

    for (; commentIdx < numComments; ++commentIdx)
    {
        huToken const * comm = huGetComment(node, commentIdx);
        if (comm->line == tok->line)
        {
            if (printer->serializeOptions->printComments &&
                printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
                { appendWs(printer, 1); }

            printTrailingComment(printer, comm);
        }
        else
            { break; } // commentIdx remains our comment cursor
    }

    return commentIdx;
}


static void printMetatags(PrintTracker * printer, huVector const * metatags, bool isTroveMetatags)
{
    huSize_t numAnnos = getVectorSize(metatags);
    if (numAnnos == 0)
        { return; }

    // if we're printing an metatag on a new line (because of a comment, say)
    if (printer->lastPrintWasNewline)
    {
        if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY && printer->currentDepth > 0)
        {
            printer->currentDepth += 1;
            appendIndent(printer);
            printer->currentDepth -= 1;
        }
    }
    else
    {
        if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY &&
            (printer->currentDepth > 0 || isTroveMetatags == false))
            { appendWs(printer, 1); }
    }

    appendColoredString(printer, "@", 1, HU_COLORCODE_PUNCMETATAG);
    if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
        { appendWs(printer, 1); }

    if (numAnnos > 1)
    {
        appendColoredString(printer, "{", 1, HU_COLORCODE_PUNCMETATAGDICT);
        if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { appendWs(printer, 1); }
    }
    for (huSize_t metatagIdx = 0; metatagIdx < numAnnos; ++metatagIdx)
    {
        if (metatagIdx > 0 && printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { ensureWs(printer); }
        huMetatag * metatag = getVectorElement(metatags, metatagIdx);
        appendColoredToken(printer, metatag->key, HU_COLORCODE_METATAGKEY);
        appendColoredString(printer, ":", 1, HU_COLORCODE_PUNCMETATAGKEYVALUESEP);
        if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { appendWs(printer, 1); }
        appendColoredToken(printer, metatag->value, HU_COLORCODE_METATAGVALUE);
    }
    if (numAnnos > 1)
    {
        if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { appendWs(printer, 1); }
        appendColoredString(printer, "}", 1, HU_COLORCODE_PUNCMETATAGDICT);
    }

    printer->lastPrintWasIndent = false;
    printer->lastPrintWasNewline = false;
}


static void printNode(PrintTracker * printer, huNode const * node)
{
    huSize_t numComments = huGetNumComments(node);
    huSize_t commentIdx = 0;

    //  print preceding comments
    commentIdx = printAllPrecedingComments(printer, node, node->valueToken, commentIdx);

    //  if parent is a dict
    //      print key
    if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
        { appendNewline(printer); }
    appendIndent(printer);
    huNode const * parentNode = huGetParent(node);

    // print key if we have one
    if (parentNode != HU_NULLNODE && parentNode->kind == HU_NODEKIND_DICT)
    {
        appendColoredToken(printer, node->keyToken, HU_COLORCODE_KEY);
        appendColoredString(printer, ":", 1, HU_COLORCODE_PUNCKEYVALUESEP);
        if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { appendWs(printer, 1); }
    }

    //  if nodekind is list
    //      print [
    //      print any same-line comments
    //      printNode()
    //      print comments preceding ]
    //      print ]
    if (node->kind == HU_NODEKIND_LIST)
    {
        appendColoredString(printer, "[", 1, HU_COLORCODE_PUNCLIST);
        printMetatags(printer, & node->metatags, false);
        commentIdx = printAllTrailingComments(printer, node, node->valueToken, commentIdx);

        // print children
        printer->currentDepth += 1;
        huNode const * chNode = NULL;
        for (huSize_t chIdx = 0; chIdx < huGetNumChildren(node); ++chIdx)
        {
            chNode = huGetChildByIndex(node, chIdx);
            printNode(printer, chNode);
        }

        commentIdx = printAllPrecedingComments(printer, node, node->lastValueToken, commentIdx);

        printer->currentDepth -= 1;
        if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { appendNewline(printer); }
        appendIndent(printer);
        appendColoredString(printer, "]", 1, HU_COLORCODE_PUNCLIST);
    }

    //  else if nodekind is dict
    //      print {
    //      print any same-line comments
    //      printNode()
    //      print comments preceding }
    //      print }
    else if (node->kind == HU_NODEKIND_DICT)
    {
        appendColoredString(printer, "{", 1, HU_COLORCODE_PUNCDICT);
        printMetatags(printer, & node->metatags, false);
        commentIdx = printAllTrailingComments(printer, node, node->valueToken, commentIdx);

        // print children
        printer->currentDepth += 1;
        huNode const * chNode = NULL;
        for (huSize_t chIdx = 0; chIdx < huGetNumChildren(node); ++chIdx)
        {
            chNode = huGetChildByIndex(node, chIdx);
            printNode(printer, chNode);
        }
        commentIdx = printAllPrecedingComments(printer, node, node->lastValueToken, commentIdx);
        printer->currentDepth -= 1;
        if (printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { appendNewline(printer); }
        appendIndent(printer);
        appendColoredString(printer, "}", 1, HU_COLORCODE_PUNCDICT);
    }

    //  else if nodekind is value
    //      if parent is a dict
    //          print key
    //      print value
    else if (node->kind == HU_NODEKIND_VALUE)
    {
        appendColoredToken(printer, node->valueToken, HU_COLORCODE_VALUE);
        printMetatags(printer, & node->metatags, false);
    }

    //  print any same-line comments
    //  print comments preceding any metatags
    //  print metatags
    commentIdx = printAllTrailingComments(printer, node, node->lastValueToken, commentIdx);
    huSize_t startIdx = commentIdx;
    for (; commentIdx < numComments; ++commentIdx)
    {
        if (commentIdx == startIdx && printer->serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY && printer->serializeOptions->printComments)
            { appendNewline(printer); }
        huToken const * comm = huGetComment(node, commentIdx);
        {
            printForwardComment(printer, comm);
        }
    }
}


void troveToPrettyString(huTrove const * trove, huVector * str, huSerializeOptions * serializeOptions)
{
    PrintTracker printer = {
        .trove = trove,
        .str = str,
        .serializeOptions = serializeOptions,
        .currentDepth = 0,
        .lastPrintWasNewline = true,
        .lastPrintWasIndent = false,
        .lastPrintWasUnquotedWord = false,
        .lastPrintWasWhitespace = false
    };

    if (printer.serializeOptions->printBom)
    {
        printUtf8Bom(& printer);
    }

    appendColor(& printer, HU_COLORCODE_TOKENSTREAMBEGIN);

    // Print trove comments that precede the root node token; These are comments that appear before
    // or amidst trove metatags, before the root. These will all be the first trove comments, so
    // just start scumming from 0.
    huSize_t troveCommentIdx = 0;
    huSize_t numTroveComments = huGetNumTroveComments(trove);
    huSize_t numTroveMetatags = huGetNumTroveMetatags(trove);
    if (numTroveMetatags > 0)
    {
        for (; troveCommentIdx < numTroveComments; ++troveCommentIdx)
        {
            huToken const * comm = huGetTroveComment(trove, troveCommentIdx);
            if (comm->line <= trove->lastMetatagToken->line)
            {
                // print comment
                printForwardComment(& printer, comm);
            }
            else
                { break; } // troveCommentIdx remains our trove comment cursor
        }
    }

    // Print trove metatags
    printMetatags(& printer, & trove->metatags, true);

    // print root node
    if (trove->nodes.numElements > 0)
    {
        printNode(& printer, huGetRootNode(trove));
    }

    // Print trove comments that we missed above.
    if (troveCommentIdx < numTroveComments)
        { appendNewline(& printer); }
    for (; troveCommentIdx < numTroveComments; ++troveCommentIdx)
    {
        huToken const * comm = huGetTroveComment(trove, troveCommentIdx);
        printForwardComment(& printer, comm);
    }

    if (printer.serializeOptions->whitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
        { appendNewline(& printer); }

    appendColor(& printer, HU_COLORCODE_TOKENSTREAMEND);
}


static void setTableEntry(huStringView table[], huColorCode colorKind, char const * str)
{
    table[(size_t) colorKind].ptr = str;
    table[(size_t) colorKind].size = (huSize_t) strlen(str);
}


void huFillAnsiColorTable(huStringView table[])
{
    setTableEntry(table, HU_COLORCODE_TOKENSTREAMBEGIN, "");
    setTableEntry(table, HU_COLORCODE_TOKENSTREAMEND, ansi_off);
    setTableEntry(table, HU_COLORCODE_TOKENEND, "");
    setTableEntry(table, HU_COLORCODE_PUNCLIST, ansi_white);
    setTableEntry(table, HU_COLORCODE_PUNCDICT, ansi_white);
    setTableEntry(table, HU_COLORCODE_PUNCKEYVALUESEP, ansi_white);
    setTableEntry(table, HU_COLORCODE_PUNCMETATAG, ansi_darkBlue);
    setTableEntry(table, HU_COLORCODE_PUNCMETATAGDICT, ansi_darkBlue);
    setTableEntry(table, HU_COLORCODE_PUNCMETATAGKEYVALUESEP, ansi_darkBlue);
    setTableEntry(table, HU_COLORCODE_KEY, ansi_darkCyan);
    setTableEntry(table, HU_COLORCODE_VALUE, ansi_lightCyan);
    setTableEntry(table, HU_COLORCODE_COMMENT, ansi_darkGreen);
    setTableEntry(table, HU_COLORCODE_METATAGKEY, ansi_darkMagenta);
    setTableEntry(table, HU_COLORCODE_METATAGVALUE, ansi_lightMagenta);
    setTableEntry(table, HU_COLORCODE_WHITESPACE, ansi_darkGray);
}

