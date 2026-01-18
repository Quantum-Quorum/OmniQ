#!/bin/bash
# Test launcher - runs in background and reports status

echo "🧪 Testing OmniQ Debugger..."
cd "$(dirname "$0")/omniq-debugger/build" || exit 1

# Try to launch (will display but not block terminal)
./omniq-debugger &
DEBUGGER_PID=$!

sleep 2

# Check if still running
if ps -p $DEBUGGER_PID > /dev/null 2>&1; then
    echo "✅ Debugger launched successfully (PID: $DEBUGGER_PID)"
    echo "   Close the window when done, or run: kill $DEBUGGER_PID"
else
    echo "❌ Debugger crashed on startup"
    exit 1
fi
