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
    getSysData()->functions->real_fopen64 = dlsym(RTLD_NEXT, "fopen64");
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

    getSysData()->functions->real_fstat = dlsym(RTLD_NEXT, "fstat");
    getSysData()->functions->real_fstat64 = dlsym(RTLD_NEXT, "fstat64");
    getSysData()->functions->real_lstat = dlsym(RTLD_NEXT, "lstat");
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
            // HASH_FIND(pathHandle, getSysData()->openFiles->filePaths, path, strlen(path), cur);
            // if (cur == NULL)
            // {
            //     return 1;
            // }
            // else
            // {
            //     return 0;
            // }
            return 1;
        }
    }
    return 0;
}

void getFilePaths(char* fileName, char*tracePath, char* subsetPath, char* ptrPath, char* writeCache)
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

        strcpy(writeCache, baseBuf);
        strcat(writeCache,".writeCache");
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
        curFile->subsetTree = insertNoCombine(curFile->subsetTree, (Interval){low, high, off, 0});
        
    }
}

#define MAX_LINE_LENGTH 1000

void readTrace(fileMetadata* curFile, char* tracePath)
{
    FILE* traceFPTR = getSysData()->functions->real_fopen(tracePath, "r");  
    CallList* newCall;

    char line[MAX_LINE_LENGTH];
    while(fgets(line, MAX_LINE_LENGTH, traceFPTR)!= NULL) /* read a line from a file */ 
    {
        newCall = malloc(sizeof(CallList));
        char* ret = strrchr(line, ':');
        memcpy(newCall->hash, ret+1, HASH_LEN);
        newCall->hash[HASH_LEN] = 0;
      
        newCall->type = getType(strtok(line, ":"));
        newCall->timeStamp = atol(strtok(NULL, ":"));
        newCall->offset = atol(strtok(NULL, ":"));
        newCall->size = atol(strtok(NULL, ":"));
        newCall->other = atol(strtok(NULL, ":"));

        addCall(curFile, newCall);
    }   
}

enum CallType getType(char* call)
{
    if(strcmp("fopen", call)==0)
        return FOPEN;
    if(strcmp("fopen64", call)==0)
        return FOPEN64;
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

    if(strcmp("fstat", call)==0)
        return FSTAT;
    if(strcmp("fstat64", call)==0)
        return FSTAT64;
    if(strcmp("lstat", call)==0)
        return LSTAT;
    
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
        metadata->originalTrace = call;
    }
}


/// @brief Print the calls for the given file
/// @param metadata Metadata structure fo the file to pritn calls for
/// @param stream stream to print to
void printCalls(fileMetadata *metadata, FILE *stream)
{
    CallList *call = metadata->originalTrace;
    while (call != NULL)
    {
        fprintf(stream, "%s %ld %ld %ld %d\n", getCharOfCall(call->type), call->timeStamp, call->offset, call->size, call->other);
        call = call->pNext;
    }
}

/// @brief Give the char version of the call type enum
/// @param  enumeration of the type of call
/// @return a char ptr to the call type string representation
char *getCharOfCall(enum CallType type)
{
    switch (type)
    {
    case FOPEN:
        /* code */
        return "fopen";
        break;   
    case OPENAT:
        /* code */
        return "openat";
        break;
    case OPEN:
        /* code */
        return "open";
        break;
    case OPEN64:
        /* code */
        return "open64";
        break;
    case FREAD:
        /* code */
        return "fread";
        break;
    case PREAD:
        /* code */
        return "pread";
        break;
    case PREAD64:
        /* code */
        return "pread64";
        break;
    case PREADCHK:
        /* code */
        return "preadchk";
        break;
    case READ:
        /* code */
        return "read";
        break;
    case FWRITE:
        /* code */
        return "fwrite";
        break;
    case WRITE:
        /* code */
        return "write";
        break;
    case PWRITE:
        /* code */
        return "Pwrite";
        break;
    case FSEEK:
        /* code */
        return "fseek";
        break;
    case LSEEK64:
        /* code */
        return "lseek64";
        break;
    case LSEEK:
        /* code */
        return "lseek";
        break;
    case FCLOSE:
        /* code */
        return "fclose";
        break;
    case CLOSE:
        /* code */
        return "close";
        break;
    case FSTAT:
        /* code */
        return "fstat";
        break;
    case FSTAT64:
        /* code */
        return "fstat64";
        break;
    case LSTAT:
        /* code */
        return "lstat";
        break;
    default:
        return "Undefined";
        break;
    }
}


