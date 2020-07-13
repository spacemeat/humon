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
            1110 0000  0000 0000 - <= 1111 1111 1111 1111   e000 - ffff    BMP code point (rarer?)

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


char const utf8_bom[] = { 0xef, 0xbb, 0xbf };
char const utf16be_bom[] = { 0xfe, 0xff };
char const utf16le_bom[] = { 0xff, 0xfe };
char const utf32be_bom[] = { 0x00, 0x00, 0xfe, 0xff };
char const utf32le_bom[] = { 0xff, 0xfe, 0x00, 0x00 };

char const * const bomDefs[] = { utf8_bom, utf16be_bom, utf16le_bom, utf32be_bom, utf32le_bom };


typedef struct ReadState_tag
{
    huLoadParams * loadParams;
    bool maybe;
    uint32_t partialCodePoint;
    int bytesRemaining;
    int numCodePoints;
    int numAsciiRangeCodePoints;
    int errorCode;
    char const * errorOffset;
    bool machineIsBigEndian;
} ReadState;


int numUtf8Bytes(uint32_t validCodePoint)
{
    if (validCodePoint < 1 << 7)
        { return 1; }
    else if (validCodePoint < (1 << 11))
        { return 2; }
    else if (validCodePoint < (1 << 16))
        { return 3; }
    return 4;
}


void initReaders(ReadState readers[], huLoadParams * loadParams)
{
    bool machineIsBigEndian = isMachineBigEndian();

    readers[HU_ENCODING_UTF8] = (ReadState)
    {
        .loadParams = loadParams,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };

    readers[HU_ENCODING_UTF16_BE] = (ReadState)
    {
        .loadParams = loadParams,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };

    readers[HU_ENCODING_UTF16_LE] = (ReadState)
    {
        .loadParams = loadParams,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };

    readers[HU_ENCODING_UTF32_BE] = (ReadState)
    {
        .loadParams = loadParams,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };

    readers[HU_ENCODING_UTF32_LE] = (ReadState)
    {
        .loadParams = loadParams,
        .partialCodePoint = 0,
        .bytesRemaining = 0,
        .maybe = true,
        .numCodePoints = 0,
        .numAsciiRangeCodePoints = 0,
        .errorCode = 0,
        .errorOffset = 0,
        .machineIsBigEndian = machineIsBigEndian
    };
}


void scanUtf8(uint8_t codeUnit, ReadState * rs)
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
        if (rs->partialCodePoint < 128)
            { rs->numAsciiRangeCodePoints += 1; }
        else if (rs->loadParams->allowOutOfRangeCodePoints == false &&
                 (rs->partialCodePoint > 0x10ffff ||
                  (rs->partialCodePoint >= 0xd800 && 
                   rs->partialCodePoint < 0xe000)))
            { rs->maybe = false; }
    }
}


void scanUtf16(uint16_t codeUnit, ReadState * rs)
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
        else if (rs->loadParams->allowUtf16UnmatchedSurrogates == false)
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
        else if (rs->loadParams->allowUtf16UnmatchedSurrogates)
        { // TODO: Not do the surrogate math on unmatched surrogates
            rs->partialCodePoint += (codeUnit - 0xdc00) + 0x10000;
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
        if (rs->partialCodePoint < 128)
            { rs->numAsciiRangeCodePoints += 1; }
    }
}


void scanUtf32(uint32_t codeUnit, ReadState * rs)
{
    // If we find out we should include surrogates (technically no, but for compatibility),
    // we'll remove the second predicate here.
    if ((codeUnit < 0xd800) ||
        (codeUnit > 0xe000 && codeUnit < 0x110000) ||
        rs->loadParams->allowOutOfRangeCodePoints)
    {
        rs->partialCodePoint = codeUnit;
        rs->numCodePoints += 1;
        if (codeUnit < 128)
            { rs->numAsciiRangeCodePoints += 1; }
    }
    else
        { rs->maybe = false; }
}


