#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"


void appendString(huVector * str, char const * addend, int size)
{
    char * dest = huGrowVector(str, size);
    if (dest != NULL)       // if we're only counting, dest is NULL
        { memcpy(dest, addend, size); } 
}


void appendWs(huVector * str, int numChars)
{
    char const spaces[] = "                "; // 16 spaces
    while (numChars > 16)
    {
        appendString(str, spaces, 16);
        numChars -= 16;
    }
    appendString(str, spaces, numChars);
}


void appendColor(huVector * str, huStringView const * colorTable, int colorCode)
{
    if (colorTable == NULL)
        { return; }
    huStringView const * color = colorTable + colorCode;
    appendString(str, color->str, color->size);
}


void endColor(huVector * str, huStringView const * colorTable)
{
    appendColor(str, colorTable, HU_COLORKIND_END);
}


void appendColoredString(huVector * str, char const * addend, int size, huStringView const * colorTable, int colorCode)
{
    appendColor(str, colorTable, colorCode);
    appendString(str, addend, size);
    appendColor(str, colorTable, HU_COLORKIND_END);
}


void printComment(huToken const * comment, huVector * str, huStringView const * colorTable)
{
    huStringView const * comstr = & comment->value;
    appendColoredString(str, comstr->str, comstr->size, 
        colorTable, HU_COLORKIND_COMMENT);
}


int printSameLineComments(huNode const * node, bool firstToken, int startingCommentIdx, huVector * str, huStringView const* colorTable)
{
    int iCom = startingCommentIdx;
    for (; iCom < huGetNumComments(node); ++iCom)
    {
        huToken const * comment = huGetComment(node, iCom);
        // If not firstToken, forego the position check. Handles enqueued comments before the end token.
        if (firstToken == false ||
                (comment->line == node->firstToken->line &&
                 comment->col < node->lastValueToken->col))
        {
            appendWs(str, 1);
            printComment(comment, str, colorTable);
        }
        else
            { break; }
    }

    return iCom;
}


void printAnnotations(huAnnotation const * annos, int numAnno, bool troveOwned, huVector * str, huStringView const * colorTable)
{
    if (numAnno == 0)
        { return; }
    
    if (troveOwned == false)
        { appendString(str, " ", 1); }

    appendColoredString(str, "@", 1, 
        colorTable, HU_COLORKIND_PUNCANNOTATE);

    if (numAnno > 1)
    {
        appendColoredString(str, "{", 1, 
            colorTable, HU_COLORKIND_PUNCANNOTATEDICT);
    }

    for (int iAnno = 0; iAnno < numAnno; ++iAnno)
    {
        huAnnotation const * anno = annos + iAnno;
        if (iAnno > 0)
            { appendWs(str, 1); }

        appendColoredString(str, anno->key->value.str, anno->key->value.size,
            colorTable, HU_COLORKIND_ANNOKEY);
        appendColoredString(str, ": ", 2,
            colorTable, HU_COLORKIND_PUNCANNOTATEKEYVALUESEP);
        appendColoredString(str, anno->value->value.str, anno->value->value.size,
            colorTable, HU_COLORKIND_ANNOVALUE);
    }

    if (numAnno > 1)
    {
        appendColoredString(str, "}", 1,
            colorTable, HU_COLORKIND_PUNCANNOTATEDICT);
    }
}


void printTroveAnnotations(huTrove const * trove, huVector * str, huStringView const * colorTable)
{
    if (huGetNumTroveAnnotations(trove) == 0)
        { return; }

    huAnnotation const * annos = huGetTroveAnnotation(trove, 0);
    printAnnotations(annos, huGetNumTroveAnnotations(trove), true, str, colorTable);
}


void printNodeAnnotations(huNode const * node, huVector * str, huStringView const * colorTable)
{
    if (huGetNumAnnotations(node) == 0)
        { return; }

    huAnnotation const * annos = huGetAnnotation(node, 0);
    printAnnotations(annos, huGetNumAnnotations(node), false, str, colorTable);
}


void printKey(huToken const * keyToken, huVector * str, huStringView const * colorTable)
{
    appendColoredString(str, keyToken->value.str, keyToken->value.size,
        colorTable, HU_COLORKIND_KEY);
    appendColoredString(str, ": ", 2,
        colorTable, HU_COLORKIND_PUNCKEYVALUESEP);
}


void printValue(huToken const * valueToken, huVector * str, huStringView const * colorTable)
{
    appendColoredString(str, valueToken->value.str, valueToken->value.size,
        colorTable, HU_COLORKIND_VALUE);
}


