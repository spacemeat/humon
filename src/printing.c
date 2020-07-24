#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"
#include "ansiColors.h"


static void printUtf8Bom(PrintTracker * printer)
{
    char bom[] = { 0xef, 0xbb, 0xbf };
    appendToVector(printer->str, bom, 3);
}


void appendString(PrintTracker * printer, char const * addend, int size)
{
    appendToVector(printer->str, addend, size);
    printer->lastPrintWasIndent = false;
    printer->lastPrintWasNewline = false;
    printer->lastPrintWasWhitespace = false;
}


static void appendWs(PrintTracker * printer, int numChars)
{
    if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_MINIMAL)
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


static void appendTabs(PrintTracker * printer, int numTabs)
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
    if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_MINIMAL || printer->lastPrintWasIndent)
        { return; }
    if (printer->storeParams->indentWithTabs)
        { appendTabs(printer, printer->currentDepth); }
    else
        { appendWs(printer, printer->storeParams->indentSize * printer->currentDepth); }
    printer->lastPrintWasIndent = true;
    printer->lastPrintWasUnquotedWord = false;
    printer->lastPrintWasWhitespace = true;
}


static void appendNewline(PrintTracker * printer)
{
    if (printer->lastPrintWasNewline)
        { return; }
    appendString(printer, printer->storeParams->newline.ptr, printer->storeParams->newline.size);
    printer->lastPrintWasNewline = true;
    printer->lastPrintWasUnquotedWord = false;
    printer->lastPrintWasWhitespace = true;
}


static void appendColor(PrintTracker * printer, int colorCode)
{
    if (printer->storeParams->usingColors == false)
        { return; }
    huStringView const * color = printer->storeParams->colorTable + colorCode;
    appendString(printer, color->ptr, color->size);
}


static void appendColoredString(PrintTracker * printer, char const * addend, int size, int colorCode)
{
    appendColor(printer, colorCode);
    appendString(printer, addend, size);
    appendColor(printer, HU_COLORCODE_TOKENEND);
    printer->lastPrintWasUnquotedWord = false;
}


static void appendColoredToken(PrintTracker * printer, huToken const * tok, int colorCode)
{
    // prevent adjacent unquoted words from abutting
    if (printer->lastPrintWasUnquotedWord && tok->kind != HU_TOKENKIND_COMMENT)
        { appendWs(printer, 1); }
    appendColor(printer, colorCode);
    if (tok->quoteChar != '\0')
        { appendString(printer, & tok->quoteChar, 1); }
    appendString(printer, tok->str.ptr, tok->str.size);
    if (tok->quoteChar != '\0')
        { appendString(printer, & tok->quoteChar, 1); }
    appendColor(printer, HU_COLORCODE_TOKENEND);
    printer->lastPrintWasUnquotedWord = tok->quoteChar == '\0';
}


