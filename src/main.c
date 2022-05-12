#include "ping.h"

#include <string.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	bzero(&g_ping, sizeof(g_ping));

	check_args(argc, argv);
	init_ping();

	signal(SIGINT, sigint_handler);
	signal(SIGALRM, sigalrm_handler);

	sigalrm_handler();

	while (!g_ping.finished)
		reply_handler();

	print_stats();

	return 0;
}
