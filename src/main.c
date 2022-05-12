#include "ping.h"

#include <string.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	(void)argc;
	bzero(&g_ping, sizeof(g_ping));

	check_args(argv);
	init_ping();

	signal(SIGINT, sigint_handler);
	signal(SIGALRM, sigalrm_handler);

	sigalrm_handler();

	while (!g_ping.finished)
		reply_handler();

	print_stats();

	return 0;
}
