# Offline AI Assistant Shell

A secure, offline AI assistant integrated into a custom C shell using llama.cpp for local inference.

## Features

- **Offline Operation**: All AI inference runs locally, no network required
- **Secure Sandboxing**: AI worker runs in isolated process with:
  - Seccomp syscall whitelist
  - Resource limits (2GB RAM, 30s CPU)
  - Namespace isolation (when available)
  - Capability dropping
- **Fallback Mode**: Graceful degradation to echo mode if model unavailable
- **Timeout Protection**: 30-second timeout prevents hanging
- **Worker Reset**: `assist-reset` command to restart AI worker

## Installation

### Prerequisites
```bash
sudo apt-get install build-essential cmake libcap-dev valgrind


# Build
# Clone and build llama.cpp
cd ~
git clone https://github.com/ggerganov/llama.cpp.git
cd llama.cpp
make libllama.a -j$(nproc)

# Download model
mkdir -p ~/.assist_ai
cd ~/.assist_ai
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O model.gguf
md5sum model.gguf > model.md5

# Build shell
cd ~/your-shell-project
mkdir build && cd build
cmake -DLLAMA_CPP_DIR=$HOME/llama.cpp ..
make -j$(nproc)

# Usage
./myshell

## Ask AI assistant
mini-shell> assist what is recursion

## Reset worker if needed
mini-shell> assist-reset

## Exit
mini-shell> exit


# Security Architecture
┌─────────────────────────────────────┐
│  Shell Process (Trusted)            │
│  - Command parsing                  │
│  - User interaction                 │
└──────────┬──────────────────────────┘
           │ fork() + pipes
           ▼
┌─────────────────────────────────────┐
│  AI Worker (Sandboxed)              │
│  - Namespaces: CLONE_NEWNS|NEWPID   │
│  - Seccomp: whitelist only          │
│  - Rlimits: 2GB RAM, 30s CPU        │
│  - Capabilities: dropped            │
│  - Model: read-only access          │
└─────────────────────────────────────┘

# Testing
## Run test suite
./test_assist.sh

## Verify model integrity
./verify_model.sh

## Stress test
./stress_test.sh

## Security audit
strace -f ./myshell



# Troubleshooting
### Worker fails to start: Check that model file exists at ~/.assist_ai/model.gguf
### Timeout errors: Reduce context size or use smaller model
### Namespace errors: Run without namespace isolation (automatically falls back)
### Seccomp kills worker: Review whitelisted syscalls in seccomp_filter.c

## Performance
### First query: ~2-3s (includes model loading)
### Subsequent queries: ~1-2s (context reused)
### Memory usage: ~1-1.5GB
### Token generation: 10-20 tokens/second (CPU-only)

# License
## MIT





