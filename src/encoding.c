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


char const utf8_bom[]    = { 0xef, 0xbb, 0xbf };
char const utf16be_bom[] = { 0xfe, 0xff };
char const utf16le_bom[] = { 0xff, 0xfe };
char const utf32be_bom[] = { 0x00, 0x00, 0xfe, 0xff };
char const utf32le_bom[] = { 0xff, 0xfe, 0x00, 0x00 };

char const * const bomDefs[] = { utf8_bom, utf16be_bom, utf16le_bom, utf32be_bom, utf32le_bom, NULL };
int const bomSizes[] = { sizeof(utf8_bom), sizeof(utf16be_bom), sizeof(utf16le_bom), sizeof(utf32be_bom), sizeof(utf32le_bom), 0 };

typedef struct ReadState_tag
{
    huDeserializeOptions * DeserializeOptions;
    bool maybe;
    uint32_t partialCodePoint;
    int bytesRemaining;
    int numCodePoints;
    int numNuls;
    int numAsciiRangeCodePoints;
    int errorCode;
    char const * errorOffset;
    bool machineIsBigEndian;
} ReadState;


static void initReaders(ReadState readers[], huDeserializeOptions * DeserializeOptions)
{
    bool machineIsBigEndian = isMachineBigEndian();

    readers[HU_ENCODING_UTF8] = (ReadState)
    {
        .DeserializeOptions = DeserializeOptions,
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
        .DeserializeOptions = DeserializeOptions,
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
        .DeserializeOptions = DeserializeOptions,
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
        .DeserializeOptions = DeserializeOptions,
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
        .DeserializeOptions = DeserializeOptions,
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
        else if (rs->DeserializeOptions->allowOutOfRangeCodePoints == false &&
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
        else if (rs->DeserializeOptions->allowUtf16UnmatchedSurrogates == false)
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
        else if (rs->DeserializeOptions->allowUtf16UnmatchedSurrogates)
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
        rs->DeserializeOptions->allowOutOfRangeCodePoints)
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


static void swagEncodingFromBlock(char const * block, int blockSize, ReadState * readers, int * numValidEncodings)
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


static ReadState * chooseFromAmbiguousEncodings(ReadState readers[], int numValidEncodings)
{
    // If we finished scanning while expecting more bytes in a multi-codeUnit 
    // encoding, that encoding is bunk. (UTF32 never has more than one code unit.)
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
            readers[HU_ENCODING_UTF16_BE].DeserializeOptions->allowUtf16UnmatchedSurrogates == false) 
        {
            readers[HU_ENCODING_UTF16_BE].maybe = false;
            numValidEncodings -= 1;
        }
        if (readers[HU_ENCODING_UTF16_LE].maybe &&
            readers[HU_ENCODING_UTF16_LE].bytesRemaining &&
            readers[HU_ENCODING_UTF16_BE].DeserializeOptions->allowUtf16UnmatchedSurrogates == false)
        {
            readers[HU_ENCODING_UTF16_LE].maybe = false;
            numValidEncodings -= 1;
        }
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
                    if (readers[i].numNuls < selectedEncoding->numNuls)
                        { selectedEncoding = readers + i; }
                    else if (readers[i].numNuls == selectedEncoding->numNuls)
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
    }

    return selectedEncoding;
}


static int getEncodingFromBom(huStringView const * data, size_t * numBomChars, bool isMachineBigEndian)
{
    int encoding = HU_ENCODING_UNKNOWN;

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
        * numBomChars = bomSizes[encoding];
        return encoding;
    }

    * numBomChars = 0;
    return HU_ENCODING_UNKNOWN;
}


