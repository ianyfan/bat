#ifndef _CONFIG_H
#define _CONFIG_H

#include "string-list.h"

#include <stdint.h>

enum position {
	CENTER,
	BOTTOM,
	LEFT,
	RIGHT,
	TOP
};

struct bat_config {
	bool use_design_capacity;
	struct string_list *batteries;
	struct string_list *outputs;
	int interval;

	int thickness; // AKA width
	uint32_t anchor; // zwlr_layer_surface_v1_anchor
	enum position position;
	int segment;

	int low_threshold;
	struct {
		uint32_t charging;
		uint32_t discharging;
		uint32_t full;
		uint32_t low;
	} colors;
};

void init_default_config(struct bat_config *config);
int parse_config(struct bat_config *config, int argc, char **argv);
void finish_config(struct bat_config *config);

#endif
