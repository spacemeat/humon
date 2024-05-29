#include <stdlib.h>
#include <string.h>
#include "humon.internal.h"

    /*  HU_ENCODING_UTF8:
            0xxx xxxx                                   or      (7 bits)
            110x xxxx  10xx xxxx                        or      (11 bits)
            1110 xxxx  10xx xxxx  10xx xxxx             or      (16 bits)
            1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx          (21 bits)

                If I detect a       10xx xxxx in b0,
                    @ cannot be utf8
                    @ can be utf16 BE non-surrogate, if b2b3 is not in surrogate range
                    @ can be utf16 LE non-surrogate, if b2b3 is not in surrogate range
                    @ cannot be utf32 BE
                If I detect a       1100 xxxx in b0,
                    @ can be utf8, if b1 is 10xx xxxx
                    @ can be utf16 BE non-surrogate, if b2b3 is not in surrogate range
                    @ can be utf16 LE non-surrogate, if b2b3 is not in surrogate range
                    @ cannot be utf32 BE
                If I detect a       1101 10xx in b0,
                    @ if b1 is      10xx xxxx, can be utf8
                    @ b0b1 is utf16be high, and
                      b2b3 must be  1101 1100 - 1101 1111-> utf16be low
                    @ cannot be utf32 BE
                If I detect a       1101 11xx bin b0,
                    @ if b1 is      10xx xxxx, can be utf8
                    @ b0b1 is utf16be low, and
                      b2b3 must be  1101 1000 - 1101 1100 -> utf16be high
                    @ cannot be utf32 BE
                If I detect that    xxxx xxxx  xxx0 0000 > 0,
                    @ cannot be utf32 BE

        UTF16:
            0000 0000  0000 0000 - <  1101 1000 0000 0000   0000 - d800    BMP code point
            1101 1000  0000 0000 - <  1101 1100 0000 0000   d800 - dc00    high surrogate
            1101 1100  0000 0000 - <  1110 0000 0000 0000   dc00 - e000    low surrogate
            1110 0000  0000 0000 - <= 1111 1111 1111 1111   e000 - ffff    BMP code point (rarer)

            0000 -  d800
            e000 - 10000

            For: d800 dc00
            high: (d800 - d800) x 400               =  0 0000
            low:  (dc00 - dc00)                     =  0 0000
                                                    +  1 0000
                                                    =  1 0000

            For: dbff dfff
            high: (dbff - d800) x 400  = 3ff x 400  =  f fc00
            low:  (dfff - dc00)                     =    3ff
                                                    +  1 0000
                                                    = 10 ffff =  0000 0000  0001 0000  1111 1111  1111 1111
        
        UTF32:
            Unicode range:
            0000 0000  0000 0000  0000 0000  0000 0000 - <  0000 0000  0000 0000  1101 1000  0000 0000
            0000 0000  0000 0000  1110 0000  0000 0000 - <  0000 0000  0001 0001  0000 0000  0000 0000
    */


char const utf8_bom[]    = { 0xef, 0xbb, 0xbf };
char const utf16be_bom[] = { 0xfe, 0xff };
char const utf16le_bom[] = { 0xff, 0xfe };
char const utf32be_bom[] = { 0x00, 0x00, 0xfe, 0xff };
char const utf32le_bom[] = { 0xff, 0xfe, 0x00, 0x00 };

char const * const bomDefs[] = { utf8_bom, utf16be_bom, utf16le_bom, utf32be_bom, utf32le_bom, NULL };
huSize_t const bomSizes[] = { sizeof(utf8_bom), sizeof(utf16be_bom), sizeof(utf16le_bom), sizeof(utf32be_bom), sizeof(utf32le_bom), 0 };

typedef struct ReadState_tag
{
    huEncoding encoding;
    huDeserializeOptions * deserializeOptions;
    bool maybe;
    uint32_t partialCodePoint;
    huSize_t bytesRemaining;
    huSize_t numCodePoints;
    huSize_t numNuls;
    huSize_t numAsciiRangeCodePoints;
    huErrorCode errorCode;
    char const * errorOffset;
    bool machineIsBigEndian;
} ReadState;


static void initReaders(ReadState readers[], huDeserializeOptions * deserializeOptions)
{
    bool machineIsBigEndian = isMachineBigEndian();

    readers[HU_ENCODING_UTF8] = (ReadState)
    {
        .encoding = HU_ENCODING_UTF8,
        .deserializeOptions = deserializeOptions,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numNuls = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };

    readers[HU_ENCODING_UTF16_BE] = (ReadState)
    {
        .encoding = HU_ENCODING_UTF16_BE,
        .deserializeOptions = deserializeOptions,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numNuls = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };

    readers[HU_ENCODING_UTF16_LE] = (ReadState)
    {
        .encoding = HU_ENCODING_UTF16_LE,
        .deserializeOptions = deserializeOptions,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numNuls = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };

    readers[HU_ENCODING_UTF32_BE] = (ReadState)
    {
        .encoding = HU_ENCODING_UTF32_BE,
        .deserializeOptions = deserializeOptions,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numNuls = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };

    readers[HU_ENCODING_UTF32_LE] = (ReadState)
    {
        .encoding = HU_ENCODING_UTF32_LE,
        .deserializeOptions = deserializeOptions,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numNuls = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };
}


