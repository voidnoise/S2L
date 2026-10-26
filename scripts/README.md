
# Audio Analysis Scripts

This directory contains two scripts, fft.py and audio_stream.py. Their purpose and use are described below.



## Running FFT.py

This script contains the code to apply a Fast Fourior Transform function on audio data https://en.wikipedia.org/wiki/Fast_Fourier_transform. The simplest way to describe what a FFT does is that it takes a sample of audio data and caulculates the presence of constituent frequencies that make up the sound in the sample. If for example you played a triad (3 note) chord, it would show the frequencies of the 3 notes that make that chord. We use this to determine how much drums, bass, guitar etc. is in the music at a point in time. This can then be used to determine the beats or rhythm, which can make the LEDs animate in time with the music.

This script can be run to use either an audio file (there are some in audio/ directory) or a microphone input.

In a terminal (vscode menu bar -> Terminal -> New Terminal) navigate to the scripts folder (cd scripts) and run the following commands.

### To use an audio file:

From the scripts directory call:

```
python fft.py --file ../audio/melody7.wav --debug true --datasize 2048
```   

### To use a microphone:

From the scripts directory call:

```
python fft.py --mic true --debug true --datasize 2048
```   

## Running audio_stream.py

This script calls the code in the FFT script and sends the FFT results over serial for the Arduino to use.

Streams the 7 frequency band levels (0-255) over serial to the Arduino, one line per audio block, e.g. `172,255,255,255,255,170,96`.

First upload a firmware program from the `firmware/` folder, then find the Arduino's serial port. The command line calls below are relative to the scripts directory. The programs are:

| Program | Upload with | What it does |
|---|---|---|
| `animations` | `pio run -d ../firmware -e animations -t upload` | Three animations on 12 LEDs: VU meter, shooting lights and beat ripples. Pick one with `--animation 1`, `2` or `3`. See `firmware/animations/README.md`. |
| `animations_fastled` | `pio run -d ../firmware -e animations_fastled -t upload` | The same three animations, written with the FastLED library. This is the easiest one to read. See `firmware/animations_fastled/README.md`. |
| `simple_animation` | `pio run -d ../firmware -t upload` | Drives 12 WS2812 LEDs from the audio data, one zone per frequency band. This is the default. |
| `serial_test` | `pio run -d ../firmware -e serial_test -t upload` | Echoes each frame back so you can check the data arrives. |
| `led_test` | `pio run -d ../firmware -e led_test -t upload` | Cycles colours, a chase and a rainbow on the LEDs. Doesn't use the audio. |

To find the port, run `ls /dev/cu.*` before and after plugging it in; on macOS it looks like `/dev/cu.usbmodem1101` or `/dev/cu.usbserial-XXXX` (on Windows it is `COM5` or similar, but the command `mode` should list avaiable ports). Close any serial monitor first, or the port will be busy.

### To stream an audio file (it plays out loud while streaming):

From the scripts directory call:

```
python audio_stream.py --file ../audio/melody7.wav --port /dev/cu.usbmodem1101
```

Add `--no-playback` to stream without playing the sound.

### To stream a microphone:

```
python audio_stream.py --port /dev/cu.usbmodem1101
```

### Checking the Arduino receives data

Upload `serial_test` first (`pio run -d ../firmware -e serial_test -t upload`). It echoes every valid line back as `ok 1,2,3,4,5,6,7` (or `err ...` if it can't parse it) and toggles the on-board LED for each valid frame. To test without Python, open a serial monitor and type a line by hand:

```
pio device monitor -d ../firmware
1,2,3,4,5,6,7
```

You should see `ok 1,2,3,4,5,6,7` come back. While `audio_stream.py` is running the on-board LED (pin 13) should flicker.

To see the Arduino's replies while streaming, add `--echo`. Each frame sent should be answered by an `arduino: ok ...` line:

```
python audio_stream.py --file ../audio/melody7.wav --port /dev/cu.usbserial-A70064SK --echo
```

