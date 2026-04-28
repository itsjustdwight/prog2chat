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

/*-----------> Constant <-----------*/

#define MAXBUF  1024
#define DEBUG_FLAG 1

/*-----------> Function Prototypes <-----------*/

void serverControl(int mainServerSocket);
void addNewClient(int mainServerSocket);
void processClientPacket(int clientSocket);
int checkArgs(int argc, char *argv[]);

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

    clientSocket = tcpAccept(mainServerSocket, DEBUG_FLAG); // accept will print client's info
    addToPollSet(clientSocket); // adding to poll set so future data from this client notifies pollCall()
}

/*-----------> processClientPacket <-----------*/
void processClientPacket(int clientSocket) {
    uint8_t dataBuffer[MAXBUF]; // buffer to hold incoming PDU payload
    int messageLen = 0; // # of data bytes received

    messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF); // full PDU

    if (messageLen > 0) { // print the received message
	printf("Message received on socket %d, length: %d Data: %s\n", 
               clientSocket, messageLen, dataBuffer);

	// send PDU back to client for testing
	sendPDU(clientSocket, dataBuffer, messageLen);
    }
    else {
    	printf("Connection closed by client on socket %d\n", clientSocket);
	removeFromPollSet(clientSocket); // remove before closing
	close(clientSocket); // free the socket fd, server will keep running
    }
}

/*-----------> check_args <-----------*/
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
