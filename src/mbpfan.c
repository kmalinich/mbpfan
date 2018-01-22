/* mbpfan.c - automatically control fan for MacBook Pro
 *
 * Copyright (C) 2010  Allan McRae <allan@archlinux.org>
 * Modifications by Rafael Vega <rvega@elsoftwarehamuerto.org>
 * Modifications (2012) by Ismail Khatib <ikhatib@gmail.com>
 * Modifications (2012-present) by Daniel Graziotin <daniel@ineed.coffee> [CURRENT MAINTAINER]
 * Modifications (2017-present) by Robert Musial <rmusial@fastmail.com>
 * Modifications (2018-present) by Kenneth Malinich <kennygprs@gmail.com>
 *
 * Notes:
 *   Assumes any number of processors and fans (max. 10)
 *   It uses only the temperatures from the processors as input.
 *   Requires coretemp and applesmc kernel modules to be loaded.
 *   Requires root use
 *
 * Tested models: see README.md
 */


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <sys/utsname.h>
#include <sys/errno.h>
#include "mbpfan.h"
#include "global.h"
#include "settings.h"

/* lazy min/max... */
#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

int min_fan_speed = 0;
int max_fan_speed = 5600;

/* temperature thresholds
 * low_temp - temperature below which fan speed will be at minimum
 * high_temp - fan will increase speed when higher than this temperature
 * max_temp - fan will run at full speed above this temperature */
int low_temp  = 35;  // try ranges 55-63
int high_temp = 45;  // try ranges 58-66
int max_temp  = 55;  // do not set it > 90

int polling_interval = 2;

t_sensors* sensors = NULL;
t_fans* fans = NULL;


static char *smprintf(const char *fmt, ...) __attribute__((format (printf, 1, 2)));
static char *smprintf(const char *fmt, ...) {
	char *buf;
	int cnt;
	va_list ap;

	// find buffer length
	va_start(ap, fmt);
	cnt = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);

	if (cnt < 0) {
		return NULL;
	}

	// create and write to buffer
	buf = malloc(cnt + 1);
	va_start(ap, fmt);
	vsnprintf(buf, cnt + 1, fmt, ap);
	va_end(ap);
	return buf;
}

t_sensors *retrieve_sensors() {
	t_sensors *sensors_head = NULL;
	t_sensors *s = NULL;

	char *path_input = NULL;
	char *path_label = NULL;

	const char *path_begin     = "/sys/devices/platform/applesmc.768/temp";
	const char *path_end_input = "_input";
	const char *path_end_label = "_label";

	printf("Looking for temperature sensors under %s\n", path_begin);

	int counter       = 3;
	int sensors_found = 0;

	for (counter = 3; counter < 4; counter++) {
		printf("Checking temperature sensor temp%d\n", counter);

		path_input = smprintf("%s%d%s", path_begin, counter, path_end_input);
		path_label = smprintf("%s%d%s", path_begin, counter, path_end_label);

		FILE *file_input = fopen(path_input, "r");
		FILE *file_label = fopen(path_label, "r");

		if (file_input != NULL) {
			s = (t_sensors *) malloc(sizeof(t_sensors));

			s->path_sensor_input = strdup(path_input);
			s->path_sensor_label = strdup(path_label);

			fscanf(file_input, "%d", &s->temperature);
			fscanf(file_label, "%s", &s->label);

			if (sensors_head == NULL) {
				sensors_head = s;
				sensors_head->next = NULL;
			}
			else {
				t_sensors *tmp = sensors_head;

				while (tmp->next != NULL) {
					tmp = tmp->next;
				}

				tmp->next = s;
				tmp->next->next = NULL;
			}

			s->file_input = file_input;
			s->file_label = file_label;
			sensors_found++;
		}

		free(path_input); path_input = NULL;
		free(path_label); path_label = NULL;
	}

	printf("Found %d temperature sensors\n", sensors_found);

	if (sensors_found == 0) {
		printf("ERROR: mbpfan could not detect any temperature sensors. Please contact the developer.\n");
		exit(EXIT_FAILURE);
	}

	return sensors_head;
}

t_fans *retrieve_fans() {
	t_fans *fans_head = NULL;
	t_fans *fan = NULL;

	char *path_output = NULL;
	char *path_manual = NULL;

	const char *path_begin      = "/sys/devices/platform/applesmc.768/fan";
	const char *path_end_output = "_output";
	const char *path_end_manual = "_manual";

	printf("Looking for fans under %s\n", path_begin);

	int counter    = 1;
	int fans_found = 0;

	for (counter = 1; counter < 7; counter++) {
		printf("Checking fan fan%d\n", counter);

		path_output = smprintf("%s%d%s", path_begin, counter, path_end_output);
		path_manual = smprintf("%s%d%s", path_begin, counter, path_end_manual);

		FILE *file_output = fopen(path_output, "w");

		if (file_output != NULL) {
			fan = (t_fans *) malloc(sizeof(t_fans));
			fan->path_fan_output = strdup(path_output);
			fan->path_fan_manual = strdup(path_manual);

			if (fans_head == NULL) {
				fans_head = fan;
				fans_head->next = NULL;
			}
			else {
				t_fans *tmp = fans_head;

				while (tmp->next != NULL) {
					tmp = tmp->next;
				}

				tmp->next = fan;
				tmp->next->next = NULL;
			}

			fan->file_output = file_output;
			fans_found++;
		}

		free(path_output); path_output = NULL;
		free(path_manual); path_manual = NULL;
	}

	printf("Found %d fans\n", fans_found);

	if (fans_found == 0) {
		printf("ERROR: mbpfan could not detect any fans. Please contact the developer.\n");
		exit(EXIT_FAILURE);
	}

	return fans_head;
}


