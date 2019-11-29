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
void server_bind(Server *, int, void (*)(Server *, unsigned int, const char *, unsigned int));
void server_send(Server *, unsigned int, const char *, unsigned int);
void server_broadcast(Server *, const char *, unsigned int);
void server_abort(Server *, unsigned int);
void server_close(Server *);

/* Client */
Conn * client_dial(const char *, unsigned int);
void client_poll(Conn *, int);
void client_send(Conn *, const char *, unsigned int);
void client_bind(Conn *, int, void (*)(Conn *, const char *, unsigned int));
void client_close(Conn *);
int client_alive(Conn *);
