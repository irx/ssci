/*
 * Copyright (c) 2019-2022  Maksymilian Mruszczak <u at one u x dot o r g>
 *
 * Simplified sockets server-side API implementation
 */

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>

enum {
	ON_MESSG,
	ON_CLOSE,
	ON_STDIN
};

typedef struct {
	int occupied;
	size_t pfdno;
} Slot;

typedef struct Server Server;
struct Server {
	unsigned int limit;
	Slot *slots;
	struct pollfd *pfd;
	struct sockaddr_in *addr;
	void (*on_messg)(Server *, unsigned int, const char *, unsigned int);
	void (*on_close)(Server *, unsigned int, const char *, unsigned int);
	void (*on_stdin)(Server *, unsigned int, const char *, unsigned int);
};

Server *
server_open(unsigned int slot_lim, unsigned int port)
{
	const int ok = 1;
	static socklen_t slen = (socklen_t)sizeof(struct sockaddr_in);

	Server *serv = (Server *)malloc(sizeof(Server));
	if (!serv)
		return serv;
	serv->on_messg = NULL;
	serv->on_close = NULL;
	serv->on_stdin = NULL;
	slot_lim += 2;
	serv->limit = slot_lim;

	serv->slots = (Slot *)malloc(sizeof(Slot)*slot_lim);
	if (!serv->slots) {
		free(serv);
		return NULL;
	}
	serv->pfd = (struct pollfd *)calloc(1, sizeof(struct pollfd)*slot_lim);
	if (!serv->pfd) {
		free(serv->slots);
		free(serv);
		return NULL;
	}
	serv->addr = (struct sockaddr_in *)calloc(1, sizeof(struct sockaddr_in)*slot_lim);
	if (!serv->addr) {
		free(serv->pfd);
		free(serv->slots);
		free(serv);
		return NULL;
	}

	for (int i=0; i < slot_lim; ++i) {
		serv->pfd[i].fd = -1;
		serv->pfd[i].events = POLLIN;
		serv->slots[i].occupied = 0;
	}

	serv->pfd[0].fd = STDIN_FILENO;
	serv->pfd[1].fd = socket(AF_INET, SOCK_STREAM, 0);
	if (serv->pfd[1].fd < 0) {
		perror("Error creating new conn socket");
		free(serv->addr);
		free(serv->pfd);
		free(serv->slots);
		free(serv);
		return NULL;
	}
	setsockopt(serv->pfd[1].fd, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok));
	serv->addr[1].sin_family = AF_INET;
	serv->addr[1].sin_addr.s_addr = INADDR_ANY;
	serv->addr[1].sin_port = htons(port);
	if (bind(serv->pfd[1].fd, (struct sockaddr *)&(serv->addr[1]), slen) < 0) {
		perror("Failed to open new conn socket");
		free(serv->addr);
		free(serv->pfd);
		free(serv->slots);
		free(serv);
		return NULL;
	}
	fcntl(serv->pfd[1].fd, F_SETFL, O_NONBLOCK);
	listen(serv->pfd[1].fd, 5);

	return serv;
}

void
server_abort(Server *serv, unsigned int clino)
{
	if (clino > serv->limit || !serv->slots[clino].occupied)
		return;
	close(serv->pfd[clino].fd);
	serv->slots[clino].occupied = 0;
	serv->pfd[clino].fd = -1;
	if (serv->on_close != NULL)
		(*serv->on_close)(serv, clino, NULL, 0);
}

void
server_poll(Server *serv, int timeout)
{
	static size_t clino;
	poll(serv->pfd, serv->limit, timeout);
	if ((serv->pfd[0].revents & POLLIN) && serv->on_stdin != NULL) {
		static char buf[1024]; // TODO read whole buf? pass fd instead of reading?
		memset((void *)buf, 0, sizeof(char)*1024);
		read(serv->pfd[0].fd, buf, 1024);
		(*serv->on_stdin)(serv, STDIN_FILENO, buf, 1024);
		// TODO handle stdin
	}
	if (serv->pfd[1].revents & POLLIN) {
		/* Incoming new connexion */
		static socklen_t slen = (socklen_t)sizeof(serv->addr[0]);
		for (clino=2; (clino < serv->limit && serv->slots[clino].occupied); ++clino);
		if (clino >= serv->limit) {
			perror("Reached slot limit");
			// TODO tell client to GTFO
		} else {
			serv->pfd[clino].fd = accept(serv->pfd[1].fd, (struct sockaddr *)&(serv->addr[clino]), &slen);
			if (serv->pfd[clino].fd < 0)
				perror("Failed to accept new client connexion");
			else {
				serv->slots[clino].occupied = 1;
				fcntl(serv->pfd[clino].fd, F_SETFL, O_NONBLOCK);
			}
		}
	}
	for (clino=2; clino < serv->limit; ++clino) {
		if (serv->pfd[clino].revents & POLLIN) {
			char buf[256]; // TODO properly handle longer messages
			memset((void *)buf, 0, sizeof(char)*256);
			int r = recv(serv->pfd[clino].fd, buf, 256, 0);
			if (r == 0) {
				close(serv->pfd[clino].fd);
				serv->slots[clino].occupied = 0;
				serv->pfd[clino].fd = -1;
				if (serv->on_close != NULL)
					(*serv->on_close)(serv, clino, NULL, 0);
			} else if (r < 0) {
				perror("Unexpected response");
				/* What do I do? */
			} else {
				/* Handle message */
				if (r > 256) r = 256;
				if (serv->on_messg != NULL)
					(*serv->on_messg)(serv, clino, buf, r);
			}
		}
	}
}

void
server_send(Server *serv, unsigned int clino, const char *msg, unsigned int len)
{
	if (clino > serv->limit || !serv->slots[clino].occupied) {
		perror("No client on given socket");
		return;
	}
	if (len > 1024) {
		perror("Message too long");
		return;
	}
	send(serv->pfd[clino].fd, msg, len, MSG_NOSIGNAL);
}

void server_broadcast(Server *serv, const char *msg, unsigned int len)
{
	int clino;
	for (clino=2; clino < serv->limit; ++clino) {
		if (serv->slots[clino].occupied)
			server_send(serv, clino, msg, len);
	}
}

void
server_bind(Server *serv, int event, void (*fn)(Server *, unsigned int, const char *, unsigned int))
{
	switch (event) {
	case ON_MESSG:
		serv->on_messg = fn;
		break;
	case ON_CLOSE:
		serv->on_close = fn;
		break;
	case ON_STDIN:
		serv->on_stdin = fn;
		break;
	}
}

void
server_close(Server *serv)
{
	for (int i=1; i < serv->limit; ++i)
		if (serv->pfd[i].fd >= 0)
			close(serv->pfd[i].fd);
	memset((void *)serv->addr, 0, sizeof(serv->addr));
	memset((void *)serv->slots, 0, sizeof(serv->slots));
	free(serv->addr);
	free(serv->pfd);
	free(serv->slots);
	free(serv);
}
