#define _XOPEN_SOURCE 500

#include "bat.h"
#include "buffer.h"
#include "output.h"
#include "string-list.h"

#include "xdg-output-unstable-v1-client-protocol.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void noop() {
	// intentionally left blank
}

static void handle_xdg_output_name(void *data, struct zxdg_output_v1 *xdg_output, const char *name) {
	zxdg_output_v1_destroy(xdg_output);

	// remove unwanted outputs
	struct bat_output *output = data;
	struct string_list *outputs = output->state->config.outputs;
	if (outputs != NULL && !string_list_contains(outputs, name)) {
		destroy_output(output);
	}
}

static void configure_layer_surface(void *data, struct zwlr_layer_surface_v1 *layer_surface,
		uint32_t serial, uint32_t width, uint32_t height) {
	struct bat_output *output = data;

	uint32_t anchor = output->state->config.anchor;
	if (anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM || anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
		output->length = width;
	} else if (anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT || anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
		output->length = height;
	}
	zwlr_layer_surface_v1_ack_configure(layer_surface, serial);

	output->busy_buffer = create_buffer(output);
	output->free_buffer = create_buffer(output);
}

static void close_layer_surface(void *data, struct zwlr_layer_surface_v1 *layer_surface) {
	struct bat_output *output = data;
	destroy_output(output);
}

static void scale_output(void *data, struct wl_output *wl_output, int32_t factor) {
	struct bat_output *output = data;
	output->scale = factor;
}

void create_output(struct bat_state *state, struct wl_output *wl_output) {
	struct bat_output *output = malloc(sizeof(*output));
	if (output == NULL) {
		fprintf(stderr, "Failed to allocate memory for output object\n");
		return;
	}

	output->state = state;
	output->wl_output = wl_output;
	output->scale = 1;

	static struct wl_output_listener output_listener = {
		.done = noop,
		.geometry = noop,
		.mode = noop,
		.scale = scale_output
	};
	wl_output_add_listener(wl_output, &output_listener, output);

	struct zxdg_output_v1 *xdg_output = zxdg_output_manager_v1_get_xdg_output(state->output_manager, output->wl_output);
	static const struct zxdg_output_v1_listener xdg_output_listener = {
		.description = noop,
		.done = noop,
		.logical_position = noop,
		.logical_size = noop,
		.name = handle_xdg_output_name
	};
	zxdg_output_v1_add_listener(xdg_output, &xdg_output_listener, output);

	output->surface = wl_compositor_create_surface(state->compositor);
	output->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
			state->layer_shell,
			output->surface,
			output->wl_output,
			ZWLR_LAYER_SHELL_V1_LAYER_TOP,
			"panel");

	static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
		.configure = configure_layer_surface,
		.closed = close_layer_surface
	};
	zwlr_layer_surface_v1_add_listener(output->layer_surface, &layer_surface_listener, output);

	uint32_t anchor = state->config.anchor;
	int height = 0, width = 0;
	if (anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM || anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
		height = state->config.thickness;
		anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
	} else if (anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT || anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT) {
		width = state->config.thickness;
		anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
	}

	zwlr_layer_surface_v1_set_anchor(output->layer_surface, anchor);
	zwlr_layer_surface_v1_set_exclusive_zone(output->layer_surface, -1);
	zwlr_layer_surface_v1_set_size(output->layer_surface, width, height);

	wl_surface_commit(output->surface);

	wl_list_insert(&state->outputs, &output->link);
}

void destroy_output(struct bat_output *output) {
	wl_list_remove(&output->link);

	destroy_buffer(output->busy_buffer);
	destroy_buffer(output->free_buffer);

	zwlr_layer_surface_v1_destroy(output->layer_surface);
	wl_surface_destroy(output->surface);
	wl_output_destroy(output->wl_output);

	free(output);
}

