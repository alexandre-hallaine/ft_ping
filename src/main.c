#include "ping.h"

#include <string.h>
#include <signal.h>

t_ping g_ping = {0};

int main(int argc, char *argv[])
{
	(void)argc;

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