static huEncoding getEncodingFromBom(huStringView const * data, huSize_t * numBomChars)
{
    huEncoding encoding = HU_ENCODING_UNKNOWN;

    // look for 32bit first, then 16bit
    if (memcmp(utf32le_bom, data->ptr, min(data->size, bomSizes[HU_ENCODING_UTF32_LE])) == 0)
        { encoding = HU_ENCODING_UTF32_LE; }
    else if (memcmp(utf32be_bom, data->ptr, min(data->size, bomSizes[HU_ENCODING_UTF32_BE])) == 0)
        { encoding = HU_ENCODING_UTF32_BE; }
    else if (memcmp(utf16le_bom, data->ptr, min(data->size, bomSizes[HU_ENCODING_UTF16_LE])) == 0)
        { encoding = HU_ENCODING_UTF16_LE; }
    else if (memcmp(utf16be_bom, data->ptr, min(data->size, bomSizes[HU_ENCODING_UTF16_BE])) == 0)
        { encoding = HU_ENCODING_UTF16_BE; }
    else if (memcmp(utf8_bom, data->ptr, min(data->size, bomSizes[HU_ENCODING_UTF8])) == 0)
        { encoding = HU_ENCODING_UTF8; }

    if (encoding != HU_ENCODING_UNKNOWN)
    {
        * numBomChars = bomSizes[(size_t) encoding];
        return encoding;
    }

    * numBomChars = 0;
    return HU_ENCODING_UNKNOWN;
}


static void scanUtf8(uint8_t codeUnit, ReadState * rs)
{
    if (rs->bytesRemaining > 0)
    {
        if ((codeUnit & 0xc0) == 0x80)
        {
            rs->bytesRemaining -= 1;
            rs->partialCodePoint |= (codeUnit & 0b00111111) << (6 * rs->bytesRemaining);
        }
        else
            { rs->maybe = false; }
    }
    else
    {
        if ((codeUnit & 0x80) == 0x00)
        {
            rs->partialCodePoint = codeUnit;
            rs->bytesRemaining = 0;
        }
        else if ((codeUnit & 0xe0) == 0xc0)
        {
            rs->partialCodePoint = ((codeUnit & 0b00011111) << 6);
            rs->bytesRemaining = 1;
        }
        else if ((codeUnit & 0xf0) == 0xe0)
        {
            rs->partialCodePoint = ((codeUnit & 0b00001111) << 12);
            rs->bytesRemaining = 2;
        }
        else if ((codeUnit & 0xf8) == 0xf0)
        {
            rs->partialCodePoint = ((codeUnit & 0b00000111) << 18);
            rs->bytesRemaining = 3;
        }
        else
            { rs->maybe = false; }
    }
    if (rs->bytesRemaining == 0)
    {
        rs->numCodePoints += 1;
        if (rs->partialCodePoint == 0)
            { rs->numNuls += 1; }
        else if (rs->partialCodePoint < 128)
            { rs->numAsciiRangeCodePoints += 1; }
        else if (rs->deserializeOptions->allowOutOfRangeCodePoints == false &&
                 (rs->partialCodePoint > 0x10ffff ||
                  (rs->partialCodePoint >= 0xd800 && 
                   rs->partialCodePoint < 0xe000)))
            { rs->maybe = false; }
    }
}


static void scanUtf16(uint16_t codeUnit, ReadState * rs)
{
    bool foundValidCodeUnit = false;
    if (rs->bytesRemaining > 0)
    {
        if (codeUnit >= 0xdc00 && codeUnit < 0xe000)
        {
            // low surrogate
            rs->partialCodePoint += (codeUnit - 0xdc00) + 0x10000;
            rs->bytesRemaining = 0;
            foundValidCodeUnit = true;
        }
        // Non-surrogate; may have to accept it for Windows filenames.
        else if (rs->deserializeOptions->allowUtf16UnmatchedSurrogates)
        {
            // undo high surrogate math from the previous code unit and add that code point
            rs->partialCodePoint = rs->partialCodePoint / 0x400 + 0xd800;
            rs->numCodePoints += 1;
            // reset for analysis of this code unit as a first unit
            rs->bytesRemaining = 0;
            foundValidCodeUnit = false;
        }
        else
            { rs->maybe = false; }
    }

    if (foundValidCodeUnit == false &&
        rs->bytesRemaining == 0)
    {
        if (codeUnit < 0xd800 || codeUnit >= 0xe000)
        {
            rs->partialCodePoint = codeUnit;
            rs->bytesRemaining = 0;
            foundValidCodeUnit = true;
        }
        else if (codeUnit >= 0xd800 && codeUnit < 0xdc00)
        {
            // high surrogate
            rs->partialCodePoint = (codeUnit - 0xd800) * 0x400;
            rs->bytesRemaining = 2;
            foundValidCodeUnit = true;
        }
            // low surrogate; invalid UTF16
        else if (rs->deserializeOptions->allowUtf16UnmatchedSurrogates)
        {
            rs->partialCodePoint += codeUnit;
            rs->bytesRemaining = 0;
            foundValidCodeUnit = true;
        }
        else
            { rs->maybe = false; }
    }

    if (foundValidCodeUnit == true &&
        rs->bytesRemaining == 0)
    {
        rs->numCodePoints += 1;
        if (rs->partialCodePoint == 0)
            { rs->numNuls += 1; }
        else if (rs->partialCodePoint < 128)
            { rs->numAsciiRangeCodePoints += 1; }
    }
}


