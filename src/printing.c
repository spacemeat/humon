#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"
#include "ansiColors.h"


void appendString(PrintTracker * printer, char const * addend, int size)
{
    huAppendToVector(printer->str, addend, size);
    printer->lastPrintWasIndent = false;
    printer->lastPrintWasNewline = false;
}


void appendWs(PrintTracker * printer, int numChars)
{
    if (printer->format == HU_OUTPUTFORMAT_MINIMAL)
        { numChars = max(numChars, 1); }

    char const spaces[] = "                "; // 16 spaces
    while (numChars > 16)
    {
        appendString(printer, spaces, 16);
        numChars -= 16;
    }
    appendString(printer, spaces, numChars);
}


void appendIndent(PrintTracker * printer)
{
    if (printer->format == HU_OUTPUTFORMAT_MINIMAL || printer->lastPrintWasIndent)
        { return; }
    appendWs(printer, printer->tabSize * printer->currentDepth);
    printer->lastPrintWasIndent = true;
}


void appendNewline(PrintTracker * printer)
{
    if (printer->lastPrintWasNewline)
        { return; }
    appendString(printer, printer->newline, printer->newlineSize);
    printer->lastPrintWasNewline = true;
}


void appendColor(PrintTracker * printer, int colorCode)
{
    if (printer->colorTable == NULL)
        { return; }
    huStringView const * color = printer->colorTable + colorCode;
    appendString(printer, color->str, color->size);
}


void appendColoredString(PrintTracker * printer, char const * addend, int size, int colorCode)
{
    appendColor(printer, colorCode);
    appendString(printer, addend, size);
    appendColor(printer, HU_COLORKIND_END);
}


void appendColoredToken(PrintTracker * printer, huToken const * tok, int colorCode)
{
    appendColor(printer, colorCode);
    if (tok->quoteChar != '\0')
        { appendString(printer, & tok->quoteChar, 1); }
    appendString(printer, tok->str.str, tok->str.size);
    if (tok->quoteChar != '\0')
        { appendString(printer, & tok->quoteChar, 1); }
    appendColor(printer, HU_COLORKIND_END);
}


void printForwardComment(PrintTracker * printer, huToken const * tok)
{
    if (printer->printComments == false)
        { return; }
    appendIndent(printer);
    appendColoredToken(printer, tok, HU_COLORKIND_COMMENT);
    appendNewline(printer);
}


void printTrailingComment(PrintTracker * printer, huToken const * tok)
{
    if (printer->printComments == false)
        { return; }
    appendColoredToken(printer, tok, HU_COLORKIND_COMMENT);
    if (tok->str.str[1] == '/')
        { appendNewline(printer); }
}


int printAllPrecedingComments(PrintTracker * printer, huNode const * node, huToken const * tok, int startingWith)
{
    int commentIdx = startingWith;
    int numComments = huGetNumComments(node);

    for (; commentIdx < numComments; ++commentIdx)
    {
        huToken const * comm = huGetComment(node, commentIdx);
        if (comm->line < tok->line ||
            (comm->line == tok->line && comm->col < tok->col))
        {
            if (printer->printComments)
                { appendNewline(printer); }
            printForwardComment(printer, comm);
        }
        else
            { break; } // commentIdx remains our comment cursor
    }

    return commentIdx;
}


int printAllTrailingComments(PrintTracker * printer, huNode const * node, huToken const * tok, int startingWith)
{
    int commentIdx = startingWith;
    int numComments = huGetNumComments(node);

    for (; commentIdx < numComments; ++commentIdx)
    {
        huToken const * comm = huGetComment(node, commentIdx);
        if (comm->line == tok->line)
        {
            if (printer->printComments &&
                printer->format == HU_OUTPUTFORMAT_PRETTY)
                { appendWs(printer, 1); }

            printTrailingComment(printer, comm);
        }
        else
            { break; } // commentIdx remains our comment cursor
    }

    return commentIdx;
}


