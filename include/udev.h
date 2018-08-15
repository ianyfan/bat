#ifndef _UDEV_H
#define _UDEV_H

#include <libudev.h>

struct bat_monitor {
	int fd;
	struct udev_monitor *udev_monitor;
};

struct bat_monitor create_monitor(void);
void finish_monitor(struct bat_monitor monitor);

void process_event(struct bat_monitor monitor);

#endif
