#include <stdio.h>
#include <signal.h>

#include "wmr200.h"
#include "macros.h"
#include "file_logger.h"
#include "rrd_logger.h"


struct wmr200 *wmr;


static void
cleanup(int signum) {
	wmr_close(wmr);
	wmr_end();

	printf("\n\nCaught signal %i, will exit\n", signum);
	exit(0);
}


static void
handler(struct wmr_reading *reading) {
	log_to_file(reading, stdout);
	log_to_rrd(reading, "/home/david/gymlit/tools/meteo/temp.rrd");
}


int
main(int argc, const char *argv[]) {
	struct sigaction sa;
	sa.sa_handler = cleanup;
	sigaction(SIGTERM, &sa, NULL); // TODO
	sigaction(SIGINT, &sa, NULL); // TODO

	wmr_init();

	wmr = wmr_open();
	if (wmr == NULL) {
		fprintf(stderr, "wmr_connect(): no WMR200 handle returned\n");
		return (1);
	}

	wmr_set_handler(wmr, handler);

	wmr_main_loop(wmr);

	wmr_close(wmr);
	wmr_end();

	return (0);
}



