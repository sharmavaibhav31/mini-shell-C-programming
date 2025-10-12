#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>
#include <sys/types.h>

// Original shell functions
void shell_loop();
char *read_line();
char **split_line(char *line);
int execute(char **args);

// AI assistant functions
#define AI_PROMPT_MAX 4096
#define AI_RESPONSE_MAX 16384
#define AI_TIMEOUT_SEC 30
#define AI_WORKER_PATH "./ai_worker"
#define AI_MODEL_PATH_ENV "HOME"

typedef struct {
    pid_t worker_pid;
    int pipe_to_worker[2];
    int pipe_from_worker[2];
    bool is_initialized;
} ai_context_t;

int init_ai_context(ai_context_t *ctx);
void cleanup_ai_context(ai_context_t *ctx);
int handle_assist_command(ai_context_t *ctx, const char *prompt);

#endif
