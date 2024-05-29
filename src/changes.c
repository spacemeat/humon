#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "humon.internal.h"


static huSize_t changeTrove(huChangeSet * changeSet, huNode const * node, huChangeKind changeKind, huSize_t childIdx, char const * replacementString, huSize_t replacementStringLength)
{
    if (changeKind == HU_CHANGEKIND_INSERT)
    {
        if (childIdx > huGetNumChildren(node))
            { return -1; }
    }
    else
    {
        if (childIdx >= huGetNumChildren(node))
            { return -1; }
    }

    huSize_t num = 1;
    huChange * newChange = growVector(& changeSet->changes, & num);
    if (num == 0)
        { return -1; }

    num = replacementStringLength;
    char * newString = growVector(& changeSet->changeStrings, & num);
    if (num == 0)
    {
        shrinkVector(& changeSet->changes, 1);
        return -1;
    }

    memcpy(newString, replacementString, replacementStringLength);

    newChange->changeKind = changeKind;
    newChange->nodeIdx = node->nodeIdx;
    newChange->childIdx = childIdx;
    newChange->newString.ptr = newString;
    newChange->newString.size = replacementStringLength;

    return changeSet->changes.numElements;
}


void huInitChangeSet(huChangeSet * changeSet, huAllocator const * allocator)
{
    initGrowableVector(& changeSet->changes, sizeof(huChange), allocator);
    initGrowableVector(& changeSet->changeStrings, sizeof(char), allocator);
}


void huDestroyChangeSet(huChangeSet * changeSet)
{
    destroyVector(& changeSet->changes);
    destroyVector(& changeSet->changeStrings);
}


huSize_t huReplaceNodeZ(huChangeSet * changeSet, huNode const * node, char const * newString)
{
#ifdef HUMON_CHECK_PARAMS
    if (newString == NULL)
        { return -1; }
#endif

    size_t newStringLenC = strlen(newString);
    if (newStringLenC > maxOfType(huSize_t))
        { return -1; }

    return huReplaceNodeN(changeSet, node, newString, (huSize_t) newStringLenC);
}


huSize_t huReplaceNodeN(huChangeSet * changeSet, huNode const * node, char const * newString, huSize_t newStringLength)
{
#ifdef HUMON_CHECK_PARAMS
    if (changeSet == NULL || node == HU_NULLNODE || newString == NULL)
        { return -1; }
#endif

    return changeTrove(changeSet, node, HU_CHANGEKIND_REPLACE, -1, newString, newStringLength);
}


huSize_t huAppendZ(huChangeSet * changeSet, huNode const * node, char const * newString)
{
#ifdef HUMON_CHECK_PARAMS
    if (newString == NULL)
        { return -1; }
#endif

    size_t newStringLenC = strlen(newString);
    if (newStringLenC > maxOfType(huSize_t))
        { return -1; }

    return huAppendN(changeSet, node, newString, newStringLenC);
}


huSize_t huAppendN(huChangeSet * changeSet, huNode const * node, char const * newString, huSize_t newStringLength)
{
#ifdef HUMON_CHECK_PARAMS
    if (changeSet == NULL || node == HU_NULLNODE || newString == NULL || newStringLength < 0)
        { return -1; }
#endif

    return changeTrove(changeSet, node, HU_CHANGEKIND_INSERT,
        huGetNumChildren(node), newString, newStringLength);
}


huSize_t huInsertAtIndexZ(huChangeSet * changeSet, huNode const * node, huSize_t idx, char const * newString)
{
#ifdef HUMON_CHECK_PARAMS
    if (newString == NULL)
        { return -1; }
#endif

    size_t newStringLenC = strlen(newString);
    if (newStringLenC > maxOfType(huSize_t))
        { return -1; }

    return huInsertAtIndexN(changeSet, node, idx, newString, newStringLenC);
}


huSize_t huInsertAtIndexN(huChangeSet * changeSet, huNode const * node, huSize_t idx, char const * newString, huSize_t newStringLength)
{
#ifdef HUMON_CHECK_PARAMS
    if (changeSet == NULL || node == HU_NULLNODE || idx < 0 || newString == NULL || newStringLength < 0)
        { return -1; }
#endif

    return changeTrove(changeSet, node, HU_CHANGEKIND_INSERT, idx, newString, newStringLength);
}


static int compareChanges(huChange * lhs, huChange * rhs)
{
    return lhs->nodeIdx - rhs->nodeIdx;
}


static void sortChanges(huChangeSet * changeSet)
{
    typedef int (*changeFn)(void const * lhs, void const * rhs);
    qsort(changeSet->changes.buffer, changeSet->changes.numElements, sizeof(huChange), (changeFn) compareChanges);
}


