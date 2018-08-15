#ifndef _BUFFER_H
#define _BUFFER_H

#include "output.h"

#include <stdbool.h>
#include <wayland-client.h>

struct bat_buffer {
	struct wl_buffer *wl_buffer;
	void *data;
	bool busy;
};

struct bat_buffer *create_buffer(struct bat_output *output);
void destroy_buffer(struct bat_buffer *buffer);

#endif