void printAnnotations(PrintTracker * printer, huVector const * annotations)
{
    int numAnnos = huGetVectorSize(annotations);
    if (numAnnos == 0)
        { return; }

    // if we're printing an annotation on a new line (because of a comment, say)
    if (printer->lastPrintWasNewline)
    {
        if (printer->format == HU_OUTPUTFORMAT_PRETTY && printer->currentDepth > 0)
        {
            printer->currentDepth += 1;
            appendIndent(printer);
            printer->currentDepth -= 1;
        }
    }
    else
    {
        if (printer->format == HU_OUTPUTFORMAT_PRETTY && printer->currentDepth > 0)
            { appendWs(printer, 1); }
    }
    
    appendColoredString(printer, "@", 1, HU_COLORKIND_PUNCANNOTATE);
    if (printer->format == HU_OUTPUTFORMAT_PRETTY)
        { appendWs(printer, 1); }

    if (numAnnos > 1)
    {
        appendColoredString(printer, "{", 1, HU_COLORKIND_PUNCANNOTATEDICT);
        if (printer->format == HU_OUTPUTFORMAT_PRETTY)
            { appendWs(printer, 1); }
    }
    for (int annoIdx = 0; annoIdx < numAnnos; ++annoIdx)
    {
        if (annoIdx > 0)
            { appendColoredString(printer, " ", 1, HU_COLORKIND_WHITESPACE); }
        huAnnotation * anno = huGetVectorElement(annotations, annoIdx);
        appendColoredToken(printer, anno->key, HU_COLORKIND_ANNOKEY);
        appendColoredString(printer, ":", 1, HU_COLORKIND_PUNCANNOTATEKEYVALUESEP);
        if (printer->format == HU_OUTPUTFORMAT_PRETTY)
            { appendWs(printer, 1); }
        appendColoredToken(printer, anno->value, HU_COLORKIND_ANNOVALUE);        
    }
    if (numAnnos > 1)
    {
        if (printer->format == HU_OUTPUTFORMAT_PRETTY)
            { appendWs(printer, 1); }
        appendColoredString(printer, "}", 1, HU_COLORKIND_PUNCANNOTATEDICT);
    }

    printer->lastPrintWasIndent = false;
    printer->lastPrintWasNewline = false;
}


void printNode(PrintTracker * printer, huNode const * node)
{    
    int numComments = huGetNumComments(node);
    int commentIdx = 0;

    //  print preceding comments
    commentIdx = printAllPrecedingComments(printer, node, node->valueToken, commentIdx);

    //  if parent is a dict
    //      print key

    if (printer->format == HU_OUTPUTFORMAT_PRETTY)
        { appendNewline(printer); }
    appendIndent(printer);
    huNode const * parentNode = huGetParentNode(node);

    // print key if we have one
    if (parentNode != hu_nullNode && parentNode->kind == HU_NODEKIND_DICT)
    {
        appendColoredToken(printer, node->keyToken, HU_COLORKIND_KEY);
        appendColoredString(printer, ":", 1, HU_COLORKIND_PUNCKEYVALUESEP);
        if (printer->format == HU_OUTPUTFORMAT_PRETTY)
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
        appendColoredString(printer, "[", 1, HU_COLORKIND_PUNCLIST);
        
        commentIdx = printAllTrailingComments(printer, node, node->valueToken, commentIdx);

        // print children
        printer->currentDepth += 1;
        huNode const * chNode = NULL;
        for (int chIdx = 0; chIdx < huGetNumChildren(node); ++chIdx)
        {
            if (chIdx > 0 &&
                printer->format == HU_OUTPUTFORMAT_MINIMAL &&
                printer->lastPrintWasNewline == false &&
                chNode->kind == HU_NODEKIND_VALUE)
                { appendWs(printer, 1); }
            chNode = huGetChildByIndex(node, chIdx);
            printNode(printer, chNode);
        }

        commentIdx = printAllPrecedingComments(printer, node, node->lastValueToken, commentIdx);
        
        printer->currentDepth -= 1;
        if (printer->format == HU_OUTPUTFORMAT_PRETTY)
            { appendNewline(printer); }
        appendIndent(printer);
        appendColoredString(printer, "]", 1, HU_COLORKIND_PUNCLIST);
    }

    //  else if nodekind is dict
    //      print {
    //      print any same-line comments
    //      printNode()
    //      print comments preceding }
    //      print }
    else if (node->kind == HU_NODEKIND_DICT)
    {
        appendColoredString(printer, "{", 1, HU_COLORKIND_PUNCDICT);
        commentIdx = printAllTrailingComments(printer, node, node->valueToken, commentIdx);

        // print children
        printer->currentDepth += 1;
        huNode const * chNode = NULL;
        for (int chIdx = 0; chIdx < huGetNumChildren(node); ++chIdx)
        {
            if (chIdx > 0 &&
                printer->format == HU_OUTPUTFORMAT_MINIMAL &&
                printer->lastPrintWasNewline == false &&
                chNode->kind == HU_NODEKIND_VALUE)
                { appendWs(printer, 1); }
            chNode = huGetChildByIndex(node, chIdx);
            printNode(printer, chNode);
        }

        commentIdx = printAllPrecedingComments(printer, node, node->lastValueToken, commentIdx);
        printer->currentDepth -= 1;
        if (printer->format == HU_OUTPUTFORMAT_PRETTY)
            { appendNewline(printer); }
        appendIndent(printer);
        appendColoredString(printer, "}", 1, HU_COLORKIND_PUNCDICT);
    }

    //  else if nodekind is value
    //      if parent is a dict
    //          print key
    //      print value
    else if (node->kind == HU_NODEKIND_VALUE)
    {
        appendColoredToken(printer, node->valueToken, HU_COLORKIND_VALUE);
    }
        
    //  print any same-line comments
    //  print comments preceding any annos
    //  print annos

    commentIdx = printAllTrailingComments(printer, node, node->lastValueToken, commentIdx);

    for (; commentIdx < numComments; ++commentIdx)
    {
        huToken const * comm = huGetComment(node, commentIdx);
        {
            printForwardComment(printer, comm);
        }
    }

    // print annotations
    printAnnotations(printer, & node->annotations);
}


