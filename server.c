#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <limits.h>
#include "helpers.h"

int main(int argc, char **argv)
{
	// if there aren't enough arguments
	DIE(argc < 2, "arguments");

	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	
	// clients array
	client *clients = calloc(CLIENTS_NUMBER, sizeof(client));
	DIE(clients == NULL, "calloc");

	// setting both udp and tcp sockets
	sockets sockets;
	sockets_initialiser(&sockets);

	// getting the port number
	uint16_t port = (uint16_t)atoi(*(argv + 1));

	// setting the addresses
	struct sockaddr_in serv_addr, udp_addr, new_tcp;
	serv_addr = set_addr(serv_addr, port, true);
	udp_addr = set_addr(udp_addr, port, true);

	// binding the sockets
	binding(&sockets, serv_addr, udp_addr);

	int check_var = listen(sockets.tcp_socket, INT_MAX);
	DIE(check_var < 0, "listen");

	// adding the file descriptors
	fd_set fd, aux_fd;
	fd_setter(&fd, &sockets.tcp_socket);
	FD_SET(sockets.udp_socket, &fd);

	socklen_t udp_len = sizeof(struct sockaddr);

	// the maximum values from file descriptors
	// between tcp and udp
	int fd_max;
	get_max(sockets.udp_socket, sockets.tcp_socket, &fd_max);

	// helpful variables
	uint8_t on = true, ok = true;

server:
	update(&aux_fd, &fd, check_var, fd_max);
	for (int i = 0; i <= fd_max; i++) {
		if (FD_ISSET(i, &aux_fd)) {
			char *buff = calloc(PACKLEN, sizeof(char));
			DIE(buff == NULL, "calloc");

			// tcp
			if (i == sockets.tcp_socket) {
				enabling(sockets.tcp_socket);
				int socket = accept(sockets.tcp_socket, (struct sockaddr *)&new_tcp, &udp_len);
				DIE(socket < 0, "accept");

				check_var = recv(socket, buff, 10, 0);
				DIE(check_var < 0, "recv");
				int pos = OUT, online = false;
				ok = true;
				for (int j = FD_START; j <= fd_max && ok; j++) {
					if (strncmp((*(clients + j)).id, buff, ID_DIM) == 0) {
						pos = j;
						online = (*(clients + j)).online;
						ok = false;
					}
				}
				// new
				if (ok) {
					// new client
					FD_SET(socket, &fd);

					// updating the maximum
					get_max(socket, fd_max, &fd_max);
					
					// a new client connected
					strcpy((*(clients + fd_max)).id, buff);
					client_connected(clients, fd_max, new_tcp, socket);

				// reconnected
				} else if (pos && !online) {
					FD_SET(socket, &fd);
					(*(clients + pos)).socket = socket;
					(*(clients + pos)).online = true;

					printf("New client %s connected from %s:%d.\n", (*(clients + pos)).id,
						   inet_ntoa(new_tcp.sin_addr), ntohs(new_tcp.sin_port));

					for (int k = 0; k < (*(clients + pos)).dim_unsent; k++) {
						check_var = send((*(clients + pos)).socket, &(*((*(clients + pos)).unsent + k)),
										 sizeof(msg_tcp), 0);
						DIE(check_var < 0, "send");
					}

					(*(clients + pos)).dim_unsent = 0;

					// already connected
				} else {
					close(socket);
					printf("Client %s already connected.\n", (*(clients + pos)).id);
				}
			// udp
			} else if (i == sockets.udp_socket) {
				recvfrom(sockets.udp_socket, buff, 1551, 0, (struct sockaddr *)&udp_addr,
						 &udp_len);

				msg_tcp send_to_tcp;

				memset(&send_to_tcp, 0, sizeof(msg_tcp));
				msg_udp *send_to_udp;

				port = udp_addr.sin_port;
				send_to_tcp.port = htons(port);
				strcpy(send_to_tcp.ip, inet_ntoa(udp_addr.sin_addr));

				send_to_udp = (msg_udp *)buff;

				strcpy(send_to_tcp.topic, (*send_to_udp).topic);

				// int
				if ((*send_to_udp).type == INT_TYPE) {
					uint32_t num;
					num = ntohl(*(uint32_t *)(((*send_to_udp).info) + 1));
					if (*((*send_to_udp).info) == 1) {
						num = num * (-1);
					}
					sprintf(send_to_tcp.info, "%d", num);

					strcpy(send_to_tcp.type, "INT");
				}

				// short
				double real;
				if ((*send_to_udp).type == SHORT_REAL) {
					real = abs(ntohs(*(uint16_t *)(((*send_to_udp).info))));
					real = real / 100;
					strcpy(send_to_tcp.type, "SHORT_REAL");
					sprintf(send_to_tcp.info, "%.2f", real);
				}

				// float
				if ((*send_to_udp).type == FLOAT_TYPE) {
					real = ntohl(*(uint32_t *)(((*send_to_udp).info) + 1));

					for (int j = 0; j < *((*send_to_udp).info + FD_START); j++)
						real = real / 10;

					strcpy(send_to_tcp.type, "FLOAT");

					if (*((*send_to_udp).info) == 1) {
						real = real * (-1);
					}
					sprintf(send_to_tcp.info, "%lf", real);
				}

				// string
				if ((*send_to_udp).type == STRING_TYPE) {
					strcpy(send_to_tcp.type, "STRING");
					strcpy(send_to_tcp.info, ((*send_to_udp).info));
				}

				ok = true;
				for (int j = FD_START; j <= fd_max; j++) {
					for (int k = 0; k < (*(clients + j)).dim_topics && ok; k++) {
						if (strncmp((*((*(clients + j)).topics + k)).name, send_to_tcp.topic, TOPIC_DIM) == 0) {
							if ((*(clients + j)).online) {
								check_var = send((*(clients + j)).socket, &send_to_tcp,
												 sizeof(msg_tcp), 0);
								DIE(check_var < 0, "send");
							}
							else if ((*((*(clients + j)).topics + k)).sf == 1) {
								(*(clients + j)).unsent[(*(clients + j)).dim_unsent++] = send_to_tcp;
							}
							
							ok = false;
						}
					}
					ok = true;
				}

			} else if (i == STDIN_FILENO) {
				// exiting
				fgets(buff, BUFF_DIM, stdin);
				(strncmp(buff, EXIT_MESSAGE, strlen(EXIT_MESSAGE)) == 0) ? (on = false) : (on = true);

				DIE(strncmp(buff, EXIT_MESSAGE, strlen(EXIT_MESSAGE)) != 0, EXIT_MESSAGE);

			} else {
				free(buff);
				buff = calloc(PACKLEN, sizeof(char));
				DIE(buff == NULL, "calloc");

				check_var = recv(i, buff, PACKLEN, 0);
				DIE(check_var < 0, "recv");

				if (check_var) {
					client *c;

					ok = true;
					for (int j = FD_START; j <= fd_max && ok; j++) {
						if (i == (*(clients + j)).socket) {
							c = &(*(clients + j));
							ok = false;
						}
					}

					Packet *input = (Packet *)buff;
					ok = true;
					int dim = ((*c).dim_topics);

					// subscribe
					if ((*input).type == SUBSCRIBE) {
						int pos = OUT;

						for (int k = 0; k < dim && ok; k++) {
							if (strncmp((*(((*c).topics) + k)).name, ((*input).topic), TOPIC_DIM) == 0) {
								pos = k;
								ok = false;
							}
						}

						if (pos == OUT) {
							strcpy((*(((*c).topics) + dim)).name, ((*input).topic));
							(*(((*c).topics) + dim)).sf = ((*input).datatype);
							((*c).dim_topics)++;
						}

					// unsubscribe
					} else if ((*input).type == UNSUBSCRIBE) {
						int pos = OUT;
						for (int k = 0; k < dim && ok; k++)
							if (strncmp((*(((*c).topics) + k)).name, ((*input).topic), TOPIC_DIM) == 0) {
								pos = k;
								ok = false;
							}
						if (pos != OUT) {
							for (int l = pos; l < dim; l++)
								*((*c).topics + l) = *((*c).topics + l + 1);

							((*c).dim_topics)--;
						}
					}
					// disconnecting the client
					else if ((*input).type == EXIT) {
						client_disconnected(clients, i, fd_max, &fd);
					}

				// again disconnecting the client
				} else {
					client_disconnected(clients, i, fd_max, &fd);
				}
			}
		}
	}
	// keeping the loop
	if (on == true) {
		goto server;
	}

	// closing
	for (int i = 3; i <= fd_max; i++) {
		if (FD_ISSET(i, &fd))
			close(i);
	}

	// freeing the memory
	free(clients);

	return 0;
}