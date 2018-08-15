#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "wlr-layer-shell-unstable-v1-client-protocol.h"

#include <wayland-client.h>

struct bat_output {
	struct bat_state *state;

	struct wl_output *wl_output;
	struct wl_list link; // bat_state::outputs

	struct wl_surface *surface;
	struct zwlr_layer_surface_v1 *layer_surface;
	struct bat_buffer *busy_buffer;
	struct bat_buffer *free_buffer;

	int32_t scale;

	int length;
};

void create_output(struct bat_state *state, struct wl_output *wl_output);
void destroy_output(struct bat_output *output);

#endif