static void scanUtf32(uint32_t codeUnit, ReadState * rs)
{
    // If we find out we should include surrogates (technically no, but for compatibility),
    // we'll remove the second predicate here.
    if ((codeUnit < 0xd800) ||
        (codeUnit > 0xe000 && codeUnit < 0x110000) ||
        rs->deserializeOptions->allowOutOfRangeCodePoints)
    {
        rs->partialCodePoint = codeUnit;
        rs->numCodePoints += 1;
        if (rs->partialCodePoint == 0)
            { rs->numNuls += 1; }
        else if (codeUnit < 128)
            { rs->numAsciiRangeCodePoints += 1; }
    }
    else
        { rs->maybe = false; }
}


static void swagEncodingFromBlock(char const * block, huSize_t blockSize, ReadState * readers, huSize_t * numValidEncodings)
{
    char const * u8Cur = block;
    char const * u16beCur = block;
    char const * u16leCur = block;
    char const * u32beCur = block;
    char const * u32leCur = block;

    char const * end = block + blockSize;

    bool machineIsBigEndian = readers[HU_ENCODING_UTF8].machineIsBigEndian;

    // scan through a 64-byte block for UTF8
    ReadState * rs = readers + HU_ENCODING_UTF8;
    while (rs->maybe && u8Cur < end)
    {
        uint8_t codeUnit = * u8Cur;
        scanUtf8(codeUnit, rs);
        u8Cur += 1;
        * numValidEncodings -= 1 * (rs->maybe == false);
    }

    // scan through a 64-byte block for UTF16_BE
    rs = readers + HU_ENCODING_UTF16_BE;
    while (rs->maybe && u16beCur < end)
    {
        uint16_t codeUnit = 0;
        if (machineIsBigEndian)
        {
            codeUnit = (uint8_t) u16beCur[0] |
                       (uint8_t) u16beCur[1] << 8;
        }
        else
        {
            codeUnit = (uint8_t) u16beCur[1] |
                       (uint8_t) u16beCur[0] << 8;
        }

        scanUtf16(codeUnit, rs);
        u16beCur += 2;
        * numValidEncodings -= 1 * (rs->maybe == false);
    }

    // scan through a 64-byte block for UTF16_LE
    rs = readers + HU_ENCODING_UTF16_LE;
    while (rs->maybe && u16leCur < end)
    {
        uint16_t codeUnit = 0;
        if (machineIsBigEndian)
        {
            codeUnit = (uint8_t) u16leCur[1] |
                       (uint8_t) u16leCur[0] << 8;
        }
        else
        {
            codeUnit = (uint8_t) u16leCur[0] |
                       (uint8_t) u16leCur[1] << 8;
        }
        scanUtf16(codeUnit, rs);
        u16leCur += 2;
        * numValidEncodings -= 1 * (rs->maybe == false);
    }

    // scan through a 64-byte block for UTF32_BE
    rs = readers + HU_ENCODING_UTF32_BE;
    while (rs->maybe && u32beCur < end)
    {
        uint32_t codeUnit = 0;
        if (machineIsBigEndian)
        {
            codeUnit = (uint8_t) u32beCur[0] |
                       (uint8_t) u32beCur[1] << 8 |
                       (uint8_t) u32beCur[2] << 16 |
                       (uint8_t) u32beCur[3] << 24;
        }
        else
        {
            codeUnit = (uint8_t) u32beCur[3] |
                       (uint8_t) u32beCur[2] << 8 |
                       (uint8_t) u32beCur[1] << 16 |
                       (uint8_t) u32beCur[0] << 24;
        }
        scanUtf32(codeUnit, rs);
        u32beCur += 4;
        * numValidEncodings -= 1 * (rs->maybe == false);
    }

    // scan through a 64-byte block for UTF32_LE
    rs = readers + HU_ENCODING_UTF32_LE;
    while (rs->maybe && u32leCur < end)
    {
        uint32_t codeUnit = 0;
        if (machineIsBigEndian)
        {
            codeUnit = (uint8_t) u32leCur[3] |
                       (uint8_t) u32leCur[2] << 8 |
                       (uint8_t) u32leCur[1] << 16 |
                       (uint8_t) u32leCur[0] << 24;
        }
        else
        {
            codeUnit = (uint8_t) u32leCur[0] |
                       (uint8_t) u32leCur[1] << 8 |
                       (uint8_t) u32leCur[2] << 16 |
                       (uint8_t) u32leCur[3] << 24;
        }
        scanUtf32(codeUnit, rs);
        u32leCur += 4;
        * numValidEncodings -= 1 * (rs->maybe == false);
    }
}


