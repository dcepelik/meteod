#include <stdio.h>
#include "wmr200.h"


int
main(int argc, const char *argv[]) {
	wmr_init();

	struct wmr200 *wmr = wmr_open();
	if (!wmr) {
		fprintf(stderr, "wmr_connect(): no WMR200 handle returner\n");
		return (1);
	}

	wmr_main_loop(wmr);

	wmr_close(wmr);
	wmr_end();

	return (0);
}
