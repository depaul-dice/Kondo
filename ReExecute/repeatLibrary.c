#include "repeatLibrary.h"

/// @brief Initialise the system to start tracking
void initSystem()
{
    // call helper to initialize the real versions of all functions
    systemData = malloc(sizeof(SysData));
    initRealFunctions();

    // Setup the struct to hold the black List and white List
    SysData *curSys = getSysData();
    curSys->IOList = malloc(sizeof(specializationList));
    curSys->IOList->whiteList = NULL;
    curSys->IOList->blackList = NULL;

    // Setup the struct to hold all open files
    curSys->openFiles = malloc(sizeof(openFileList));
    curSys->openFiles->fileDescs = NULL;
    curSys->openFiles->filePtrs = NULL;
    curSys->openFiles->filePaths = NULL;
    curSys->metaadata = NULL;

    // Read White List
    FILE *fptr = curSys->functions->real_fopen("WhiteList.txt", "r");
    int n;
    fscanf(fptr, "%d\n", &n);
    pathList *newFile;
    for (int i = 0; i < n; i++)
    {
        newFile = malloc(sizeof(pathList));
        fscanf(fptr, "%s\n", newFile->path);
        HASH_ADD_STR(curSys->IOList->whiteList, path, newFile);
    }
    curSys->functions->real_fclose(fptr);

    // Read Black List
    fptr = curSys->functions->real_fopen("BlackList.txt", "r");
    fscanf(fptr, "%d\n", &n);
    for (int i = 0; i < n; i++)
    {
        newFile = malloc(sizeof(pathList));
        fscanf(fptr, "%s\n", newFile->path);
        HASH_ADD_STR(curSys->IOList->blackList, path, newFile);
    }
    curSys->functions->real_fclose(fptr);

    // If everything goes well set setup to 1
    setup = 1;
}


/// @brief Given the name of a file return if we are tracking the file or not
/// @param filename name of file to check tracking for
/// @return 1 if tracking 0 if not
int checkTrackingPath(const char *filename)
{
    if (setup == 0)
    {
        initSystem();
    }
    char path[PATH_MAX];
    char *res = realpath(filename, path);
    if (res == NULL)
    {
        return 0;
    }
    else
    {
        return inList(res);
    }
}

/// @brief Given the pointer of a file return if we are tracking the file or not
/// @param fptr pointer of file to check tracking for
/// @return 1 if tracking 0 if not
int checkTrackingFptr(FILE *fptr)
{
    if (setup == 0)
    {
        initSystem();
    }
    openFile *cur = NULL;
    HASH_FIND(fptrHandle, getSysData()->openFiles->filePtrs, &fptr, sizeof(FILE *), cur);
    if (cur != NULL)
    {
        return 1;
    }
    return 0;
}

/// @brief Given the desc of a file return if we are tracking the file or not
/// @param fd desc of file to check tracking for
/// @return 1 if tracking 0 if not
int checkTrackingDesc(int fd)
{
    if (setup == 0)
    {
        initSystem();
    }
    openFile *cur = NULL;
    HASH_FIND(fdHandle, getSysData()->openFiles->fileDescs, &fd, sizeof(int), cur);
    if (cur != NULL)
    {
        return 1;
    }
    return 0;
}

