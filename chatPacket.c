/* chatPacket.c */

/*-----------> Includes / Constants <-----------*/

#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#include "chatFlags.h"
#include "chatPacket.h"

/*-----------> Function Definitions <-----------*/

// builder packet functions
int handleOnlyPacket(uint8_t *buffer, uint8_t flag, const char *srcHandle) { // packet builder for flags 1, 7, and 12
    uint8_t handleLen = strlen(srcHandle); // computing handle len from the source handle, starting @ offset 1
    if (handleLen < 1 || handleLen > 100) {
        return -1;
    }

    int offset = 0; // creating offset to be used to index through buffer
    buffer[offset++] = flag; // setting byte 1 of buffer to input flag

    buffer[offset++] = handleLen; // starting at byte 2, reading length of input handle

    memcpy(buffer + offset, srcHandle, handleLen); // copy srcHandle into buffer starting at byte 3

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

    if (strlen(srcHandle) < 1 || strlen(srcHandle) > 100) {
	return -1;
    }

    for (int i = 0; i < numOfDst; i++) {
	if (strlen(dstHandles[i]) < 1 || strlen(dstHandles[i]) > 100) {
	    return -1;
	}
    }

    if (textLen > TEXT_LEN_MAX) {
	return -1;
    }

    uint8_t handleLen = strlen(srcHandle); 
    
    int offset = 0; // creating offset to be used to index into buffer
    buffer[offset++] = flag; // setting flag byte at offset 0
    buffer[offset++] = handleLen; // setting srcHandle's length starting at offset 1
    memcpy(buffer + offset, srcHandle, handleLen);
    offset += handleLen;
    buffer[offset++] = numOfDst;
    
    for (int i = 0; i < numOfDst; i++) {
	uint8_t dhl = strlen(dstHandles[i]);
	buffer[offset++] = dhl;
	memcpy(buffer + offset, dstHandles[i], dhl);
	offset += dhl;	
    }

    memcpy(buffer + offset, text, textLen);
    offset += textLen; 

    return offset;
}

int broadcastPacket(uint8_t *buffer, const char *srcHandle, const char *text, 
                    int textLen) { // packet builder for flag 4

    if (strlen(srcHandle) < 1 || strlen(srcHandle) > 100) {
        return -1;
    }

    if (textLen > TEXT_LEN_MAX) {
        return -1;
    }
    
    uint8_t handleLen = strlen(srcHandle);

    int offset = 0; // creating offset to be used to index into buffer
    buffer[offset++] = BROADCAST_FLAG; // setting flag byte at offset 0
    buffer[offset++] = handleLen; // setting srcHandle's length starting at offset 1
    memcpy(buffer + offset, srcHandle, handleLen);
    offset += handleLen;

    memcpy(buffer + offset, text, textLen);
    offset += textLen;

    return offset;
}


int listCountPacket(uint8_t *buffer, uint32_t count) { // packet builder for flag 11
    int offset = 0;
    buffer[offset++] = HANDLE_LIST_RESP_FLAG; // setting flag byte to offset 0

    uint32_t netOrderCount = htonl(count); // converting count to network order
    memcpy(buffer + offset, &netOrderCount, 4);

    return 5;
}

// parser packet functions
uint8_t getFlag(const uint8_t *pdu) {
    return pdu[0]; // flag is always the first byte of buffer
}

int getHandleAt(const uint8_t *pdu, int offset, char *outHandle) {
    uint8_t handleLen = pdu[offset];

    memcpy(outHandle, pdu + offset + 1, handleLen); // copying handle bytes into outHandle
    outHandle[handleLen] = '\0'; // null-termination

    return offset + 1  + handleLen;
}
