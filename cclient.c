/* cclient.c */

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

void client_control(int socket_num);
void process_stdin(int socket_num);
void process_msg_from_server(int socket_num);
int read_from_stdin(uint8_t *buffer);
void check_args(int argc, char *argv[]);

/*-----------> Main <-----------*/
int main(int argc, char *argv[]) {
    int socket_num = 0; // socket fd for connecting to the server

    check_args(argc, argv);

    socket_num = tcpClientSetup(argv[1], argv[2], DEBUG_FLAG); // connect to the server

    setupPollSet(); // initialize poll set before anything is added
    addToPollSet(socket_num); // server socket so that pollCall is notified when for incoming packets
    addToPollSet(STDIN_FILENO); // stdin just an fd, poll() still works

    client_control(socket_num); // handle stdin input and server messages

    close(socket_num); // only reached if client_controls return (not meant to)
    return 0;
}

/*-----------> client_control <-----------*/
void client_control(int socket_num) {
    int ready_fd = 0; // fd's that are returned by pollCall for the sockets with data ready

    while (1) { // runs until ^C is pressed from user
	ready_fd = pollCall(POLL_WAIT_FOREVER); // block forever waiting for input

	if (ready_fd == STDIN_FILENO) { // user typed
	    process_stdin(socket_num);
	}
	else if (ready_fd == socket_num) { // server sent a message (or closed)
	    process_msg_from_server(socket_num);
	}
    }
}

/*-----------> process_stdin <-----------*/
void process_stdin(int socket_num) {
    uint8_t buf[MAXBUF]; // buffer holding one line of user input
    int send_len = 0; // # of bytes read from stdin (including null)

    send_len = read_from_stdin(buf); // prompts user and fills buffer

    sendPDU(socket_num, buf, send_len); // one send() of full PDU
}

/*-----------> process_msg_from_server <-----------*/
void process_msg_from_server(int socket_num) {
    uint8_t data_buf[MAXBUF]; // buffer to receive incoming PDU payload
    int message_len = 0; // # of data bytes received

    message_len = recvPDU(socket_num, data_buf, MAXBUF);

    if (message_len > 0) { // server to echo message back to user
	printf("Message received from server, length: %d Data: %s\n", message_len, data_buf);
    }
    else { // 0 bytes -> server closed connection
	printf("Server has terminated\n");
	close(socket_num); // clean up before exit
	exit(0); // client exites on server termination
    }
}

/*-----------> read_from_stdin <-----------*/
int read_from_stdin(uint8_t *buffer) {
    // reads one line of input then null terminates it
    int a_char = 0; // to detect EOF which is -1 (int)
    int input_len = 0;

    buffer[0] = '\0'; // start with empty string
    printf("Enter data: ");
    fflush(stdout); 
    while (input_len < (MAXBUF - 1) && a_char != '\n') {
	a_char = getchar();
	if (a_char == EOF) { // user closed stdin with ^D
	    exit(0);
	}
	if (a_char != '\n') {
	    buffer[input_len] = (char)a_char;
	    input_len++;
	}
    }

    buffer[input_len] = '\0'; // null terminator for safe printing
    input_len++; // include null terminator in byte count thats sent

    return input_len;
}

/*-----------> check_args <-----------*/
void check_args(int argc, char *argv[]) {
    if (argc != 3) {
	fprintf(stderr, "Usage: %s host-name port-number\n", argv[0]);
	exit(1);
    }
}
