## ✨ Overview

The **Offline AI Assistant Shell** is a C-based shell environment enhanced with an integrated **local AI assistant**.  
It runs **entirely offline**, ensuring **data privacy**, **sandboxed execution**, and **system-level security**.

---

## ⚙️ Features

 **Offline Operation** — No internet, no APIs, full privacy.  
 **Secure Sandboxing** — Multiple layers of protection for safe inference.  
 **Graceful Fallback** — Reverts to echo mode if the model is missing.  
 **Timeout Protection** — Automatically terminates slow AI tasks.  
 **Reset Command** — Restart the AI worker safely anytime.  

---

## 🔐 Security Architecture

The assistant is **architected for isolation** — separating the trusted shell process from the sandboxed AI worker.

| Component | Role | Isolation Mechanisms |
|------------|------|----------------------|
| **Shell Process (`myshell`)** | Trusted interface. Handles parsing, I/O, and AI worker management. | — |
| **AI Worker (`ai_worker`)** | Sandboxed process for model inference. | `Seccomp`, `Rlimits`, `Namespaces`, Capability Dropping |

graph TD
    A[Shell Process (Trusted)] -->|fork() + Pipes| B(AI Worker - Sandboxed)
    B --> C[Load Model (Read-Only)]
    B --> D[Apply Seccomp Filter]
    B --> E[Drop Capabilities]
    B --> F[Set Resource Limits]

Isolation Layers
Seccomp (syscall whitelist) — Only necessary system calls allowed.
Rlimits — Caps RAM (~2 GB) and CPU time (30 s).
Namespaces — File system and PID isolation when supported.
Capability Dropping — Removes unnecessary kernel privileges.

##  Installation

### Prerequisites

### Install required dependencies:

sudo apt-get install build-essential cmake libcap-dev valgrind

### 1️⃣ Clone & Build llama.cpp

cd ~  
git clone https://github.com/ggerganov/llama.cpp.git  
cd llama.cpp  
rm -rf build && mkdir build && cd build  
cmake .. -DBUILD_SHARED_LIBS=OFF  
make llama -j$(nproc)  


### 2️⃣ Download Model

mkdir -p ~/.assist_ai  
cd ~/.assist_ai  
wget https://huggingface.co/TheBloke/TinyLlama-1.1B-Chat-v1.0-GGUF/resolve/main/tinyllama-1.1b-chat-v1.0.Q4_K_M.gguf -O model.gguf  
md5sum model.gguf > model.md5  

### 3️⃣ Build the Shell Project

cd ~/Documents/C_project/mini-shell  
mkdir build && cd build  
cmake ..  
make -j$(nproc)  

### 💡 Usage
Run the compiled shell executable:  
./myshell  
Action	Command  
Ask the AI assistant	mini-shell> assist what is recursion  
Reset the AI worker	mini-shell> assist-reset  
Exit the shell	mini-shell> exit  

### 🧪 Testing
Test	Command	Description  
Run test suite	./test_assist.sh	Automated functional testing  
Verify model integrity	./verify_model.sh	Confirms MD5 hash  
Stress test	./stress_test.sh	Load and stability testing  
Security audit	strace -f ./myshell	Check Seccomp syscall filtering  

### ⚠️ Troubleshooting
Problem	Possible Cause Solution  
Worker fails to start	Missing model file Ensure ~/.assist_ai/model.gguf exists  
Timeout errors	Context too large	Reduce n_ctx or use a smaller model  
Namespace errors	Unsupported kernel feature	Shell auto-fallback will disable isolation  
Seccomp kills worker	Missing allowed syscall	Update whitelist in seccomp_filter.c  

### 📊 Performance (CPU-Only)
Metric	Value	Notes  
First query time	~2–3 s	Includes model loading  
Subsequent queries	~1–2 s	Reuses context  
Memory usage	~1–1.5 GB	Depends on model size  
Token generation	10–20 tokens/s	CPU-dependent  

### 🛡️ Design Philosophy
“Local intelligence should be private, safe, and performant.”  
This project shows how AI inference can be embedded directly into system-level utilities while maintaining offline operation and secure sandboxing — suitable for embedded, defense, and research systems.  

### 📜 License
This project is licensed under the MIT License.  
See the LICENSE file for details.  

### 🤝 Contributing
Pull requests and security improvements are welcome!  
Please open an issue before submitting major changes.  

### Acknowledgments
llama.cpp — Local LLM inference engine  
TheBloke — Model provider  
Inspired by research in secure AI sandboxing and trusted computing  

<p align="center"> <sub>🧠 Built for privacy. Designed for performance. Secured by sandboxing.</sub> </p>
