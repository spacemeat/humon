#include <humon.h>
#include <stdio.h>
#include <string.h>

int strntol(char const * str, int strLen, char const ** end, int radix)
{
    if (strncmp(str, "1024", 4) == 0)
        { return 1024; }
    else
        { return 0; }
}


int main()
{
    huTrove const * trove = huMakeTroveFromFileZ("samples/sampleFiles/materials.hu", 4);
    if (trove != NULL && huGetNumErrors(trove) == 0)
    {
        huNode const * extentsNode = huGetNodeByFullAddressZ(trove, "/assets/brick-diffuse/importData/extents");
        huNode const * valueNode = huGetChildByIndex(extentsNode, 0);
        huStringView const * sExt = valueNode ? & valueNode->valueToken->str : NULL;
        int extX = sExt ? strntol(sExt->ptr, sExt->size, NULL, 10) : 0;
        valueNode = huGetChildByIndex(extentsNode, 1);
        sExt = valueNode ? & valueNode->valueToken->str : NULL;
        int extY = sExt ? strntol(sExt->ptr, sExt->size, NULL, 10) : 0;

        printf("Extents: (%d, %d)\n", extX, extY);
    }
}