// Can return HU_ENCODING_UNKNOWN, in which case we should keep checking blocks.
static huEncoding findLikelyEncoding(ReadState readers[], huSize_t numValidEncodings)
{
    if (numValidEncodings == 0) // shouldn't ever happen
        { return HU_ENCODING_UNKNOWN; }

    if (numValidEncodings == 1)
    {
        for (huEncoding i = 0; i < HU_ENCODING_UNKNOWN; ++i)
        {
            if (readers[(size_t)i].maybe)
                { return i; }
        }
    }

    else
    {
        // static insertion sort
        /*  Because there is a small number of encodings, I can make an array of
            2^n-1 empty slots. Then, starting from the middle, I insert readers
            in priority order, each time halving the offset distance in the array
            until I'm down to 1. Then a linear scan through the array picks up
            the read states in sorted order. This alg probably has an actual name. 
            O(nlogn) for all n insertions, O(2^n) search, but we're talking n = 5 
            here, and two cachelines' worth of RAM, the first of which is guaranteed
            to contain the winner.*/
        ReadState * sortedReaders[(1 << HU_ENCODING_UNKNOWN) - 1] = { 0 };

        for (size_t i = 0; i < (size_t) HU_ENCODING_UNKNOWN; ++i)
        {
            size_t offset = (1 << HU_ENCODING_UNKNOWN) / 2 - 1; // 15
            size_t cursor = offset;
            // inserting b into sorted array
            ReadState * b = & readers[i];
            for (size_t j = 0; j < i; ++j)
            {
                // Empty slot; take it and bail
                ReadState * a = sortedReaders[cursor];
                if (a == NULL)
                    { break; }

                // if this reader (b) is worse than the previous reader (a), bin b to the right (compare = 1)
                // else let b bin left
                int compare = -1;    // 1 or -1

                // losers finish last
                if (b->maybe == false)
                    { compare = 1; }
                // Prefer the encoding with the fewer nul codepoints.
                else if (b->numNuls > a->numNuls)
                    { compare = 1; }
                else if (b->numNuls == a->numNuls)
                {
                    // Prefer the encoding with more codepoints < 127, since all Humon
                    // punctuation is in that range.
                    if (b->numAsciiRangeCodePoints < a->numAsciiRangeCodePoints)
                        { compare = 1; }
                    else if (b->numAsciiRangeCodePoints == a->numAsciiRangeCodePoints &&
                             // All else failing to disambiguate, prefer the encoding with
                             // more codepoints (shorter code units, really; utf8 over utf16).
                             b->numCodePoints < a->numCodePoints)
                        { compare = 1; }
                }

                offset = (1 << HU_ENCODING_UNKNOWN) / (1 << (j + 2));
                // j=0                          32  /  4        = 8
                // j=1                          32  /  8        = 4
                // j=2                          32  /  16       = 2
                // j=3                          32  /  32       = 1
                cursor += offset * compare;
            }

            sortedReaders[cursor] = b;
        }

        // now find the first two; these are highest-rated encodings
        // this is why we prefer to insert left; they get found earlier
        ReadState * a = NULL;
        ReadState * b = NULL;
        for (size_t i = 0; i < sizeof(sortedReaders) / sizeof(ReadState *); ++i)
        {
            if (sortedReaders[i])
            {
                if (a == NULL) { a = sortedReaders[i]; }
                else if (b == NULL) { b = sortedReaders[i]; break; }
            }
        }

        // if a is significantly better than b, then we've found our king
        huSize_t HU_NUL_THRESHOLD = 2;
        huSize_t HU_ASCII_THRESHOLD = 10;
        huSize_t HU_NUMCP_THRESHOLD = 10;
        if (a->numNuls - b->numNuls >= HU_NUL_THRESHOLD)
            { return a->encoding; }
        else if (a->numAsciiRangeCodePoints - b->numAsciiRangeCodePoints >= HU_ASCII_THRESHOLD)
            { return a->encoding; }
        else if (a->numCodePoints - b->numCodePoints >= HU_NUMCP_THRESHOLD)
            { return a->encoding; }

        return HU_ENCODING_UNKNOWN;
    }

    // satisfy the compiler, but we'll never get here
    return HU_ENCODING_UNKNOWN;
}


// This must only be called if we've read all the way through the string/file and cannot
// determine the correct encoding yet. This checks whether there are any incomplete encodings
// at the end of the string that would disqualify the encodings from consideration. It's a
// hail mary check if somehow the encodings are completely ambiguous otherwise, and this is
// miraculously the deciding factor. Shrug.
static void disqualifyIncompleteEncodings(ReadState readers[], huSize_t numValidEncodings)
{
    // If we finished scanning while expecting more bytes in a multi-codeUnit 
    // encoding, that encoding is bunk.
    if (numValidEncodings > 1)
    {
        if (readers[HU_ENCODING_UTF8].maybe && 
            readers[HU_ENCODING_UTF8].bytesRemaining)
        {
            readers[HU_ENCODING_UTF8].maybe = false; 
            numValidEncodings -= 1;
        }
        if (readers[HU_ENCODING_UTF16_BE].maybe &&
            readers[HU_ENCODING_UTF16_BE].bytesRemaining &&
            readers[HU_ENCODING_UTF16_BE].deserializeOptions->allowUtf16UnmatchedSurrogates == false) 
        {
            readers[HU_ENCODING_UTF16_BE].maybe = false;
            numValidEncodings -= 1;
        }
        if (readers[HU_ENCODING_UTF16_LE].maybe &&
            readers[HU_ENCODING_UTF16_LE].bytesRemaining &&
            readers[HU_ENCODING_UTF16_BE].deserializeOptions->allowUtf16UnmatchedSurrogates == false)
        {
            readers[HU_ENCODING_UTF16_LE].maybe = false;
            numValidEncodings -= 1;
        }
        // UTF32 never has more than one code unit per codepoint.
    }
}


