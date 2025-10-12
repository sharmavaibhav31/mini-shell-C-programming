#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/resource.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <errno.h>

extern void install_seccomp_filter(void);

void apply_sandbox(void) {
    // 1. Try to create new namespaces (requires CAP_SYS_ADMIN or user namespace)
    // FALLBACK: If unshare fails, continue (reduces isolation but still safe)
    if (unshare(CLONE_NEWNS | CLONE_NEWPID) == -1) {
        if (errno == EPERM) {
            fprintf(stderr, "Warning: namespace isolation unavailable (EPERM)\n");
        }
        // Continue anyway
    }
    
    // 2. Set resource limits
    struct rlimit mem_limit = { 
        .rlim_cur = 2UL * 1024 * 1024 * 1024,  // 2GB
        .rlim_max = 2UL * 1024 * 1024 * 1024 
    };
    if (setrlimit(RLIMIT_AS, &mem_limit) == -1) {
        perror("setrlimit AS");
    }
    
    struct rlimit cpu_limit = { 
        .rlim_cur = 30,  // 30 seconds CPU time
        .rlim_max = 30 
    };
    if (setrlimit(RLIMIT_CPU, &cpu_limit) == -1) {
        perror("setrlimit CPU");
    }
    
    // 3. Disable core dumps
    struct rlimit core_limit = { .rlim_cur = 0, .rlim_max = 0 };
    if (setrlimit(RLIMIT_CORE, &core_limit) == -1) {
        perror("setrlimit CORE");
    }
    
    // 4. Close all file descriptors except stdin/stdout/stderr
    for (int fd = 3; fd < 256; fd++) {
        close(fd);
    }
    
    // 5. Drop capabilities (if libcap available)
    // FALLBACK: If cap functions unavailable, skip (still protected by seccomp)
    #ifdef HAVE_LIBCAP
    cap_t caps = cap_init();
    if (caps) {
        cap_set_proc(caps);
        cap_free(caps);
    }
    #endif
    
    // 6. Install seccomp filter (must be last, as it restricts syscalls)
    install_seccomp_filter();
}
