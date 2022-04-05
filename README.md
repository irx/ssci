Somewhat simple communication interface
=======================================

This is a simple BSD sockets library. It provides a minimal interface for
continuous message (plain char/byte strings) exchange between a single or
multiple client[s] and a server.

The functionality of this library revolves around two structs;
namely `Server` and `Conn` (client) which can be thought of as objects since
they both serve as context to their respective 'methods'.


## Usage

Creating a server and opening a socket on a specific port is both done by:

```c
Server * server_open(unsigned int number_of_slots, unsigned int port);
```

So to open socket and start listening:

```c
Server *s = server_open(32, 8080);

if (!s)
	/* Handle failure */

for (;;)
	server_poll(s, -1);
```

But that would be rather useless unless we handle incoming messages.

Consider following callback function:

```c
void
respond(Server *s, unsigned int clino, char *msg, unsigned int len)
{
	printf("Received message from %d: %s\n", clino, msg);
	server_send(s, clino, "Hello!", 6);
}
```

This will display any received message and send back `Hello!`.

Such function should be bound to the server before `server_poll` invocation.

```c
server_bind(s, ON_MESSG, &respond);
```

The client-side procedure is analogous.
The main difference is the fact that client connects to a single server,
while server itself has to take care of (potentially) multiple clients.


So in order to connect to a previously created server, send 'Hey!', and wait
`500ms` for response before closing consider such example:

```c
static void respond(Conn *, char *, unsigned int);

int
main(void)
{
	Conn *c = client_dial("127.0.0.1", 8080);

	if (!c) {
		fprintf(stderr, "Dial failed\n");
		return 1;
	}

	client_bind(c, ON_MESSG, &respond);

	client_send(c, "Hey!", 4);
	client_poll(c, 500);

	client_close(c);

	return 0;
}

static void
respond(Conn *c, char *msg, unsigned int len)
{
	printf("Received message: %s\n", msg);
}
```

---

See `example-server.c`, `example-client.c` and `example-http.c` for more.
