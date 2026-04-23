/* chatFlags.h */

#ifndef CHATFLAGS_H
#define CHATFLAGS_H

/*-----------> Constants <-----------*/

// flags
#define INIT_CONNECT_FLAG	1 // flag for initial client connection to server
#define GOOD_CONNECT_FLAG	2 // initial connection passed, unique handle
#define BAD_CONNECT_FLAG	3 // initial connection failed, handle already exists
#define BROADCAST_FLAG	 	4 // broadcast from one client to all other clients (%B)
#define UNICAST_FLAG		5 // unicast message from one client to another (%M)
#define MULTICAST_FLAG	 	6 // multicast message from one client to (2-9) others (%C)
#define HANDLE_ERR_FLAG 	7 // non-existent handle found in multicast call; sending one flag for each NEH
#define UNUSED_8		8
#define UNUSED_9	 	9
#define HANDLE_LIST_FLAG 	10 // client requesting the list of handles (%L)
#define HANDLE_LIST_RESP_FLAG	11 // server responding to client request
#define HANDLE_ITEM_FLAG	12 // one flag 12 packet per handle in server's handle table
#define HANDLE_FINISH_FLAG 	13 // packet following flag 12 to tell client the %L is finished

// sizes
#define CHAT_HEADER_LEN 	3 // chat header len 3 bytes: 2 bytes PDU (network order), 1 byte flag
#define HANDLE_NAME_MAX 	100 // 100 bytes (800 bits) as a maximum for handle name
#define TEXT_LEN_MAX 		200 // messages have a max of 200 bytes (199 + '\0')
#define TEXT_DATA_CHUNK 	199 // max message size sent by client
#define HANDLES_MIN 		2 // minimum number of handles for multicast call
#define HANDLES_MAX 		9 // max number of handles for multicast call
#define STDIN_MAX		1400 // max size to send through STDIN
#define PDU_LEN_MAX		1500 // max size for the PDU sent through network

#endif
