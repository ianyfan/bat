#include "udev.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct bat_monitor create_monitor(void) {
	struct bat_monitor monitor = { .fd = -1 };

	struct udev *udev = udev_new();
	if (udev == NULL) {
		fprintf(stderr, "Failed to create udev object\n");
		return monitor;
	}

	if ((monitor.udev_monitor = udev_monitor_new_from_netlink(udev, "udev")) == NULL) {
		fprintf(stderr, "Failed to create battery monitor\n");
	} else if (udev_monitor_filter_add_match_subsystem_devtype(monitor.udev_monitor, "power_supply", NULL) < 0) {
		fprintf(stderr, "Failed to filter battery monitor\n");
	} else if (udev_monitor_enable_receiving(monitor.udev_monitor) < 0) {
		fprintf(stderr, "Failed to start battery monitor\n");
	} else {
		// success
		monitor.fd = udev_monitor_get_fd(monitor.udev_monitor);
		udev_unref(udev);
		return monitor;
	}

	// clean-up on failure
	finish_monitor(monitor);
	udev_unref(udev);

	return monitor;
}

void finish_monitor(struct bat_monitor monitor) {
	udev_monitor_unref(monitor.udev_monitor);
}

void process_event(struct bat_monitor monitor) {
	// just discard for now
	udev_device_unref(udev_monitor_receive_device(monitor.udev_monitor));
}
