/* pdu.c */

/*-----------> Includes <-----------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "pdu.h"
#include "safeUtil.h"

/*-----------> sendPDU Function <-----------*/
int sendPDU(int socketNumber, uint8_t *dataBuffer, int lengthOfData) {
    int pdu_len = lengthOfData + PDU_HEADER_LEN; // PDU length (header + data)
    uint8_t pdu_buf[pdu_len]; // buffer to hold the full PDU to be sent
    uint16_t pdu_len_network = htons(pdu_len); // PDU length in network order for the header

    memcpy(pdu_buf, &pdu_len_network, PDU_HEADER_LEN); // writing 2 byte header into buffer

    memcpy(pdu_buf + PDU_HEADER_LEN, dataBuffer, lengthOfData); // writing data after the header

    // sending full PDU in one send() call where safeSend() will handle all errors internally
    int bytes_sent = safeSend(socketNumber, pdu_buf, pdu_len, 0);

    return bytes_sent - PDU_HEADER_LEN; // just data bytes sent being returned
}

/*-----------> recvPDU Function <-----------*/
int recvPDU(int clientSocket, uint8_t *dataBuffer, int bufferSize) {
    uint8_t header_buf[PDU_HEADER_LEN]; // buffer just for the header
    uint16_t pdu_len_network = 0; // PDU length read off the wire (network order)
    uint16_t pdu_len = 0; // PDU length in host order
    int data_len = 0; // number of data bytes in incoming PDU

    // first recv() to grab PDU length header using flag
    int bytes_recv = safeRecv(clientSocket, header_buf, PDU_HEADER_LEN, MSG_WAITALL);
    if (bytes_recv == 0) { // connection closed by other side
	return 0;
    }

    memcpy(&pdu_len_network, header_buf, PDU_HEADER_LEN); // pulling the length out of header
    pdu_len = ntohs(pdu_len_network); // converting to host order to use
    data_len = pdu_len - PDU_HEADER_LEN; // data size in data portion of PDU

    // error checking buffer to ensure it's large enough
    if (data_len > bufferSize) {
	fprintf(stderr, "recvPDU: PDU payload (%d bytes) larger than buffer (%d bytes)\n", data_len, bufferSize);
	exit(-1);
    }

    // second recv() to grab the payload using MSG_WAITALL flag so it's all gotten within one call
    bytes_recv = safeRecv(clientSocket, dataBuffer, data_len, MSG_WAITALL);

    return bytes_recv; // number of bytes received, or 0 if connection closed midway through PDU processing
}