/*  POST: The encoding detected from the file is returned. If HU_ENCODING_UNKNONWN
    is returned, then there was an error; we'll always just choose a legal one if
    we can't decide which is best.*/
huEncoding swagEncodingFromString(huStringView const * data, huSize_t * numBomChars, huDeserializeOptions * deserializeOptions)
{
    huEncoding bomEncoding = getEncodingFromBom(data, numBomChars);

#ifdef HUMON_CAVEPERSON_DEBUGGING
    if (bomEncoding != HU_ENCODING_UNKNOWN)
        { printf("%sEncoding detected from BOM: %s%d%s\n", ansi_darkGreen, ansi_lightGreen, bomEncoding, ansi_off); }
    else
        { printf("%sEncoding not detected from BOM%s\n", ansi_darkYellow, ansi_off); }
#endif

    if (bomEncoding != HU_ENCODING_UNKNOWN)
        { return bomEncoding; }

    huSize_t numValidEncodings = (huSize_t) HU_ENCODING_UNKNOWN;
    huEncoding bestEncoding = HU_ENCODING_UNKNOWN;
    ReadState readers[HU_ENCODING_UNKNOWN];

    initReaders(readers, deserializeOptions);

    char const * block = data->ptr;
    huSize_t blockSize = min(data->size, HUMON_SWAG_BLOCKSIZE);
    huSize_t bytesRead = 0;

    // scan through the input string in BLOCKSIZE-byte blocks
    while (bytesRead < data->size)
    {
        swagEncodingFromBlock(block, blockSize, readers, & numValidEncodings);

        // if there's a clear winning encoding even among legal ones, choose it early
        huEncoding winningEncoding = findLikelyEncoding(readers, numValidEncodings);
        if (winningEncoding != HU_ENCODING_UNKNOWN)
        {
            bestEncoding = winningEncoding;
            break;
        }
        
        bytesRead += blockSize;
        block += HUMON_SWAG_BLOCKSIZE;
        blockSize = min(HUMON_SWAG_BLOCKSIZE, data->size - bytesRead);
    }

    // If we finished and still can't decide, maybe some candidates will be disqualified
    // by broken code units at end-of-file. It's -real- unlikely.
    if (bytesRead == data->size)
    {
        disqualifyIncompleteEncodings(readers, numValidEncodings);
        huEncoding winningEncoding = findLikelyEncoding(readers, numValidEncodings);
        if (winningEncoding != HU_ENCODING_UNKNOWN)
            { bestEncoding = winningEncoding; }
    }

    if (bestEncoding == HU_ENCODING_UNKNOWN)
    {
        // We always choose a legal one, even if one isn't obviously the best.
        for (size_t i = 0; i < (size_t) HU_ENCODING_UNKNOWN; ++i)
        {
            if (readers[i].maybe)
                { return readers[i].encoding; }
        }
    }

#ifdef HUMON_CAVEPERSON_DEBUGGING
    if (bestEncoding != HU_ENCODING_UNKNOWN)
        { printf("%sEncoding determined from bit pattern: %s%s%s\n", ansi_darkGreen, ansi_lightGreen, huEncodingToString(bestEncoding), ansi_off); }
    else
        { printf("%sEncoding not determined from bit pattern%s\n", ansi_darkRed, ansi_off); }
#endif

    return bestEncoding;
}


/*  PRE: fp is opened in binary mode, and is pointing to the beginning of the data.
    POST: The encoding detected from the file is returned. If HU_ENCODING_UNKNONWN
    is returned, then there was an error; we'll always just choose a legal one if
    we can't decide which is best.*/
