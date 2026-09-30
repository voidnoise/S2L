#!/usr/bin/env bash
# Sets up the Python virtual environment and PlatformIO dependencies for S2L Workshop.
# Run this once after cloning the repo: ./setup.sh
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

command -v python3 >/dev/null 2>&1 || {
    echo "python3 was not found on PATH. Install Python 3.9+ and re-run this script." >&2
    exit 1
}

if [ "$(uname -s)" = "Linux" ]; then
    if ! ldconfig -p 2>/dev/null | grep -q libportaudio; then
        echo "libportaudio2 (needed by the 'sounddevice' package) was not found."
        if command -v apt-get >/dev/null 2>&1; then
            echo "Installing it via apt-get (you may be prompted for your password)..."
            sudo apt-get update && sudo apt-get install -y libportaudio2
        else
            echo "Warning: could not auto-install libportaudio2 (apt-get not found). Install it manually via your distro's package manager." >&2
        fi
    fi
fi

echo "Creating virtual environment (.venv)..."
if [ ! -d "$REPO_ROOT/.venv" ]; then
    python3 -m venv "$REPO_ROOT/.venv"
else
    echo ".venv already exists, skipping creation."
fi

VENV_PYTHON="$REPO_ROOT/.venv/bin/python"

echo "Upgrading pip..."
"$VENV_PYTHON" -m pip install --upgrade pip

echo "Installing Python dependencies from requirements.txt..."
"$VENV_PYTHON" -m pip install -r "$REPO_ROOT/requirements.txt"

echo "Installing PlatformIO project dependencies (firmware/platformio.ini)..."
VENV_PIO="$REPO_ROOT/.venv/bin/pio"
if [ -x "$VENV_PIO" ]; then
    "$VENV_PIO" pkg install --project-dir "$REPO_ROOT/firmware"
else
    echo "Warning: pio not found in .venv; skipping firmware dependency install. The PlatformIO VS Code extension will install these automatically when the firmware folder is opened." >&2
fi

echo ""
echo "Setup complete."
echo "Next steps:"
echo "  1. Open this folder in VS Code."
echo "  2. Install the recommended extensions when prompted (Python, Pylance, PlatformIO IDE)."
echo "  3. VS Code will automatically select .venv as the Python interpreter."
