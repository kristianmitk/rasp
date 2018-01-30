#ifndef rasp_nodes_h
#define rasp_nodes_h

extern "C" {
    #include <stdint.h>
}

#define RASP_NUM_SERVERS  4
#define RASP_DEFAULT_PORT 1337


typedef struct rasp_server {
    const char *IP;
    uint32_t    ID;
} rasp_server;

/**
 * -----------------------------------------------------------------------------
 * NOTE: Currently servers are static, so membership changes are not possible.
 * If you want to add more servers, make sure to assign a static ip (at the
 * corresponding router) and add it here.
 * -----------------------------------------------------------------------------
 * Chip-ID/Mac-Address/IP-Address of every node in the actual cluster.
 * -----------------------------------------------------------------------------
 * ChipID 42712
 *  dhcp-host=2C:3A:E8:00:A6:D8,dev-thing-1,192.168.1.21
 * ChipID 44293
 *  dhcp-host=2C:3A:E8:00:AD:05,dev-thing-2,192.168.1.22
 * ChipID 9085487
 *  dhcp-host=5C:CF:7F:8A:A2:2F,dev-thing-3,192.168.1.23
 * ChipID 9053586
 *  dhcp-host=5C:CF:7F:8A:25:92,dev-thing-4,192.168.1.24
 * -----------------------------------------------------------------------------
 */

// NOTE: server running this code is included as well
const rasp_server servers[RASP_NUM_SERVERS] = {
    { "192.168.1.21", 42712   },
    { "192.168.1.22", 44293   },
    { "192.168.1.23", 9085487 },
    { "192.168.1.24", 9053586 },
};
#endif // ifndef rasp_nodes_h
