#define _GNU_SOURCE

#include "repeatLibrary.h"


SysData *systemData;
int setup = 0;

FILE *fopen(const char *filename, const char *mode)
{
    if(setup == 0)
    {
        initSystem();
    }
    FILE* ret;
    if(checkTrackingPath(filename))
    {
        logOpen(filename, NULL, &ret, FOPEN);
        return ret;
    }
    return getSysData()->functions->real_fopen(filename, mode);
    return ret;
}