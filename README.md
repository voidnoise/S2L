# S2L Workshop

Welcome to the Sound To Light workshop code repo. This is where you will find all of the supporting code for the workshop. We have tried to lay things out logically with instructions and guides where we can, but please reach out during the session if you have any questions. This workshop is aimed as an introduction to what we call the "pipeline" for turning sound into light. With that, we are providing as much code as we can so you dont get stuck in the technicality of coding, and instead get to explore how plugging the different processes together can make the whole system work and light up. However, this does mean that you will need to bear with some concepts, copy and paste and be patient with some things that might feel complicated. The benefit is, that from having the system work at the end, you can then go back and explore individual sections in more detail later if you want. This github repo is public and shared under Creative Commons license, and you are free to use it after the workshop to continue your work. 


## Project Tools & Environment

This project uses a mix of Python to analyse audio, and arduino to create light animations and control the LEDs. We have set this up in VS Code, so everything is in one place. We could have used the Arduino IDE, and you would be welcome to use the Arduino code there, but we suggest keeping with VS Code and a plugin called PlatformIO which allows you to program devices like Arduino from within VS Code. We will guide you through using this during the worksho.

## Project layout

```
firmware/       PlatformIO project (Arduino firmware)
scripts/        Python scripts/tooling
requirements.txt  Python dependencies
setup.ps1       Windows setup script (The project should be setup on the machine for you before the workshop)
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

## General Pointers

- We will make use of a terminal throughout the project. A terminal is a command line prompt and VS Code has a build in terminal accessible through the menu bar -> Terminal -> New Terminal. This will then open up at the lower region of the window. We will guide you through commands and using the terminal in the workshop. 
- Python scripts live in `scripts/` and run against the `.venv` interpreter. venv is a way to create issolated python environments, so if you installed a package inside the venv it wont affect the rest of your system. It also makes it easier to share projects with easily recreatable environments. For example, to install on a new machine you would use python's pip to install the requirements with `pip install -r requirements`
- To activate the python virtual environment 'venv'
   - Mac : source .venv/bin/activate
   - Windos: .venv/Scripts/activate
- Firmware lives in `firmware/`; build/upload using commands like `pio run -t upload --project-dir firmware`.
- Add new Python packages to `requirements.txt`, new firmware libraries to `firmware/platformio.ini` under `lib_deps`.

## Workshop Outline

The agenda will be as follows:

- Welcome & Intro
- What is sound to light
- Meet the toolkit
- What is sound data
- Environment setup
- Analysing sound - FFTs
- Connecting and wiring an Arduino
- Sending data over serial
- Linking FFT data to drive LED animations on the Arduino
