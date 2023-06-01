#ifndef REPEAT_UTILITIES_H
#define REPEAT_UTILITIES_H

#include "repeatStructures.h"
#include "../IntervalTree.h"
#include <sys/stat.h>
#include <openssl/ssl.h>
#define SUBSET_DIR "../AuditLog/SubsetData/" 

extern int setup;
extern SysData* systemData;

/// @brief gets the structure which holds all metadata for the system
/// @return metadata strucutre for the system
SysData *getSysData();

/// @brief create all the function pointers to the real versions of the
/// i/o functions
void initRealFunctions();

/// @brief Check whether the file at the given path is supposed to be interposed or not
/// @param path path to the file we are checking interposition for
/// @return 1 if the file is supposed to be interposed 0 if not
int checkInterposition(char *path);

/// @brief Get the metadata structure for ther file whos' pointer is given
/// @param fptr file pointer to the file we want metadata of
/// @return the file metadata structure
fileMetadata *getMetadata(FILE *fptr,int fd);

/// @brief Check if we are tracking the file of given path
/// @param path Path to check
/// @return 1 if tracking else 0
int inList(char *path);

void getFilePaths(char* fName, char*tracePath, char* subsetPath, char* ptrPath, char* writeCache);

void readSubsetTree(fileMetadata* curFile, char* ptrPath);

void readTrace(fileMetadata* curFile, char* tracePath);

enum CallType getType(char* call);

/// @brief Add the given call to the given metadata structure
/// @param metadata metadata structure of file to which we want to add the call
/// @param call Call to add to the structure
void addCall(fileMetadata *metadata, CallList *call);

/// @brief Print the calls for the given file
/// @param metadata Metadata structure fo the file to pritn calls for
/// @param stream stream to print to
void printCalls(fileMetadata *metadata, FILE *stream);

/// @brief Give the char version of the call type enum
/// @param  enumeration of the type of call
/// @return a char ptr to the call type string representation
char *getCharOfCall(enum CallType type);

void reverse(NodeList** head_ref);

void getBytes(fileMetadata* metadata, NodeList* pHead, void* ptr);

void compareCalls(fileMetadata* metadata, CallList* curCall);

size_t flushToCache(fileMetadata* metadata, FILE* fptr, const void* ptr, off_t wrtteSize);

unsigned char* getSHA256(void* buf, int len);
#endif
