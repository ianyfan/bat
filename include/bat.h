#ifndef _BAT_H
#define _BAT_H

#include "config.h"

#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

#include <stdbool.h>
#include <wayland-client.h>

struct bat_state {
	struct bat_config config;

	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shm *shm;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct zxdg_output_manager_v1 *output_manager;
	struct wl_list outputs; // bat_output::link

	bool running;
};

#endif
