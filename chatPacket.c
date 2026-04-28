/* chatPacket.c */

/*-----------> Includes / Constants <-----------*/

#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#include "chatFlags.h"
#include "chatPacket.h"

/*-----------> Function Definitions <-----------*/

// builder packet functions
int initConnectPacket(uint8_t *buffer, const char *srcHandle) { // packet builder for flag 1
    uint8_t handleLen = strlen(srcHandle); // computing handle len from the source handle, starting @ offset 1
    if (handleLen < 1 || handleLen > 100) {
	return -1;
    }

    buffer[0] = INIT_CONNECT_FLAG; // setting byte one of buffer to flag 1
 
    handleLen = buffer + 1; // now starting at the payload

    memcpy(buffer + 2, srcHandle, sizeOf(srcHandle));

    return 2 + handleLen;
}

int chatHeaderPacket(uint8_t *buffer, uint8_t flag) { // packet builder for flags 2, 3, 10, and 13
    buffer[0] = flag; // setting flag byte at offset 0

    return 1; // indicates success
}

int messagePacket(uint8_t *buffer, uint8_t flag, const char *srcHandle, const char *dstHandles[], 
                  int numOfDst, const char *text, int textLen) { // packet builder for flags 5 (%M) & 6 (%C)
    if (numOfDst < 1 || numOfDst > 9) {
	return -1;
    }

    if (strlen(srcHandle) > 100) {
	return -1;
    }

    for (int i = 0; i < 8; i++) {
	if (strlen(dstHandles[i]) > 100) {
	    return -1;
	}
    }

    if (text > TEXT_LEN_MAX) {
	return -1;
    } 

    buffer[0] = flag; // setting flag byte at offset 0
    buffer + 1 = strlen(srcHandle); // setting srcHandle's length starting at offset 1
    memcpy(buffer + 2, srcHandle, sizeOf(srcHandle));
    buffer += strlen(srcHandle);
    buffer = numOfDst;
    buffer += 1;
    for (int i = 0; i < sizeOf(dstHandles); i++) {
	uint8_t dhl = atoi(dstHandles[i]);
	
    }

}

int broadcastPacket(uint8_t *buffer, const char *srcHandle, const char *text, 
                    int textLen) { // packet builder for flag 4

}

int notFoundPacket(uint8_t *buffer, const char *unknownHandle) { // packet builder for flag 7
    uint8_t handleLen = strlen(unknownHandle); // computing handle len from the source handle, starting @ offset 1
    if (handleLen < 1 || handleLen > 100) {
        return -1;
    }

    buffer[0] = HANDLE_ERR_FLAG; // setting byte one of buffer to flag 1

    handleLen = buffer + 1; // now starting at the payload

    memcpy(buffer + 2, unknownHandle, sizeOf(unknownHandle));

    return 2 + handleLen;
}


int listCountPacket(uint8_t *buffer, uint32_t count) { // packet builder for flag 11
    buffer[0] = HANDLE_LIST_RESP_FLAG; // setting flag byte to offset 0

    uint32_t netOrderCount = htonl(count); // converting count to network order
    memcpy(buffer + 1, &netOrderCount, 4);

    return 5;
}

int listHandlePacket(uint8_t *buffer, const char *handle) { // packet builder for flag 12
    uint8_t handleLen = strlen(handle); // computing handle len from the source handle, starting @ offset 1
    if (handleLen < 1 || handleLen > 100) {
        return -1;
    }

    buffer[0] = HANDLE_ITEM_FLAG; // setting byte one of buffer to flag 1

    handleLen = buffer + 1; // now starting at the payload

    memcpy(buffer + 2, handle, sizeOf(handle));

    return 2 + handleLen;   
}

// parser packet functions
int getFlag(const uint8_t *pdu) {
    return pdu[0]; // flag is always the first byte of buffer
}

int getHandleAt(const uint8_t *pdu, int offset, char *outHandle) {
    uint8_t handleLen = pdu[offset];

    memcpy(outHandle, pdu + offset + 1, handleLen); // copying handle bytes into outHandle
    outHandle[handleLen] = '\0'; // null-termination

    return offset + 1  + handleLen;
}
