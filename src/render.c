#include "battery.h"
#include "buffer.h"
#include "config.h"
#include "render.h"
#include "output.h"

#include "wlr-layer-shell-unstable-v1-client-protocol.h"

#include <stdlib.h>
#include <stdio.h>
#include <wayland-client.h>

static void draw_frame(struct bat_output *output, struct bat_info *info) {
	if (output->free_buffer == NULL) {
		output->free_buffer = create_buffer(output);
		if (output->free_buffer == NULL) return;
	}
	if (output->free_buffer->busy) return;

	// render top-down, then rotate to correct orientation
	// if centred, render half-size and mirror

	struct bat_config config = output->state->config;

	double percentage = config.use_design_capacity ? info->percentage_of_design : info->percentage;
	bool probably_full = info->status == FULL || (info->status == UNKNOWN && percentage > 0.9);
	uint32_t color = probably_full ? config.colors.full :
		info->status == CHARGING ? config.colors.charging :
		percentage*100 < config.low_threshold ? config.colors.low : config.colors.discharging;

	int length = output->length/(config.position == CENTER ? 2 : 1);
	int size = length*config.thickness;
	int bar_size = (int)(percentage*length)*config.thickness;

	uint32_t *pixels = output->free_buffer->data;
	if (config.position == CENTER) pixels += size;

	for (int p = 0; p < bar_size; ++p) pixels[p] = color;
	for (int p = bar_size; p < size; ++p) pixels[p] = 0; // transparent

	if (config.segment > 0) {
		int segment_size = config.segment*size/100;
		for (int p = segment_size; p < size; p += segment_size) {
			for (int i = 0; i < 2*config.thickness; ++i) pixels[p + i] = 0; // separate segments by 2 pixels
		}
	}

	if (config.position == CENTER) { // mirror bar
		for (int p = 0; p < size; ++p) *(pixels - 1 - p) = pixels[p];
	}

	uint32_t anchor = config.anchor;
	if (anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM || anchor == ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP) {
		wl_surface_set_buffer_transform(output->surface,
				config.position == RIGHT ? WL_OUTPUT_TRANSFORM_270 : WL_OUTPUT_TRANSFORM_90);
	} else if (config.position == BOTTOM) {
		wl_surface_set_buffer_transform(output->surface, WL_OUTPUT_TRANSFORM_180);
	}
	wl_surface_attach(output->surface, output->free_buffer->wl_buffer, 0, 0);
	wl_surface_damage_buffer(output->surface, 0, 0, config.thickness, output->length);
	wl_surface_commit(output->surface);

	struct bat_buffer *tmp = output->free_buffer;
	output->free_buffer = output->busy_buffer;
	output->busy_buffer = tmp;
}

void render_frame(struct bat_state *state) {
	struct bat_info info = get_battery_info(state->config.batteries);
	if (info.percentage == 0) return;

	struct bat_output *output;
	wl_list_for_each(output, &state->outputs, link) {
		draw_frame(output, &info);
	}
}