static void printForwardComment(PrintTracker * printer, huToken const * tok)
{
    if (printer->storeParams->printComments == false)
        { return; }
    appendIndent(printer);
    appendColoredToken(printer, tok, HU_COLORCODE_COMMENT);
    printer->lastPrintWasUnquotedWord = false;
    if (tok->str.ptr[1] == '/' || printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
        { appendNewline(printer); }
}


static void printTrailingComment(PrintTracker * printer, huToken const * tok)
{
    if (printer->storeParams->printComments == false)
        { return; }
    appendColoredToken(printer, tok, HU_COLORCODE_COMMENT);
    printer->lastPrintWasUnquotedWord = false;
    if (tok->str.ptr[1] == '/')
        { appendNewline(printer); }
}


static int printAllPrecedingComments(PrintTracker * printer, huNode const * node, huToken const * tok, int startingWith)
{
    int commentIdx = startingWith;
    int numComments = huGetNumComments(node);

    for (; commentIdx < numComments; ++commentIdx)
    {
        huToken const * comm = huGetComment(node, commentIdx);
        if (comm->line < tok->line ||
            (comm->line == tok->line && comm->col < tok->col))
        {
            if (commentIdx == startingWith && printer->storeParams->printComments)
                { appendNewline(printer); }
            printForwardComment(printer, comm);
        }
        else
            { break; } // commentIdx remains our comment cursor
    }

    return commentIdx;
}


static int printAllTrailingComments(PrintTracker * printer, huNode const * node, huToken const * tok, int startingWith)
{
    int commentIdx = startingWith;
    int numComments = huGetNumComments(node);

    for (; commentIdx < numComments; ++commentIdx)
    {
        huToken const * comm = huGetComment(node, commentIdx);
        if (comm->line == tok->line)
        {
            if (printer->storeParams->printComments &&
                printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
                { appendWs(printer, 1); }

            printTrailingComment(printer, comm);
        }
        else
            { break; } // commentIdx remains our comment cursor
    }

    return commentIdx;
}


static void printAnnotations(PrintTracker * printer, huVector const * annotations, bool isTroveAnnotations)
{
    int numAnnos = getVectorSize(annotations);
    if (numAnnos == 0)
        { return; }

    // if we're printing an annotation on a new line (because of a comment, say)
    if (printer->lastPrintWasNewline)
    {
        if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY && printer->currentDepth > 0)
        {
            printer->currentDepth += 1;
            appendIndent(printer);
            printer->currentDepth -= 1;
        }
    }
    else
    {
        if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY && 
            (printer->currentDepth > 0 || isTroveAnnotations == false))
            { appendWs(printer, 1); }
    }
    
    appendColoredString(printer, "@", 1, HU_COLORCODE_PUNCANNOTATE);
    if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
        { appendWs(printer, 1); }

    if (numAnnos > 1)
    {
        appendColoredString(printer, "{", 1, HU_COLORCODE_PUNCANNOTATEDICT);
        if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { appendWs(printer, 1); }
    }
    for (int annoIdx = 0; annoIdx < numAnnos; ++annoIdx)
    {
        if (annoIdx > 0 && printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { ensureWs(printer); }
        huAnnotation * anno = getVectorElement(annotations, annoIdx);
        appendColoredToken(printer, anno->key, HU_COLORCODE_ANNOKEY);
        appendColoredString(printer, ":", 1, HU_COLORCODE_PUNCANNOTATEKEYVALUESEP);
        if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { appendWs(printer, 1); }
        appendColoredToken(printer, anno->value, HU_COLORCODE_ANNOVALUE);        
    }
    if (numAnnos > 1)
    {
        if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
            { appendWs(printer, 1); }
        appendColoredString(printer, "}", 1, HU_COLORCODE_PUNCANNOTATEDICT);
    }

    printer->lastPrintWasIndent = false;
    printer->lastPrintWasNewline = false;
}


static void printNode(PrintTracker * printer, huNode const * node)
{    
    int numComments = huGetNumComments(node);
    int commentIdx = 0;

    //  print preceding comments
    commentIdx = printAllPrecedingComments(printer, node, node->valueToken, commentIdx);

    //  if parent is a dict
    //      print key
    if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
        { appendNewline(printer); }
    appendIndent(printer);
    huNode const * parentNode = huGetParentNode(node);

    // print key if we have one
    if (parentNode != hu_nullNode && parentNode->kind == HU_NODEKIND_DICT)
    {
        appendColoredToken(printer, node->keyToken, HU_COLORCODE_KEY);
        appendColoredString(printer, ":", 1, HU_COLORCODE_PUNCKEYVALUESEP);
        if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
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
        commentIdx = printAllTrailingComments(printer, node, node->valueToken, commentIdx);

        // print children
        printer->currentDepth += 1;
        huNode const * chNode = NULL;
        for (int chIdx = 0; chIdx < huGetNumChildren(node); ++chIdx)
        {
            chNode = huGetChildByIndex(node, chIdx);
            printNode(printer, chNode);
        }

        commentIdx = printAllPrecedingComments(printer, node, node->lastValueToken, commentIdx);
        
        printer->currentDepth -= 1;
        if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
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
        commentIdx = printAllTrailingComments(printer, node, node->valueToken, commentIdx);

        // print children
        printer->currentDepth += 1;
        huNode const * chNode = NULL;
        for (int chIdx = 0; chIdx < huGetNumChildren(node); ++chIdx)
        {
            chNode = huGetChildByIndex(node, chIdx);
            printNode(printer, chNode);
        }
        commentIdx = printAllPrecedingComments(printer, node, node->lastValueToken, commentIdx);
        printer->currentDepth -= 1;
        if (printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
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
    }
        
    //  print any same-line comments
    //  print comments preceding any annos
    //  print annos
    commentIdx = printAllTrailingComments(printer, node, node->lastValueToken, commentIdx);
    int startIdx = commentIdx;
    for (; commentIdx < numComments; ++commentIdx)
    {
        if (commentIdx == startIdx && printer->storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY && printer->storeParams->printComments)
            { appendNewline(printer); }
        huToken const * comm = huGetComment(node, commentIdx);
        {
            printForwardComment(printer, comm);
        }
    }

    // print annotations
    printAnnotations(printer, & node->annotations, false);
}


void troveToPrettyString(huTrove const * trove, huVector * str, huStoreParams * storeParams)
{
    PrintTracker printer = {
        .trove = trove,
        .str = str,
        .storeParams = storeParams,
        .currentDepth = 0,
        .lastPrintWasNewline = false,
        .lastPrintWasIndent = false,
        .lastPrintWasUnquotedWord = false,
        .lastPrintWasWhitespace = false
    };

    if (printer.storeParams->printBom)
    {
        printUtf8Bom(& printer);
    }

    appendColor(& printer, HU_COLORCODE_TOKENSTREAMBEGIN);

    // Print trove comments that precede the root node token; These are comments that appear before 
    // or amidst trove annotations, before the root. These will all be the first trove comments, so 
    // just start scumming from 0.
    int troveCommentIdx = 0;
    int numTroveComments = huGetNumTroveComments(trove);
    int numTroveAnnotations = huGetNumTroveAnnotations(trove);
    if (numTroveAnnotations > 0)
    {
        for (; troveCommentIdx < numTroveComments; ++troveCommentIdx)
        {
            huToken const * comm = huGetTroveComment(trove, troveCommentIdx);
            if (comm->line <= trove->lastAnnoToken->line)
            {
                // print comment
                printForwardComment(& printer, comm);
            }
            else
                { break; } // troveCommentIdx remains our trove comment cursor
        }
    }

    // Print trove annotations
    printAnnotations(& printer, & trove->annotations, true);

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

    if (printer.storeParams->WhitespaceFormat == HU_WHITESPACEFORMAT_PRETTY)
        { appendNewline(& printer); }
    
    appendColor(& printer, HU_COLORCODE_TOKENSTREAMEND);
}


static void setTableEntry(huStringView table[], int colorKind, char const * str)
{
    table[colorKind].ptr = str; table[colorKind].size = strlen(str);
}


void huFillAnsiColorTable(huStringView table[])
{
    setTableEntry(table, HU_COLORCODE_TOKENSTREAMBEGIN, "");
    setTableEntry(table, HU_COLORCODE_TOKENSTREAMEND, ansi_off);
    setTableEntry(table, HU_COLORCODE_TOKENEND, "");
    setTableEntry(table, HU_COLORCODE_PUNCLIST, ansi_white);
    setTableEntry(table, HU_COLORCODE_PUNCDICT, ansi_white);
    setTableEntry(table, HU_COLORCODE_PUNCKEYVALUESEP, ansi_white);
    setTableEntry(table, HU_COLORCODE_PUNCANNOTATE, ansi_darkBlue);
    setTableEntry(table, HU_COLORCODE_PUNCANNOTATEDICT, ansi_darkBlue);
    setTableEntry(table, HU_COLORCODE_PUNCANNOTATEKEYVALUESEP, ansi_darkBlue);
    setTableEntry(table, HU_COLORCODE_KEY, ansi_darkCyan);
    setTableEntry(table, HU_COLORCODE_VALUE, ansi_lightCyan);
    setTableEntry(table, HU_COLORCODE_COMMENT, ansi_darkGreen);
    setTableEntry(table, HU_COLORCODE_ANNOKEY, ansi_darkMagenta);
    setTableEntry(table, HU_COLORCODE_ANNOVALUE, ansi_lightMagenta);
    setTableEntry(table, HU_COLORCODE_WHITESPACE, ansi_darkGray);
}

