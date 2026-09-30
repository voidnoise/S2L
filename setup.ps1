# Sets up the Python virtual environment and PlatformIO dependencies for S2L Workshop.
# Run this once after cloning the repo: .\setup.ps1

$ErrorActionPreference = "Stop"
$RepoRoot = $PSScriptRoot

function Assert-Command($name, $hint) {
    if (-not (Get-Command $name -ErrorAction SilentlyContinue)) {
        Write-Error "'$name' was not found on PATH. $hint"
        exit 1
    }
}

Write-Host "Checking prerequisites..."
Assert-Command "python" "Install Python 3.9+ from https://www.python.org/downloads/ and re-run this script."

Write-Host "Creating virtual environment (.venv)..."
$VenvPath = Join-Path $RepoRoot ".venv"
if (-not (Test-Path $VenvPath)) {
    python -m venv $VenvPath
} else {
    Write-Host ".venv already exists, skipping creation."
}

$VenvPython = Join-Path $VenvPath "Scripts\python.exe"

Write-Host "Upgrading pip..."
& $VenvPython -m pip install --upgrade pip

Write-Host "Installing Python dependencies from requirements.txt..."
& $VenvPython -m pip install -r (Join-Path $RepoRoot "requirements.txt")

Write-Host "Installing PlatformIO project dependencies (firmware/platformio.ini)..."
$VenvPio = Join-Path $VenvPath "Scripts\pio.exe"
if (Test-Path $VenvPio) {
    & $VenvPio pkg install --project-dir (Join-Path $RepoRoot "firmware")
} else {
    Write-Warning "pio.exe not found in .venv; skipping firmware dependency install. The PlatformIO VS Code extension will install these automatically when the firmware folder is opened."
}

Write-Host ""
Write-Host "Setup complete." -ForegroundColor Green
Write-Host "Next steps:"
Write-Host "  1. Open this folder in VS Code."
Write-Host "  2. Install the recommended extensions when prompted (Python, Pylance, PlatformIO IDE)."
Write-Host "  3. VS Code will automatically select .venv as the Python interpreter."
