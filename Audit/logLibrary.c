#include "logLibrary.h"

/// @brief Initialise the system to start tracking
void initSystem()
{
    // Maker directories for the logs and backups
    mkdir(AUDIT_LOG, 0755);
    mkdir(BACKUP_DEST, 0755);

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
        return inList((char *)filename);
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

/// @brief Add all the apt structures for the given file
/// @param buf Path to the file
/// @param fd FD of the file or -1
/// @param fptr File pointer fo the file or NULL
/// @param type an Enumeration of what type of open call was made
void logOpen(const char *buf, int fd, FILE *fptr, enum CallType type)
{
    // check if system is intiialized if not init it
    if (setup == 0)
    {
        initSystem();
    }

    // create a struct to hold the opened file strucgt
    openFile *newFile = malloc(sizeof(openFile));

    // get absolute path
    char path[PATH_MAX];
    char *res = realpath(buf, path);

    // copy over the metadata to the struct for open files
    strcpy(newFile->path, res);
    newFile->fd = fd;
    newFile->fptr = fptr;
    SysData *curSys = getSysData();

    // add to the three different hash tables

    if (fd != -1)
    {
        // If it has FD add to FD table
        HASH_ADD(fdHandle, curSys->openFiles->fileDescs, fd, sizeof(int), newFile);
    }
    else if (fptr != NULL)
    {
        // If it has FPTR add to FPTR table
        HASH_ADD(fptrHandle, curSys->openFiles->filePtrs, fptr, sizeof(FILE *), newFile);
    }
    // add to Path table
    HASH_ADD(pathHandle, curSys->openFiles->filePaths, path, strlen(path), newFile);

    // check if metadata hodler already exists
    fileMetadata *curFile = NULL;

    HASH_FIND(pathHandle, curSys->metaadata, path, strlen(path), curFile);
    if (curFile == NULL)
    {
        // Create new metadata holder and add it
        curFile = malloc(sizeof(fileMetadata));

        // set path fd and fptr
        curFile->fd = fd;
        curFile->fptr = fptr;
        strcpy(curFile->path, path);
        
        // set size
        struct stat st;
        stat(curFile->path, &st);
        curFile->fileSize = st.st_size;

        // Set the trees to NULL
        curFile->readTree = NULL;
        curFile->writeTree = NULL;

        // set file pointer to start of the file
        curFile->filePointer = 0;

        // set status to open
        curFile->openStatus = 1;

        // Create open call
        CallList *call;
        call = malloc(sizeof(CallList));
        call->type = type;
        call->pNext = NULL;
        call->pPrev = NULL;
        call->timeStamp = 0;
        call->other = -1;
        call->offset = -1;
        call->size = -1;
        call->hash = NULL;
        curFile->listOfCalls = NULL;

        // add the open call
        addCall(curFile, call);

        // Start with no backups
        curFile->backups = NULL;

        // Add to hash table
        HASH_ADD(pathHandle, curSys->metaadata, path, strlen(path), curFile);
    }
    else
    {
        // set it to open
        curFile->openStatus = 1;

        // set fd and fptr
        curFile->fd = fd;
        curFile->fptr = fptr;

        // set file pointer to start of the file
        curFile->filePointer = 0;

        // add new open call
        CallList *call = malloc(sizeof(CallList));
        call->type = type;
        call->offset = -1;
        call->size = -1;
        call->other = -1;
        call->hash = NULL;

        addCall(curFile, call);
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
    call->hash = NULL;
    addCall(curFile, call);
    free(cur);

    char *fileName = strrchr(curFile->path, '/');
    char *buf1 = calloc(1, PATH_MAX);
    strcpy(buf1, AUDIT_LOG);
    strcat(buf1, fileName + 1);
    FILE *newFPTR = getSysData()->functions->real_fopen(buf1, "w+");
    printMetadata(curFile, newFPTR);
}

/// @brief Log a read for the file pointed to by fptr. If offset is -1 then file pointer is not changed, else it is
/// @param offset -1 if we want to use the file pointer or a given offset if call is like pread where file pointer is not imapcted
/// @param readSize Size to read
/// @param fptr Pointer to the file pointer have to supply either fd or fptr
/// @param fd File desc of the file have to supply either fd or fptr
/// @param type an Enumeration of what type of open call was made
/// @param ptr Pointer to buffer with data that has been read
void logRead(off_t offset, size_t readSize, FILE *fptr, int fd, enum CallType type, void* ptr)
{
    fileMetadata *metadata = getMetadata(fptr, fd);
    // create a read call
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
    call->type = type;
    call->size = readSize;
    call->other = -1;

    call->hash = getSHA256(ptr, readSize);
    

    // add the read call
    addCall(metadata, call);
    NodeList *newReads = getNonIntersectingRead(metadata->writeTree, &(Interval){call->offset,call->offset + readSize, -1});
    while (newReads != NULL)
    {
        metadata->readTree = insertInterval(metadata->readTree, (Interval){newReads->pInterval->low, newReads->pInterval->high, -1});
        newReads = newReads->pNext;
    }
    // update file poitner only if we used it
    if (offset == -1)
    {
        metadata->filePointer += readSize;
    }
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
    call->hash = NULL;
    addCall(metadata, call);
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

/// @brief given the offset at which write is happening, or -1 if the write is from the current file pointer
/// log the write call and perform the needed backups
/// @param offset offset for the write or -1 if using the current file pointer
/// @param wrtteSize size fo the write
/// @param fptr Pointer to the file pointer have to supply either fd or fptr
/// @param fd File desc of the file have to supply either fd or fptr
/// @param type an Enumeration of what type of open call was made
/// @param ptr Pointer of buffer holding data to write
void logWrite(off_t offset, size_t wrtteSize, FILE *fptr, int fd, enum CallType type, const void* ptr)
{
    fileMetadata *metadata = getMetadata(fptr, fd);

    CallList *call = malloc(sizeof(CallList));
    // add to write tree
    if (offset == -1)
    {
        call->offset = metadata->filePointer;
    }
    else
    {
        call->offset = offset;
    }
    call->type = type;
    call->size = wrtteSize;
    call->other = -1;
    call->hash = getSHA256((void*)ptr, wrtteSize);
    addCall(metadata, call);

    performBackup(metadata, &(Interval){call->offset, call->offset+ call->size, -1});
    metadata->writeTree = insertInterval(metadata->writeTree, (Interval){call->offset, call->offset+call->size, -1, -1});

}


/// @brief Log a stat call for given fd
/// @param path Path of file we are
/// @param fptr File Pointer of file to log call for
/// @param fd FD of file to log call for
/// @param type Type of call we are logging
void logStat(const char* path, FILE* fptr, int fd, enum CallType type)
{
    fileMetadata *metadata = NULL;
    if(path !=NULL)
    {
        HASH_FIND(pathHandle, getSysData()->metaadata, path, strlen(path), metadata);
    }
    else
    {
        metadata = getMetadata(fptr, fd);
    }

    CallList *call = malloc(sizeof(CallList));   
    call->type = type;
    call->size = -1;
    call->offset = -1;
    call->other =-1;
    call->hash = NULL;
    addCall(metadata, call);
}