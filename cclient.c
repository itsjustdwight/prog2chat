/* cclient.c */

/*-----------> Includes <-----------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "pdu.h"
#include "chatFlags.h"
#include "chatPacket.h"

/*-----------> Global <-----------*/

static char myHandle[HANDLE_NAME_MAX + 1];

/*-----------> Function Prototypes <-----------*/

void initConnectHandle(int socketNumber, const char *handle);
void clientControl(int socketNumber);
void processStdin(int socketNumber);
void processMsgFromServer(int socketNumber);
int readFromStdin(uint8_t *buffer);
void checkArgs(int argc, char *argv[]);

// commmand handlers
void sendBroadcast(int socketNumber, uint8_t *buffer, int len);
void sendUnicast(int socketNumber, uint8_t *buffer, int len);
void sendMulticast(int socketNumber, uint8_t *buffer, int len);
void sendHandleList(int socketNumber);

// receiving handlers
void flagBroadcastRecv(uint8_t *pdu, int pduLen);
void flagUnicastRecv(uint8_t *pdu, int pduLen);
void flagMulticastRecv(uint8_t *pdu, int pduLen);
void flagNotFoundRecv(uint8_t *pdu, int pduLen);
void flagListBegin(int socketNumber, uint8_t *pdu, int pduLen);

/*-----------> Main <-----------*/
int main(int argc, char *argv[]) {
    int socketNumber = 0; // socket fd for connecting to the server

    checkArgs(argc, argv);
    strncpy(myHandle, argv[1], HANDLE_NAME_MAX);
    myHandle[HANDLE_NAME_MAX] = '\0';
    socketNumber = tcpClientSetup(argv[2], argv[3], 0); // connect to the server
    initConnectHandle(socketNumber, argv[1]);
    setupPollSet(); // initialize poll set before anything is added
    addToPollSet(socketNumber); // server socket so that pollCall is notified when for incoming packets
    addToPollSet(STDIN_FILENO); // stdin just an fd, poll() still works
    clientControl(socketNumber); // handle stdin input and server messages

    close(socketNumber); // only reached if client_controls return (not meant to)
    return 0;
}

/*-----------> initConnectHandle <-----------*/
void initConnectHandle(int socketNumber, const char *handle) {
    uint8_t outPDUBuffer[PDU_LEN_MAX];
    int builderLen; // storing length returned by build function (handleOnlyPacket())
    uint8_t respBuffer[16]; // buffer to receive response for flag 2 / flag 3
    int recvLen; // storing bytes received from recvPDU()

    builderLen = handleOnlyPacket(outPDUBuffer, INIT_CONNECT_FLAG, handle);
    sendPDU(socketNumber, outPDUBuffer, builderLen);

    recvLen = recvPDU(socketNumber, respBuffer, sizeof(respBuffer));
    if (recvLen == 0) {
	printf("Server Terminated\n");
	exit(1);
    }
    uint8_t flag = getFlag(respBuffer);
    if (flag == GOOD_CONNECT_FLAG) {
	return;
    }
    if (flag == BAD_CONNECT_FLAG) {
	printf("Handle already in use: %s\n", handle);
	exit(0);
    }

    printf("Unexpected server response\n");
    exit(0);   
}
/*-----------> clientControl <-----------*/
void clientControl(int socketNumber) {
    int readyFD = 0; // fd's that are returned by pollCall for the sockets with data ready

    while (1) { // runs until ^C is pressed from user
	readyFD = pollCall(POLL_WAIT_FOREVER); // block forever waiting for input

	if (readyFD == STDIN_FILENO) { // user typed
	    processStdin(socketNumber);
	}
	else if (readyFD == socketNumber) { // server sent a message (or closed)
	    processMsgFromServer(socketNumber);
	}
    }
}

/*-----------> processStdin <-----------*/
void processStdin(int socketNumber) {
    uint8_t buffer[PDU_LEN_MAX]; // buffer holding one line of user input
    int sendLen = 0; // # of bytes read from stdin (including null)

    sendLen = readFromStdin(buffer); // prompts user and fills buffer
    if (sendLen < 2) { // checking if enough args are given by user
	return;
    }

    if (buffer[0] != '%') { // checking if first char of command is typped correctly
	printf("Invalid command\n");
	return;
    }

    int inputCommand = tolower(buffer[1]); // specifying the command user inputs, generalizing to lowercase

    switch (inputCommand) {
	case 'b':
	    sendBroadcast(socketNumber, buffer, sendLen);
	    break;
	case 'm':
	    sendUnicast(socketNumber, buffer, sendLen);
	    break;
	case 'c':
	    sendMulticast(socketNumber, buffer, sendLen);
	    break;
	case 'l':
	    sendHandleList(socketNumber);
	    break;
	default:
	    printf("Invalid command\n");
	    break;
    }

    fflush(stdout);
}

/*-----------> sendBroadcast <-----------*/
void sendBroadcast(int socketNumber, uint8_t *buffer, int len) {
    
}

