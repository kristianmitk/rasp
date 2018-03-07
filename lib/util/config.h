#ifndef rasp_config_h
#define rasp_config_h

/* +++++++++++++++++++++++++++++++ ENV ++++++++++++++++++++++++++++++++++++++ */

// specified if some initial setup routines have to be executed
// by now only used by the RASPFS class

// #define INITIAL_SETUP

// not used by now - evetually will minimize debug messages on serial output
#define RASP_DEBUG

/* +++++++++++++++++++++++++++++ CONSTANTS ++++++++++++++++++++++++++++++++++ */

// SERIAL BAUD RATE
#define DEFAULT_BAUD_RATE 115200

// SSID CREDENTIALS
#define SSID "PiFun1337"
#define SSID_PW "RaSpFun1337!!"

// RAFT TIMEOUTS
#define MIN_ELECTION_TIMEOUT 500
#define MAX_ELECTION_TIMEOUT 800
#define HEARTBEAT_TIMEOUT 200

// UDP SERVER
#define UDP_INCOMING_BUFFER_SIZE 512
#define RASP_DEFAULT_PORT 1337

// LOG
#define LOG_SIZE 0x4000
#define NUM_LOG_ENTRIES 512

#endif // ifndef rasp_config_h
