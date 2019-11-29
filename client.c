/*
 * Copyright (c) 2019  Maksymilian Mruszczak <u at one u x dot o r g>
 *
 * Simplified sockets client-side API implementation
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>

enum {
	ON_MESSG,
	ON_CLOSE,
	ON_STDIN
};

typedef struct Conn Conn;
struct Conn {
	struct sockaddr_in addr;
	struct pollfd pfd[2];
	void (*on_messg)(Conn *, const char *, unsigned int);
	void (*on_close)(Conn *, const char *, unsigned int);
	void (*on_stdin)(Conn *, const char *, unsigned int);
};

Conn *
client_dial(const char *saddr, unsigned int port)
{
	static socklen_t slen = (socklen_t)sizeof(struct sockaddr_in);

	Conn *conn = (Conn *)calloc(1, sizeof(Conn));
	if (!conn)
		return conn;
	conn->on_messg = NULL;
	conn->on_close = NULL;
	conn->on_stdin = NULL;

	conn->pfd[0].fd = -1;
	conn->pfd[0].events = POLLIN;
	conn->pfd[1].fd = socket(AF_INET, SOCK_STREAM, 0);
	conn->pfd[1].events = POLLIN;
	if (conn->pfd[1].fd < 0) {
		perror("Failed to create socket!");
		free(conn);
		return NULL;
	}

	conn->addr.sin_family = AF_INET;
	conn->addr.sin_addr.s_addr = inet_addr(saddr);
	conn->addr.sin_port = htons(port);

	if (connect(conn->pfd[1].fd, (struct sockaddr *)&conn->addr, slen) < 0) {
		perror("Failed to connex!");
		free(conn);
		return NULL;
	}
	fcntl(conn->pfd[1].fd, F_SETFL, O_NONBLOCK);

	return conn;
}

void
client_poll(Conn *conn, int timeout)
{
	poll(conn->pfd, 2, timeout);
	if ((conn->pfd[0].revents & POLLIN) && conn->on_stdin != NULL) {
		static char buf[1024]; // tmp
		bzero((void *)buf, 1024);
		read(conn->pfd[0].fd, buf, 1024);
		(*conn->on_stdin)(conn, buf, 1024);
		// TODO handle stdin
	}
	if ((conn->pfd[1].revents & POLLIN) && conn->on_messg != NULL) {
		static char buf[1024]; // tmp
		bzero((void *)buf, 1024);
		if (recv(conn->pfd[1].fd, buf, 1024, 0))
			(*conn->on_messg)(conn, buf, 1024);
		else {
			perror("Lost connexion with server");
			conn->pfd[1].fd = -1;
			if (conn->on_close != NULL)
				(*conn->on_close)(conn, NULL, 0);
		}
	}
}

void
client_send(Conn *conn, const char *msg, unsigned int len)
{
	// TODO maybe some checks
	if (len > 1024) {
		perror("Message too long");
		return;
	}
	send(conn->pfd[1].fd, msg, len, 0);
}

void
client_bind(Conn *conn, int event, void (*fn)(Conn *, const char *, unsigned int))
{
	switch (event) {
	case ON_MESSG:
		conn->on_messg = fn;
		break;
	case ON_CLOSE:
		conn->on_close = fn;
		break;
	case ON_STDIN:
		conn->on_stdin = fn;
		if (fn == NULL)
			conn->pfd[0].fd = -1;
		else
			conn->pfd[0].fd = fileno(stdin);
		break;
	}
}

void
client_close(Conn *conn)
{
	close(conn->pfd[1].fd);
	bzero((void *)conn, sizeof(Conn));
	free(conn);
}

int
client_alive(Conn *conn)
{
	if (!conn)
		return 0;
	if (conn->pfd[1].fd < 0)
		return 0;
	return 1;
}
