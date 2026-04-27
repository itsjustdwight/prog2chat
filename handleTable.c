/* handleTable.c */

/*-----------> Includes / Constants <-----------*/

#include <string.h>
#include <stdio.h>

#include "safeUtil.h"
#include "handleTable.h"
#include "chatFlags.h"

#define INIT_TABLE_SIZE	10

/*-----------> Struct <-----------*/

typedef struct {

    char handleName[HANDLE_NAME_MAX + 1]; // making handleName a string with max char limit of 100 (bytes)
    int socketNumber; // assigned by OS upon socketFD creation
    int validFlag; // set to 0 or 1 for validity; 0 = socket (handleName) not in use / 1 = socket (handleName) in use

} HandleEntry;

/*-----------> Global Variables <-----------*/

static HandleEntry *handleTable = NULL; // pointer to table itself, an array of structs or table entires
static int tableCapacity = 0; // keeps track of how many slots are allocated
static int tableCount = 0; // keeps track of the number of used entries (valid == 1)

/*-----------> Helper Prototupe <-----------*/

static void growTable(void);

/*-----------> Function Definitions <-----------*/

void initHandleTable(void) {
    // initializing table w/ 10 entries, sCalloc (calloc wrapper) zeros out all 10 entry's values
    handleTable = (HandleEntry *) sCalloc(INIT_TABLE_SIZE, sizeof(HandleEntry));
    tableCapacity = INIT_TABLE_SIZE; // setting upper bound for table, limit before reallocation
    tableCount = 0; // initialize count to zero, no clients intially

    printf("HandleTable created (%p)\n", handleTable);
}

static void growTable(void) {
    int oldCapacity = tableCapacity; // storing capacity before growth
    int newCapacity = tableCapacity * 2; // doubling capacity for more space

    handleTable = (HandleEntry *) srealloc(handleTable, newCapacity * sizeof(HandleEntry));
    // dynamically allocate more space
    // multiply by sizeof(HandleEntry) to get size in bytes according to struct
  
    for (int i = oldCapacity; i < newCapacity; i++) {
        handleTable[i].validFlag = 0;
    }
    
    tableCapacity = newCapacity; // update the new capacity to reflect from realloc
}

int addHandle(const char *handleName, int socketNumber) {
    if (handleName == NULL) { // validating handleName isn't NULL
	return -1;
    }

    // checking to see if handleName is empty or more than 100 bytes
    if (strlen(handleName) <= 0 || strlen(handleName) > HANDLE_NAME_MAX) { 
	return -1;
    }

    // reusing lookupSocket to check for duplicate handleName
    if (lookupSocket(handleName) != -1) {
	return -1;
    }

    int currSlotIndex = -1; 

    // looping (scanning) through the indices of the tableto find and empty slot
    for (int i = 0; i < tableCapacity; i++) { 
	if (handleTable[i].validFlag == 0) {
	    currSlotIndex = i;
	    break;
	}
    }

    int oldCapacity = 0; // storing old capacity temporarily to be used for resizing

    if (foundEmptyIndex == -1) {
	oldCapacity = tableCapacity; // storing old capacity to be used later
        growTable(); // (private) helper to grow table when count = capacity
        currSlotIndex = oldCapacity; // pointing currSlotIndex to first slot of newly created space in table
    }

    // copying the string (handle name) of current handle into the struct
    strncpy(handleTable[currSlotIndex].handleName, handleName, HANDLE_NAME_MAX);
    handleTable[currSlotIndex].handleName[HANDLE_NAME_MAX] = '\0'; // ensuring null terminator is there

    handleTable[currSlotIndex].socketNumber = socketNumber; // copying over socket number

    handleTable[currSlotIndex].validFlag = 1; // setting valid flag for table

    tableCount++; // updating tableCount for each new handle entry
       

    return 0;
}

void removeHandle(int socketNumber) {
    for (int i = 0; i < tableCapacity; i++) { // iterating through table to find valid entries
	if (handleTable[i].validFlag == 1 && handleTable[i].socketNumber == socketNumber) {
	// when valid entry is found and socketNumber matches, 
	// remove by setting validFlag to 0, and decrease tableCount
	    handleTable[i].validFlag = 0;
	    tableCount--;
	}
    }
}

int lookupSocket(const char *handleName) {
    if (handleName == NULL) {
	return -1;
    }

    for (int i = 0; i < tableCapacity; i++) { // iterating through table entires
	if (handleTable[i].validFlag == 1 && strcmp(handleTable[i].handleName, handleName) == 0) {
	// checking to see if validFlag and handleName's match
	// strcmp() returns 0 on match, -number when 1st str is < 2nd str, +number when 1st > 2nd
	    return handleTable[i].socketNumber; // returning socketNumber for valid and matching handleName
	}
    }
    return -1;
}

const char *lookupHandle(int socketNumber) {

    return NULL
}

int getHandleCount(void) {
    return tableCount; // updated through addHandle() and removeHandle(), so will always be accurate
}

int getHandleAtIndex(int index, char *outHandle, int *outSocket) {
    if (index < 0 || index >= tableCapacity) {
	return 0; // index desired is out of bound w/ repsect to table
    }

    if (outHandle == NULL || outSocket == NULL) {
	return 0; // handling trying to write data into null pointer (segfault)
    }

    if (handleTable[index].validFlag != 1) {
	return 0; // empty slot, should be skipped
    }

    strncpy(outHandle, handleTable[index].handleName, HANDLE_NAME_MAX); // copy handleName into outHandle
    outHandle[HANDLE_NAME_MAX] = '\0'; // null-terminated for extra precaution

    *outSocket = handleTable[index].socketNumber; // copying socketNumber
    
    return 1;   
}

int getTableCapacity(void) {
    return tableCapacity; // updated w/in growTable() helper, so always accurate
}