static void set_fans_mode(t_fans *fans, int mode) {
	t_fans *tmp = fans;
	FILE *file;

	printf("Setting fans to manual control\n");

	while (tmp != NULL) {
		file = fopen(tmp->path_fan_manual, "rw+");

		if (file != NULL) {
			fprintf(file, "%d", mode);
			fclose(file);
		}

		tmp = tmp->next;
	}
}

void set_fans_man(t_fans *fans) {
	set_fans_mode(fans, 1);
}

void set_fans_auto(t_fans *fans) {
	set_fans_mode(fans, 0);
}


t_sensors *refresh_sensors(t_sensors *sensors) {
	t_sensors *tmp = sensors;

	while (tmp != NULL) {
		if (tmp->file_input != NULL) {
			char buf[16];
			int len = pread(fileno(tmp->file_input), buf, sizeof(buf), /*offset=*/ 0);
			buf[len] = '\0';
			sscanf(buf, "%d", &tmp->temperature);
		}

		tmp = tmp->next;
	}

	return sensors;
}


/* Controls the speed of the fan */
void set_fan_speed(t_fans* fans, int speed) {
	t_fans *tmp = fans;

	while (tmp != NULL) {
		if (tmp->file_output != NULL) {
			char buf[16];
			int len = snprintf(buf, sizeof(buf), "%d", speed);
			pwrite(fileno(tmp->file_output), buf, len, /*offset=*/ 0);
		}

		tmp = tmp->next;
	}
}

unsigned short get_temp(t_sensors* sensors) {
	sensors = refresh_sensors(sensors);
	int sum_temp = 0;
	unsigned short temp = 0;

	t_sensors* tmp = sensors;

	int number_sensors = 0;

	while (tmp != NULL) {
		sum_temp += tmp->temperature;
		tmp = tmp->next;
		number_sensors++;
	}

	// Just to be safe
	if (number_sensors == 0) {
		number_sensors++;
	}

	temp = (unsigned short)( ceil( (float)( sum_temp ) / (number_sensors * 1000) ) );
	return temp;
}


void retrieve_settings(const char* settings_path) {
	Settings *settings = NULL;
	int result = 0;
	FILE *f = NULL;

	if (settings_path == NULL) {
		f = fopen("/etc/mbpfan.conf", "r");
	}
	else {
		f = fopen(settings_path, "r");
	}

	if (f == NULL) {
		/* Could not open configfile */
		printf("Couldn't open configfile, using defaults\n");
	}
	else {
		settings = settings_open(f);
		fclose(f);

		if (settings == NULL) {
			/* Could not read configfile */
			printf("Couldn't read configfile\n");
		}
		else {
			printf("Read config file at /etc/mbpfan.conf\n");

			/* Read configfile values */
			result = settings_get_int(settings, "general", "min_fan_speed");
			if (result != 0) { min_fan_speed = result; }

			result = settings_get_int(settings, "general", "max_fan_speed");
			if (result != 0) { max_fan_speed = result; }

			result = settings_get_int(settings, "general", "low_temp");
			if (result != 0) { low_temp = result; }

			result = settings_get_int(settings, "general", "high_temp");
			if (result != 0) { high_temp = result; }

			result = settings_get_int(settings, "general", "max_temp");
			if (result != 0) { max_temp = result; }

			result = settings_get_int(settings, "general", "polling_interval");
			if (result != 0) { polling_interval = result; }

			/* Destroy the settings object */
			settings_delete(settings);
		}
	}
}


void mbpfan() {
	int old_temp, new_temp, fan_speed, steps;
	int temp_change;
	int step_up, step_down;

	retrieve_settings(NULL);

	printf("Retrieving sensors\n");
	sensors = retrieve_sensors();

	printf("Retrieving fans\n");
	fans = retrieve_fans();

	set_fans_man(fans);

	new_temp = get_temp(sensors);

	fan_speed = min_fan_speed;
	set_fan_speed(fans, fan_speed);

	printf("Polling interval set to %d seconds\n", polling_interval);

	printf("Sleeping for %d seconds to get first temp delta\n", polling_interval);

	sleep(polling_interval);

	step_up   = (float)(max_fan_speed - min_fan_speed) / (float)( (max_temp - high_temp) * (max_temp - high_temp + 1) / 2 );
	step_down = (float)(max_fan_speed - min_fan_speed) / (float)( (max_temp - low_temp)  * (max_temp - low_temp + 1)  / 2 );

	while (1) {
		old_temp = new_temp;
		new_temp = get_temp(sensors);

		if (new_temp >= max_temp && fan_speed != max_fan_speed) {
			fan_speed = max_fan_speed;
		}

		if (new_temp <= low_temp && fan_speed != min_fan_speed) {
			fan_speed = min_fan_speed;
		}

		temp_change = new_temp - old_temp;

		if (temp_change > 0 && new_temp > high_temp && new_temp < max_temp) {
			steps = ( new_temp - high_temp ) * ( new_temp - high_temp + 1 ) / 2;
			fan_speed = max( fan_speed, ceil(min_fan_speed + steps * step_up) );
		}

		if (temp_change < 0 && new_temp > low_temp && new_temp < max_temp) {
			steps = ( max_temp - new_temp ) * ( max_temp - new_temp + 1 ) / 2;
			fan_speed = min( fan_speed, floor(max_fan_speed - steps * step_down) );
		}

		printf("Old %d: new: %d, change: %d, speed: %d\n", old_temp, new_temp, temp_change, fan_speed);

		set_fan_speed(fans, fan_speed);

		fflush(stdout);

		sleep(polling_interval);
	}
}
