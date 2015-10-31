#include "rrd_logger.h"
#include "wmr200.h"
#include "time.h"
#include "strbuf.h"
#include "common.h"

#include <rrd.h>


static void
rrd_temp_entry(struct wmr_reading *reading, strbuf *buf) {
	strbuf_append(
		buf,
		"%li:%.1f",
		time(NULL),
		reading->temp.temp
	);
}


void
log_to_rrd(struct wmr_reading *reading, char *rrd_file) {
	strbuf buf;
	strbuf_init(&buf);

	switch (reading->type) {
	case TEMP_DATA:
		rrd_temp_entry(reading, &buf);
	
		char *update_params[] = {
			"rrdupdate",
			rrd_file,
			buf.str,
			NULL
		};


		fprintf(stderr, "Shape: %s\n", buf.str);
		int ret = rrd_update(3, update_params);

		if (ret != 0) {
			DEBUG_MSG("rrd_update() failed with return code %i\n", ret);
			DEBUG_MSG("rrd: '%s'\n", rrd_get_error());

			rrd_clear_error();
		}
		break;
	}

	strbuf_free(&buf);
}


