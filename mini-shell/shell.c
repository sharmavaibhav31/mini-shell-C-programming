#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "shell.h"

#define MAX_LINE 1024
#define MAX_ARGS 64
#define DELIM " \t\r\n\a"

void shell_loop() {
    char *line;
    char **args;
    int status = 1;

    do {
        printf("mini-shell> ");
        line = read_line();
        args = split_line(line);
        status = execute(args);

        free(line);
        free(args);
    } while (status);
}

char *read_line() {
    char *line = NULL;
    size_t bufsize = 0;
    getline(&line, &bufsize, stdin);
    return line;
}

char **split_line(char *line) {
    int bufsize = MAX_ARGS, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    token = strtok(line, DELIM);
    while (token != NULL) {
        tokens[position++] = token;
        token = strtok(NULL, DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int execute(char **args) {
    if (args[0] == NULL) return 1;

    // Built-in commands
    if (strcmp(args[0], "exit") == 0) return 0;
    if (strcmp(args[0], "clear") == 0) { system("clear"); return 1; }
    if (strcmp(args[0], "help") == 0) {
        printf("Mini AI Shell - Built-in Commands:\n");
        printf("  help - Show this message\n");
        printf("  clear - Clear the screen\n");
        printf("  exit - Exit the shell\n");
        return 1;
    }

    // Run external commands
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1)
            perror("ai-shell");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("ai-shell");
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
    return 1;
}

