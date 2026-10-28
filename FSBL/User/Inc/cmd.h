#pragma once

/**
 * @brief Parse and run a command line.
 *
 * @param line NUL-terminated line, tokenized in place.
 * @return 0 on success or bad args (usage printed), -EINVAL on empty line
 *         or unknown command, otherwise the command's return value.
 */
int cmd_execute(char *line);
