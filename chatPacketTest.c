/* chatPacketTest.c */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "chatPacket.h"
#include "chatFlags.h"

static void printBuffer(const uint8_t *buffer, int len) {
    if (len < 0) {
        printf(" (function returned %d - error)\n\n", len);
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
    uint8_t buffer[1500];
    int len;

    /* ============================================================ */
    /* chatHeaderPacket — flags 2, 3, 10, 13 (just the flag byte)   */
    /* ============================================================ */

    printf("===== chatHeaderPacket: flag-2 (good connect) =====\n");
    len = chatHeaderPacket(buffer, GOOD_CONNECT_FLAG);
    printBuffer(buffer, len);
    /* Expected: flag byte 0x02, total 1 byte */

    printf("===== chatHeaderPacket: flag-3 (bad connect) =====\n");
    len = chatHeaderPacket(buffer, BAD_CONNECT_FLAG);
    printBuffer(buffer, len);
    /* Expected: flag byte 0x03, total 1 byte */

    printf("===== chatHeaderPacket: flag-10 (list request) =====\n");
    len = chatHeaderPacket(buffer, HANDLE_LIST_FLAG);
    printBuffer(buffer, len);
    /* Expected: flag byte 0x0a, total 1 byte */

    printf("===== chatHeaderPacket: flag-13 (list done) =====\n");
    len = chatHeaderPacket(buffer, HANDLE_FINISH_FLAG);
    printBuffer(buffer, len);
    /* Expected: flag byte 0x0d, total 1 byte */

    /* ============================================================ */
    /* handleOptionsPacket — flag + 1-byte length + handle bytes    */
    /* ============================================================ */

    printf("===== handleOnlyPacket: flag-1 with 'alice' =====\n");
    len = handleOnlyPacket(buffer, INIT_CONNECT_FLAG, "alice");
    printBuffer(buffer, len);
    /* Expected: 01 05 61 6c 69 63 65, total 7 bytes */

    printf("===== handleOnlyPacket: flag-7 with 'bob' =====\n");
    len = handleOnlyPacket(buffer, HANDLE_ERR_FLAG, "bob");
    printBuffer(buffer, len);
    /* Expected: 07 03 62 6f 62, total 5 bytes */

    printf("===== handleOnlyPacket: flag-12 with 'carol' =====\n");
    len = handleOnlyPacket(buffer, HANDLE_ITEM_FLAG, "carol");
    printBuffer(buffer, len);
    /* Expected: 0c 05 63 61 72 6f 6c, total 7 bytes */

    printf("===== handleOnlyPacket: empty handle (should fail) =====\n");
    len = handleOnlyPacket(buffer, INIT_CONNECT_FLAG, "");
    printf("returned: %d (expected -1)\n\n", len);

    printf("===== handleOnlyPacket: oversized handle (should fail) =====\n");
    char tooLong[150];
    memset(tooLong, 'x', 149);
    tooLong[149] = '\0';
    len = handleOnlyPacket(buffer, INIT_CONNECT_FLAG, tooLong);
    printf("returned: %d (expected -1)\n\n", len);

    printf("===== handleOnlyPacket: exactly 100-char handle (should succeed) =====\n");
    char maxHandle[101];
    memset(maxHandle, 'a', 100);
    maxHandle[100] = '\0';
    len = handleOnlyPacket(buffer, INIT_CONNECT_FLAG, maxHandle);
    printf("returned: %d (expected 102)\n", len);
    printBuffer(buffer, len);
    /* Expected: byte 0 = 0x01 flag, byte 1 = 0x64 (100), then 100 'a's */

    /* ============================================================ */
    /* listCountPacket — flag + 4-byte count in network byte order  */
    /* ============================================================ */

    printf("===== listCountPacket: count = 0 =====\n");
    len = listCountPacket(buffer, 0);
    printBuffer(buffer, len);
    /* Expected: 0b 00 00 00 00, total 5 bytes */

    printf("===== listCountPacket: count = 1 =====\n");
    len = listCountPacket(buffer, 1);
    printBuffer(buffer, len);
    /* Expected: 0b 00 00 00 01, total 5 bytes */

    printf("===== listCountPacket: count = 256 =====\n");
    len = listCountPacket(buffer, 256);
    printBuffer(buffer, len);
    /* Expected: 0b 00 00 01 00, total 5 bytes */

    printf("===== listCountPacket: count = 0x12345678 (network order check) =====\n");
    len = listCountPacket(buffer, 0x12345678);
    printBuffer(buffer, len);
    /* Expected: 0b 12 34 56 78, total 5 bytes
       If you see "0b 78 56 34 12", htonl is missing/broken. */

    /* ============================================================ */
    /* broadcastPacket — flag + srclen + src + null-term text       */
    /* ============================================================ */

    printf("===== broadcastPacket: 'alice' broadcasts 'hello' =====\n");
    len = broadcastPacket(buffer, "alice", "hello", 6);
    printBuffer(buffer, len);
    /* Expected: 04 05 'alice' 'hello' 00, total 13 bytes */

    printf("===== broadcastPacket: 'alice' broadcasts empty text =====\n");
    len = broadcastPacket(buffer, "alice", "", 1);
    printBuffer(buffer, len);
    /* Expected: 04 05 'alice' 00, total 8 bytes */

    printf("===== broadcastPacket: empty src handle (should fail) =====\n");
    len = broadcastPacket(buffer, "", "hello", 6);
    printf("returned: %d (expected -1)\n\n", len);

    printf("===== broadcastPacket: text too long (should fail) =====\n");
    char longText[300];
    memset(longText, 'x', 299);
    longText[299] = '\0';
    len = broadcastPacket(buffer, "alice", longText, 300);
    printf("returned: %d (expected -1)\n\n", len);

    /* ============================================================ */
    /* messagePacket — flag + srclen + src + dstcount + dsts + text */
    /* ============================================================ */

    printf("===== messagePacket: %%M from 'alice' to 'bob' saying 'hello' =====\n");
    const char *dstSingle[] = {"bob"};
    len = messagePacket(buffer, UNICAST_FLAG, "alice", dstSingle, 1, "hello", 6);
    printBuffer(buffer, len);
    /* Expected: 05 05 'alice' 01 03 'bob' 'hello' 00, total 18 bytes */

    printf("===== messagePacket: %%C from 'alice' to 3 dests saying 'hi' =====\n");
    const char *dstMulti[] = {"bob", "carol", "dave"};
    len = messagePacket(buffer, MULTICAST_FLAG, "alice", dstMulti, 3, "hi", 3);
    printBuffer(buffer, len);
    /* Expected: 06 05 'alice' 03 03 'bob' 05 'carol' 04 'dave' 'hi' 00,
       total 26 bytes */

    printf("===== messagePacket: numDst = 0 (should fail) =====\n");
    len = messagePacket(buffer, MULTICAST_FLAG, "alice", dstMulti, 0, "hi", 3);
    printf("returned: %d (expected -1)\n\n", len);

    printf("===== messagePacket: numDst = 10 (should fail) =====\n");
    const char *dstTooMany[] = {"a","b","c","d","e","f","g","h","i","j"};
    len = messagePacket(buffer, MULTICAST_FLAG, "alice", dstTooMany, 10, "hi", 3);
    printf("returned: %d (expected -1)\n\n", len);

    printf("===== messagePacket: empty src handle (should fail) =====\n");
    len = messagePacket(buffer, UNICAST_FLAG, "", dstSingle, 1, "hi", 3);
    printf("returned: %d (expected -1)\n\n", len);

    printf("===== messagePacket: empty dst handle (should fail) =====\n");
    const char *dstEmpty[] = {""};
    len = messagePacket(buffer, UNICAST_FLAG, "alice", dstEmpty, 1, "hi", 3);
    printf("returned: %d (expected -1)\n\n", len);

    /* ============================================================ */
    /* Round-trip parser test: handleOptionsPacket                  */
    /* ============================================================ */

    printf("===== Round-trip: build flag-1 'alice', parse it back =====\n");
    len = handleOnlyPacket(buffer, INIT_CONNECT_FLAG, "alice");
    printBuffer(buffer, len);

    uint8_t flag = getFlag(buffer);
    char outName[101];
    int newOffset = getHandleAt(buffer, 1, outName);
    printf("parsed flag:    %d (expected 1)\n", flag);
    printf("parsed handle:  '%s' (expected 'alice')\n", outName);
    printf("ending offset:  %d (expected %d)\n\n", newOffset, len);

    /* ============================================================ */
    /* Round-trip parser test: messagePacket with multiple dests    */
    /* ============================================================ */

    printf("===== Round-trip: build %%C alice -> bob,carol,dave 'hi', parse it back =====\n");
    len = messagePacket(buffer, MULTICAST_FLAG, "alice", dstMulti, 3, "hi", 3);
    printBuffer(buffer, len);

    flag = getFlag(buffer);
    printf("parsed flag: %d (expected 6)\n", flag);

    char src[101], d1[101], d2[101], d3[101];
    int off = 1;  /* skip flag byte */
    off = getHandleAt(buffer, off, src);
    printf("src handle:  '%s' (expected 'alice'), offset now %d\n", src, off);

    uint8_t numDst = buffer[off++];
    printf("numDst:      %d (expected 3), offset now %d\n", numDst, off);

    off = getHandleAt(buffer, off, d1);
    printf("dst1:        '%s' (expected 'bob'),   offset now %d\n", d1, off);
    off = getHandleAt(buffer, off, d2);
    printf("dst2:        '%s' (expected 'carol'), offset now %d\n", d2, off);
    off = getHandleAt(buffer, off, d3);
    printf("dst3:        '%s' (expected 'dave'),  offset now %d\n", d3, off);

    const char *parsedText = (const char *)(buffer + off);
    printf("text:        '%s' (expected 'hi')\n", parsedText);
    printf("final offset: %d (expected %d)\n\n", off + (int)strlen(parsedText) + 1, len);

    return 0;
}
