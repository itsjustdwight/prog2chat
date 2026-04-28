/* cclient.c */

/*-----------> Includes <-----------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "pdu.h"
#include "chatFlags.h"
#include "chatPacket.h"

/*-----------> Function Prototypes <-----------*/

void initConnectHandle(int socketNumber, const char *handle);
void clientControl(int socketNumber);
void processStdin(int socketNumber);
void processMsgFromServer(int socketNumber);
int readFromStdin(uint8_t *buffer);
void checkArgs(int argc, char *argv[]);

/*-----------> Main <-----------*/
int main(int argc, char *argv[]) {
    int socketNumber = 0; // socket fd for connecting to the server

    checkArgs(argc, argv);

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
	printf("Server terminated unintentionally\n");
	exit(1);
    }
    uint8_t flag = getFlag(respBuffer);
    if (flag == GOOD_CONNECT_FLAG) {
	return;
    }
    if (flag == BAD_CONNECT_FLAG) {
	printf("Handle already in use: <%s>\n", handle);
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

    sendPDU(socketNumber, buffer, sendLen); // one send() of full PDU
}

/*-----------> processMsgFromServer <-----------*/
void processMsgFromServer(int socketNumber) {
    uint8_t dataBuffer[PDU_LEN_MAX]; // buffer to receive incoming PDU payload
    int messageLen = 0; // # of data bytes received

    messageLen = recvPDU(socketNumber, dataBuffer, PDU_LEN_MAX);

    if (messageLen > 0) { // server to echo message back to user
	printf("Message received from server, length: %d Data: %s\n", messageLen, dataBuffer);
    }
    else { // 0 bytes -> server closed connection
	printf("Server has terminated\n");
	close(socketNumber); // clean up before exit
	exit(0); // client exites on server termination
    }
}

/*-----------> readFromStdin <-----------*/
int readFromStdin(uint8_t *buffer) {
    // reads one line of input then null terminates it
    int aChar = 0; // to detect EOF which is -1 (int)
    int inputLen = 0;

    buffer[0] = '\0'; // start with empty string
    printf("Enter data: ");
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
	fprintf(stderr, "Invalid handle, handle length longer than 100 characters: <%s>\n", argv[1]);
    }
}
