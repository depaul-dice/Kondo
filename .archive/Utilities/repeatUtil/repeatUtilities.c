#define _GNU_SOURCE
#include <limits.h>
#include <sys/types.h>
#include <dlfcn.h>
#include "repeatUtilities.h"


/// @brief gets the structure which holds all metadata for the system
/// @return metadata strucutre for the system
SysData *getSysData()
{
    return systemData;
}

/// @brief create all the function pointers to the real versions of the
/// i/o functions
void initRealFunctions()
{
    systemData->functions = malloc(sizeof(realFunctions));

    getSysData()->functions->real_fopen = dlsym(RTLD_NEXT, "fopen");
    getSysData()->functions->real_open = dlsym(RTLD_NEXT, "open");
    getSysData()->functions->real_open64 = dlsym(RTLD_NEXT, "open64");

    getSysData()->functions->real_fclose = dlsym(RTLD_NEXT, "fclose");
    getSysData()->functions->real_close = dlsym(RTLD_NEXT, "close");

    getSysData()->functions->real_fread = dlsym(RTLD_NEXT, "fread");
    getSysData()->functions->real_pread = dlsym(RTLD_NEXT, "pread");
    getSysData()->functions->real_pread64 = dlsym(RTLD_NEXT, "pread64");
    getSysData()->functions->real_read = dlsym(RTLD_NEXT, "read");
    getSysData()->functions->real___pread_chk = dlsym(RTLD_NEXT, "__pread_chk");

    getSysData()->functions->real_fwrite = dlsym(RTLD_NEXT, "fwrite");
    getSysData()->functions->real_write = dlsym(RTLD_NEXT, "write");
    getSysData()->functions->real_pwrite = dlsym(RTLD_NEXT, "pwrite");


    getSysData()->functions->real_fseek = dlsym(RTLD_NEXT, "fseek");
    getSysData()->functions->real_lseek = dlsym(RTLD_NEXT, "lseek");
    getSysData()->functions->real_lseek64 = dlsym(RTLD_NEXT, "lseek64");
}