/// @brief Add allm thje apt structures for the given file
/// @param buf Path to the file
/// @param fd FD of the file or -1
/// @param fptr File pointer fo the file or NULL
/// @param type an Enumeration of what type of open call was made
void logOpen(const char *buf, int *fd, FILE **fptr, enum CallType type)
{
    // We have to first create the struct and if it is being opened for the first time
    if(setup == 0)
    {
        initSystem();
    }

    // Get absolute path
    char path[PATH_MAX];
    char *res = realpath(buf, path);

    //check if opening for the first time
    SysData* curSys = getSysData();
    fileMetadata *curFile = NULL;
    HASH_FIND(pathHandle, curSys->metaadata, path, strlen(path), curFile);

    CallList* curCall = malloc(sizeof(CallList));
    curCall->offset = -1;
    curCall->size = -1;
    curCall->other = -1;
    curCall->type = type;
    // if opening for the first time
    if (curFile== NULL)
    {
        curFile = malloc(sizeof(fileMetadata));
        // opened for the first time
        // read the trace, open the subset
        
        char traceBuf[PATH_MAX] = {0};
        char subsetBuf[PATH_MAX] = {0};
        char ptrBuf[PATH_MAX] = {0};
        char writeCache[PATH_MAX] = {0};
        getFilePaths((char*) buf, (char*)traceBuf, (char*)subsetBuf, (char*)ptrBuf, (char*) writeCache);

        strcpy(curFile->path, path);
        strcpy(curFile->subsetPath, subsetBuf);

        curFile->subsetHandle = curSys->functions->real_fopen(subsetBuf, "r");
        curFile->writeCache = curSys->functions->real_fopen(writeCache, "a");
        curFile->writeCacheSize = 0 ;
        // Assign it a duummy ptr or fd
        if(type == OPEN || type == OPEN64 || type == OPENAT)
        {
            *fd = curSys->functions->real_open(subsetBuf, 0);
            curFile->fd = *fd;
            curFile->subsetDec = *fd;
            curFile->fptr = NULL;
        }
        else
        {
            *fptr = curFile->subsetHandle;
            curFile->fd = -1;
            curFile->subsetDec = -1;
            curFile->fptr = *fptr;
        }


        curFile->subsetTree = NULL;

        // read in the subset Tree and file size
        readSubsetTree(curFile, ptrBuf);
        // set file pointer to start
        curFile->filePointer = 0;

        // set file to open
        curFile->openStatus = 1;
        
        // set timestamp
        curFile->curTimeStamp = 0;

        // read in the original trace
        readTrace(curFile, traceBuf);
        curFile->currentCall = curFile->originalTrace;

        // Add to hash table
        addOpenFile(curFile->fd, curFile->fptr, curFile->path);
        HASH_ADD(pathHandle, curSys->metaadata, path, strlen(path), curFile);
    }
    else
    {

        curFile->curTimeStamp += 1;
        curFile->subsetHandle = curSys->functions->real_fopen(curFile->subsetPath, "r");
        if(type == OPEN || type == OPEN64 || type == OPENAT)
        {
            *fd = curSys->functions->real_open(curFile->subsetPath, 0);
            curFile->fd = *fd;
            curFile->subsetDec = *fd;
            curFile->fptr = NULL;
        }
        else
        {
            *fptr = curFile->subsetHandle;
            curFile->fd = -1;
            curFile->subsetDec = -1;
            curFile->fptr = *fptr;
        }
        addOpenFile(curFile->fd, curFile->fptr, curFile->path);
        curFile->filePointer = 0;
    }
    compareCalls(curFile, curCall);
}


/// @brief Log a read for the file pointed to by fptr. If offset is -1 then file pointer is not changed, else it is
/// @param offset -1 if we want to use the file pointer or a given offset if call is like pread where file pointer is not imapcted
/// @param readSize Size to read
/// @param fptr Pointer to the file pointer have to supply either fd or fptr
/// @param fd File desc of the file have to supply either fd or fptr
/// @param type an Enumeration of what type of open call was made
/// @param ptr Pointer to the buffer where to store data
/// @return Returns the number of bytes read
size_t logRead(off_t offset, size_t readSize, FILE *fptr, int fd, enum CallType type, void* ptr)
{

    fileMetadata *metadata = getMetadata(fptr, fd);

    CallList *call = malloc(sizeof(CallList));
    // if offset is -1 then we use file pointer
    if (offset == -1)
    {
        call->offset = metadata->filePointer;
    }
    else
    {
        // else we read from provided offset
        call->offset = offset;
    }
    call->size = readSize;
    call->other = -1;
    call->type = type;
    // Now we need to search for all intersections between the interval for this read
    // and the subset tree and combine them
    // make sure the are sorted in the correct order
    NodeList* pInter = NULL;
    NodeList* pLeft = NULL;
    getIntersectionsAndChopInterval(metadata->subsetTree, &(Interval){call->offset, call->offset+call->size, -1}, &pInter, &pLeft);
    reverse(&pInter);

    // Add to interval object a flag for writeCache vs SubsetD
    // traverse LL and read and combine all and return
    getBytes(metadata, pInter, ptr);
    compareCalls(metadata, call);
    return 0;
}

