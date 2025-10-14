#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_PROMPT_LEN 4096
#define MAX_RESPONSE_LEN 16384

extern void apply_sandbox(void);

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

#ifdef USE_LLAMA

#include "llama.h"

int llama_worker(const char *model_path) {
    apply_sandbox();
    
    llama_backend_init();
    
    struct llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 0;
    
    struct llama_model *model = llama_model_load_from_file(model_path, model_params);
    if (!model) {
        fprintf(stderr, "Failed to load model\n");
        return 1;
    }
    
    const struct llama_vocab *vocab = llama_model_get_vocab(model);
    
    char prompt[MAX_PROMPT_LEN];
    
    while (fgets(prompt, MAX_PROMPT_LEN, stdin)) {
        prompt[strcspn(prompt, "\n")] = '\0';
        
        if (strlen(prompt) == 0) {
            printf("\n");
            fflush(stdout);
            continue;
        }
        
        // Create fresh context for each query
        struct llama_context_params ctx_params = llama_context_default_params();
        ctx_params.n_ctx = 512;
        ctx_params.n_batch = 128;
        ctx_params.n_threads = 2;
        
        struct llama_context *ctx = llama_init_from_model(model, ctx_params);
        if (!ctx) {
            fprintf(stderr, "Failed to create context\n");
            printf("Error: context creation failed\n");
            fflush(stdout);
            continue;
        }
        
        // Create sampler for this query
        struct llama_sampler_chain_params sampler_params = llama_sampler_chain_default_params();
        struct llama_sampler *sampler = llama_sampler_chain_init(sampler_params);
        llama_sampler_chain_add(sampler, llama_sampler_init_greedy());
        
        // Tokenize
        int n_prompt_tokens = llama_tokenize(
            vocab, prompt, strlen(prompt), NULL, 0, true, false
        );
        
        if (n_prompt_tokens < 0) n_prompt_tokens = -n_prompt_tokens;
        
        llama_token *prompt_tokens = malloc(n_prompt_tokens * sizeof(llama_token));
        if (!prompt_tokens) {
            fprintf(stderr, "Failed to allocate tokens\n");
            printf("Error: memory allocation failed\n");
            fflush(stdout);
            llama_sampler_free(sampler);
            llama_free(ctx);
            continue;
        }
        
        llama_tokenize(vocab, prompt, strlen(prompt), prompt_tokens, n_prompt_tokens, true, false);
        
        // Create batch
        struct llama_batch batch = llama_batch_get_one(prompt_tokens, n_prompt_tokens);
        
        // Evaluate
        if (llama_decode(ctx, batch) != 0) {
            fprintf(stderr, "Decode failed\n");
            free(prompt_tokens);
            printf("Error: inference failed\n");
            fflush(stdout);
            llama_sampler_free(sampler);
            llama_free(ctx);
            continue;
        }
        
        // Generate response
        char response[MAX_RESPONSE_LEN] = {0};
        int response_len = 0;
        int n_generated = 0;
        
        while (n_generated < 256) {
            llama_token new_token_id = llama_sampler_sample(sampler, ctx, -1);
            
            if (llama_vocab_is_eog(vocab, new_token_id)) break;
            
            char token_buf[16];
            int token_len = llama_token_to_piece(vocab, new_token_id, token_buf, sizeof(token_buf), 0, false);
            
            if (token_len > 0 && response_len + token_len < MAX_RESPONSE_LEN - 1) {
                memcpy(response + response_len, token_buf, token_len);
                response_len += token_len;
            }
            
            llama_token next_tokens[1] = {new_token_id};
            batch = llama_batch_get_one(next_tokens, 1);
            
            if (llama_decode(ctx, batch) != 0) {
                fprintf(stderr, "Decode failed during generation\n");
                break;
            }
            
            n_generated++;
        }
        
        response[response_len] = '\0';
        
        // Output response
        printf("%s\n", response);
        fflush(stdout);
        
        // Cleanup for this query
        free(prompt_tokens);
        llama_sampler_free(sampler);
        llama_free(ctx);
    }
    
    // Final cleanup
    llama_model_free(model);
    llama_backend_free();
    
    return 0;
}

#endif

int main(int argc, char **argv) {
    bool echo_mode = false;
    const char *model_path = NULL;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--echo") == 0) {
            echo_mode = true;
        } else {
            model_path = argv[i];
        }
    }
    
    if (echo_mode) {
        apply_sandbox();
        return echo_worker();
    }
    
    if (model_path == NULL) {
        fprintf(stderr, "No model path provided, using echo mode\n");
        apply_sandbox();
        return echo_worker();
    }
    
    if (access(model_path, R_OK) != 0) {
        fprintf(stderr, "Model file not accessible, using echo mode\n");
        apply_sandbox();
        return echo_worker();
    }
    
#ifdef USE_LLAMA
    return llama_worker(model_path);
#else
    fprintf(stderr, "Compiled without llama.cpp support, using echo mode\n");
    apply_sandbox();
    return echo_worker();
#endif
}