void swagEncodingFromBlock(char const * block, int blockSize, ReadState * readers, int * numValidEncodings)
{
    char const * u8Cur = block;
    char const * u16beCur = block;
    char const * u16leCur = block;
    char const * u32beCur = block;
    char const * u32leCur = block;

    char const * end = block + blockSize;

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
        uint16_t codeUnit = * u16beCur;
        scanUtf16(codeUnit, rs);
        u16beCur += 2;
        * numValidEncodings -= 1 * (rs->maybe == false);
    }

    // scan through a 64-byte block for UTF16_LE
    rs = readers + HU_ENCODING_UTF16_LE;
    while (rs->maybe && u16leCur < end)
    {
        uint16_t codeUnit = (* u16leCur & 0xff00) >> 8 |
                            (* u16leCur & 0x00ff) << 8;
        scanUtf16(codeUnit, rs);
        u16leCur += 2;
        * numValidEncodings -= 1 * (rs->maybe == false);
    }

    // scan through a 64-byte block for UTF32_BE
    rs = readers + HU_ENCODING_UTF32_BE;
    while (rs->maybe && u32beCur < end)
    {
        uint32_t codeUnit = * u32beCur;
        scanUtf32(codeUnit, rs);
        u32beCur += 4;
        * numValidEncodings -= 1 * (rs->maybe == false);
    }

    // scan through a 64-byte block for UTF32_LE
    rs = readers + HU_ENCODING_UTF32_LE;
    while (rs->maybe && u32leCur < end)
    {
        uint32_t codeUnit = (* u32leCur & 0xff000000) >> 24 |
                            (* u32leCur & 0x00ff0000) << 8 |
                            (* u32leCur & 0x0000ff00) << 8 |
                            (* u32leCur & 0x000000ff) << 24;
        scanUtf32(codeUnit, rs);
        u32leCur += 4;
        * numValidEncodings -= 1 * (rs->maybe == false);
    }
}


ReadState * chooseFromAmbiguousEncodings(ReadState readers[], int numValidEncodings)
{
    // If we finished scanning while expecting more bytes in a multi-codeUnit 
    // encoding, that encoding is bunk. (UTF32 never has more than one code unit.)
    if (numValidEncodings > 1)
    {
        if (readers[HU_ENCODING_UTF8].maybe &&     readers[HU_ENCODING_UTF8].bytesRemaining)     { readers[HU_ENCODING_UTF8].maybe = false; numValidEncodings -= 1; }
        if (readers[HU_ENCODING_UTF16_BE].maybe && readers[HU_ENCODING_UTF16_BE].bytesRemaining) { readers[HU_ENCODING_UTF16_BE].maybe = false; numValidEncodings -= 1; }
        if (readers[HU_ENCODING_UTF16_LE].maybe && readers[HU_ENCODING_UTF16_LE].bytesRemaining) { readers[HU_ENCODING_UTF16_LE].maybe = false; numValidEncodings -= 1; }
    }

    ReadState * selectedEncoding = NULL;
    if (numValidEncodings == 1)
    {
        for (int i = 0; i < HU_ENCODING_UNKNOWN; ++i)
        {
            if (readers[i].maybe)
            {
                selectedEncoding = readers + i;
                break;
            }
        }
    }
    else if (numValidEncodings > 1)
    {
        for (int i = 0; i < HU_ENCODING_UNKNOWN; ++i)
        {
            if (readers[i].maybe)
            {
                if (selectedEncoding == NULL)
                    { selectedEncoding = readers + i; }
                else
                {
                    if (readers[i].numAsciiRangeCodePoints > selectedEncoding->numAsciiRangeCodePoints)
                        { selectedEncoding = readers + i; }
                    else if (readers[i].numAsciiRangeCodePoints == selectedEncoding->numAsciiRangeCodePoints &&
                        readers[i].numCodePoints > selectedEncoding->numCodePoints)
                        { selectedEncoding = readers + i; }                        
                }
            }
        }
    }

    return selectedEncoding;
}


