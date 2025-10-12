#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
#include "shell.h"

#define MAX_LINE 1024
#define MAX_ARGS 64
#define DELIM " \t\r\n\a"

// Global AI context
static ai_context_t g_ai_ctx = {0};

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
    
    // Cleanup on exit
    cleanup_ai_context(&g_ai_ctx);
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

    // Check for assist command
    if (strcmp(args[0], "assist") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "Usage: assist <query>\n");
            return 1;
        }
        
        // Concatenate all arguments after "assist"
        char prompt[AI_PROMPT_MAX] = {0};
        int offset = 0;
        for (int i = 1; args[i] != NULL && offset < AI_PROMPT_MAX - 2; i++) {
            int len = snprintf(prompt + offset, AI_PROMPT_MAX - offset, "%s ", args[i]);
            if (len > 0) offset += len;
        }
        
        // Initialize AI context if needed
        if (!g_ai_ctx.is_initialized) {
            if (init_ai_context(&g_ai_ctx) != 0) {
                fprintf(stderr, "AI assistant unavailable\n");
                return 1;
            }
        }
        
        return handle_assist_command(&g_ai_ctx, prompt) == 0 ? 1 : 1;
    }
    
    // Check for assist-reset command
    if (strcmp(args[0], "assist-reset") == 0) {
        cleanup_ai_context(&g_ai_ctx);
        printf("AI assistant reset\n");
        return 1;
    }

    if (strcmp(args[0], "exit") == 0) return 0;
    if (strcmp(args[0], "clear") == 0) { system("clear"); return 1; }
    if (strcmp(args[0], "help") == 0) {
        printf("Mini Shell - Built-in Commands:\n");
        printf("  help          - Show this message\n");
        printf("  clear         - Clear the screen\n");
        printf("  assist <query> - Ask AI assistant\n");
        printf("  assist-reset  - Restart AI worker\n");
        printf("  exit          - Exit the shell\n");
        return 1;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        if (execvp(args[0], args) == -1)
            perror("mini-shell");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("mini-shell");
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
    return 1;
}

// AI context management
int init_ai_context(ai_context_t *ctx) {
    // Build model path
    char model_path[512];
    const char *home = getenv("HOME");
    if (!home) home = "/tmp";
    snprintf(model_path, sizeof(model_path), "%s/.assist_ai/model.gguf", home);
    
    // Create pipes
    if (pipe(ctx->pipe_to_worker) == -1) {
        perror("pipe to worker");
        return -1;
    }
    
    if (pipe(ctx->pipe_from_worker) == -1) {
        perror("pipe from worker");
        close(ctx->pipe_to_worker[0]);
        close(ctx->pipe_to_worker[1]);
        return -1;
    }
    
    // Fork AI worker
    ctx->worker_pid = fork();
    
    if (ctx->worker_pid == -1) {
        perror("fork");
        close(ctx->pipe_to_worker[0]);
        close(ctx->pipe_to_worker[1]);
        close(ctx->pipe_from_worker[0]);
        close(ctx->pipe_from_worker[1]);
        return -1;
    }
    
    if (ctx->worker_pid == 0) {
        // Child process: AI worker
        
        // Redirect stdin/stdout to pipes
        dup2(ctx->pipe_to_worker[0], STDIN_FILENO);
        dup2(ctx->pipe_from_worker[1], STDOUT_FILENO);
        
        // Close unused pipe ends
        close(ctx->pipe_to_worker[0]);
        close(ctx->pipe_to_worker[1]);
        close(ctx->pipe_from_worker[0]);
        close(ctx->pipe_from_worker[1]);
        
        // Execute AI worker
        execl(AI_WORKER_PATH, "ai_worker", model_path, NULL);
        
        // If exec fails, try echo mode
        execl(AI_WORKER_PATH, "ai_worker", "--echo", NULL);
        
        // If still fails, exit
        perror("execl ai_worker");
        exit(EXIT_FAILURE);
    }
    
    // Parent: close unused pipe ends
    close(ctx->pipe_to_worker[0]);
    close(ctx->pipe_from_worker[1]);
    
    ctx->is_initialized = true;
    printf("AI assistant initialized (PID: %d)\n", ctx->worker_pid);
    
    return 0;
}

void cleanup_ai_context(ai_context_t *ctx) {
    if (!ctx->is_initialized) return;
    
    // Close pipes
    close(ctx->pipe_to_worker[1]);
    close(ctx->pipe_from_worker[0]);
    
    // Kill worker
    if (ctx->worker_pid > 0) {
        kill(ctx->worker_pid, SIGTERM);
        waitpid(ctx->worker_pid, NULL, 0);
    }
    
    ctx->is_initialized = false;
    ctx->worker_pid = 0;
}

int handle_assist_command(ai_context_t *ctx, const char *prompt) {
    char safe_prompt[AI_PROMPT_MAX];
    char response[AI_RESPONSE_MAX];
    
    // Sanitize prompt
    size_t prompt_len = strnlen(prompt, AI_PROMPT_MAX - 2);
    strncpy(safe_prompt, prompt, prompt_len);
    safe_prompt[prompt_len] = '\n';
    safe_prompt[prompt_len + 1] = '\0';
    
    // Send prompt to worker
    ssize_t written = write(ctx->pipe_to_worker[1], safe_prompt, prompt_len + 1);
    if (written == -1) {
        perror("write to AI worker");
        cleanup_ai_context(ctx);
        return -1;
    }
    
    // Wait for response with timeout
    fd_set readfds;
    struct timeval timeout = { .tv_sec = AI_TIMEOUT_SEC, .tv_usec = 0 };
    
    FD_ZERO(&readfds);
    FD_SET(ctx->pipe_from_worker[0], &readfds);
    
    int ready = select(ctx->pipe_from_worker[0] + 1, &readfds, NULL, NULL, &timeout);
    
    if (ready == -1) {
        perror("select");
        cleanup_ai_context(ctx);
        return -1;
    } else if (ready == 0) {
        fprintf(stderr, "AI worker timeout\n");
        cleanup_ai_context(ctx);
        return -1;
    }
    
    // Read response
    ssize_t bytes_read = read(ctx->pipe_from_worker[0], response, AI_RESPONSE_MAX - 1);
    
    if (bytes_read <= 0) {
        fprintf(stderr, "AI worker connection lost\n");
        cleanup_ai_context(ctx);
        return -1;
    }
    
    response[bytes_read] = '\0';
    
    // Display response
    printf("AI: %s", response);
    if (response[bytes_read - 1] != '\n') {
        printf("\n");
    }
    
    return 0;
}
