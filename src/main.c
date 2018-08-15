#define _POSIX_SOURCE

#include "bat.h"
#include "config.h"
#include "event-loop.h"
#include "wayland.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

static const char *usage =
	"Usage: bat [options...]\n"
	"\n"
	"  -h, --help                  Show help message and exit\n"
	"\n"
	"Options (numbers must be integers):\n"
	"      --use-design-capacity   Use percentage of design capacity instead\n"
	"                              of current capacity\n"
	"  -a, --anchor ANCHOR         Location of bar, can be bottom, left, right\n"
	"                              or top; default is top\n"
	"  -b, --battery BATTERY       Only track BATTERY; can be specified\n"
	"                              multiple times; omit to automatically detect\n"
	"  -i, --interval <n>          Update every n seconds; default is 10; set\n"
	"                              to 0 to disable, relying on udev events only\n"
	"  -o, --output OUTPUT         Only display on OUTPUT; can be specified\n"
	"                              multiple times; omit to show on every output\n"
	"  -p, --position POSITION     Positioning of bar, can be bottom, center,\n"
	"                              left, right or top; default is center\n"
	"  -s, --segment <n>           Divide bar into segments for better visual\n"
	"                              clarity; default is 20; set to 0 to disable\n"
	"  -t, --threshold <n>         Percentage below which is low; default is 10\n"
	"  -w, --width <n>             Thickness of bar in pixels; default is 1\n"
	"\n"
	"Colors, using #RRGGBB[AA] format:\n"
	"  -c, --charging <color>      Color when charging; default is yellow\n"
	"  -d, --discharging <color>   Color when discharging; default is white\n"
	"  -f, --full <color>          Color when full; default is green\n"
	"  -l, --low <color>           Color when below threshold; default is red\n";

static struct bat_state state = {0};

static void handle_signal(int signum) {
	state.running = false;
}

int main(int argc, char **argv) {
	init_default_config(&state.config);
	switch (parse_config(&state.config, argc, argv)) {
		case 1:
			printf(usage);
			return EXIT_SUCCESS;
		case -1: return EXIT_FAILURE;
	} // ignore 0
	if (state.config.batteries == NULL) {
		fprintf(stderr, "Failed to find any batteries\n");
		return EXIT_FAILURE;
	}

	if (!init_wayland(&state)) return EXIT_FAILURE;

	struct sigaction sa = { .sa_handler = handle_signal };
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	run_event_loop(&state);

	finish_wayland(&state);
	finish_config(&state.config);

	return EXIT_SUCCESS;
}