int getEncodingFromBom(huStringView const * data, size_t * numBomChars)
{
    // look for 32bit first, then 16bit
    if (memcmp(utf32le_bom, data->ptr, min(data->size, sizeof(utf32le_bom))) == 0)
    {
        * numBomChars = sizeof(utf32le_bom);
        return HU_ENCODING_UTF32_LE;
    }
    if (memcmp(utf32be_bom, data->ptr, min(data->size, sizeof(utf32be_bom))) == 0)
    {
        * numBomChars = sizeof(utf32be_bom);
        return HU_ENCODING_UTF32_BE;
    }
    if (memcmp(utf16le_bom, data->ptr, min(data->size, sizeof(utf16le_bom))) == 0)
    {
        * numBomChars = sizeof(utf16le_bom);
        return HU_ENCODING_UTF16_LE;
    }
    if (memcmp(utf16be_bom, data->ptr, min(data->size, sizeof(utf16be_bom))) == 0)
    {
        * numBomChars = sizeof(utf16be_bom);
        return HU_ENCODING_UTF16_BE;
    }
    if (memcmp(utf8_bom, data->ptr, min(data->size, sizeof(utf8_bom))) == 0)
    {
        * numBomChars = sizeof(utf8_bom);
        return HU_ENCODING_UTF8;
    }

    * numBomChars = 0;
    return HU_ENCODING_UNKNOWN;
}


int swagEncodingFromString(huStringView const * data, size_t * numBomChars, huLoadParams * loadParams)
{
    int bomEncoding = getEncodingFromBom(data, numBomChars);
    if (bomEncoding != HU_ENCODING_UNKNOWN)
        { return bomEncoding; }

    int const BLOCKSIZE = 64;

    int numValidEncodings = HU_ENCODING_UNKNOWN;
    ReadState * selectedEncoding = NULL;
    ReadState readers[HU_ENCODING_UNKNOWN];

    initReaders(readers, loadParams);

    char const * block = data->ptr;
    int blockSize = min(data->size, BLOCKSIZE);
    int bytesRead = 0;

    // scan through the input string in BLOCKSIZE-byte blocks
    while (bytesRead < data->size)
    {
        swagEncodingFromBlock(block, blockSize, readers, & numValidEncodings);

        if (numValidEncodings == 1)
            { break; }
        
        bytesRead += blockSize;
        block += BLOCKSIZE;
        blockSize = min(BLOCKSIZE, data->size - bytesRead);
    }

    selectedEncoding = chooseFromAmbiguousEncodings(readers, numValidEncodings);
    if (selectedEncoding != NULL)
        { return selectedEncoding - readers; }
    else
        { return HU_ENCODING_UNKNOWN; }
}


/*  PRE: fp is opened in binary mode, and is pointing to the beginning of the data.
*/
int swagEncodingFromFile(FILE * fp, int fileSize, size_t * numBomChars, huLoadParams * loadParams)
{
    int const BLOCKSIZE = 64;

    char block[BLOCKSIZE];
    int blockSize = fileSize % BLOCKSIZE;
    int bytesRead = 0;
    int numValidEncodings = HU_ENCODING_UNKNOWN;
    ReadState * selectedEncoding = NULL;
    ReadState readers[HU_ENCODING_UNKNOWN];

    initReaders(readers, loadParams);

    blockSize = fread(block, 1, BLOCKSIZE, fp);
    if (blockSize == 0)
        { return HU_ENCODING_UNKNOWN; } // ERROR! WTF
    bytesRead += blockSize;

    huStringView sv = { .ptr = block, .size = blockSize };
    int bomEncoding = getEncodingFromBom(& sv, numBomChars);
    if (bomEncoding != HU_ENCODING_UNKNOWN)
        { return bomEncoding; }

    // scan through the input file in BLOCKSIZE-byte blocks
    do
    {
        swagEncodingFromBlock(block, blockSize, readers, & numValidEncodings);

        if (bytesRead < fileSize || numValidEncodings == 1)
            { break; }

        blockSize = fread(block, 1, BLOCKSIZE, fp);
        if (blockSize == 0)
            { return HU_ENCODING_UNKNOWN; } // ERROR! WTF
        bytesRead += blockSize;
    }
    while (true);

    selectedEncoding = chooseFromAmbiguousEncodings(readers, numValidEncodings);
    if (selectedEncoding != NULL)
        { return selectedEncoding - readers; }
    else
        { return HU_ENCODING_UNKNOWN; }
}


