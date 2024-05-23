#ifndef _HELPERS_H
#define _HELPERS_H 1

// DIE macro
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

enum my_bool {
	false, true
};

// helpful macros
#define OUT -1
#define INT_TYPE 0
#define SHORT_REAL 1
#define FLOAT_TYPE 2
#define STRING_TYPE 3

#define EXIT_MESSAGE "exit"
#define SUBSCRIBE 's'
#define UNSUBSCRIBE 'u'
#define EXIT 'e'

#define FD_START 5

#define CLIENTS_NUMBER 1000
#define BUFF_DIM 100
#define INFO_DIM 1501
#define TOPIC_DIM 51
#define IP_DIM 16
#define ID_DIM 10

// a structure to keep both 
// tcp and udp sockets together
typedef struct sockets {
	int udp_socket;
	int tcp_socket;
} sockets;

typedef struct msg_tcp {
	char ip[IP_DIM];
	uint16_t port;
	char type[11];
	char topic[TOPIC_DIM];
	char info[INFO_DIM];
} msg_tcp;

typedef struct msg_udp {
	char topic[TOPIC_DIM - 1];
	char type;
	char info[INFO_DIM];
} msg_udp;

typedef struct topic{
	char name[TOPIC_DIM];
	uint8_t sf;
} topic;

// client structure
typedef struct client{
	char id[ID_DIM];
	short socket;
	int dim_topics;
	int dim_unsent;
	msg_tcp unsent[BUFF_DIM];
	topic topics[BUFF_DIM];
	uint8_t online; // 1 online, 0 nu
} client;

// packet structure
typedef struct Packet {
	char type;  // exit = e, subscribe = s, unsubscribe = u
	char topic[TOPIC_DIM];
	char datatype;
	char info[INFO_DIM];
	char ip[IP_DIM];
	uint16_t port;
	uint8_t sf;
} Packet;

#define PACKLEN sizeof(Packet)

// helper functions for server


int enabling (int tcp_socket) {
	int enable = true;
	setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, &enable, 4);

	return tcp_socket;
}

// function to update the maximum
void get_max (int val1, int val2, int *max) {
	(val1 > val2) ? (*max = val1) : (*max = val2);
}

// function to initialise the sockets
void sockets_initialiser (sockets *sockets) {
	(*sockets).udp_socket = socket(PF_INET, SOCK_DGRAM, 0);
	(*sockets).tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	DIE((*sockets).udp_socket < 0 || (*sockets).tcp_socket < 0, "socket");
}

// function to set the adresses used for both server and client
struct sockaddr_in set_addr(struct sockaddr_in serv_addr, uint16_t port, uint16_t sel) {
	if (sel == true) {
		memset((char *)&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_addr.s_addr = INADDR_ANY;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	return serv_addr;
}

// function for binding the sockets
sockets binding (sockets *sockets, struct sockaddr_in serv_addr, struct sockaddr_in udp_addr) {
	int tcp_bnd  = bind((*sockets).tcp_socket, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
	DIE(tcp_bnd < 0, "bind");

	int udp_bnd  = bind((*sockets).udp_socket, (struct sockaddr *)&udp_addr, sizeof(struct sockaddr));
	DIE(udp_bnd < 0, "bind");

	return *sockets;
}

// function to set the file descriptors
void fd_setter(fd_set *fd, int *tcp_socket) {
	FD_ZERO(&(*fd));
	FD_SET(*tcp_socket, &(*fd));
	FD_SET(STDIN_FILENO, &(*fd));
}

// function to connect a client
client *client_connected(client *clients, int i, struct sockaddr_in new_tcp, int socket) {
	(*(clients + i)).socket = socket;
	(*(clients + i)).online = true;
	printf("New client %s connected from %s:%d\n", (*(clients + i)).id,
	inet_ntoa(new_tcp.sin_addr), ntohs(new_tcp.sin_port));

	return clients;
}

// function to disconnect a client
client *client_disconnected(client *clients, int i, int n, fd_set *fd) {
	uint16_t ok = true;
	for (int j = FD_START; j <= n && ok; j++) {
		if((*(clients + j)).socket == i) {
			printf("Client %s disconnected.\n", (*(clients + j)).id);
			(*(clients + j)).online = false;
			(*(clients + j)).socket = OUT;
			FD_CLR(i, &(*fd));
			close(i);
			ok = false;
		}
	}

	return clients;
}

void update (fd_set *aux_fd, fd_set *fd, int check_var, int fd_max) {
	*aux_fd = *fd;
	check_var = select(fd_max + 1, &(*aux_fd), NULL, NULL, NULL);
	DIE(check_var < 0, "select");
}

#endif
