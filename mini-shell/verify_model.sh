#!/bin/bash

MODEL_DIR="$HOME/.assist_ai"
MODEL_FILE="$MODEL_DIR/model.gguf"
CHECKSUM_FILE="$MODEL_DIR/model.md5"

echo "=== Model Integrity Verification ==="

if [ ! -f "$MODEL_FILE" ]; then
    echo "✗ Model file not found at $MODEL_FILE"
    exit 1
fi

echo "Model file: $MODEL_FILE"
ls -lh "$MODEL_FILE"

if [ -f "$CHECKSUM_FILE" ]; then
    echo "Verifying checksum..."
    cd "$MODEL_DIR"
    md5sum -c model.md5
    if [ $? -eq 0 ]; then
        echo "✓ Checksum valid"
    else
        echo "✗ Checksum mismatch - possible tampering"
        exit 1
    fi
else
    echo "Creating checksum..."
    md5sum "$MODEL_FILE" > "$CHECKSUM_FILE"
    echo "✓ Checksum created"
fi

echo "=== Verification Complete ==="
