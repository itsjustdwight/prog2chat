/* handleTable.h */

#ifndef HANDLETABLE_H
#define HANDLETABLE_H

/*-----------> Function Prototypes <-----------*/

void initHandleTable(void); // allocates memory for initial array and zeros valid flags

int addHandle(const char *handleName, int socketNumber); // adds new handle/socket mapping

void removeHandle(int socketNumber); // uses socket (rather than handle) to remove client when client disconnects

int lookupSocket(const char *handleName); // searches table for socket given a valid handle name

const char *lookupHandle(int socketNumber); // searches table for handle given a valid socket

int getHandleCount(void); // returns number of valid entires in the table

int getHandleAtIndex(int index, char *outHandle, int *outSocket); 
// used for %L to print each of the handle names
// outHandle needs to be at least (HANDLE_NAME_MAX + 1) bytes

int getTableCapacity(void); // returns current table capacity to ensure out of bounds isn't done 

#endif
