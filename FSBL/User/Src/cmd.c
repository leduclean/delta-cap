#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd.h"

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
