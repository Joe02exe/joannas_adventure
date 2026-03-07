#!/bin/sh
cd "$(dirname "$0")"

# Set HOME to the current directory so any local saves or configs stay in the game folder
export HOME="$(dirname "$0")"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$(dirname "$0")/lib"

# Launch the game executable and capture logs
./main