/// @brief Check whether the file at the given path is supposed to be interposed or not
/// @param path path to the file we are checking interposition for
/// @return 1 if the file is supposed to be interposed 0 if not
int checkInterposition(char *path)
{
    pathList *cur, *tmp, *cur1, *tmp1;
    HASH_ITER(hh, getSysData()->IOList->whiteList, cur, tmp)
    {
        // Check if path is in white list
        if (strcmp(cur->path, path) == 0 || (strstr(path, cur->path) != NULL))
        {
            HASH_ITER(hh, getSysData()->IOList->blackList, cur1, tmp1)
            {
                // check if it is in black list
                if ((strcmp(cur1->path, path) == 0) || (strstr(path, cur1->path) != NULL))
                {
                    return 0;
                }
            }
            openFile *cur;
            // Check if it is already opened or not
            HASH_FIND(pathHandle, getSysData()->openFiles->filePaths, path, strlen(path), cur);
            if (cur == NULL)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    return 0;
}

/// @brief Get the metadata structure for ther file whos' pointer is given
/// @param fptr file pointer to the file we want metadata of
/// @param fd file descriptor of file to find metadata of
/// @return the file metadata structure
fileMetadata *getMetadata(FILE *fptr, int fd)
{
    openFile *cur = NULL;
    if(fptr != NULL)
    HASH_FIND(fptrHandle, getSysData()->openFiles->filePtrs, &fptr, sizeof(FILE *), cur);
    else
    HASH_FIND(fdHandle, getSysData()->openFiles->fileDescs, &fd, sizeof(int), cur);
    fileMetadata *ret = NULL;
    HASH_FIND(pathHandle, getSysData()->metaadata, &cur->path, strlen(cur->path), ret);
    return ret;
}


/// @brief Check if we are tracking the file of given path
/// @param path Path to check
/// @return 1 if tracking else 0
int inList(char *path)
{
    pathList *cur, *tmp, *cur1, *tmp1;
    HASH_ITER(hh, getSysData()->IOList->whiteList, cur, tmp)
    {
        // Check if path is in white list
        if (strcmp(cur->path, path) == 0 || (strstr(path, cur->path) != NULL))
        {
            HASH_ITER(hh, getSysData()->IOList->blackList, cur1, tmp1)
            {
                // check if it is in black list
                if ((strcmp(cur1->path, path) == 0) || (strstr(path, cur1->path) != NULL))
                {
                    return 0;
                }
            }
            openFile *cur;
            // Check if it is already opened or not
            HASH_FIND(pathHandle, getSysData()->openFiles->filePaths, path, strlen(path), cur);
            if (cur == NULL)
            {
                return 1;
            }
            else
            {
                return 0;
            }
        }
    }
    return 0;
}

void getFilePaths(char* fileName, char*tracePath, char* subsetPath, char* ptrPath)
{
        char* fName = strrchr(fileName, '/');
        if(fName == NULL)
        {
            fName = (char *)fileName;
        }

        char baseBuf[PATH_MAX] = {0};
        strcpy(baseBuf, SUBSET_DIR);
        strcat(baseBuf, fName);
        
        strcpy(tracePath, baseBuf);
        strcat(tracePath,".trace");
        
        strcpy(subsetPath, baseBuf);
        strcat(subsetPath,".subset");
        
        strcpy(ptrPath, baseBuf);
        strcat(ptrPath,".pointers");
}

void readSubsetTree(fileMetadata* curFile, char* ptrPath)
{
    FILE* ptrFPTR = getSysData()->functions->real_fopen(ptrPath, "r");
    fscanf(ptrFPTR, "%s", curFile->path);
    fscanf(ptrFPTR, "%d", &curFile->fileSize);
    int low;
    int high;
    int off;
    while(1)
    {
        if(fscanf(ptrFPTR, "%d:%d:%d", &low, &high, &off) == EOF)
            break;
        curFile->subsetTree = insertNoCombine(curFile->subsetTree, (Interval){low, high, off});
        
    }
}


void readTrace(fileMetadata* curFile, char* tracePath)
{
    FILE* traceFPTR = getSysData()->functions->real_fopen(tracePath, "r");  
    CallList* newCall;
    char* tmp;
    while(1)
    {
        newCall = malloc(sizeof(CallList));
        int ret = fscanf(traceFPTR, "%s %d %d %d %d\n", tmp, &newCall->timeStamp, &newCall->offset, &newCall->size, &newCall->other );
        if(ret == EOF)
            break;
        newCall->type = getType(tmp);
        addCall(curFile, newCall);
    }
}

enum CallType getType(char* call)
{
    if(strcmp("fopen", call)==0)
        return FOPEN;
    if(strcmp("open", call)==0)
        return OPEN;
    if(strcmp("open64", call)==0)
        return OPEN64;
    if(strcmp("openat", call)==0)
        return OPENAT;

    if(strcmp("fclose", call)==0)
        return FCLOSE;
    if(strcmp("close", call)==0)
        return CLOSE;


    if(strcmp("fread", call)==0)
        return FREAD;
    if(strcmp("pread", call)==0)
        return PREAD;
    if(strcmp("read", call)==0)
        return READ;
    if(strcmp("pread64", call)==0)
        return PREAD64;
    if(strcmp("preadchk", call)==0)
        return PREADCHK;
    
    
    if(strcmp("fwrite", call)==0)
        return FWRITE;
    if(strcmp("write", call)==0)
        return WRITE;
    if(strcmp("pwrite", call)==0)
        return PWRITE;
    

    if(strcmp("fseek", call)==0)
        return FSEEK;
    if(strcmp("lseek", call)==0)
        return LSEEK;
    if(strcmp("lseek64", call)==0)
        return LSEEK64;
    return UNINIT;
}
/// @brief Add the given call to the given metadata structure
/// @param metadata metadata structure of file to which we want to add the call
/// @param call Call to add to the structure
void addCall(fileMetadata *metadata, CallList *call)
{
    if (metadata->originalTrace == NULL)
    {
        metadata->originalTrace = call;
        call->pNext = NULL;
        call->pPrev = NULL;
    }
    else
    {
        call->pNext = metadata->originalTrace;
        call->pPrev = NULL;
        metadata->originalTrace->pPrev = call;
        call->timeStamp = metadata->originalTrace->timeStamp + 1;
        metadata->originalTrace = call;
    }
}