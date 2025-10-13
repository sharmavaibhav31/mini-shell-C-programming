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
    // 1. Try to create new namespaces
    if (unshare(CLONE_NEWNS | CLONE_NEWPID) == -1) {
        if (errno == EPERM) {
            fprintf(stderr, "Warning: namespace isolation unavailable (EPERM)\n");
        }
    }
    
    // 2. Set resource limits
    struct rlimit mem_limit = { 
        .rlim_cur = 2UL * 1024 * 1024 * 1024,
        .rlim_max = 2UL * 1024 * 1024 * 1024 
    };
    setrlimit(RLIMIT_AS, &mem_limit);
    
    struct rlimit cpu_limit = { 
        .rlim_cur = 30,
        .rlim_max = 30 
    };
    setrlimit(RLIMIT_CPU, &cpu_limit);
    
    // 3. Disable core dumps
    struct rlimit core_limit = { .rlim_cur = 0, .rlim_max = 0 };
    setrlimit(RLIMIT_CORE, &core_limit);
    
    // 4. Close all file descriptors except stdin/stdout/stderr
    for (int fd = 3; fd < 256; fd++) {
        close(fd);
    }
    
    // 5. Drop capabilities
    #ifdef HAVE_LIBCAP
    cap_t caps = cap_init();
    if (caps) {
        cap_set_proc(caps);
        cap_free(caps);
    }
    #endif
    
    // 6. TEMPORARILY DISABLE SECCOMP FOR TESTING
    // TODO: Add more syscalls to whitelist once we identify which ones are needed
    // install_seccomp_filter();
    
    fprintf(stderr, "Sandbox applied (seccomp disabled for testing)\n");
}