/* Function to reverse a Doubly Linked List */
void reverse(NodeList** head_ref)
{
    NodeList* temp = NULL;
    NodeList* current = *head_ref;
 
    /* swap next and prev for all nodes of
    doubly linked list */
    while (current != NULL) {
        temp = current->pPrev;
        current->pPrev = current->pNext;
        current->pNext = temp;
        current = current->pPrev;
    }
 
    /* Before changing the head, check for the cases like
       empty list and list with only one node */
    if (temp != NULL)
        *head_ref = temp->pPrev;
}

void getBytes(fileMetadata* metadata, NodeList* pHead, void* ptr)
{
    NodeList* pCur = pHead;
    off_t size = 0;
    while(pCur!=NULL)
    {
        if(pCur->pInterval->fileFlag == 0)
        {
            getSysData()->functions->real_fseek(metadata->subsetHandle, pCur->pInterval->off, SEEK_SET);
            getSysData()->functions->real_fread(ptr+size,  pCur->pInterval->high - pCur->pInterval->low, 1, metadata->subsetHandle);
        }
        else
        {
            getSysData()->functions->real_fseek(metadata->writeCache, pCur->pInterval->off, SEEK_SET);
            getSysData()->functions->real_fread(ptr+size,  pCur->pInterval->high - pCur->pInterval->low, 1, metadata->writeCache);
        }
        size += pCur->pInterval->high - pCur->pInterval->low;
        pCur = pCur->pNext;
    }
}


void compareCalls(fileMetadata* metadata, CallList* curCall)
{
    CallList* cmpCall = metadata->currentCall;
    int flg = 0;
    //fprintf(stdout, "comparing: %s og to %s now\n",getCharOfCall(cmpCall->type), getCharOfCall(curCall->type));
    if(curCall->type != cmpCall->type)
    {
        fprintf(stdout, "Types don't match\n");
        flg = 1;
    }
    if(curCall->size != cmpCall->size)
    {
        fprintf(stdout, "Size doesn't match\n");
        flg = 1;
    }
    if(curCall->offset != cmpCall->offset)
    {
        fprintf(stdout, "Offset doesn't match\n");
        flg = 1;
    }
    if(metadata->curTimeStamp != cmpCall->timeStamp)
    {
        fprintf(stdout, "Timestamps don't match\n");
        flg = 1;
    }
    if(curCall->other != cmpCall->other)
    {
        fprintf(stdout, "Others don't match\n");
        flg = 1;
    }
    if(strcmp(cmpCall->hash, curCall->hash)!=0)
    {
        fprintf(stdout, "Hashes don't match\n");
        flg = 1;
    }
    if(flg == 0)
    {
        //fprintf(stdout, "calls matched\n");
    }
    metadata->currentCall = metadata->currentCall->pNext;
    metadata->curTimeStamp +=1;
    
}
size_t flushToCache(fileMetadata* metadata, FILE* fptr, const void* ptr, off_t wrtteSize)
{
    // seek to end of file 
    getSysData()->functions->real_fseek(fptr, 0, SEEK_END);
    // write
    size_t dat = getSysData()->functions->real_fwrite(ptr, wrtteSize, 1, fptr);

    // return the
    size_t tmp = metadata->writeCacheSize;
    metadata->writeCacheSize += wrtteSize;
    return tmp;
}

unsigned char *getSHA256(void *buf, int len)
{
    const char ibuf[] = "compute sha1";
    unsigned char* ret = malloc(HASH_LEN);

    SHA256((const unsigned char*) buf, len, ret);
    return ret;
    
}