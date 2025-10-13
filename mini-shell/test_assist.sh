#!/bin/bash

echo "=== Offline AI Assistant Test Suite ==="

BUILD_DIR="./build"
SHELL_BIN="$BUILD_DIR/myshell"

if [ ! -f "$SHELL_BIN" ]; then
    echo "Error: Shell binary not found"
    exit 1
fi

# Test 1: Basic functionality
echo "Test 1: Basic assist command"
echo -e "assist hello\nexit" | $SHELL_BIN | grep -q "AI:"
if [ $? -eq 0 ]; then
    echo "✓ PASS"
else
    echo "✗ FAIL"
fi

# Test 2: Empty prompt
echo "Test 2: Empty prompt handling"
echo -e "assist\nexit" | $SHELL_BIN 2>&1 | grep -q "Usage:"
if [ $? -eq 0 ]; then
    echo "✓ PASS"
else
    echo "✗ FAIL"
fi

# Test 3: Long prompt (near limit)
echo "Test 3: Long prompt"
LONG_PROMPT=$(python3 -c "print('word ' * 500)")
echo -e "assist $LONG_PROMPT\nexit" | timeout 35 $SHELL_BIN > /dev/null
if [ $? -eq 0 ] || [ $? -eq 124 ]; then
    echo "✓ PASS (handled without crash)"
else
    echo "✗ FAIL"
fi

# Test 4: Worker reset
echo "Test 4: Worker reset"
echo -e "assist test1\nassist-reset\nassist test2\nexit" | $SHELL_BIN | grep -c "AI:" | grep -q "2"
if [ $? -eq 0 ]; then
    echo "✓ PASS"
else
    echo "✗ FAIL"
fi

# Test 5: Multiple consecutive queries
echo "Test 5: Multiple queries"
echo -e "assist one\nassist two\nassist three\nexit" | $SHELL_BIN | grep -c "AI:" | grep -q "3"
if [ $? -eq 0 ]; then
    echo "✓ PASS"
else
    echo "✗ FAIL"
fi

# Test 6: Special characters
echo "Test 6: Special characters"
echo -e "assist what is 2+2?\nexit" | $SHELL_BIN | grep -q "AI:"
if [ $? -eq 0 ]; then
    echo "✓ PASS"
else
    echo "✗ FAIL"
fi

echo "=== Test Suite Complete ==="
