/* server.c */

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
#include "handleTable.h"
#include "chatPacket.h"
#include "chatFlags.h"

/*-----------> Function Prototypes <-----------*/

void serverControl(int mainServerSocket);
void addNewClient(int mainServerSocket);
void processClientPacket(int clientSocket);
int checkArgs(int argc, char *argv[]);

// flag handlers
void flagConnect(uint8_t *pdu, int pduLen, int clientSocket);
void flagBroadcast(uint8_t *pdu, int pduLen, int clientSocket);
void flagUnicast(uint8_t *pdu, int pduLen, int clientSocket);
void flagMulticast(uint8_t *pdu, int pduLen, int clientSocket);
void flagHandleList(int clientSocket);

/*-----------> Main <-----------*/
int main(int argc, char *argv[]) {
    int mainServerSocket = 0; // socket fd for listening server socket
    int portNumber = 0; // port # to bind the server on for connections

    portNumber = checkArgs(argc, argv); // validating args and pulling port
    initHandleTable(); // initialize the server's handle table
    mainServerSocket = tcpServerSetup(portNumber); // bind->listen then print the port

    setupPollSet(); // initializing the poll set before anything is added
    addToPollSet(mainServerSocket); // main server socket added to be able to accept new clients
    serverControl(mainServerSocket); // accept new clients and service existing

    close(mainServerSocket); // only gets here if server_control ever returned (not meant to)
    
    return 0;
}

/*-----------> serverControl <-----------*/
void serverControl (int mainServerSocket) {
    int readySocket = 0; // socket returned by pollCall that has its data ready

    while (1) { // server will run until ^C
    	readySocket = pollCall(POLL_WAIT_FOREVER); // block until socket is ready

	if (readySocket == mainServerSocket) { // new client trying to connect to server
	    addNewClient(mainServerSocket); // adding the new client to pollSet and hashTable
	}
	else { // current client has its data sent (or closed its side)
	    processClientPacket(readySocket);
	}
    }
}

/*-----------> addNewClient <-----------*/
void addNewClient(int mainServerSocket) {
    int clientSocket = 0; // socket fd for recent accepted clients

    clientSocket = tcpAccept(mainServerSocket, 0); // accept will print client's info
    addToPollSet(clientSocket); // adding to poll set so future data from this client notifies pollCall()
}

/*-----------> processClientPacket <-----------*/
void processClientPacket(int clientSocket) {
    uint8_t dataBuffer[PDU_LEN_MAX]; // buffer to hold incoming PDU payload
    int messageLen = 0; // # of data bytes received

    messageLen = recvPDU(clientSocket, dataBuffer, PDU_LEN_MAX); // full PDU received from client

    if (messageLen == 0) { // print the received message
	removeHandle(clientSocket); // cleaning up disconnected socket from handle table
	removeFromPollSet(clientSocket); // remove before closing
	close(clientSocket); // free the socket fd, server will keep running, cleanup
	return;
    }
    if (messageLen > 0) {
        uint8_t flag = getFlag(dataBuffer);

        switch (flag) {
	    case INIT_CONNECT_FLAG: // flag 1
	    	flagConnect(dataBuffer, messageLen, clientSocket); // handle client attempting to connect
	    	break; 
	    case BROADCAST_FLAG: // flag 4
	    	flagBroadcast(dataBuffer, messageLen, clientSocket); // handle client broadcast request
	    	break;
	    case UNICAST_FLAG: // flag 5
	    	flagUnicast(dataBuffer, messageLen, clientSocket); // handle client messgae request to another client
	    	break;
	    case MULTICAST_FLAG: // flag 6
	    	flagMulticast(dataBuffer, messageLen, clientSocket); // handle client multicast to certain clients
	    	break;
	    case HANDLE_LIST_FLAG: // flag 10
	    	flagHandleList(clientSocket); // handle client request for handle table list
	    	break;
	    default:
		fprintf(stderr, "Server received unexpected flag %d on socket %d\n", flag, clientSocket); 
    	}
    }
}

