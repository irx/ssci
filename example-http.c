/* Copyright (c) 2020-2022  Maksymilian Mruszczak <u at one u x dot o r g>
 * Simple http server example using libssci
 *
 * Not an actual http server; it ignores contents of requests and responds
 * always with 200 OK
 * Build with `make example-http', start `./example-http' and open
 * http://127.0.0.1:3000 in a web browser
 * Type `q' in terminal to stop the server
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sscilib.h"

static int alive;

static const char *day_of_week[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
static const char *month[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

static const char *tmpl =
"HTTP/1.1 200 OK\r\n"
"Date: %s, %d %s %d %02d:%02d:%02d GMT\r\n"
"Access-Control-Allow-Origin: *\r\n"
"Server: ssci\r\n"
"Last-Modified: Tue, 10 Mar 2020 10:51:48 GMT\r\n"
"Content-Type: text/html; charset=utf-8\r\n"
"Content-Length: 169\r\n"
"Connection: Closed\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head>\r\n"
"<style>body{color:red;font-family:'Comic Sans MS','Chalkboard SE','Comic Neue',sans-serif;}</style></head>\r\n"
"<body>Welcome!</body></html>\r\n";

static void onmess(Server *, unsigned int, const char *, unsigned int);
static void oncmd(Server *, unsigned int, const char *, unsigned int);

static void
onmess(Server *s, unsigned int clino, const char *messg, unsigned int len)
{
	char resp[512], *rem;
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	printf("Got message of length %u:\n%s\n", len, messg);
	/* if length of recv msg is over 255 its probably inclomplete
	 * and we should wait for the rest
	 */
	if (len > 255)
		return;
	snprintf(resp, 512, tmpl, day_of_week[tm.tm_wday-1], tm.tm_mday, month[tm.tm_mon],
	         tm.tm_year+1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
	/* resp is too big to send as one msg */
	rem = &resp[256];
	server_send(s, clino, resp, 256);
	server_send(s, clino, rem, strlen(rem));
	server_abort(s, clino);
}

static void
oncmd(Server *s, unsigned int clino, const char *cmd, unsigned int len)
{
	/* close server on `q' command */
	if (cmd[0] == 'q' && cmd[1] == '\n')
		alive = 0;
}

int
main(void)
{
	Server *s = server_open(2, 3000);
	/*                      |    |
	 * allow 2 simult conn -+    |
	 * use port 3000 ------------+
	 */
	server_bind(s, ON_MESSG, onmess);
	server_bind(s, ON_STDIN, oncmd);
	printf("Listening on http://127.0.0.1:3000\n");
	alive = 1;
	while (alive)
		server_poll(s, -1);
	printf("Exiting...\n");
	server_close(s);
	return 0;
}