void addOpenFile(int fd, FILE *fptr, char *path)
{

    SysData* curSys = getSysData();
    // We have to first create the struct and if it is being opened for the first time
    // create a struct to hold the opened file struct
    openFile *newFile = malloc(sizeof(openFile));
     // add to the three different hash tables

    strcpy(newFile->path, path);
    newFile->fd = fd;
    newFile->fptr = fptr;

    if (newFile->fd != -1)
    {
        // If it has FD add to FD table
        HASH_ADD(fdHandle, curSys->openFiles->fileDescs, fd, sizeof(int), newFile);
    }
    else if (newFile->fptr != NULL)
    {
        // If it has FPTR add to FPTR table
        HASH_ADD(fptrHandle, curSys->openFiles->filePtrs, fptr, sizeof(FILE *), newFile);
    }
    // add to Path table
    HASH_ADD(pathHandle, curSys->openFiles->filePaths, path, strlen(path), newFile);

}


/// @brief Log the seek call and change metadata accordingly
/// @param off offset for the seek call
/// @param fptr Pointer to the file pointer have to supply either fd or fptr
/// @param fd File desc of the file have to supply either fd or fptr
/// @param whence mode for the seek SET/CUR/END
/// @param type an Enumeration of what type of open call was made
void logSeek(long off, FILE* fptr, int fd, int whence, enum CallType type)
{

    fileMetadata* metadata = getMetadata(fptr, fd);
    CallList* call = malloc(sizeof(CallList));
    call->type = type;
    call->offset = off;
    call->size = -1;
    call->other = whence;
    compareCalls(metadata, call);
    if(whence == SEEK_CUR)
    {
        metadata->filePointer += off;
    }
    else if(whence == SEEK_SET)
    {
        metadata->filePointer = off;
    }   
    else if(whence == SEEK_END)
    {
        // TODO
    }

}


/// @brief Close the file structures aptly and flush all metadata, provide 1 of fd or fptr
/// @param fd fd if the file was opened using Fd else -1
/// @param fptr fptr if it ws using pointer else NULL
/// @param type an Enumeration of what type of open call was made
void logClose(int fd, FILE *fptr, enum CallType type)
{
    // check if metadata hodler already exists
    fileMetadata *curFile = NULL;

    SysData *curSys = getSysData();
    if (setup == 0)
    {
        initSystem();
    }
    // pointer to hold struct returned
    openFile *cur;
    // check if file was using desc or fptr
    if (fd != -1)
    {
        // find and delete
        HASH_FIND(fdHandle, curSys->openFiles->fileDescs, &fd, sizeof(int), cur);
        // setToClose(cur->path);
        HASH_DELETE(fdHandle, curSys->openFiles->fileDescs, cur);
    }
    else if (fptr != NULL)
    {
        // find and delete
        HASH_FIND(fptrHandle, curSys->openFiles->filePtrs, &fptr, sizeof(FILE *), cur);
        // setToClose(cur->path);
        HASH_DELETE(fptrHandle, curSys->openFiles->filePtrs, cur);
    }
    // delete from path table
    HASH_DELETE(pathHandle, curSys->openFiles->filePaths, cur);
    // free it

    HASH_FIND(pathHandle, curSys->metaadata, cur->path, strlen(cur->path), curFile);
    CallList *call = malloc(sizeof(CallList));
    call->type = type;
    call->offset = -1;
    call->size = -1;
    call->other = -1;
    compareCalls(curFile, call);
    free(cur);

}