/*-----------> sendUnicast <-----------*/
void sendUnicast(int socketNumber, uint8_t *buffer, int len) {
    int offset = 2; // setting offset to immediately get to dst-handle from command
    while (isspace(buffer[offset])) {
	offset++;
    }
    if (buffer[offset] == '\0') {
        printf("Invalid command format\n");
	return;
    }

    char dstHandle[101];
    int dstLen = 0;
    while (!isspace(buffer[offset]) && buffer[offset] != '\0') {
        dstHandle[dstLen++] = buffer[offset++];
        if (dstLen > 100) {
            printf("Invalid command format\n");
	    return;
        }
    }

    dstHandle[dstLen] = '\0';

    if (dstLen == 0) {
        printf("Invalid command format\n");
	return;
    }

    while (isspace(buffer[offset])) {
	offset++;
    }

    const char *text = (const char *)&buffer[offset];
    int totalTextLen = len - offset - 1;

    const char *dsts[1] = { dstHandle };
    uint8_t pduBuffer[PDU_LEN_MAX];

    if (totalTextLen < 0) {
	totalTextLen = 0;
    }
    if (totalTextLen <= 0) {
        char emptyBuffer[1] = { '\0' };
	int packetLen = messagePacket(pduBuffer, UNICAST_FLAG, myHandle, dsts, 1, emptyBuffer, 1);
	sendPDU(socketNumber, pduBuffer, packetLen);
	return;
    }

    int sent = 0;
    char textChunkBuffer[TEXT_LEN_MAX];

    while (sent < totalTextLen) {
        int remaining = totalTextLen - sent;
        int chunkSize = (remaining < TEXT_DATA_CHUNK) ? remaining : TEXT_DATA_CHUNK;

        memcpy(textChunkBuffer, text + sent, chunkSize);
        textChunkBuffer[chunkSize] = '\0';

        int packetLen = messagePacket(pduBuffer, UNICAST_FLAG, myHandle, dsts,
                                      1, textChunkBuffer, chunkSize + 1);
        sendPDU(socketNumber, pduBuffer, packetLen);
        
	sent += chunkSize;
    }
}

/*-----------> sendMulticast <-----------*/
void sendMulticast(int socketNumber, uint8_t *buffer, int len) {
    int offset = 2;
    while (isspace(buffer[offset])) {
	offset++;
    }
    if (buffer[offset] == '\0') {
	pirntf("Invalid command format\n");
	return;
    }

    char countStr[8]; // small buffer because count is one digit
    int countLen = 0;
    while (isspace(buffer[offset]) && buffer[offset] != '\0') {
	countStr[countLen++] = buffer[offset++];
	if (countLen > 6) {
	    printf("Invalid command format\n");
	    return;
	}
    }

    countStr[countLen] = '\0';
    int numOfDst = atoi(countStr);

    if (numOfDst < HANDLES_MIN || numOfDst > HANDLES_MAX) {
	printf("Invalid number of handles, must be 2-9\n");
	return;
    }

    char dstHandles[HANDLES_MAX][HANDLE_NAME_MAX + 1];
    const char *dstPointers[HANDLES_MAX]; // array of pointer specifically for messagePacket()

    while (isspace(buffer[offset])) {
	offset++;
    }
    if (buffer[offset] == '\0') {
	printf("Invalid command format\n");
	return;
    }

    int dstLen = 0;
    while (!isspace(buffer[offset]) && buffer[offset] != '\0') {
	dstHandles[i][dstLen++] = buffer[offset++];
	if (dstLen > HANDLE_NAME_MAX) {
	    printf("Invalid command format\n");
	    return;
	}
    }

    dstHandles[i][dstLen] = '\0';

    if (dstLen == 0) {
	printf("Invalid command format\n");
	return;
    }

    dstPointers[i] = dstHandles[i];

    while (isspace(buffer[offset])) {
	offset++;
    }
    const char *text = (const char *)&buffer[offset];
    int totalTextLen = len - offset - 1;
    if (totalTextLen < 0) {
	totalTextLen = 0;
    }

    uint8_t pduBuffer[PDU_LEN_MAX];
    if (totalTextLen <= 0) {
	char emptyBuffer[1] = {'\0'};
	int packetLen = messagePacket(pduBuffer, MULTICAST_FLAG, myHandle,
				      dstPointers, numOfDst, emptyBuffer, 1);
	sendPDU(socketNumber, pduBuffer, packetLen);
	return;
    }

    int sent = 0;
    char textChunkBuffer[TEXT_LEN_MAX];
    while (sent < totalTextLen) {
	int remaining = totalTextLen - sent;
	int chunkSize = (remaining < TEXT_DATA_CHUNK) ? remaining : TEXT_DATA_CHUNK;

	memcpy(textChunkBuffer, text + sent, chunkSize);
	textChunkBuffer[chunkSize] = '\0';

	int packetLen = messagePacket(pduBuffer, MULTICAST_FLAG, myHandle, dstPointers, numOfDst,
				      textChunkBuffer, chunkSize + 1);
	sendPDU(socketNumber, pduBuffer, packetLen);

	sent += chunkSize;
    }
}

