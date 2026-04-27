/* handleTableTest.c */

#include <stdio.h>
#include "handleTable.h"
#include "chatFlags.h"


int main(void) {

    printf("-----> Initialization <-----\n");
    initHandleTable();

    printf("-----> Testing Count & Capacity <-----\n");
    printf("Handle Count = %d\n", getHandleCount());
    printf("Table Capacity = %d\n", getTableCapacity());

    printf("-----> Adding and Looking Up Handles <-----\n");
    printf("Test1 Status: %d\n", addHandle("test1", 5));

    printf("Handle Count = %d\n", getHandleCount());

    printf("Test1 Socket Number = %d\n", lookupSocket("test1"));

    printf("Test2 Socket Number = %d\n", lookupSocket("test2"));

    printf("Test1Dup Status: %d\n", addHandle("test1", 15));

    printf("Test1 Socket Number = %d\n", lookupSocket("test1"));

    addHandle("test2", 6);
    printf("Handle Count = %d\n", getHandleCount());
    addHandle("test3", 7);
    printf("Handle Count = %d\n", getHandleCount());
    addHandle("test4", 8);
    printf("Handle Count = %d\n", getHandleCount());
    addHandle("test5", 9);
    printf("Handle Count = %d\n", getHandleCount());
    addHandle("test6", 10);
    printf("Handle Count = %d\n", getHandleCount());
    addHandle("test7", 11);
    printf("Handle Count = %d\n", getHandleCount());
    addHandle("test8", 12);
    printf("Handle Count = %d\n", getHandleCount());
    addHandle("test9", 13);
    printf("Handle Count = %d\n", getHandleCount());
    addHandle("test10", 14);
    printf("Handle Count = %d\n", getHandleCount());

    removeHandle(8);
    lookupSocket("test4");

    addHandle("test11", 8);
    getHandleAtIndex(3, "test11", 8);

}
