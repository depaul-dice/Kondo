#define _GNU_SOURCE

#include "logUtilities.h"

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

/// @brief Add the given call to the given metadata structure
/// @param metadata metadata structure of file to which we want to add the call
/// @param call Call to add to the structure
void addCall(fileMetadata *metadata, CallList *call)
{
    if (metadata->listOfCalls == NULL)
    {
        metadata->listOfCalls = call;
        call->pNext = NULL;
        call->pPrev = NULL;
    }
    else
    {
        call->pNext = metadata->listOfCalls;
        call->pPrev = NULL;
        metadata->listOfCalls->pPrev = call;
        call->timeStamp = metadata->listOfCalls->timeStamp + 1;
        metadata->listOfCalls = call;
    }
}

/// @brief Given a write tree and the intervals to a read call return
/// a list of intervals to append to the write tree
/// @param writeTree Write tree for the given file
/// @param readInterval  Interval of the current read to process
/// @return NULL if nothing to add a LL of intervals otherwise
NodeList *getNonIntersectingRead(Node *writeTree, Interval *readInterval)
{
    NodeList *pIntersection = NULL;
    NodeList *pLeftover = NULL;
    getIntersectionsAndChopInterval(writeTree, readInterval, &pIntersection, &pLeftover);
    return pLeftover;
}

/// @brief Print all the metadata for the given file
/// @param metadata Metadata structure for which we have to print the metadata for
/// @param stream Stream to print it to
void printMetadata(fileMetadata *metadata, FILE* stream)
{
    fprintf(stream, "%s:%ld\n",metadata->path, metadata->fileSize);
    fprintf(stream, "Read Tree\n");
    print_intervals(metadata->readTree, stream, 0);
    fprintf(stream, "Write Tree\n");
    print_intervals(metadata->writeTree, stream, 0);
    fprintf(stream, "Calls \n");
    printCalls(metadata, stream);
    fprintf(stream, "Backups \n");
    printBackup(metadata, stream);
    fflush(stream);
}

/// @brief Print the calls for the given file
/// @param metadata Metadata structure fo the file to pritn calls for
/// @param stream stream to print to
void printCalls(fileMetadata *metadata, FILE *stream)
{
    CallList *call = metadata->listOfCalls;
    while (call != NULL)
    {
        if(call->hash==NULL)
            fprintf(stream, "%s %ld %ld %ld %d %s\n", getCharOfCall(call->type), call->timeStamp, call->offset, call->size, call->other, "none");
        else
        {
            fprintf(stream, "%s %ld %ld %ld %d ", getCharOfCall(call->type), call->timeStamp, call->offset, call->size, call->other);
            for(int i = 0; i <HASH_LEN; i++)
            {
                fprintf(stream, "%02x", call->hash[i]);
            }
            fprintf(stream, "\n");
        }
        call = call->pNext;
    }
}

/// @brief Given a file metadata structure and a write interval perform the backup
/// and remove neccessary parts from the write tree
/// @param metadata File metadata structure of the file in question
/// @param writeInterval Interval for the write call
void performBackup(fileMetadata *metadata, Interval *writeInterval)
{
    NodeList *pIntersection = NULL;
    metadata->readTree = chopAndReturn(metadata->readTree, *writeInterval, &pIntersection);
    // add the backups to the file bny reading it etc
    NodeList *pCur = pIntersection;
    char* buf;
    while (pCur != NULL)
    {
        // read the file
        buf = readFile(metadata->fptr, metadata->fd, pCur->pInterval->low, pCur->pInterval->high-pCur->pInterval->low, metadata->filePointer);
        // flush the backup
        flushBackup(metadata->path, buf);
        // add the backup
        addBackup(metadata, pCur->pInterval);
        pCur = pCur->pNext;
    }
}

/// @brief Flush the char buf to the apt backup file for the file we are given the path of
/// @param fName abs path of file we want to flush our backup for
/// @param buf buf we want to flush
void flushBackup(char *fName, char *buf)
{
    char *fileName = strrchr(fName, '/');
    char *buf1 = calloc(1, PATH_MAX);
    strcpy(buf1, BACKUP_DEST);
    strcat(buf1, fileName + 1);
    FILE *newFPTR = getSysData()->functions->real_fopen(buf1, "a+");
    getSysData()->functions->real_fwrite(buf, strlen(buf), 1, newFPTR);
    getSysData()->functions->real_fclose(newFPTR);
}

/// @brief Add the backup structure to the file metadata strucutre
/// @param metadata File emtadata for th efile to add backup to
/// @param interval Interval of file for which we are adding the backup
void addBackup(fileMetadata *metadata, Interval *interval)
{
    if (metadata->backups != NULL)
    {
        metadata->backups->pPrev = malloc(sizeof(NodeList));
        metadata->backups->pPrev->pInterval = interval;
        metadata->backups->pPrev->pNext = metadata->backups;
        metadata->backups->pPrev->pPrev = NULL;
        metadata->backups = metadata->backups->pPrev;
    }
    else
    {
        metadata->backups = malloc(sizeof(NodeList));
        metadata->backups->pNext = NULL;
        metadata->backups->pPrev = NULL;
        metadata->backups->pInterval = interval;
    }
}

/// @brief Read size amount of data at readOffset and then reset file pointer to to oldOffset
/// @param fptr File pointer of the file to read from
/// @param fd File desc of the file to read from
/// @param readOffset offset to read from
/// @param size size to read
/// @param oldOffset offset to set to after read
/// @return char buffer of data read
char *readFile(FILE *fptr, int fd, off_t readOffset, off_t size, off_t oldOffset)
{
    char *buf = malloc(size);
    if(fptr != NULL)
    {
        getSysData()->functions->real_fseek(fptr, readOffset, SEEK_SET);
        getSysData()->functions->real_fread(buf, size, 1, fptr);
        getSysData()->functions->real_fseek(fptr, oldOffset, SEEK_SET);
    }
    else
    {

    }
    return buf;
}

/// @brief Given a file metadata struct print all the backups info into stream provided
/// @param metadata File metadata struct to print for
/// @param stream Stream to print to
void printBackup(fileMetadata *metadata, FILE *stream)
{
    NodeList *node = metadata->backups;
    while (node != NULL)
    {
        fprintf(stream, "%d:%d\n", node->pInterval->low, node->pInterval->high);
        node = node->pNext;
    }
}

unsigned char *getSHA256(void *buf, int len)
{
    const char ibuf[] = "compute sha1";
    unsigned char* ret = malloc(HASH_LEN);

    SHA256((const unsigned char*) buf, len, ret);
    return ret;
    
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
    case FOPEN64:
        /* code */
        return "fopen64";
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
        return "lstat";
        break;
    default:
        return "Undefined";
        break;
    }
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