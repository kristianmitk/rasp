#ifndef rasp_config_h
#define rasp_config_h

/* +++++++++++++++++++++++++++++++ ENV ++++++++++++++++++++++++++++++++++++++ */

// specified if some initial setup routines have to be executed
// by now only used by the RASPFS class

// #define INITIAL_SETUP

// not used by now - evetually will minimize debug messages on serial output
#define RASP_DEBUG

// USE PRINT_DEBUG INSTEAD
#define PRINT_DEBUG 0

/* ++++++++++++++++++++++++ PRINT_DEBUG CONDITIONAL +++++++++++++++++++++++++ */

// instead of using `printf("lorem ipsum %d", intVal)`
// rather use `DEBUG("lorem ipsum %d", intVal)` which will substitude
// depending on if PRINT_DEBUG is set or not, i.e:
// IF PRINT_DEBUG == 1
// DEBUG("lorem ipsum %d", intVal) evals to printf("lorem ipsum %d", intVal)
// ELSE
// DEBUG("lorem ipsum %d", intVal) evals to nothing - void

#define CONCATENATE(a, b) a ## _ ## b

#define IF(c, t, e) CONCATENATE(IF, c)(t, e)
#define IF_0(t, e) e
#define IF_1(t, e) t

#define RASPDBG(...) IF(PRINT_DEBUG, Serial.printf(__VA_ARGS__); , )


/* +++++++++++++++++++++++++++++ CONSTANTS ++++++++++++++++++++++++++++++++++ */

// SERIAL BAUD RATE
#define DEFAULT_BAUD_RATE 115200

// SSID CREDENTIALS
#define SSID "PiFun1337"
#define SSID_PW "RaSpFun1337!!"

// RAFT TIMEOUTS
#define MIN_ELECTION_TIMEOUT 150
#define MAX_ELECTION_TIMEOUT 300
#define HEARTBEAT_TIMEOUT 75

// UDP SERVER
#define UDP_INCOMING_BUFFER_SIZE 512
#define RASP_DEFAULT_PORT 1337

// LOG
#define LOG_SIZE 0x4000
#define NUM_LOG_ENTRIES 512

#endif // ifndef rasp_config_h
