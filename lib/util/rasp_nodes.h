#ifndef rasp_nodes_h
#define rasp_nodes_h

extern "C" {
    #include <stdint.h>
}

/**
 * -----------------------------------------------------------------------------
 * NOTE: Currently servers are static, so membership changes are not possible.
 * If you want to add more servers, make sure to assign a static ip (at the
 * corresponding router) and add it here.
 * -----------------------------------------------------------------------------
 * Chip-ID/Mac-Address/IP-Address of every node in the actual cluster.
 * -----------------------------------------------------------------------------
 * chipId 42712
 *  dhcp-host=2C:3A:E8:00:A6:D8,dev-thing-1,192.168.1.21
 * chipId 44293
 *  dhcp-host=2C:3A:E8:00:AD:05,dev-thing-2,192.168.1.22
 * chipId 9085487
 *  dhcp-host=5C:CF:7F:8A:A2:2F,dev-thing-3,192.168.1.23
 * chipId 9053586
 *  dhcp-host=5C:CF:7F:8A:25:92,dev-thing-4,192.168.1.24
 * -----------------------------------------------------------------------------
 */

typedef struct raspServer {
    uint8_t  IP[4];
    uint32_t ID;
} raspServer_t;

// NOTE: server running this code is included as well
const raspServer_t servers[] = {
    // { { 192, 168, 1, 21 }, 42712   },
    { { 192, 168, 1, 22 }, 44293   },
    { { 192, 168, 1, 23 }, 9085487 },
    { { 192, 168, 1, 24 }, 9053586 },
};

const uint8_t RASP_NUM_SERVERS = sizeof(servers) / sizeof(servers[0]);

#endif // ifndef rasp_nodes_h