huEncoding swagEncodingFromFile(FILE * fp, huSize_t fileSize, huSize_t * numBomChars, huDeserializeOptions * deserializeOptions)
{
    huSize_t numValidEncodings = HU_ENCODING_UNKNOWN;
    ReadState readers[HU_ENCODING_UNKNOWN];

    initReaders(readers, deserializeOptions);

    char buf[HUMON_SWAG_BLOCKSIZE];
    char * block = buf;
    huSize_t blockSize = 0;
    huSize_t bytesRead = 0;

    blockSize = (huSize_t) fread(block, 1, HUMON_SWAG_BLOCKSIZE, fp);
    if (blockSize == 0)
        { return HU_ENCODING_UNKNOWN; } // ERROR! WTF
    bytesRead += blockSize;

    huStringView sv = { .ptr = block, .size = blockSize };
    huEncoding bestEncoding = getEncodingFromBom(& sv, numBomChars);
    if (bestEncoding != HU_ENCODING_UNKNOWN)
        { return bestEncoding; }

    // scan through the input file in HUMON_SWAG_BLOCKSIZE-byte blocks
    do
    {
        swagEncodingFromBlock(block, blockSize, readers, & numValidEncodings);

        // if there's a clear winning encoding even among legal ones, choose it early
        huEncoding winningEncoding = findLikelyEncoding(readers, numValidEncodings);
        if (winningEncoding != HU_ENCODING_UNKNOWN)
        {
            bestEncoding = winningEncoding;
            break;
        }
        
        blockSize = (huSize_t) fread(block, 1, HUMON_SWAG_BLOCKSIZE, fp);
        if (blockSize == 0)
            { return HU_ENCODING_UNKNOWN; } // ERROR! WTF
        block = buf;
        bytesRead += blockSize;
    }
    while (bytesRead < fileSize);

    // If we finished and still can't decide, maybe some candidates will be disqualified
    // by broken code units at end-of-file. It's -real- unlikely.
    if (bytesRead == fileSize)
    {
        disqualifyIncompleteEncodings(readers, numValidEncodings);
        huEncoding winningEncoding = findLikelyEncoding(readers, numValidEncodings);
        if (winningEncoding != HU_ENCODING_UNKNOWN)
            { bestEncoding = winningEncoding; }
    }

    if (bestEncoding == HU_ENCODING_UNKNOWN)
    {
        // We always choose a legal one, even if one isn't obviously the best.
        for (size_t i = 0; i < (size_t) HU_ENCODING_UNKNOWN; ++i)
        {
            if (readers[i].maybe)
                { return readers[i].encoding; }
        }
    }

#ifdef HUMON_CAVEPERSON_DEBUGGING
    if (bestEncoding != HU_ENCODING_UNKNOWN)
        { printf("%sEncoding determined from bit pattern: %s%s%s\n", ansi_darkGreen, ansi_lightGreen, huEncodingToString(bestEncoding), ansi_off); }
    else
        { printf("%sEncoding not determined from bit pattern%s\n", ansi_darkRed, ansi_off); }
#endif

    return bestEncoding;
}


static huSize_t appendEncodedUtf8CodePoint(char * dest, uint32_t codePoint)
{
    huSize_t encodedLen = 0;

    // encode reader->codePoint
    if (codePoint < (1 << 7))
    {
        * dest = codePoint;
        dest += 1;
        encodedLen += 1;
    }
    else if (codePoint < (1 << 11))
    {
        * dest = 0b11000000 | ((codePoint >> 6) & (0b00011111));
        dest += 1;
        * dest = 0b10000000 | (codePoint & 0b00111111);
        dest += 1;
        encodedLen += 2;
    }
    else if (codePoint < (1 << 16))
    {
        * dest = 0b11100000 | ((codePoint >> 12) & (0b00001111));
        dest += 1;
        * dest = 0b10000000 | ((codePoint >> 6) & (0b00111111));
        dest += 1;
        * dest = 0b10000000 | (codePoint & 0b00111111);
        dest += 1;
        encodedLen += 3;
    }
    else
    {
        * dest = 0b11110000 | ((codePoint >> 18) & (0b00000111));
        dest += 1;
        * dest = 0b10000000 | ((codePoint >> 12) & (0b00111111));
        dest += 1;
        * dest = 0b10000000 | ((codePoint >> 6) & (0b00111111));
        dest += 1;
        * dest = 0b10000000 | (codePoint & 0b00111111);
        dest += 1;
        encodedLen += 4;
    }

    return encodedLen;
}


static huSize_t transcodeToUtf8FromBlock_utf8(char * dest, char const * block, huSize_t blockSize, ReadState * reader)
{
    uint8_t const * uCur = (uint8_t const *) block;
    huSize_t encodedLen = 0;

    while (uCur < (uint8_t const *) (block + blockSize))
    {
        uint8_t codeUnit = * uCur;
        scanUtf8(codeUnit, reader);
        uCur += 1;

        if (reader->maybe == false)
        {
            reader->errorOffset = (char const *) uCur;
            return 0;
        }
        
        if (reader->bytesRemaining == 0)
        {
            encodedLen += appendEncodedUtf8CodePoint(dest + encodedLen, 
                reader->partialCodePoint);
        }
    }

    return encodedLen;
}


static huSize_t transcodeToUtf8FromBlock_utf16be(char * dest, char const * block, huSize_t blockSize, ReadState * reader)
{
    char const * uCur = block;
    huSize_t encodedLen = 0;

    while (uCur < block + blockSize)
    {
        uint16_t codeUnit = 0;
        if (reader->machineIsBigEndian)
        {
            codeUnit = (uint8_t) uCur[0] |
                       (uint8_t) uCur[1] << 8;
        }
        else
        {
            codeUnit = (uint8_t) uCur[1] |
                       (uint8_t) uCur[0] << 8;
        }
        scanUtf16(codeUnit, reader);
        uCur += 2;

        if (reader->maybe == false)
        {
            reader->errorOffset = uCur;
            return 0;
        }
        
        if (reader->bytesRemaining == 0)
        {
            encodedLen += appendEncodedUtf8CodePoint(dest + encodedLen, 
                reader->partialCodePoint);
        }
    }

    return encodedLen;
}


