#define _POSIX_C_SOURCE 199309L

#include "bat.h"
#include "config.h"
#include "event-loop.h"
#include "render.h"
#include "udev.h"

#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

enum bat_events {
	BAT_WAYLAND_EVENT,
	BAT_TIMER_EVENT,
	BAT_UDEV_EVENT,
	BAT_EVENT_COUNT
};

void run_event_loop(struct bat_state *state) {
	struct bat_monitor monitor = create_monitor();

	const struct itimerspec timer_spec = {
		.it_value = { .tv_sec = state->config.interval, .tv_nsec = 0 }
	};

	int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
	struct pollfd events[] = {
		[BAT_WAYLAND_EVENT] = (struct pollfd) {
			.fd = wl_display_get_fd(state->display),
			.events = POLLIN
		},
		[BAT_TIMER_EVENT] = (struct pollfd) {
			.fd = timer_fd,
			.events = POLLIN
		},
		[BAT_UDEV_EVENT] = (struct pollfd) {
			.fd = monitor.fd,
			.events = POLLIN
		}
	};
	timerfd_settime(timer_fd, 0, &timer_spec, NULL);

	render_frame(state);
	int polled = 0;
	state->running = true;
	while (state->running) {
		while (wl_display_prepare_read(state->display) != 0) {
			wl_display_dispatch_pending(state->display);
		}
		wl_display_flush(state->display);

		polled =  poll(events, BAT_EVENT_COUNT, -1);
		if (polled < 0) {
			wl_display_cancel_read(state->display);
			break;
		}

		// read wayland events
		if (events[BAT_WAYLAND_EVENT].revents & POLLIN) {
			if (wl_display_read_events(state->display) != 0) {
				fprintf(stderr, "Failed to process wayland event\n");
				break;
			}
		} else {
			wl_display_cancel_read(state->display);
		}

		// read timer events
		if (events[BAT_TIMER_EVENT].revents & POLLIN) {
			uint64_t expirations;
			ssize_t n = read(timer_fd, &expirations, sizeof(expirations));
			if (n < 0) {
				fprintf(stderr, "Failed to process timer event\n");
				break;
			}
			render_frame(state);
			timerfd_settime(timer_fd, 0, &timer_spec, NULL);
		}

		// read udev events
		if (events[BAT_UDEV_EVENT].revents & POLLIN) {
			process_event(monitor);
			render_frame(state);
		}
	}

	finish_monitor(monitor);
}
