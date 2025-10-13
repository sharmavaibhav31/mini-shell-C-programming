# Quick Start Guide

## Setup

# Download and extract
cd ~/your-shell-project/build
./myshell

# Try it
mini-shell> assist hello world
AI: Echo: hello world

mini-shell> exit
If Model Available
Verify model
ls -lh ~/.assist_ai/model.gguf

# Should see ~600MB file

# Run with inference
./myshell
mini-shell> assist explain AI in one sentence
AI: [AI-generated response]
Test Commands
bash# Basic query
assist what is 2 plus 2

# Multi-word prompt
assist explain the difference between RAM and ROM

# Reset if worker hangs
assist-reset

# Exit shell
exit