static huSize_t transcodeToUtf8FromBlock_utf16le(char * dest, char const * block, huSize_t blockSize, ReadState * reader)
{
    char const * uCur = block;
    huSize_t encodedLen = 0;

    while (uCur < block + blockSize)
    {
        uint16_t codeUnit = 0;
        if (reader->machineIsBigEndian)
        {
            codeUnit = (uint8_t) uCur[1] |
                       (uint8_t) uCur[0] << 8;
        }
        else
        {
            codeUnit = (uint8_t) uCur[0] |
                       (uint8_t) uCur[1] << 8;
        }
        scanUtf16(codeUnit, reader);
        codeUnit = ((codeUnit & 0x00ff) << 8) | ((codeUnit & 0xff00) >> 8);
        uCur += 2;

        if (reader->maybe == false)
        {
            reader->errorOffset = uCur;
            return 0;
        }
        
        if (reader->bytesRemaining == 0)
        {
            encodedLen += appendEncodedUtf8CodePoint(dest + encodedLen, 
                reader->partialCodePoint);
        }
    }

    return encodedLen;
}


static huSize_t transcodeToUtf8FromBlock_utf32be(char * dest, char const * block, huSize_t blockSize, ReadState * reader)
{
    char const * uCur = block;
    huSize_t encodedLen = 0;

    while (uCur < block + blockSize)
    {
        uint32_t codeUnit = 0;
        if (reader->machineIsBigEndian)
        {
            codeUnit = (uint8_t) uCur[0] |
                       (uint8_t) uCur[1] << 8 |
                       (uint8_t) uCur[2] << 16 |
                       (uint8_t) uCur[3] << 24;
        }
        else
        {
            codeUnit = (uint8_t) uCur[3] |
                       (uint8_t) uCur[2] << 8 |
                       (uint8_t) uCur[1] << 16 |
                       (uint8_t) uCur[0] << 24;
        }
        scanUtf32(codeUnit, reader);
        uCur += 4;

        if (reader->maybe == false)
        {
            reader->errorOffset = uCur;
            return 0;
        }
        
        if (reader->bytesRemaining == 0)
        {
            encodedLen += appendEncodedUtf8CodePoint(dest + encodedLen, 
                reader->partialCodePoint);
        }
    }

    return encodedLen;
}


static huSize_t transcodeToUtf8FromBlock_utf32le(char * dest, char const * block, huSize_t blockSize, ReadState * reader)
{
    char const * uCur = block;
    huSize_t encodedLen = 0;

    while (uCur < block + blockSize)
    {
        uint32_t codeUnit = 0;
        if (reader->machineIsBigEndian)
        {
            codeUnit = (uint8_t) uCur[3] |
                       (uint8_t) uCur[2] << 8 |
                       (uint8_t) uCur[1] << 16 |
                       (uint8_t) uCur[0] << 24;
        }
        else
        {
            codeUnit = (uint8_t) uCur[0] |
                       (uint8_t) uCur[1] << 8 |
                       (uint8_t) uCur[2] << 16 |
                       (uint8_t) uCur[3] << 24;
        }
        scanUtf32(codeUnit, reader);
        uCur += 4;

        if (reader->maybe == false)
        {
            reader->errorOffset = uCur;
            return 0;
        }
        
        if (reader->bytesRemaining == 0)
        {
            encodedLen += appendEncodedUtf8CodePoint(dest + encodedLen, 
                reader->partialCodePoint);
        }
    }

    return encodedLen;
}


static huSize_t transcodeToUtf8FromBlock(char * dest, char const * block, huSize_t blockSize, ReadState * reader)
{
    switch(reader->deserializeOptions->encoding)
    {
    case HU_ENCODING_UTF8:
        return transcodeToUtf8FromBlock_utf8(dest, block, blockSize, reader);
    case HU_ENCODING_UTF16_BE:
        return transcodeToUtf8FromBlock_utf16be(dest, block, blockSize, reader);
    case HU_ENCODING_UTF16_LE:
        return transcodeToUtf8FromBlock_utf16le(dest, block, blockSize, reader);
    case HU_ENCODING_UTF32_BE:
        return transcodeToUtf8FromBlock_utf32be(dest, block, blockSize, reader);
    case HU_ENCODING_UTF32_LE:
        return transcodeToUtf8FromBlock_utf32le(dest, block, blockSize, reader);
    default:
        return 0;
    }
}


