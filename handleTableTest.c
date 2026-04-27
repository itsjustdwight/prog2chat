/* handleTableTest.c */

#include <stdio.h>
#include "handleTable.h"
#include "chatFlags.h"


int main(void) {

    printf("-----> Initialization <-----\n");
    initHandleTable();

    printf("-----> Testing Count & Capacity <-----\n\n");
    printf("Handle Count = %d (expected 0)\n", getHandleCount());
    printf("Table Capacity = %d (expected 10)\n", getTableCapacity());

    printf("-----> Adding and Looking Up Handles <-----\n\n");
    printf("Test1 Status: %d (expected 0)\n", addHandle("test1", 5));

    printf("Handle Count = %d (expected 1)\n", getHandleCount());

    printf("Test1 Socket Number = %d (expetced 5)\n", lookupSocket("test1"));

    printf("Test2 Socket Number = %d (expected -1)\n", lookupSocket("test2"));

    printf("Test1Dup Status: %d (expected -1)\n", addHandle("test1", 15));

    printf("Test1 Socket Number = %d (expected 5)\n", lookupSocket("test1"));

    printf("-----> Filling Up Capacity to Force Realloc <-----\n\n");
    addHandle("test2", 6);
    printf("Handle Count = %d (expected 2)\n", getHandleCount());
    addHandle("test3", 7);
    printf("Handle Count = %d (expected 3)\n", getHandleCount());
    addHandle("test4", 8);
    printf("Handle Count = %d (expected 3)\n", getHandleCount());
    addHandle("test5", 9);
    printf("Handle Count = %d (expected 4)\n", getHandleCount());
    addHandle("test6", 10);
    printf("Handle Count = %d (expected 5)\n", getHandleCount());
    addHandle("test7", 11);
    printf("Handle Count = %d (expected 6)\n", getHandleCount());
    addHandle("test8", 12);
    printf("Handle Count = %d (expected 7)\n", getHandleCount());
    addHandle("test9", 13);
    printf("Handle Count = %d (expected 8)\n", getHandleCount());
    addHandle("test10", 14);
    printf("Handle Count = %d (expected 9)\n", getHandleCount());

    printf("-----> Removing Handle <-----\n\n");
    removeHandle(8);
    printf("Handle Count = %d (expected 8)\n", getHandleCount());
    lookupSocket("test4");
    printf("Lookup Value for test4 = %d (expected -1)\n", lookupSocket("test4"));

    printf("-----> Finding Handle at Index <-----\n\n");
    addHandle("test11", 8);
    printf("Handle Count = %d (expected 9)\n", getHandleCount());
    char outName[HANDLE_NAME_MAX + 1];
    int outSock;
    int valid = getHandleAtIndex(3, outName, &outSock);
    if (valid == 1) {
	printf("Index 3: %s, socket %d\n", outName, outSock);
    } else {
	printf("Index 3: empty slot\n");
    }

    printf("-----> Growth Past Capacity <-----\n\n");
    addHandle("test12", 15);
    printf("Handle Count = %d (expected 10)\n", getHandleCount());
    addHandle("test13", 16);
    printf("Handle Count = %d (expected 11)\n", getHandleCount());
    printf("Table Capacity = %d (expected 20)\n", getTableCapacity());

    printf("-----> Testing Edge Cases <-----\n\n");
    addHandle(NULL, 0);
    printf("Invalid handle name = %d (expected -1)\n", addHandle(NULL, 0));
    addHandle("", 0);
    printf("Empty handle name = %d (expected -1)\n", addHandle("", 0));
    char tooLong[150];
    memset(tooLong, 'z', 149);
    tooLong[149] = '\0';
    addHandle(tooLong, 0)
    printf(">100 bytes handle name = %d (expected -1)\n", addHandle(tooLong, 0));
    printf("Lookup NULL handle = %d (expected -1)\n", lookupSocket(NULL));
    printf("Out of range handle = %d (expected 0)\n", getHandleAtIndex(-1, name, &sock));
    printf("Out of range handle = %d (expected 0)\n", getHandleAtIndex(9999, name, &sock));
    removeHandle(99999); 

    return 0;
}
