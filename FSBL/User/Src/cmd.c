#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"
#include "motor.h"
#include "xnucleoihm02a1.h"

#define MAX_ARGS 5 /**< Max args of a command parsed */

typedef int (*cmd_func_t)(int argc, char *argv[]);

struct cmd {
	cmd_func_t f;
	const char *name;
	const char *desc;
	const char *usage;
};

/* Forward Declarations */
static int cmd_help(int argc, char *argv[]);
static int cmd_goto(int argc, char *argv[]);
static int cmd_mgoto(int argc, char *argv[]);
static int cmd_msw(int argc, char *argv[]);
static int cmd_mhome(int argc, char *argv[]);

/**
 * @brief Command registry.
 *
 * To add a command: declare cmd_name() above, add its entry here, then
 * define it below.
 */
static const struct cmd registry[] = {
	/* f, name, desc, usage */
	{ cmd_help, "help", "Show all the commands", "help" },
	{ cmd_goto, "goto", "goto a specific position", "goto <x> <y> <z>" },
	{ cmd_mgoto, "mgoto", "Send a manual command to the motors",
	  "mgoto <angle0> <angle1> <angle2>" },
	{ cmd_msw, "msw", "Wait for a software change on one of the motors.",
	  "mswwait <motor number>" },
	{ cmd_mhome, "mhome", "Trigger the motor homing.", "mhome" },

};

#define REGISTRY_SIZE (sizeof(registry) / sizeof(registry[0]))

static int cmd_help([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	for (size_t i = 0; i < REGISTRY_SIZE; i++) {
		const struct cmd *current = &registry[i];
		printf("- %s: %s \r\n  Usage: %s\r\n", current->name,
		       current->desc, current->usage);
	}

	return 0;
}

static int cmd_goto(int argc, char *argv[])
{
	if (argc < 4) {
		return -EINVAL;
	}

	char *endptr;
	float x = strtof(argv[1], &endptr);
	if (*endptr != '\0') {
		return -EINVAL;
	}
	float y = strtof(argv[2], &endptr);
	if (*endptr != '\0') {
		return -EINVAL;
	}
	float z = strtof(argv[3], &endptr);
	if (*endptr != '\0') {
		return -EINVAL;
	}
	printf("Not implemented but called with: %d, %d, %d \r\n", (int)x,
	       (int)y, (int)z);
	//TODO: Implement this function

	return 0;
}

static int cmd_mgoto(int argc, char *argv[])
{
	if (argc < MOTOR_NUM) {
		return -EINVAL;
	}

	char *endptr;
	float angles[MOTOR_NUM];
	for (int i = 0; i < MOTOR_NUM; i++) {
		angles[i] = strtof(argv[i + 1], &endptr);
		if (*endptr != '\0') {
			return -EINVAL;
		}
	}

	int res = motor_goto_angles(angles);
	if (res < 0) {
		printf("Motor goto failed with error code %d, status 0x%04x 0x%04x\r\n",
		       res, BSP_L6470_GetStatus(0, 0),
		       BSP_L6470_GetStatus(0, 1));
		return res;
	}

	return 0;
}

static int cmd_msw(int argc, char *argv[])
{
	if (argc < 2) {
		return -EINVAL;
	}
	char *endptr;
	enum motor_t m = strtol(argv[1], &endptr, 10);
	if (*endptr != '\0') {
		return -EINVAL;
	}

	bool current = motor_sw_closed(m);
	printf("Initial software status: %d \r\n", current);
	printf("Waiting for a sw change \r\n");
	for (;;) {
		bool new = motor_sw_closed(m);
		if (current != new) {
			printf("New software status: %d \r\n", new);
			break;
		}
	}

	return 0;
}

static int cmd_mhome([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
	if (argc < 1) {
		return -EINVAL;
	}

	int res = motor_homing();
	if (res < 0) {
		printf("Motor homing failed with error code %d, status 0x%04x 0x%04x\r\n",
		       res, BSP_L6470_GetStatus(0, 0),
		       BSP_L6470_GetStatus(0, 1));
	}

	return res;
}

static int parse_line(char *line, char *argv[], int max_args)
{
	int argc = 0;
	char *token = strtok(line, " ");
	while (token && argc < max_args - 1) {
		argv[argc] = token;
		argc++;
		token = strtok(NULL, " ");
	}
	return argc;
}

int cmd_execute(char *line)
{
	int res = 0;
	char *argv[MAX_ARGS];
	int argc = parse_line(line, argv, MAX_ARGS);

	if (argc == 0) {
		return -EINVAL;
	}
	char *name = argv[0];

	for (size_t i = 0; i < REGISTRY_SIZE; i++) {
		const struct cmd *current = &registry[i];
		if (strcmp(name, current->name) == 0) {
			res = current->f(argc, argv);
			if (res == -EINVAL) {
				printf("Error Invalid Args use it like this: %s\r\n",
				       current->usage);
				return 0;
			}
			return res;
		}
	}

	return -EINVAL;
}
