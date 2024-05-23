#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include "helpers.h"
#include <sys/select.h>

// function for sending the specific message
void sending(int *tcp_socket, Packet *pack, char subscribe, char *buff)
{
	// breaking the buffer into tokens
	// common part of the code
	char *token = strtok(buff, " ");
	(*pack).type = SUBSCRIBE;
	token = strtok(NULL, " ");
	strcpy((*pack).topic, token);
	token = strtok(NULL, " ");

	// in case of subscribe or unsubscribe
	(subscribe == SUBSCRIBE) ? ((*pack).datatype = *token - '0') : ((*pack).datatype = *token);

	int check_var = send(*tcp_socket, &(*pack), PACKLEN, 0);
	DIE(check_var < 0, "send");

	// again printing the different message for the cases
	(subscribe == SUBSCRIBE) ? (printf("Subscribed to topic.\n")) : (printf("Unsubscribed to topic.\n"));
}

int main(int argc, char **argv)
{
	// if there aren't enough arguments
	DIE(argc < 4, "arguments");

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);

	// setting the tcp socket
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_socket < 0, "socket");

	// setting the addresses
	struct sockaddr_in server_data;
	uint16_t port = (uint16_t)atoi(*(argv + 3));
	server_data = set_addr(server_data, port, false);
	inet_aton(*(argv + 2), &server_data.sin_addr);

	// setting the file descriptors
	fd_set fd, aux_fd;
	fd_setter(&fd, &tcp_socket);

	// checking the connect
	int check_var = connect(tcp_socket, (struct sockaddr *)&server_data, sizeof(server_data));
	DIE(check_var < 0, "connect");

	check_var = send(tcp_socket, *(argv + 1), 10, 0);
	DIE(check_var < 0, "send");

	Packet pack;
	uint8_t on = true;

subscriber:
	// updating the auxiliary fd
	update(&aux_fd, &fd, check_var, tcp_socket + 1);

	// stdin commands
	if (FD_ISSET(STDIN_FILENO, &aux_fd)) {
		enabling(tcp_socket);

		char *buff;
		buff = calloc(BUFF_DIM, sizeof(char));
		DIE(buff == NULL, "calloc");

		fgets(buff, BUFF_DIM, stdin);
		memset(&pack, 0, PACKLEN);

		if (strncmp(buff, EXIT_MESSAGE, strlen(EXIT_MESSAGE)) == 0) {
			pack.type = EXIT;
			check_var = send(tcp_socket, &pack, PACKLEN, 0);
			DIE(check_var < 0, "send");
			on = false;

		// subscribing
		} else if (strncmp(buff, "subscribe", strlen("subscribe")) == 0) {
			sending(&tcp_socket, &pack, SUBSCRIBE, buff);

		// unsubscribing
		} else if (strncmp(buff, "unsubscribe", (strlen("unsubscribe"))) == 0) {
			sending(&tcp_socket, &pack, UNSUBSCRIBE, buff);
		}

		free(buff);
	}

	if (FD_ISSET(tcp_socket, &aux_fd)) {
		char *buff = calloc(1, sizeof(msg_tcp));
		DIE(buff == NULL, "calloc");

		// message from server
		check_var = recv(tcp_socket, buff, sizeof(msg_tcp), 0);
		DIE(check_var < 0, "receive");

		(check_var == 0) ? (on = false) : (on = true);
		msg_tcp *pack_send = (msg_tcp *)buff;

		printf("%s:%u - %s - %s - %s\n", (*pack_send).ip, (*pack_send).port,
			   (*pack_send).topic, (*pack_send).type, (*pack_send).info);

		free(buff);
	}

	// loop
	if (on == true)
		goto subscriber;

	// closing the socket
	close(tcp_socket);
	return 0;
}