/*-----------> sendHandleList <-----------*/
void sendHandleList(int socketNumber) {
    uint8_t buffer[8]; // buffer for flag byte
    int len = chatHeaderPacket(buffer, HANDLE_LIST_FLAG); // storing length of flag byte (10)
    sendPDU(socketNumber, buffer, len); // sending PDU for flag 10
}

/*-----------> processMsgFromServer <-----------*/
void processMsgFromServer(int socketNumber) {
    uint8_t dataBuffer[PDU_LEN_MAX]; // buffer to receive incoming PDU payload
    int messageLen = recvPDU(socketNumber, dataBuffer, PDU_LEN_MAX); // # of data bytes received
    
    if (messageLen == 0) { // 0 bytes -> server closed connection
	printf("Server Terminated\n");
	close(socketNumber); // clean up before exit
	exit(0); // client exites on server termination
    }

    uint8_t flag = getFlag(dataBuffer);

    switch (flag) {
	case BROADCAST_FLAG:
	    flagBroadcastRecv(dataBuffer, messageLen);
	    break;
	case UNICAST_FLAG:
	    flagUnicastRecv(dataBuffer, messageLen);
	    break;
	case MULTICAST_FLAG:
	    flagMulticastRecv(dataBuffer, messageLen);
	    break;
	case HANDLE_ERR_FLAG:
	    flagNotFoundRecv(dataBuffer, messageLen);
	    break;
	case HANDLE_LIST_RESP_FLAG:
	    flagListBegin(socketNumber, dataBuffer, messageLen);
	    break;
	default:
	    fprintf(stderr, "unexpected flag %d\n", flag);
	    break;
    }

    printf("$: ");
    fflush(stdout);
}

/*-----------> flagBroadcastRecv <-----------*/

void flagBroadcastRecv(uint8_t *pdu, int pduLen) {

}

/*-----------> flagUnicastRecv <-----------*/

void flagUnicastRecv(uint8_t *pdu, int pduLen) {
    char srcHandle[HANDLE_NAME_MAX + 1]; 
    int offset = 1; // used to skip flag byte
    offset = getHandleAt(pdu, offset, srcHandle);

    uint8_t numOfDst = pdu[offset];
    offset++; // skip count

    for (int i = 0; i < numOfDst; i++) {
	uint8_t dstLen = pdu[offset];
	offset += 1 + dstLen;
    }

    const char *text = (const char *)(pdu + offset);

    printf("\n%s: %s\n", srcHandle, text);
}

/*-----------> flagMulticastRecv <-----------*/

void flagMulticastRecv(uint8_t *pdu, int pduLen) {
    flagUnicastRecv(pdu, pduLen);
}

/*-----------> flagNotFoundRecv <-----------*/

void flagNotFoundRecv(uint8_t *pdu, int pduLen) {
    char missingHandle[HANDLE_NAME_MAX + 1];
    getHandleAt(pdu, 1, missingHandle);

    printf("\nClient with handle %s does not exist.\n", missingHandle);
}

/*-----------> flagListBegin <-----------*/

void flagListBegin(int socketNumber, uint8_t *pdu, int pduLen) {
    uint32_t netOrderCount; // count received in network order
    memcpy(&netOrderCount, pdu + 1, 4);
    uint32_t count = ntohl(netOrderCount);

    printf("Number of clients: %d\n", count);

    uint8_t recvBuffer[PDU_LEN_MAX]; // buffer to receive all client handles in handle table

    while (1) {
	int len = recvPDU(socketNumber, recvBuffer, PDU_LEN_MAX);
 	if (len == 0) {
	    printf("Server Terminated\n");
	    close(socketNumber);
	    exit(0);
	}

	uint8_t flag = getFlag(recvBuffer);
	if (flag == HANDLE_ITEM_FLAG) {
	    char handle[HANDLE_NAME_MAX + 1];
	    getHandleAt(recvBuffer, 1, handle);
	    printf("	%s\n", handle);
	}
	else if (flag == HANDLE_FINISH_FLAG) {
	    break;
	}
	else {
	    break;
	}
    }
}

/*-----------> readFromStdin <-----------*/
int readFromStdin(uint8_t *buffer) {
    // reads one line of input then null terminates it
    int aChar = 0; // to detect EOF which is -1 (int)
    int inputLen = 0;

    buffer[0] = '\0'; // start with empty string
    printf("$: ");
    fflush(stdout); 
    while (inputLen < (PDU_LEN_MAX - 1) && aChar != '\n') {
	aChar = getchar();
	if (aChar == EOF) { // user closed stdin with ^D
	    exit(0);
	}
	if (aChar != '\n') {
	    buffer[inputLen] = (char)aChar;
	    inputLen++;
	}
    }

    buffer[inputLen] = '\0'; // null terminator for safe printing
    inputLen++; // include null terminator in byte count thats sent

    return inputLen;
}

/*-----------> checkArgs <-----------*/
void checkArgs(int argc, char *argv[]) {
    if (argc != 4) {
	fprintf(stderr, "Usage: %s handle server-name server-port\n", argv[0]);
	exit(1);
    }
    if (strlen(argv[1]) > HANDLE_NAME_MAX) {
	fprintf(stderr, "Invalid handle, handle length longer than 100 characters: %s\n", argv[1]);
	exit(1);
    }
}
