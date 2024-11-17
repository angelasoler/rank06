#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/select.h>
#include <netinet/in.h>

// write, close and select
// socket, accept, listen, send, recv and bind
// strstr, strlen, strcpy, strcat, memset and bzero
// malloc, realloc, free, calloc, atoi and exit
// sprintf
typedef struct {
	int id;
	char cmsg[300000];
}  t_clients;

t_clients clients[FD_SETSIZE - 1];

int	sockfd, nfds, nclients = 0;
fd_set	rfds, wfds, curfds;
struct sockaddr_in	servaddr;
socklen_t			len;
char				msg[300000];

void	fatal_error() {
	write(2, "Fatal error\n", 12);
	exit(1);
}

void validate_args(int nargs) {
	if (nargs != 2) {
		printf("wrongs args\n");
		exit(1);
	}
}

void init_socket(char *port) {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) 
		fatal_error();
	nfds = sockfd;

	FD_ZERO(&curfds);
	FD_SET(sockfd, &curfds);
	bzero(clients, sizeof(clients));
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(port));
	len = sizeof(servaddr);
	if (bind(sockfd, (struct sockaddr *)&servaddr, len) < 0 || listen(sockfd, 100) < 0)
		fatal_error();
}

void send_to_all(char *msg, int client_fd) {
	for (int fd = 0; fd <= nfds; fd++) {
		if (fd != sockfd && fd != client_fd)
			send(fd, msg, strlen(msg), 0);
	}
}

void desconnect_client(int client_fd) {
	FD_CLR(client_fd, &curfds);
	close(client_fd);
	bzero(msg, 300000);
	sprintf(msg, "server: client %d just left\n", clients[client_fd].id);
	send_to_all(msg, client_fd);
}

void treat_msg(char *buf, int client_fd) {
	for (size_t i = 0, j = strlen(clients[client_fd].cmsg); i < strlen(buf); i++, j++) {
		clients[client_fd].cmsg[j] = buf[i];
		if (buf[i] == '\n') {
			clients[client_fd].cmsg[i] = '\0';
			sprintf(msg, "client %d: %s\n", clients[client_fd].id, clients[client_fd].cmsg);
			send_to_all(msg, client_fd);
			write(1, msg, strlen(msg));
			bzero(clients[client_fd].cmsg, strlen(clients[client_fd].cmsg));
			bzero(msg, sizeof(msg));
			j = -1;
		}
	}
}

void receive_msg(int client_fd) {
	char buf[500] = {0};
	int bytes = recv(client_fd, buf, 300000, 0);

	if (bytes <= 0) {
		desconnect_client(client_fd);
		return ;
	}
	bzero(msg, 300000);
	treat_msg(buf, client_fd);
}

void connect_client() {
	int client_fd = accept(sockfd, (struct sockaddr *)&servaddr, &len);

	if (client_fd < 0)
		return ;
	if (client_fd > nfds)
		nfds = client_fd;
	FD_SET(client_fd, &curfds);
	clients[client_fd].id = nclients;
	nclients++;
	bzero(msg, 300000);
	sprintf(msg, "server: client %d just arrived\n", clients[client_fd].id);
	send_to_all(msg, client_fd);
}

int	main(int argc, char *argv[]) {
	validate_args(argc);
	init_socket(argv[1]);

	while(1) {
		rfds = wfds = curfds;
		if (select(nfds + 1, &rfds, &wfds, 0, 0) < 0)
			continue;
		for (int fd = 0; fd <= nfds; fd++) {
			if (FD_ISSET(fd, &rfds)) {
				if (fd == sockfd)
					connect_client();
				else
					receive_msg(fd);
				break ;
			}
		}
	}
}