static void getChangeExtremeTokens(huToken const ** start, huToken const ** end, huChange const * ch, huTrove const * trove)
{
    huNode const * chNode = huGetNodeByIndex(trove, ch->nodeIdx);
    if (ch->childIdx >= 0)
    {
        huSize_t numChildren = huGetNumChildren(chNode);
        if (ch->childIdx < numChildren)
        {
            // Here we're getting the first token of the element.
            chNode = huGetChildByIndex(chNode, ch->childIdx);
            * start = chNode->firstToken;
            * end = chNode->lastToken;
        }
        else
        {
            // Here we're appending to the list/dict, so ch->childIdx is past the end.
            // So get the last token in the last element, and point right after.
            chNode = huGetChildByIndex(chNode, numChildren - 1);
            * start = chNode->lastToken + 1;
            * end = * start;
        }
        return;
    }

    // No child, the node referenced is the one we want.
    * start = chNode->firstToken;
    * end = chNode->lastToken;
}


static void buildNewTokenString(huVector * str, huTrove const * trove, huChangeSet * changeSet)
{
    /*
        for each change:
            copy up to change token
            insert the repl text
            if replace:
                move cursor beyond this node
    */

    huSize_t numChanges = getVectorSize(& changeSet->changes);
    if (numChanges > 0)
    {
        char const * srcCursor = trove->dataString;
        huToken const * startTok = NULL;
        huToken const * endTok = NULL;

        for (int i = 0; i < numChanges; ++i)
        {
            huChange * ch = getVectorElement(& changeSet->changes, i);
            getChangeExtremeTokens(& startTok, & endTok, ch, trove);

            // If this change references a thing we've already erased,
            // it will be detected by this. Just ignore the change.
            if (endTok->rawStr.ptr + endTok->rawStr.size < srcCursor)
                { continue; }

            // copy data up to this guy's first token
            appendToVector(str, srcCursor, startTok->rawStr.ptr - srcCursor);

            // insert replacement text
            if (ch->newString.size > 0)
            {
                appendToVector(str, "\n", 1);
                appendToVector(str, ch->newString.ptr, ch->newString.size);
                appendToVector(str, "\n", 1);
            }

            if (ch->changeKind == HU_CHANGEKIND_REPLACE)
            {
                srcCursor = endTok->rawStr.ptr + endTok->rawStr.size;
            }
            else
            {
                srcCursor = startTok->rawStr.ptr;
            }
        }

        // copy remaining data
        appendToVector(str, srcCursor, trove->dataString + trove->dataStringSize - srcCursor);
    }
    else
    {
        appendToVector(str, trove->dataString, trove->dataStringSize);
    }
}


static void serializeChangedTokenString(char * dest, huSize_t * destLength, huTrove const * srcTrove, huChangeSet * changeSet)
{
    /*
        make new token string
        build the string
    */

    huVector str;
    if (dest == NULL)
    {
        initVectorForCounting(& str); // counting only
    }
    else
    {
        initVectorPreallocated(& str, dest, sizeof(char), * destLength);
    }

    buildNewTokenString(& str, srcTrove, changeSet);

    if (dest == NULL)
        { * destLength = str.numElements; }
}


huErrorCode huMakeChangedTrove(huTrove ** newTrove, huTrove const * srcTrove, huChangeSet * changeSet)
{
#ifdef HUMON_CHECK_PARAMS
    if (newTrove == NULL || srcTrove == HU_NULLNODE || changeSet == NULL)
        { return HU_ERROR_BADPARAMETER; }
#endif

    /*
        sort changes by token ID
        make the token string from changes
        make new trove from new string
        clean up new string? Or tell new trove to manage it?
    */

    sortChanges(changeSet);

    huSize_t len = 0;
    char * newString = NULL;
    serializeChangedTokenString(NULL, & len, srcTrove, changeSet);

    if (len < 0)
    {
        return HU_ERROR_OUTOFMEMORY;
    }

    huDeserializeOptions dso;
    huInitDeserializeOptions(& dso, HU_ENCODING_UTF8, false, srcTrove->inputTabSize, & srcTrove->allocator, HU_BUFFERMANAGEMENT_MOVEANDOWN);

    if (len > 0)
    {
        newString = ourAlloc(& srcTrove->allocator, len);
        serializeChangedTokenString(newString, & len, srcTrove, changeSet);

        huErrorCode ret = huDeserializeTroveN(newTrove, newString, len, & dso, HU_ERRORRESPONSE_MUM);
        // We told it to move instead of copy, but it can decide better depending on things.
        // This is just defensive though, and is unlikely to occur.
        if ((*newTrove)->bufferManagement == HU_BUFFERMANAGEMENT_COPYANDOWN)
        {
            ourFree(& srcTrove->allocator, newString);
        }
        return ret;
    }
    else
    {
        // zero-length string is still valid humon. :)
        return huDeserializeTroveN(newTrove, "", 0, & dso, HU_ERRORRESPONSE_MUM);
    }
}
