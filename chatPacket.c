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
 
    handleLen = strlen(srcHandle) + 1; // now starting at the payload

    memcpy(handleLen + 2, srcHandle, sizeOf(srcHandle));

    return 2 + handleLen;
}

int chatHeaderPacket(uint8_t *buffer, uint8_t flag) { // packet builder for flags 2, 3, 10, and 13
    buffer[0] = flag; // setting flag byte at offset 0

    return 1; // indicates success
}

int messagePacket(uint8_t *buffer, uint8_t flag, const char *srcHandle, const chat *dstHandles[], 
                  int numOfDst, const chat *text, int textLen) { // packet builder for flags 5 (%M) & 6 (%C)
    if (numOfDst < 1 || numOfDst > 9) {
	return -1;
    }
}

int broadcastPacket(uint8_t *buf, const char *srcHandle, const char *text, int textLen) { // packet builder for flag 4

}

int notFoundPacket(uint8_t *buf, const char *unknownHandle) { // packet builder for flag 7

}



