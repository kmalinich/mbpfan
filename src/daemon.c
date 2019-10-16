/*  Copyright (C) 2012  Peter Lombardo <http://peterlombardo.wikidot.com/linux-daemon-in-c>
 *  Modifications (2012)         by Ismail Khatib <ikhatib@gmail.com>
 *  Modifications (2012-present) by Daniel Graziotin <daniel@ineed.coffee>
 *  Modifications (2017-present) by Robert Musial <rmusial@fastmail.com>
 *  Modifications (2018-present) by Kenneth Malinich <kennygprs@gmail.com>
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include "mbpfan.h"
#include "global.h"
#include "daemon.h"

int write_pid(int pid) {
	FILE *file = NULL;
	file = fopen(PROGRAM_PID, "w");

	if (file != NULL) {
		fprintf(file, "%d", pid);
		fclose(file);
		return 1;

	} else {
		return 0;
	}
}

int read_pid() {
	FILE *file = NULL;
	int pid = -1;
	file = fopen(PROGRAM_PID, "r");

	if (file != NULL) {
		fscanf(file, "%d", &pid);
		fclose(file);

		if (kill(pid, 0) == -1 && errno == ESRCH) { /* a process with such a pid does not exist, remove the pid file */
			if (remove(PROGRAM_PID) ==  0) {
				return -1;
			}
		}
		return pid;
	}

	return -1;
}

int delete_pid() {
	return remove(PROGRAM_PID);
}

static void cleanup_and_exit(int exit_code) {
	delete_pid();
	set_fans_auto(fans);

	struct s_fans *next_fan;
	while (fans != NULL) {
		next_fan = fans->next;

		if (fans->file_output != NULL) {
			fclose(fans->file_output);
		}

		free(fans->path_fan_output);
		free(fans->path_fan_manual);
		free(fans);
		fans = next_fan;
	}

	struct s_sensors *next_sensor;

	while (sensors != NULL) {
		next_sensor = sensors->next;
		if (sensors->file_input != NULL) {
			fclose(sensors->file_input);
		}

		free(sensors->path);
		free(sensors);

		sensors = next_sensor;
	}

	exit(exit_code);
}

void signal_handler(int signal) {
	switch(signal) {
		case SIGHUP:
			printf("Received SIGHUP signal\n");
			retrieve_settings(NULL);
			break;

		case SIGTERM:
			printf("Received SIGTERM signal\n");
			cleanup_and_exit(EXIT_SUCCESS);
			break;

		case SIGQUIT:
			printf("Received SIGQUIT signal\n");
			cleanup_and_exit(EXIT_SUCCESS);
			break;

		case SIGINT:
			printf("Received SIGINT signal\n");
			cleanup_and_exit(EXIT_SUCCESS);
			break;

		default:
			printf("Unhandled signal (%d) %s\n", signal, strsignal(signal));
	}
}

void go_daemon(void (*fan_control)()) {
	// Setup signal handling before we start
	signal(SIGHUP, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGINT, signal_handler);

	printf("%s starting up\n", PROGRAM_NAME);

	int current_pid = getpid();

	if (read_pid() == -1) {
		printf("Writing a new .pid file with value %d at: %s\n", current_pid, PROGRAM_PID);

		if (write_pid(current_pid) == 0) {
			printf("ERROR: Can not create a .pid file at: %s. Aborting\n", PROGRAM_PID);
			exit(EXIT_FAILURE);
		}
		else {
			printf("Successfully written a new .pid file with value %d at: %s\n", current_pid, PROGRAM_PID);
		}
	}
	else {
		printf("ERROR: a previously created .pid file exists at: %s.\n Aborting\n", PROGRAM_PID);
		exit(EXIT_FAILURE);
	}

	fan_control();

	return;
}
