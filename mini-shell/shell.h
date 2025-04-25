#ifndef SHELL_H
#define SHELL_H

void shell_loop();
char *read_line();
char **split_line(char *line);
int execute(char **args);

#endif