/*-----------> flagConnect <-----------*/
void flagConnect(uint8_t *pdu, int pduLen, int clientSocket) {
// store client's handle, send flag 2 or flag 3

    char handleBuffer[HANDLE_NAME_MAX + 1]; // buffer to store the parsed handle name
    int result; // storing the result of addHandle output
    uint8_t respBuffer[8]; // buffer to build the response packet
    int respLen; // used to store response packet lenght (returned by chatHeaderPacket())

    getHandleAt(pdu, 1, handleBuffer); // returns the byte after the handle

    result = addHandle(handleBuffer, clientSocket);
    if (result == 0) {
	respLen = chatHeaderPacket(respBuffer, GOOD_CONNECT_FLAG);
	sendPDU(clientSocket, respBuffer, respLen);	
    }
    else {
	respLen = chatHeaderPacket(respBuffer, BAD_CONNECT_FLAG);
	sendPDU(clientSocket, respBuffer, respLen);
	removeFromPollSet(clientSocket);
	close(clientSocket);
    }
}

/*-----------> flagBroadcast <-----------*/
void flagBroadcast(uint8_t *pdu, int pduLen, int clientSocket) {
// send untouched PDU to all clients except for the sending client

}

/*-----------> flagUnicast <-----------*/
void flagUnicast(uint8_t *pdu, int pduLen, int clientSocket) {
// parse %M, send message to dest. or send flag 7 if client not found
    
    char srcHandle[HANDLE_NAME_MAX + 1];
    char dstHandle[HANDLE_NAME_MAX + 1];
    int offset = 1; // skip flag byte

    offset = getHandleAt(pdu, offset, srcHandle); // skip past src
 
    offset++;

    offset = getHandleAt(pdu, offset, dstHandle);

    int dstSocketFD = lookupSocket(dstHandle);
    if (dstSocketFD != -1) {
	sendPDU(dstSocketFD, pdu, pduLen);
    }
    if (dstSocketFD == -1) {
	uint8_t errorBuffer[PDU_LEN_MAX];
	int errorLen = handleOnlyPacket(errorBuffer, HANDLE_ERR_FLAG, dstHandle);
	sendPDU(clientSocket, errorBuffer, errorLen);
    }
}

/*-----------> flagMulticast <-----------*/
void flagMulticast(uint8_t *pdu, int pduLen, int clientSocket) {
// parse %C, send message to each dest., send flag 7 for each client that's not found
    char srcHandle[HANDLE_NAME_MAX + 1];
    char dstHandle[HANDLE_NAME_MAX + 1];
    int offset = 1; // skip flag byte

    offset = getHandleAt(pdu, offset, srcHandle); // skip past src

    uint8_t numOfDst = pdu[offset];
    offset++;

    for (int i = 0; i < numOfDst; i++) {
    	offset = getHandleAt(pdu, offset, dstHandle);
    	int dstSocketFD = lookupSocket(dstHandle);

    	if (dstSocketFD != -1) {
            sendPDU(dstSocketFD, pdu, pduLen); // forwarding the PDU
    	}
    	if (dstSocketFD == -1) {
            uint8_t errorBuffer[PDU_LEN_MAX];
            int errorLen = handleOnlyPacket(errorBuffer, HANDLE_ERR_FLAG, dstHandle);
            sendPDU(clientSocket, errorBuffer, errorLen); // telling sender of nonexistent handles
    	}
    }
}

/*-----------> flagHandleList <-----------*/
void flagHandleList(int clientSocket) { 
// send flag 11 with table count, the one flag 12 per handle, then complete send with flag 13

    uint32_t count = getHandleCount();
    uint8_t buffer[PDU_LEN_MAX];
    int len = listCountPacket(buffer, count);
    sendPDU(clientSocket, buffer, len);

    for (int i = 0; i < getTableCapacity(); i++) {
	char handle[HANDLE_NAME_MAX + 1];
	int sock; // unused, but for getHandleAt param requirement
	if (getHandleAtIndex(i, handle, &sock) == 1) {
	    len = handleOnlyPacket(buffer, HANDLE_ITEM_FLAG, handle);
	    sendPDU(clientSocket, buffer, len);
	}
    }

    len = chatHeaderPacket(buffer, HANDLE_FINISH_FLAG);
    sendPDU(clientSocket, buffer, len);
}

/*-----------> checkArgs <-----------*/
int checkArgs(int argc, char *argv[]) {
    // port # optional, if blank 0 is passed to kernel
    int portNumber = 0;

    if (argc > 2) {
	fprintf(stderr, "Usage: %s [optional port number]\n", argv[0]);
	exit(-1);
    }

    if (argc == 2) {
	portNumber = atoi(argv[1]);
    }

    return portNumber;
}
