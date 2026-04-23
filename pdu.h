/* pdu.h */

#ifndef PDU_H
#define PDU_H

/*-----------> Includes/Constants <-----------*/

#include <stdint.h>

#define PDU_HEADER_LEN	2 // header len is 2 bytes added to beginning of every PDU (in network order)

/*-----------> Function Prototypes <-----------*/

// creates a PDU of 2 bytes + lengthOfData and sends in one send() call, then returns the number of data bytes sent (doesn't include the 2 byte header)
int sendPDU(int socketNumber, uint8_t *dataBuffer, int lengthOfData);

// receives the PDU using two recv() calls with the MSG_WAITALL flag. the first recv() will get the 2 byte header, the second recv() will get the data, then it'll return the number of data bytes received or 0 if connection gets closed
int recvPDU(int clientSocket, uint8_t *dataBuffer, int bufferSize);

#endif
