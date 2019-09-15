#include <stdio.h>
#include "sscilib.h"

/* report every msg received from server */
void
on_messg(Conn *cli, const char *msg, const unsigned int len)
{
	printf("Message from server: %s\n", msg);
}

/* Send text from std input */
void
on_stdin(Conn *ctx, const char *msg, const unsigned int len)
{
	client_send(ctx, msg, len);
}

/* Report status on closed connexion */
void
status(Conn *cli)
{
	printf("Check if client is alive: %d\n", client_alive(cli));
}

int
main(void)
{
	/* Dial server at 127.0.0.1:1337 */
	Conn *cli = client_dial("127.0.0.1", 1337);
	if (!cli)
		return 1;

	/* Bind callback functions to events */
	client_bind(cli, ON_MESSG, &on_messg);
	client_bind(cli, ON_STDIN, &on_stdin);
	client_bind(cli, ON_CLOSE, &status);

	status(cli);

	/* Greet the server and wait for response */
	client_send(cli, "Hello", 5);
	client_poll(cli, -1);
	/* Wait 5s for an event and close */
	client_poll(cli, 5000);
	client_close(cli);

	return 0;
}
