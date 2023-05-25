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

    // if opening for the first time
    if (curFile== NULL)
    {
        curFile = malloc(sizeof(fileMetadata));
        // opened for the first time
        // read the trace, open the subset
        
        char traceBuf[PATH_MAX] = {0};
        char subsetBuf[PATH_MAX] = {0};
        char ptrBuf[PATH_MAX] = {0};
        getFilePaths((char*) buf, (char*)traceBuf, (char*)subsetBuf, (char*)ptrBuf);

        strcpy(curFile->path, path);
        strcpy(curFile->subsetPath, subsetBuf);

        curFile->subsetHandle = curSys->functions->real_fopen(subsetBuf, "r");
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
        print_intervals(curFile->subsetTree, stdout);
        // set file pointer to start
        curFile->filePointer = 0;

        // set file to open
        curFile->openStatus = 1;
        
        // set timestamp
        curFile->curTimeStamp = 1;

        // read in the original trace
        readTrace(curFile, traceBuf);

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
}


/// @brief Log a read for the file pointed to by fptr. If offset is -1 then file pointer is not changed, else it is
/// @param offset -1 if we want to use the file pointer or a given offset if call is like pread where file pointer is not imapcted
/// @param readSize Size to read
/// @param fptr Pointer to the file pointer have to supply either fd or fptr
/// @param fd File desc of the file have to supply either fd or fptr
/// @param type an Enumeration of what type of open call was made
/// @return Returns the number of bytes read
size_t logRead(off_t offset, size_t readSize, FILE *fptr, int fd, enum CallType type)
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
    NodeList* pInter;
    NodeList* pLeft;
    getIntersectionsAndChopInterval(metadata->subsetTree, &(Interval){call->offset, call->offset+call->size, -1}, &pInter, &pLeft);
    reverse(&pInter);
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