/*  PRE: dest points to a string at least as long as srcLen.
    PRE: srcEncoding is not HU_ENCODING_UNKNOWN
    Sets *numBytesEncoded and returns a HU_ERROR_*.
*/
huErrorCode transcodeToUtf8FromString(char * dest, huSize_t * numBytesEncoded, huStringView const * src, huDeserializeOptions * deserializeOptions)
{
    if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
        { return HU_ERROR_BADPARAMETER; }
    
    // faster codepath for UTF8->UTF8 transcoding
    // This path doesn't check for invalid or overlong UTF8 sequences. It just
    // slams memory. You can separately specify strictUnicode for output for
    // huTroveTo* functions. Humon won't be fooled by overlong imposters.
    if (deserializeOptions->encoding == HU_ENCODING_UTF8 && 
        deserializeOptions->allowOutOfRangeCodePoints == true)
    {
        // skip the BOM if there is one
        if (memcmp(src, utf8_bom, sizeof(utf8_bom)) == 0)
            { memcpy(dest, src->ptr + sizeof(utf8_bom), src->size - sizeof(utf8_bom)); }
        else
            { memcpy(dest, src->ptr, src->size); }
        * numBytesEncoded = src->size;
        return HU_ERROR_NOERROR;
    }
    else
    {
        ReadState reader;
        reader.encoding = deserializeOptions->encoding;
        reader.deserializeOptions = deserializeOptions;
        reader.maybe = true;
        reader.partialCodePoint = 0;
        reader.bytesRemaining = 0;
        reader.numNuls = 0;
        reader.numAsciiRangeCodePoints = 0;
        reader.numCodePoints = 0;
        reader.errorCode = 0;
        reader.errorOffset = 0;
        reader.machineIsBigEndian = isMachineBigEndian();

        huSize_t encodedLen = 0;

        char const * block = src->ptr;
        huSize_t blockSize = min(src->size, HUMON_TRANSCODE_BLOCKSIZE);
        huSize_t bytesRead = 0;

        // skip the BOM if there is one
        char const * bom = bomDefs[(size_t) deserializeOptions->encoding];
        huSize_t bomLen = bomSizes[(size_t) deserializeOptions->encoding];
        if (bomLen > 0 && memcmp(src->ptr, bom, bomLen) == 0)
        {
            block += bomLen;
            blockSize -= bomLen;
            bytesRead = bomLen;
        }

        huSize_t enc = transcodeToUtf8FromBlock(dest, block, blockSize, & reader);
        if (reader.errorCode != 0)
        {
            * numBytesEncoded = 0;
            return HU_ERROR_BADENCODING;
        }

        encodedLen += enc;

        // scan through the input string in HUMON_TRANSCODE_BLOCKSIZE-byte blocks
        while (bytesRead < src->size)
        {
            bytesRead += blockSize;
            block += blockSize;
            blockSize = min(HUMON_TRANSCODE_BLOCKSIZE, src->size - bytesRead);

            huSize_t enc = transcodeToUtf8FromBlock(dest + encodedLen, block, blockSize, & reader);
            if (reader.errorCode != 0)
            {
                * numBytesEncoded = 0;
                return HU_ERROR_BADENCODING;
            }

            encodedLen += enc;
        }

        * numBytesEncoded = encodedLen;
        return HU_ERROR_NOERROR;
    }
}


huErrorCode transcodeToUtf8FromFile(char * dest, huSize_t * numBytesEncoded, FILE * fp, huSize_t srcLen, huDeserializeOptions * deserializeOptions)
{
    if (deserializeOptions->encoding == HU_ENCODING_UNKNOWN)
        { return HU_ERROR_BADPARAMETER; }
    
    // faster codepath for UTF8->UTF8 transcoding
    // This path doesn't check for invalid or overlong UTF8 sequences. It just
    // slams memory. You can separately specify strictUnicode for output for
    // huTroveTo* functions. Humon won't be fooled by overlong imposters.
    if (deserializeOptions->encoding == HU_ENCODING_UTF8 && 
        deserializeOptions->allowOutOfRangeCodePoints == true)
    {
        * numBytesEncoded = (huSize_t) fread(dest, 1, srcLen, fp);
        return HU_ERROR_NOERROR;
    }
    else
    {
        ReadState reader;
        reader.encoding = deserializeOptions->encoding;
        reader.deserializeOptions = deserializeOptions;
        reader.maybe = true;
        reader.partialCodePoint = 0;
        reader.bytesRemaining = 0;
        reader.numNuls = 0;
        reader.numAsciiRangeCodePoints = 0;
        reader.numCodePoints = 0;
        reader.errorCode = 0;
        reader.errorOffset = 0;
        reader.machineIsBigEndian = isMachineBigEndian();

        huSize_t encodedLen = 0;
        * numBytesEncoded = 0;

        // As stack allocations go, this might be large. HUMON_TRANSCODE_BLOCKSIZE is
		// considered a build parameter.
        char buf[HUMON_TRANSCODE_BLOCKSIZE];
        char * block = buf;
        huSize_t blockSize = (huSize_t) fread(block, 1, HUMON_TRANSCODE_BLOCKSIZE, fp);
        if (blockSize == 0)
            { return HU_ERROR_BADFILE; }
        huSize_t bytesRead = blockSize;

        // skip the BOM if there is one
        char const * bom = bomDefs[(size_t) deserializeOptions->encoding];
        huSize_t bomLen = bomSizes[(size_t) deserializeOptions->encoding];
        if (bomLen > 0 && memcmp(block, bom, bomLen) == 0)
        {
            block += bomLen;
            blockSize -= bomLen;
        }

        huSize_t enc = transcodeToUtf8FromBlock(dest, block, blockSize, & reader);
        if (reader.errorCode != 0)
            { return HU_ERROR_BADENCODING; }
        encodedLen += enc;
        block = buf;

        while (bytesRead < srcLen)
        {
            blockSize = (huSize_t) fread(block, 1, HUMON_TRANSCODE_BLOCKSIZE, fp);
            if (blockSize == 0)
                { return HU_ERROR_BADFILE; }
            bytesRead += blockSize;

            enc = transcodeToUtf8FromBlock(dest + encodedLen, block, blockSize, & reader);
            if (reader.errorCode != 0)
                { return HU_ERROR_BADENCODING; }
            encodedLen += enc;
        } 

        * numBytesEncoded = encodedLen;
        return HU_ERROR_NOERROR;
    }
}