int swagEncodingFromString(huStringView const * data, size_t * numBomChars, huDeserializeOptions * DeserializeOptions)
{
    int bomEncoding = getEncodingFromBom(data, numBomChars, isMachineBigEndian());

#ifdef HUMON_CAVEPERSON_DEBUGGING
    if (bomEncoding != HU_ENCODING_UNKNOWN)
        { printf("Encoding detected from BOM: %d\n", bomEncoding); }
    else
        { printf("Encoding not detected from BOM\n"); }
#endif

    if (bomEncoding != HU_ENCODING_UNKNOWN)
        { return bomEncoding; }

    int const BLOCKSIZE = 64;

    int numValidEncodings = HU_ENCODING_UNKNOWN;
    ReadState * selectedEncoding = NULL;
    ReadState readers[HU_ENCODING_UNKNOWN];

    initReaders(readers, DeserializeOptions);

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

#ifdef HUMON_CAVEPERSON_DEBUGGING
    if (selectedEncoding != NULL)
        { printf("Encoding determined from bit pattern: %d\n", (int)(selectedEncoding - readers)); }
    else
        { printf("Encoding not determined from bit pattern\n"); }
#endif

    if (selectedEncoding != NULL)
        { return selectedEncoding - readers; }
    else
        { return HU_ENCODING_UNKNOWN; }
}


/*  PRE: fp is opened in binary mode, and is pointing to the beginning of the data.
*/
int swagEncodingFromFile(FILE * fp, int fileSize, size_t * numBomChars, huDeserializeOptions * DeserializeOptions)
{
    int const BLOCKSIZE = 64;

    int numValidEncodings = HU_ENCODING_UNKNOWN;
    ReadState * selectedEncoding = NULL;
    ReadState readers[HU_ENCODING_UNKNOWN];

    initReaders(readers, DeserializeOptions);

    char buf[BLOCKSIZE];
    char * block = buf;
    int blockSize = fileSize % BLOCKSIZE;
    int bytesRead = 0;

    blockSize = fread(block, 1, BLOCKSIZE, fp);
    if (blockSize == 0)
        { return HU_ENCODING_UNKNOWN; } // ERROR! WTF
    bytesRead += blockSize;

    huStringView sv = { .ptr = block, .size = blockSize };
    int bomEncoding = getEncodingFromBom(& sv, numBomChars, isMachineBigEndian());
    if (bomEncoding != HU_ENCODING_UNKNOWN)
        { return bomEncoding; }

    // scan through the input file in BLOCKSIZE-byte blocks
    do
    {
        swagEncodingFromBlock(block, blockSize, readers, & numValidEncodings);

        blockSize = fread(block, 1, BLOCKSIZE, fp);
        if (blockSize == 0)
            { return HU_ENCODING_UNKNOWN; } // ERROR! WTF
        block = buf;
        bytesRead += blockSize;
    }
    while (bytesRead < fileSize && numValidEncodings > 1);

    selectedEncoding = chooseFromAmbiguousEncodings(readers, numValidEncodings);
    if (selectedEncoding != NULL)
        { return selectedEncoding - readers; }
    else
        { return HU_ENCODING_UNKNOWN; }
}


static size_t appendEncodedUtf8CodePoint(char * dest, uint32_t codePoint)
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


static size_t transcodeToUtf8FromBlock_utf8(char * dest, char const * block, int blockSize, ReadState * reader)
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


