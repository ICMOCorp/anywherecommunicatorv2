

#define SOCKET_ERROR       -1
#define SOCKET_INIT         0
#define SOCKET_OPENED       1
#define SOCKET_CONNECTED    2
#define SOCKET_PROCESSING   3

int open_socket(struct pollfd* socketfds, struct pollfd* clientfds);
int verify_connection(struct pollfd* clientfds);
int ping_connection();

int send_msg(struct pollfd* clientfds, char* msg, uint32_t msgLength);
int read_msg(struct pollfd* clientfds, char* msg);

int error(struct pollfd* clientfds);

void socket_job();