/* chatPacket.h */

#ifndef CHATPACKET_H
#define CHATPACKET_H

/*-----------> Includes <-----------*/

#include <stdint.h>

/*-----------> Function Prototypes <-----------*/

int initConnectPacket(uint8_t *buffer, const char *srcHandle);

int chatHeaderPacket(uint8_t *buffer, uint8_t flag);

int messagePacket(uint8_t *buffer, uint8_t flag, const char *srcHandle, const char *dstHandles[],
		  int numOfDst, const char *text, int textLen);

int broadcastPacket(uint8_t *buffer, const char *srcHandle, const char *text, int textLen);

int notFoundPacket(uint8_t *buffer, const char *unknownHandle);

int listCountPacket(uint8_t *buffer, uint32_t count);

int listHandlePacket(uint8_t *buffer, const char *handle);

int getFlag(const uint8_t *pdu);

int getHandleAt(const uint8_t *pdu, int offset, char *outHandle); 

#endif