void troveToPrettyString(huTrove const * trove, huVector * str, int outputFormat, bool printComments, int outputTabSize, char const * newline, int newlineSize, huStringView const * colorTable)
{
    PrintTracker printer = {
        .trove = trove,
        .str = str,
        .format = outputFormat,
        .printComments = printComments,
        .tabSize = outputTabSize,
        .newline = newline,
        .newlineSize = newlineSize,
        .colorTable = colorTable,
        .currentDepth = 0,
        .currentLine = 1,
        .currentCol = 1,
        .lastPrintWasNewline = false    // or should it say, 'needs indent'?
    };

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
    printAnnotations(& printer, & trove->annotations);

    // print root node
    if (trove->nodes.numElements > 0)
    {
        printNode(& printer, huGetRootNode(trove));
    }

    // Print trove comments that we missed above.
    for (; troveCommentIdx < numTroveComments; ++troveCommentIdx)
    {
        huToken const * comm = huGetTroveComment(trove, troveCommentIdx);
        if (comm->line <= trove->lastAnnoToken->line)
        {
            // print comment
            printForwardComment(& printer, comm);
        }
    }
}


void setTableEntry(huStringView table[], int colorKind, char const * str)
{
    table[colorKind].str = str; table[colorKind].size = strlen(str);
}


void huFillAnsiColorTable(huStringView table[])
{
    setTableEntry(table, HU_COLORKIND_NONE, darkGray);
    setTableEntry(table, HU_COLORKIND_END, "");
    setTableEntry(table, HU_COLORKIND_PUNCLIST, white);
    setTableEntry(table, HU_COLORKIND_PUNCDICT, white);
    setTableEntry(table, HU_COLORKIND_PUNCKEYVALUESEP, white);
    setTableEntry(table, HU_COLORKIND_PUNCANNOTATE, darkBlue);
    setTableEntry(table, HU_COLORKIND_PUNCANNOTATEDICT, darkBlue);
    setTableEntry(table, HU_COLORKIND_PUNCANNOTATEKEYVALUESEP, darkBlue);
    setTableEntry(table, HU_COLORKIND_KEY, darkCyan);
    setTableEntry(table, HU_COLORKIND_VALUE, lightCyan);
    setTableEntry(table, HU_COLORKIND_COMMENT, darkGreen);
    setTableEntry(table, HU_COLORKIND_ANNOKEY, darkMagenta);
    setTableEntry(table, HU_COLORKIND_ANNOVALUE, lightMagenta);
    setTableEntry(table, HU_COLORKIND_WHITESPACE, darkGray);
}