size_t appendEncodedUtf8CodePoint(char * dest, uint32_t codePoint)
{
    int encodedLen = 0;

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


size_t transcodeToUtf8FromBlock_utf8(char * dest, char const * block, int blockSize, ReadState * reader)
{
    uint8_t const * uCur = (uint8_t const *) block;
    int encodedLen = 0;

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


size_t transcodeToUtf8FromBlock_utf16be(char * dest, char const * block, int blockSize, ReadState * reader)
{
    uint16_t const * uCur = (uint16_t const *) block;
    int encodedLen = 0;

    while (uCur < (uint16_t const *) (block + blockSize))
    {
        uint16_t codeUnit = * uCur;
        scanUtf16(codeUnit, reader);
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


size_t transcodeToUtf8FromBlock_utf16le(char * dest, char const * block, int blockSize, ReadState * reader)
{
    uint16_t const * uCur = (uint16_t const *) block;
    int encodedLen = 0;

    while (uCur < (uint16_t const *) (block + blockSize))
    {
        uint16_t codeUnit = * uCur;
        scanUtf16(codeUnit, reader);
        codeUnit = ((codeUnit & 0x00ff) << 8) | ((codeUnit & 0xff00) >> 8);
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


size_t transcodeToUtf8FromBlock_utf32be(char * dest, char const * block, int blockSize, ReadState * reader)
{
    uint32_t const * uCur = (uint32_t const *) block;
    int encodedLen = 0;

    while (uCur < (uint32_t const *) (block + blockSize))
    {
        uint32_t codeUnit = * uCur;
        scanUtf32(codeUnit, reader);
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


size_t transcodeToUtf8FromBlock_utf32le(char * dest, char const * block, int blockSize, ReadState * reader)
{
    uint32_t const * uCur = (uint32_t const *) block;
    int encodedLen = 0;

    while (uCur < (uint32_t const *) (block + blockSize))
    {
        uint32_t codeUnit = * uCur;
        scanUtf32(codeUnit, reader);
        codeUnit = ((codeUnit & 0xff000000) >> 24) |
                   ((codeUnit & 0x00ff0000) >> 8) |
                   ((codeUnit & 0x0000ff00) << 8) |
                   ((codeUnit & 0x000000ff) << 24);
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


size_t transcodeToUtf8FromBlock(char * dest, char const * block, int blockSize, ReadState * reader)
{
    switch(reader->loadParams->encoding)
    {
    case HU_ENCODING_UTF8:
        return transcodeToUtf8FromBlock_utf8(dest, block, blockSize, reader);
    case HU_ENCODING_UTF16_BE:
        return reader->machineIsBigEndian
            ? transcodeToUtf8FromBlock_utf16be(dest, block, blockSize, reader)
            : transcodeToUtf8FromBlock_utf16le(dest, block, blockSize, reader);
    case HU_ENCODING_UTF16_LE:
        return reader->machineIsBigEndian
            ? transcodeToUtf8FromBlock_utf16le(dest, block, blockSize, reader)
            : transcodeToUtf8FromBlock_utf16be(dest, block, blockSize, reader);
    case HU_ENCODING_UTF32_BE:
        return reader->machineIsBigEndian
            ? transcodeToUtf8FromBlock_utf32be(dest, block, blockSize, reader)
            : transcodeToUtf8FromBlock_utf32le(dest, block, blockSize, reader);
    case HU_ENCODING_UTF32_LE:
        return reader->machineIsBigEndian
            ? transcodeToUtf8FromBlock_utf32le(dest, block, blockSize, reader)
            : transcodeToUtf8FromBlock_utf32be(dest, block, blockSize, reader);
    default:
        return 0;
    }
}


/*  PRE: dest points to a string at least as long as srcLen.
    PRE: srcEncoding is not HU_ENCODING_UNKNOWN
    Sets *numBytesEncoded and returns a HU_ERROR_*.
*/
int transcodeToUtf8FromString(char * dest, size_t * numBytesEncoded, huStringView const * src, huLoadParams * loadParams)
{
    if (loadParams->encoding == HU_ENCODING_UNKNOWN)
        { return 0; }
    
    // faster codepath for UTF8->UTF8 transcoding
    // This path doesn't check for invalid or overlong UTF8 sequences. It just
    // slams memory. You can separately specify strictUnicode for output for
    // huTroveTo* functions. Humon won't be fooled by overlong imposters.
    if (loadParams->encoding == HU_ENCODING_UTF8 && 
        loadParams->allowOutOfRangeCodePoints == true &&
        loadParams->allowIllegalCodePoints == true &&
        loadParams->allowOverlongEncodings == true)
    {
        // skip the BOM if there is one
        if (memcmp(src, utf8_bom, sizeof(utf8_bom)) == 0)
            { memcpy(dest, src->ptr + sizeof(utf8_bom), src->size - sizeof(utf8_bom)); }
        else
            { memcpy(dest, src->ptr, src->size); }
        return src->size;
    }
    else
    {
        int const BLOCKSIZE = HUMON_FILE_BLOCK_SIZE;

        ReadState reader;
        reader.loadParams = loadParams;
        reader.maybe = true;
        reader.partialCodePoint = 0;
        reader.bytesRemaining = 0;
        reader.numAsciiRangeCodePoints = 0;
        reader.numCodePoints = 0;
        reader.errorCode = 0;
        reader.errorOffset = 0;
        reader.machineIsBigEndian = isMachineBigEndian();

        int encodedLen = 0;

        char const * block = src->ptr;
        int blockSize = min(src->size, BLOCKSIZE);
        int bytesRead = 0;

        // skip the BOM if there is one
        char const * bom = bomDefs[loadParams->encoding];
        int bomLen = strlen((char const *) bom);
        if (memcmp(src, bom, bomLen) == 0)
        {
            block += bomLen;
            blockSize -= bomLen;
        }

        // scan through the input string in BLOCKSIZE-byte blocks
        while (bytesRead < src->size)
        {
            int enc = transcodeToUtf8FromBlock(dest, block, blockSize, & reader);
            if (reader.errorCode != 0)
            {
                * numBytesEncoded = 0;
                return HU_ERROR_BADENCODING;
            }

            encodedLen += enc;
            
            bytesRead += blockSize;
            block += BLOCKSIZE;
            blockSize = min(BLOCKSIZE, src->size - bytesRead);
        }

        * numBytesEncoded = encodedLen;
        return HU_ERROR_NOERROR;
    }
}


int transcodeToUtf8FromFile(char * dest, size_t * numBytesEncoded, FILE * fp, int srcLen, huLoadParams * loadParams)
{
    if (loadParams->encoding == HU_ENCODING_UNKNOWN)
        { return 0; }
    
    // faster codepath for UTF8->UTF8 transcoding
    // This path doesn't check for invalid or overlong UTF8 sequences. It just
    // slams memory. You can separately specify strictUnicode for output for
    // huTroveTo* functions. Humon won't be fooled by overlong imposters.
    if (loadParams->encoding == HU_ENCODING_UTF8 && 
        loadParams->allowOutOfRangeCodePoints == true &&
        loadParams->allowIllegalCodePoints == true &&
        loadParams->allowOverlongEncodings == true)
        { return fread(dest, 1, srcLen, fp); }
    else
    {
        int const BLOCKSIZE = HUMON_FILE_BLOCK_SIZE;

        ReadState reader;
        reader.loadParams = loadParams;
        reader.maybe = true;
        reader.partialCodePoint = 0;
        reader.bytesRemaining = 0;
        reader.numAsciiRangeCodePoints = 0;
        reader.numCodePoints = 0;
        reader.errorCode = 0;
        reader.errorOffset = 0;
        reader.machineIsBigEndian = isMachineBigEndian();

        int encodedLen = 0;

        // As stack allocations go, this might be large. Set
        // HUMON_FILE_BLOCK_SIZE to change it.
        char block[BLOCKSIZE];
        int blockSize = srcLen % BLOCKSIZE;
        int bytesRead = 0;

        // scan through the input file in BLOCKSIZE-byte blocks
        while (bytesRead < srcLen)
        {
            blockSize = fread(block, 1, BLOCKSIZE, fp);
            if (blockSize == 0)
                { return HU_ENCODING_UNKNOWN; } // ERROR! WTF

            bytesRead += blockSize;
            size_t enc = transcodeToUtf8FromBlock(dest, block, blockSize, & reader);
            if (reader.errorCode != 0)
            {
                * numBytesEncoded = 0;
                return HU_ERROR_BADENCODING;
            }

            encodedLen += enc;
            blockSize = min(BLOCKSIZE, srcLen - bytesRead);
        }

        * numBytesEncoded = encodedLen;
        return HU_ERROR_NOERROR;
    }
}
