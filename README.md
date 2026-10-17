# S2L Workshop

Python tooling + PlatformIO Arduino firmware for the S2L workshop.

## Project layout

```
firmware/       PlatformIO project (Arduino firmware)
scripts/        Python scripts/tooling
requirements.txt  Python dependencies
setup.ps1       Windows setup script
setup.sh        macOS/Linux setup script
```

## Setup (first time on a new machine)

1. Install [VS Code](https://code.visualstudio.com/) and [Python 3.9+](https://www.python.org/downloads/).
2. Clone this repo and open the folder in VS Code.
3. Run the setup script from a terminal in the repo root:
   - Windows (PowerShell): `.\setup.ps1`
   - macOS/Linux: `./setup.sh` (On maxc you may need to change the file permissions with `chmod +x`)
4. When VS Code prompts you to install the recommended extensions (Python, Pylance, PlatformIO IDE), accept. PlatformIO will finish installing its own toolchain (compilers, board platforms) automatically on first activation.
5. VS Code will automatically select `.venv` as the Python interpreter. If it doesn't, run **Python: Select Interpreter** from the command palette and choose `.venv`.

> **Linux only:** the `sounddevice` package needs the system `libportaudio2` library. `setup.sh` installs it automatically via `apt-get` if available; on other distros install it manually (e.g. `libportaudio2` / `portaudio` from your package manager) before running the script.

## Day-to-day use

- Python scripts live in `scripts/` and run against the `.venv` interpreter.
- To activate the python virtual environment 'venv'
   - Mac : source .venv/bin/activate
   - Windos: .venv/Scripts/activate
- Firmware lives in `firmware/`; build/upload via the PlatformIO toolbar in VS Code or `pio run -t upload --project-dir firmware`.
- Add new Python packages to `requirements.txt`, new firmware libraries to `firmware/platformio.ini` under `lib_deps`.
