/* server.c */

/*-----------> Includes <-----------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "pdu.h"

/*-----------> Constant <-----------*/

#define MAXBUF  1024
#define DEBUG_FLAG 1

/*-----------> Function Prototypes <-----------*/

void server_control(int main_server_socket);
void add_new_socket(int main_server_socket);
void process_client(int client_socket);
int check_args(int argc, char *argv[]);

/*-----------> Main <-----------*/
int main(int argc, char *argv[]) {
    int main_server_socket = 0; // socket fd for listening server socket
    int port_number = 0; // port # to bind the server on for connections

    port_number = check_args(argc, argv); // validating args and pulling port

    main_server_socket = tcpServerSetup(port_number); // bind->listen then print the port

    setupPollSet(); // initializing the poll set before anything is added
    addToPollSet(main_server_socket); // main server socket added to be able to accept new clients

    server_control(main_server_socket); // accept new clients and service existing

    close(main_server_socket); // only gets here if server_control ever returned (not meant to)
    return 0;
}

/*-----------> server_control <-----------*/
void server_control (int main_server_socket) {
    int ready_socket = 0; // socket returned by pollCall that has its data ready

    while (1) { // server will run until ^C
    	ready_socket = pollCall(POLL_WAIT_FOREVER); // block until socket is ready

	if (ready_socket == main_server_socket) {
	    add_new_socket(main_server_socket);
	}
	else { // current client has its data sent (or closed its side)
	    process_client(ready_socket);
	}
    }
}

/*-----------> add_new_socket <-----------*/
void add_new_socket(int main_server_socket) {
    int client_socket = 0; // socket fd for recent accepted clients

    client_socket = tcpAccept(main_server_socket, DEBUG_FLAG); // accept will print client's info
    addToPollSet(client_socket); // adding to poll set so future data from this client notifies pollCall()
}

/*-----------> process_client <-----------*/
void process_client(int client_socket) {
    uint8_t data_buf[MAXBUF]; // buffer to hold incoming PDU payload
    int message_len = 0; // # of data bytes received

    message_len = recvPDU(client_socket, data_buf, MAXBUF); // full PDU

    if (message_len > 0) { // print the received message
	printf("Message received on socket %d, length: %d Data: %s\n", 
               client_socket, message_len, data_buf);

	// send PDU back to client for testing
	sendPDU(client_socket, data_buf, message_len);
    }
    else {
    	printf("Connection closed by client on socket %d\n", client_socket);
	removeFromPollSet(client_socket); // remove before closing
	close(client_socket); // free the socket fd, server will keep running
    }
}

/*-----------> check_args <-----------*/
int check_args(int argc, char *argv[]) {
    // port # optional, if blank 0 is passed to kernel
    int port_number = 0;

    if (argc > 2) {
	fprintf(stderr, "Usage: %s [optional port number]\n", argv[0]);
	exit(-1);
    }

    if (argc == 2) {
	port_number = atoi(argv[1]);
    }

    return port_number;
}
