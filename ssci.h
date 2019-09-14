/*
 * Copyright (c) 2019  MM <u at one u x dot o r g>
 *
 * Simplified sockets API
 */

enum {
	ON_MESSG,
	ON_CLOSE,
	ON_STDIN
};

typedef struct Server Server;
typedef struct Conn Conn;


/* Server */
Server * server_open(unsigned int, const unsigned int);
void server_poll(Server *, int);
void server_bind(Server *, int, const void * (void));
void server_send(const Server *, const unsigned int, const char *, const unsigned int);
void server_broadcast(const Server *, const char *, const unsigned int);
void server_abort(Server *, const unsigned int);
void server_close(Server *);

/* Client */
Conn * client_dial(const char *, const unsigned int);
void client_poll(Conn *, int);
void client_send(Conn *, const char *, const unsigned int);
void client_bind(Conn *, int, const void * (void));
void client_close(Conn *);
int client_alive(Conn *);
