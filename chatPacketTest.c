/* chatPacketTest.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "chatPacket.h"
#include "chatFlags.h"

static void printBuffer(const uint8_t *buffer, int len) {
    if (len < 0) {
	printf(" (function returned %d - error)\n", len);
	return;
    }
    for (int i = 0; i < len; i++) {
	uint8_t byte = buffer[i];
	if (isprint(byte)) {
	    printf(" [%2d] 0x%02x  '%c'\n", i, byte, byte);
	}
	else {
	    printf(" [%2d] 0x%02x\n", i, byte);
	}
    }
    printf(" total: %d bytes\n\n", len);
}

int main(void) {

    uint8_t buffer[16];
    int len = chatHeaderPacket(buffer, INIT_CONNECT_FLAG);
    printf("flag-2 packet: ");
    printBuffer(buffer, len); // expected = 02 (1 byte total)

    len = chatHeaderPacket(buffer, HANDLE_LIST_FLAG);
    printf("flag-10 packet: ");
    printBuffer(buffer, len); // expected = 0a (1 byte total)

    len = handleOptionsPacket(buffer, INIT_CONNECT_FLAG, "test1");
    // expected = 01 05 61 6c 69 63 65 (7 bytes)
    //            flag len a l i c e
    
    len = handleOptionsPacket(buffer, HANDLE_ERR_FLAG, "bob");
    // expected = 08 03 62 6f 62 (5 bytes)
    //            flag len b o b


    len = handleOptionsPacket(buffer, HANDLE_ITEM_FLAG, "carol");
    // expected = 0c 05 63 61 72 6f 6c (7 bytes)
    //            flag len c a r o l
    

    int result = handleOptionsPacket(buffer, INIT_CONNECT_FLAG, "");
    printf("empty handle result: %d (expected -1)\n", result);

    char tooLong[150];
    memset(tooLong, 'x', 149);
    tooLong[149] = '\0';
    result = handleOptionsPacket(buffer, INIT_CONNECT_FLAG, tooLong);
    printf("oversized handle result: %d (expected -1)\n", result);


    char max[101];
    memset(max, 'a', 100);
    max[100] = '\0';
    result = handleOptionsPacket(buffer, INIT_CONNECT_FLAG, max);
    printf("max-length handle result: %d (expected 102)\n", result);
    // first two bytes should be: 01 64 (flag = 1, length = 100)
    printBuffer(buffer, result);

    len = listCountPacket(buffer, 0);
    // expected = 0b 00 00 00 (5 bytes)
    //            flag, then 4 zero bytes

    len = listCountPacket(buffer, 1);
    // expected = 0b 00 00 00 01 (5 bytes)
    //            flag, then 1 in network/big-endian order

    len = listCountPacket(buffer, 256);
    // expected = 0b 00 00 01 00 (5 bytes)
    //            256 = 0x00000100 in network order

    len = listCountPacket(buffer, 0x12345678);
    // expected = 0b 12 34 56 78 (5 bytes)
    //            verifies high byte first, low byte last

    len = broadcastPacket(buffer, "alice", "hello", 6);
    // expected = 04 05 61 69 63 65 68 65 6c 6c 6f 00 (13 bytes)
    //            flag len a l i c e h e l l o null
    
    len = broadcastPacket(buffer, "alice", "", 1);
    // expected = 04 05 61 6c 69 63 65 00 (8 bytes)
    
    const char *dsts[] = {"bob"};
    len = messagePacket(buffer, UNICAST_FLAG, "alice", dsts, 1, "hello", 6);
    // expected = 05 05 61 6c 69 63 65 01 03 62 6f 62 68 65 6c 6c 6f 00
    //            flag srclen "alice" dstcount dstlen "bob" "hello\0"
    //                         (5)    (1)             (3)
    
    const char *dsts2[] = {"bob", "carol", "dave"};
    len = messagePacket(buffer, MULTICAST_FLAG, "alice", dsts2, 3, "hi", 3);
    // expected = 06 05 alice 03 03 bob 05 carol 04 dave 'h' 'i' 0
    //            flag srclen=5 dstcount=3
    
    uint8_t buffer[64];
    int len = handleOptionsPacket(buffer, INIT_CONNECT_FLAG, "alice");

    // parse it back
    uint8_t flag = getFlag(buffer);
    char outName[101];
    int newOffset = getHandleAt(buffer, 1, outName);

    printf("parsed flag: %d (expected 1)\n", flag);
    printf("parsed handle: %s (expected alice)\n", outName);
    printf("new offset: %d (expected %d)\n", newOffset, len);

    return 0;
}
