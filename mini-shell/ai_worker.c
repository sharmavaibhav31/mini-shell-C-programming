#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_PROMPT_LEN 4096
#define MAX_RESPONSE_LEN 16384

// Simple echo worker (fallback when model unavailable)
int echo_worker() {
    char prompt[MAX_PROMPT_LEN];
    
    while (fgets(prompt, MAX_PROMPT_LEN, stdin)) {
        prompt[strcspn(prompt, "\n")] = '\0';
        
        if (strlen(prompt) == 0) continue;
        
        printf("Echo: %s\n", prompt);
        fflush(stdout);
    }
    
    return 0;
}

int main(int argc, char **argv) {
    bool echo_mode = false;
    const char *model_path = NULL;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--echo") == 0) {
            echo_mode = true;
        } else {
            model_path = argv[i];
        }
    }
    
    // If echo mode or no model, use echo worker
    if (echo_mode || model_path == NULL) {
        return echo_worker();
    }
    
    // Check if model exists
    if (access(model_path, R_OK) != 0) {
        fprintf(stderr, "Model file not accessible, falling back to echo mode\n");
        return echo_worker();
    }
    
    // TODO: Real inference will be added later
    fprintf(stderr, "Model loading not yet implemented, using echo mode\n");
    return echo_worker();
}
