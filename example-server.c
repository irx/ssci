#include <stdio.h>
#include <stdlib.h>

#include "sscilib.h"

static int alive;

/* Print any msg received from clients */
static void
on_messg(Server *ctx, unsigned int clino, const char *msg, const unsigned int len)
{
	printf("Client %d: %s\n", clino, msg);
	server_broadcast(ctx, msg, len);
}

/* Quit server on entering `q' command or broadcast msg */
static void
on_stdin(Server *ctx, const char *msg, const unsigned int len)
{
	if (msg[0] == 'q' && msg[1] == '\n')
		alive = 0;
	else
		server_broadcast(ctx, msg, len);
}

int
main(void)
{
	/* Create and open server on port 1337 with 10 slots */
	Server *srv = server_open(10, 1337);
	if (srv == NULL) {
		fprintf(stderr, "Couldn't open socket!\n");
		return 1;
	}
	/* Bind callback functions to events */
	server_bind(srv, ON_MESSG, &on_messg);
	server_bind(srv, ON_STDIN, &on_stdin);

	/* Listen */
	alive = 1;
	while (alive)
		server_poll(srv, -1);

	return 0;
}