void troveToPrettyStringRec(huNode const * node, huVector * str, int depth, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable)
{
    bool lineIsDirty = false;

    // print preceeding comments
    int iCom = 0;
    for (; iCom < huGetNumComments(node); ++iCom)
    {
        huToken const * comment = huGetComment(node, iCom);
        if (comment->line < node->firstToken->line)
        {
            if (node->parentNodeIdx != -1)
                { appendString(str, "\n", 1); }
            appendWs(str, outputTabSize * depth);
            printComment(comment, str, colorTable);
            lineIsDirty = true;
        }
        else
            { break; }
    }
    
    huToken const * keyToken = huGetKey(node);

    // if node has a key, print key:
    if (keyToken != NULL)
    {
        appendString(str, "\n", 1);
        appendWs(str, outputTabSize * depth);

        printKey(keyToken, str, colorTable);
        lineIsDirty = true;
    }

    // print [ or {
    if (node->kind == HU_NODEKIND_LIST)
    {
        if (node->keyToken == NULL)
        {
            if (node->childIdx == -1)
            {
                if (lineIsDirty)
                    { appendString(str, "\n", 1); }
                else
                {
                    if (node->nodeIdx != 0)
                        { appendWs(str, 1); }
                }
            }
            else
            {
                if (lineIsDirty)
                    { appendString(str, "\n", 1); }
                else
                    { appendWs(str, 1); }
            }
        }
        appendColoredString(str, "[", 1, 
            colorTable, HU_COLORKIND_PUNCLIST);
    }
    else if (node->kind == HU_NODEKIND_DICT)
    {
        if (node->keyToken == NULL)
        {
            if (node->childIdx == -1)
            {
                if (lineIsDirty)
                    { appendString(str, "\n", 1); }
                else
                {
                    if (node->nodeIdx != 0)
                        { appendWs(str, 1); }
                }
            }
            else
            {
                if (lineIsDirty)
                    { appendString(str, "\n", 1); }
                else
                    { appendWs(str, 1); }
            }
        }
        appendColoredString(str, "{", 1, 
            colorTable, HU_COLORKIND_PUNCDICT);
    }

    if (node->kind == HU_NODEKIND_LIST ||
            node->kind == HU_NODEKIND_DICT)
    {
        // print annotations on one line
        printNodeAnnotations(node, str, colorTable);

        // print any same-line comments
        iCom = printSameLineComments(node, true, iCom, str, colorTable);

        // recursive calls
        int numCh = huGetNumChildren(node);
        for (int i = 0; i < numCh; ++i)
        {
            huNode const * chNode = huGetChildNodeByIndex(node, i);
            troveToPrettyStringRec(chNode, str, depth + 1, outputFormat, excludeComments, outputTabSize, colorTable);
        }

        // print ]
        if (node->kind == HU_NODEKIND_LIST)
        {
            appendString(str, "\n", 1);
            appendWs(str, outputTabSize * depth);
            appendColoredString(str, "]", 1, 
                colorTable, HU_COLORKIND_PUNCLIST);
        }
        else
        {
            appendString(str, "\n", 1);
            appendWs(str, outputTabSize * depth);
            appendColoredString(str, "}", 1, 
                colorTable, HU_COLORKIND_PUNCLIST);
        }
    }
    else if (node->kind == HU_NODEKIND_VALUE)
    {
        if (node->keyToken == NULL && 
                 (node->parentNodeIdx != -1 ||
                    node->firstToken != node->firstValueToken))
        {
            appendString(str, "\n", 1);
            appendWs(str, outputTabSize * depth);
        }
        printValue(node->firstValueToken, str, colorTable);
        printNodeAnnotations(node, str, colorTable);
    }

    // print post-object comments on same line
    printSameLineComments(node, false, iCom, str, colorTable);
}


void troveToPrettyString(huTrove const * trove, huVector * str, int outputFormat, bool excludeComments, int outputTabSize, huStringView const * colorTable)
{
    printTroveAnnotations(trove, str, colorTable);

    huNode const * nodes = (huNode const *) trove->nodes.buffer;

    if (nodes != NULL)
    {
        troveToPrettyStringRec(& nodes[0], str, 0, outputFormat, excludeComments, outputTabSize, colorTable);
        appendString(str, "\n", 1);
    }

    for (int iCom = 0; iCom < huGetNumTroveComments(trove); ++iCom)
    {
        huToken const * comment = huGetTroveComment(trove, iCom);
        printComment(comment, str, colorTable);
        appendString(str, "\n", 1);
    }

    //appendString(str, "\0", 1);
}