static size_t transcodeToUtf8FromBlock_utf16be(char * dest, char const * block, int blockSize, ReadState * reader)
{
    char const * uCur = block;
    int encodedLen = 0;

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


static size_t transcodeToUtf8FromBlock_utf16le(char * dest, char const * block, int blockSize, ReadState * reader)
{
    char const * uCur = block;
    int encodedLen = 0;

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


static size_t transcodeToUtf8FromBlock_utf32be(char * dest, char const * block, int blockSize, ReadState * reader)
{
    char const * uCur = block;
    int encodedLen = 0;

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


static size_t transcodeToUtf8FromBlock_utf32le(char * dest, char const * block, int blockSize, ReadState * reader)
{
    char const * uCur = block;
    int encodedLen = 0;

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


static size_t transcodeToUtf8FromBlock(char * dest, char const * block, int blockSize, ReadState * reader)
{
    switch(reader->DeserializeOptions->encoding)
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
int transcodeToUtf8FromString(char * dest, size_t * numBytesEncoded, huStringView const * src, huDeserializeOptions * DeserializeOptions)
{
    if (DeserializeOptions->encoding == HU_ENCODING_UNKNOWN)
        { return 0; }
    
    // faster codepath for UTF8->UTF8 transcoding
    // This path doesn't check for invalid or overlong UTF8 sequences. It just
    // slams memory. You can separately specify strictUnicode for output for
    // huTroveTo* functions. Humon won't be fooled by overlong imposters.
    if (DeserializeOptions->encoding == HU_ENCODING_UTF8 && 
        DeserializeOptions->allowOutOfRangeCodePoints == true)
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
        reader.DeserializeOptions = DeserializeOptions;
        reader.maybe = true;
        reader.partialCodePoint = 0;
        reader.bytesRemaining = 0;
        reader.numNuls = 0;
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
        char const * bom = bomDefs[DeserializeOptions->encoding];
        int bomLen = bomSizes[DeserializeOptions->encoding];
        if (bomLen > 0 && memcmp(src->ptr, bom, bomLen) == 0)
        {
            block += bomLen;
            blockSize -= bomLen;
            bytesRead = bomLen;
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
            block += bytesRead;
            blockSize = min(BLOCKSIZE, src->size - bytesRead);
        }

        * numBytesEncoded = encodedLen;
        return HU_ERROR_NOERROR;
    }
}


int transcodeToUtf8FromFile(char * dest, size_t * numBytesEncoded, FILE * fp, int srcLen, huDeserializeOptions * DeserializeOptions)
{
    if (DeserializeOptions->encoding == HU_ENCODING_UNKNOWN)
        { return 0; }
    
    // faster codepath for UTF8->UTF8 transcoding
    // This path doesn't check for invalid or overlong UTF8 sequences. It just
    // slams memory. You can separately specify strictUnicode for output for
    // huTroveTo* functions. Humon won't be fooled by overlong imposters.
    if (DeserializeOptions->encoding == HU_ENCODING_UTF8 && 
        DeserializeOptions->allowOutOfRangeCodePoints == true)
        { return fread(dest, 1, srcLen, fp); }
    else
    {
        int const BLOCKSIZE = HUMON_FILE_BLOCK_SIZE;

        ReadState reader;
        reader.DeserializeOptions = DeserializeOptions;
        reader.maybe = true;
        reader.partialCodePoint = 0;
        reader.bytesRemaining = 0;
        reader.numNuls = 0;
        reader.numAsciiRangeCodePoints = 0;
        reader.numCodePoints = 0;
        reader.errorCode = 0;
        reader.errorOffset = 0;
        reader.machineIsBigEndian = isMachineBigEndian();

        int encodedLen = 0;

        // As stack allocations go, this might be large. Set
        // HUMON_FILE_BLOCK_SIZE to change it.
        char buf[BLOCKSIZE];
        char * block = buf;
        int blockSize = srcLen % BLOCKSIZE;
        int bytesRead = 0;

        blockSize = fread(block, 1, BLOCKSIZE, fp);
        if (blockSize == 0)
            { return HU_ENCODING_UNKNOWN; } // ERROR! WTF
        bytesRead += blockSize;

        // skip the BOM if there is one
        char const * bom = bomDefs[DeserializeOptions->encoding];
        int bomLen = bomSizes[DeserializeOptions->encoding];
        if (bomLen > 0 && memcmp(block, bom, bomLen) == 0)
        {
            block += bomLen;
            blockSize -= bomLen;
            bytesRead = bomLen;
        }

        do
        {
            size_t enc = transcodeToUtf8FromBlock(dest, block, blockSize, & reader);
            if (reader.errorCode != 0)
            {
                * numBytesEncoded = 0;
                return HU_ERROR_BADENCODING;
            }

            encodedLen += enc;
            bytesRead += blockSize;
            blockSize = min(BLOCKSIZE, srcLen - bytesRead);
            block = buf;
        } while (bytesRead < srcLen);
        
        * numBytesEncoded = encodedLen;
        return HU_ERROR_NOERROR;
    }
}
