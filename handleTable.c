/* handleTable.c */

/*-----------> Includes / Constants <-----------*/

#include <string.h>

#include "safeUtil.h"
#include "handleTable.h"
#include "chatFlags.h"

#define INIT_TABLE_SIZE	10

/*-----------> Struct <-----------*/

typedef struct {

    char handleName[HANDLE_NAME_MAX + 1];
    int socketNumber;
    int validFlag;

} HandleEntry;

/*-----------> Global Variables <-----------*/

static HandleEntry *handleTable = NULL; // pointer to table itself, an array of structs or table entires
static int tableCapacity = 0; // keeps track of how many slots are allocated
static int tableCount = 0; // keeps track of the number of used entries (valid == 1)

/*-----------> Function Definitions <-----------*/

void initHandleTable(void) {
    // initializing table w/ 10 entries, sCalloc (calloc wrapper) zeros out all 10 entry's values
    handleTable = (HandleEntry *) sCalloc(INIT_TABLE_SIZE, sizeof(HandleEntry));
    tableCapacity = INIT_TABLE_SIZE; // setting upper bound for table, limit before reallocation
    tableCount = 0; // initialize count to zero, no clients intially

    printf("HandleTable created (%p)\n", handleTable);
}

int addHandle(const char *handleName, int socketNumber) {

    if (handleName == NULL) { // validating handleName isn't NULL
	exit(-1);
    }

    // checking to see if handleName is empty or more than 100 bytes
    if (strlen(handleName) <= 0 || strlen(handleName) > HANDLE_NAME_MAX) { 
	exit(-1);
    }

    

    return 0;
}

void removeHandle(int socketNumber) {

}

int lookupSocket(const char *handleName) {

    return 0;
}

const char *lookupHandle(int socketNumber) {

    return NULL
}

int getHandleCount(void) {

    return 0;
}

int getHandleAtIndex(int index, char *outHandle, int *outSocket) {

    return 0;
}

int getTableCapacity(void) {

    return 0;
}
