#ifndef _GLOBAL_H_
#define _GLOBAL_H_

extern const char* PROGRAM_NAME;
extern const char* PROGRAM_PID;

struct s_fans {
	FILE* file_output;
	FILE* file_label;

	char* path;  // TODO: unused
	char* label;
	char* path_fan_output;
	char* path_fan_manual;

	struct s_fans *next;
};

struct s_sensors {
	FILE* file_input;
	FILE* file_label;

	char* path;
	char* label;
	char* path_sensor_input;
	char* path_sensor_label;

	unsigned int temperature;

	struct s_sensors *next;
};

typedef struct s_fans    t_fans;
typedef struct s_sensors t_sensors;

extern t_fans*    fans;
extern t_sensors* sensors;

#endif
