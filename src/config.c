#include "battery.h"
#include "config.h"
#include "string-list.h"

#include "wlr-layer-shell-unstable-v1-client-protocol.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

void init_default_config(struct bat_config *config) {
	config->use_design_capacity = false;
	config->batteries = detect_batteries();
	config->outputs = NULL;
	config->interval = 10;
	config->thickness = 1;
	config->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
	config->position = CENTER;
	config->segment = 20;
	config->low_threshold = 10;
	config->colors.charging = 0xffffff00;
	config->colors.discharging = 0xffffffff;
	config->colors.full = 0xff00ff00;
	config->colors.low = 0xffff0000;
}

static bool parse_int(const char *string, int *out) {
	errno = 0;
	char *end;
	*out = (int) strtoul(string, &end, 10);
	return errno == 0 && *end == '\0';
}

static bool parse_color(const char *string, uint32_t *out) {
	if (*string++ != '#') return false;

	size_t len = strlen(string);
	if (len != 6 && len != 8) {
		return false;
	}

	errno = 0;
	char *end;
	*out = (uint32_t) strtoul(string, &end, 16);
	if (errno != 0 || *end != '\0') return false;

	if (len == 6) *out = (*out << 8) | 0xFF;
	return true;
}

static bool parse_option(const char c, const char *value, struct bat_config *config) {
	switch (c) {
		case 'b': return is_battery(optarg) && string_list_add(config->batteries, optarg);
		case 'o': return string_list_add(config->outputs, optarg);
		case 'i': return parse_int(optarg, &config->interval);
		case 'w': return parse_int(optarg, &config->thickness);
		case 'a':
			if (strcasecmp(optarg, "bottom") == 0) config->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
			else if (strcasecmp(optarg, "left") == 0) config->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
			else if (strcasecmp(optarg, "right") == 0) config->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
			else if (strcasecmp(optarg, "top") == 0) config->anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
			else return false;
			return true;
		case 'p':
			if (strcasecmp(optarg, "bottom") == 0) config->position = BOTTOM;
			else if (strcasecmp(optarg, "center") == 0) config->position = CENTER;
			else if (strcasecmp(optarg, "left") == 0) config->position = LEFT;
			else if (strcasecmp(optarg, "right") == 0) config->position = RIGHT;
			else if (strcasecmp(optarg, "top") == 0) config->position = TOP;
			else return false;
			return true;
		case 's': return parse_int(optarg, &config->segment);
		case 't': return parse_int(optarg, &config->low_threshold);
		case 'c': return parse_color(optarg, &config->colors.charging);
		case 'd': return parse_color(optarg, &config->colors.discharging);
		case 'f': return parse_color(optarg, &config->colors.full);
		case 'l': return parse_color(optarg, &config->colors.low);
		default: return false;
	}
}

int parse_config(struct bat_config *config, int argc, char **argv) {
	static const struct option long_options[] = {
		{"help", no_argument, NULL, 'h'},
		{"use-design-capacity", no_argument, NULL, 0},
		{"battery", required_argument, NULL, 'b'},
		{"output", required_argument, NULL, 'o'},
		{"interval", required_argument, NULL, 'i'},
		{"width", required_argument, NULL, 'w'},
		{"anchor", required_argument, NULL, 'a'},
		{"position", required_argument, NULL, 'p'},
		{"segment", required_argument, NULL, 's'},
		{"threshold", required_argument, NULL, 't'},
		{"charging", required_argument, NULL, 'c'},
		{"discharging", required_argument, NULL, 'd'},
		{"full", required_argument, NULL, 'f'},
		{"low", required_argument, NULL, 'l'},
		{0}
	};

	int number_of_batteries = 0;
	int number_of_outputs = 0;
	while (true) {
		int c = getopt_long(argc, argv, "hb:o:i:w:a:p:s:t:c:d:f:l:", long_options, NULL);
		if (c == -1) break;
		if (c == ':' || c == '?') return -1;
		if (c == 'h') return 1;

		if (c == 'b') ++number_of_batteries;
		else if (c == 'o') ++number_of_outputs;
	}

	if (number_of_batteries > 0) {
		string_list_destroy(config->batteries);
		config->batteries = string_list_init(number_of_batteries);
	}
	if (number_of_outputs > 0) config->outputs = string_list_init(number_of_outputs);
	optind = 1;
	while (true) {
		int option_index = -1;
		int c = getopt_long(argc, argv, "hb:o:i:w:a:p:s:t:c:d:f:l:", long_options, &option_index);
		if (c == -1) return 0;

		if (c == 0 && strcmp(long_options[option_index].name, "use-design-capacity") == 0) {
			config->use_design_capacity = true;
		} else if (!parse_option(c, optarg, config)) {
			if (option_index != -1) {
				fprintf(stderr, "Invalid argument for option '--%s': '%s'\n", long_options[option_index].name, optarg);
			} else {
				fprintf(stderr, "Invalid argument for option '-%c': '%s'\n", c, optarg);
			}
			return -1;
		}
	}
}

void finish_config(struct bat_config *config) {
	string_list_destroy(config->batteries);
	string_list_destroy(config->outputs);
}
