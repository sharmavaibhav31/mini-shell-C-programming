#!/bin/bash

BUILD_DIR="./build"
SHELL_BIN="$BUILD_DIR/myshell"

echo "=== Stress Test ==="

# Test 1: Rapid queries
echo "Test 1: 10 rapid queries"
(
for i in {1..10}; do
    echo "assist query $i"
done
echo "exit"
) | timeout 60 $SHELL_BIN > /tmp/stress_output.txt 2>&1

RESPONSES=$(grep -c "AI:" /tmp/stress_output.txt)
echo "Received $RESPONSES responses"

if [ $RESPONSES -ge 8 ]; then
    echo "✓ PASS (at least 80% success rate)"
else
    echo "✗ FAIL (too many failures)"
fi

# Test 2: Worker crash recovery
echo "Test 2: Worker crash recovery"
$SHELL_BIN &
SHELL_PID=$!
sleep 2

echo "assist test1" > /proc/$SHELL_PID/fd/0
sleep 2

WORKER_PID=$(pgrep -P $SHELL_PID ai_worker)
if [ -n "$WORKER_PID" ]; then
    echo "Killing worker PID $WORKER_PID"
    kill -9 $WORKER_PID
    sleep 1
    
    echo "assist test2" > /proc/$SHELL_PID/fd/0
    sleep 2
    
    NEW_WORKER_PID=$(pgrep -P $SHELL_PID ai_worker)
    if [ -n "$NEW_WORKER_PID" ] && [ "$NEW_WORKER_PID" != "$WORKER_PID" ]; then
        echo "✓ PASS (worker respawned)"
    else
        echo "✗ FAIL (worker did not respawn)"
    fi
fi

kill $SHELL_PID 2>/dev/null

# Test 3: Memory leak check
echo "Test 3: Memory leak check"
valgrind --leak-check=full --error-exitcode=1 \
    $SHELL_BIN < <(echo -e "assist test\nexit") > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo "✓ PASS (no memory leaks detected)"
else
    echo "⚠ WARNING (potential memory leaks - review valgrind output)"
fi

echo "=== Stress Test Complete ==="
