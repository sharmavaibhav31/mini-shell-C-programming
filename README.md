# 🧠 Offline AI Assistant Shell

> A **secure, offline AI assistant** integrated directly into a custom C shell — powered by [`llama.cpp`](https://github.com/ggerganov/llama.cpp) for local inference.

---
## 🌱 Project Story
I started this project around six months ago, soon after switching to Kali Linux. Coming from a different operating system, I found Kali a bit challenging at first — even for basic tasks. Like most developers today, I often relied on ChatGPT and other online resources to learn commands and troubleshoot issues.  

That’s when an idea struck me:  

“If AI assistants can help us with everything online, why can’t we have one that lives inside the terminal itself?”  

Curiosity drove me to explore how an AI could operate directly within the command line, assisting users interactively. But as I dug deeper, I realized that this idea came with serious implications — allowing AI models system-level access could lead to data corruption, privilege escalation, unauthorized command execution, privacy breaches, and critical system instability.  
Understanding the gravity of these risks made me more determined to approach this responsibly. I began studying how AI agents function inside operating systems, how sandboxing and syscall filtering can protect sensitive operations, and how to build an offline, secure environment for inference.  
This exploration led to “A Secure Offline AI Assistant” — a project that embeds a local language model within a custom-built shell. The assistant helps users execute, interpret, and learn terminal commands through natural language, while ensuring that no data ever leaves the local system.  

### ⚙️ Tech Stack & Security Architecture  
The project is built using:  
C — for system-level programming and shell implementation.  
CMake — for build automation.  
llama.cpp — to run the local LLM efficiently on CPU.  
TinyLlama-1.1B-Chat — for conversational intelligence.  
Seccomp (Secure Computing Mode) — to restrict dangerous syscalls and sandbox AI execution.  
This architecture ensures that the assistant operates entirely offline, within a controlled and limited system context, minimizing potential exposure and ensuring safety even at the command-line level.  

### 🧠 How It Works
Once launched, the shell initializes a secure sandbox where the AI model processes natural language queries locally.  
For example:  

mini-shell> assist what is recursion  
The assistant interprets the question and responds conversationally — without accessing the internet or external APIs.  
All interactions happen within the local memory context, and system calls are strictly filtered through Seccomp to prevent unauthorized operations.  

### 🛡️ Responsible Exploration  
I’m fully aware that integrating AI into system-level environments is a high-risk domain — one that demands continuous learning and careful design. This project doesn’t claim to solve all the security challenges but is built with privacy, isolation, and safety at its core.  
I’m eager to learn more about the advanced security mechanisms, kernel-level protections, and safer inference methods that could make such integrations even more robust.  
I’m open to feedback, mentorship, and suggestions from the community to strengthen this project further — both technically and ethically.  

### Beyond the Code
This work also inspired me to write an article titled   
“Agents Within the OS: Crafting Permissions for Always-On Memory”,   
which explores the balance between autonomy, control, and security in operating systems enhanced by AI agents.   
With this MVP, my goal is to show that AI doesn’t always have to live in the cloud — it can exist securely within our terminals, private, local, and safe by design.    

Link to Docs: https://drive.google.com/drive/folders/1bONRZUK0zjzxpHoM70sj2O1UjQ9VMGkp?usp=sharing



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
