#define _XOPEN_SOURCE 500

#include "bat.h"
#include "buffer.h"
#include "output.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>

static int create_pool_file(int size, char path[64]) {
	static const char *template = "bat-XXXXXX";
	const char *dir = getenv("XDG_RUNTIME_DIR");
	if (dir == NULL) {
		fprintf(stderr, "XDG_RUNTIME_DIR is not set\n");
		return -1;
	}

	snprintf(path, 64, "%s/%s", dir, template);

	int fd = mkstemp(path);
	if (fd == -1) {
		fprintf(stderr, "Failed to create pool file\n");
		return -1;
	}

	if (ftruncate(fd, size) == -1) {
		fprintf(stderr, "Failed to resize pool file\n");
		close(fd);
		return -1;
	}

	return fd;
}

static void release_buffer(void *data, struct wl_buffer *wl_buffer) {
	struct bat_buffer *buffer = data;
	buffer->busy = false;
}

struct bat_buffer *create_buffer(struct bat_output *output) {
	struct bat_buffer *buffer = malloc(sizeof(*buffer));
	if (buffer == NULL) {
		fprintf(stderr, "Failed to allocate memory for buffer object\n");
		return NULL;
	}
	buffer->busy = false;

	int thickness = output->state->config.thickness;
	int stride = 4 * thickness;
	int size = stride * output->length;
	char path[64];
	int fd = create_pool_file(size, path);
	if (fd >= 0) {
		buffer->data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (buffer->data != MAP_FAILED) {
			struct wl_shm_pool *pool = wl_shm_create_pool(output->state->shm, fd, size);

			buffer->wl_buffer = wl_shm_pool_create_buffer(pool, 0,
					thickness, output->length, stride, WL_SHM_FORMAT_ARGB8888);
			static const struct wl_buffer_listener buffer_listener = {
				.release = release_buffer
			};
			wl_buffer_add_listener(buffer->wl_buffer, &buffer_listener, buffer);

			wl_shm_pool_destroy(pool);
		} else {
			fprintf(stderr, "Failed to map pool file to memory\n");
			free(buffer);
			buffer = NULL;
		}
	} else {
		free(buffer);
		buffer = NULL;
	}

	close(fd);
	unlink(path);

	return buffer;
}

void destroy_buffer(struct bat_buffer *buffer) {
	wl_buffer_destroy(buffer->wl_buffer);
	free(buffer);
}
