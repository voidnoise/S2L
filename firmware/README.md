# Firmware

The programs that run on the Arduino. Each one is a folder in here, and `platformio.ini`
defines how to build and upload each of them for the PlatformIO tool.

The Arduino doesn't listen to music itself. The computer runs `scripts/audio_stream.py`,
which splits the sound into 7 frequency bands and sends the levels over USB. The firmware
turns those levels into light on a strip of 12 WS2812 LEDs.

## The programs

| Folder / environment | What it does | Needs the audio script? |
|---|---|---|
| `led_test` | Tests the LED strip: cycles red, green, blue and white, then a chase, then a rainbow. **Start here** to check your wiring. | No |
| `serial_test` | Checks the USB connection: reads the 7 band levels and sends them back as `ok 12,45,...`. Blinks the board's own LED for each frame. | Yes (or type into a serial monitor) |
| `simple_animation` | The simplest audio display: the strip is split into 7 coloured zones, one per frequency band, and each glows with its band's level. | Yes |
| `animations` | Three animations written with the Adafruit NeoPixel library: VU meter, shooting lights, beat ripples. | Yes |
| `animations_fastled` | The same three animations written with the FastLED library, plus `custom`, an empty template for writing your own. **The easiest one to read.** | Yes |

`animations` and `animations_fastled` have their own `README.md` that explains each animation and what you can tweak.

A good order to work through them: `led_test` (is the strip wired up?), then `serial_test`
(is the data arriving?), then `animations_fastled` (the real thing).

## Wiring

All programs expect the same wiring. It is in each program's comments too.

| LED strip | Arduino |
|---|---|
| DIN | pin 6 |
| 5V | 5V |
| GND | GND |

A 470 uF capacitor across 5V and GND at the strip is a good idea (but not required).
`serial_test` doesn't use the strip at all.

## Building and uploading

[this should have been done before the workshop for you -->] You need [PlatformIO](https://platformio.org/). `setup.sh` (or `setup.ps1` on Windows) in the repository
root installs it into the project's `.venv`, so the `pio` command is `.venv/bin/pio`, or just `pio` if
you have activated the virtual environment (`source .venv/bin/activate`). The commands below
assume that and are run from this `firmware` folder (`cd firmware`). From the repository root,
add `-d firmware` after `pio run`.

**Build only** (checks the code compiles, doesn't touch the Arduino):

```
pio run -e led_test
```

**Build and upload** (plug the Arduino in first):

```
pio run -e led_test -t upload
```

Replace `animations_fastled` with the environment name of the program you want from the table above.
The first build of each program downloads its library (NeoPixel or FastLED), so it needs an internet
connection and takes a little longer.

If you leave out `-e`, PlatformIO uses the `default_envs` setting, which is `simple_animation`.
So `pio run -t upload` on its own uploads that one.

**Watch what the Arduino sends back** (a serial monitor):

```
pio device monitor
```

Press Ctrl+C to leave it. Close the monitor before you run `audio_stream.py`, because only one
program can use the USB port at a time.


## platformio.ini: the settings, and the board setting in particular

### The board setting

This is the one setting you may need to change, and the upload fails if it is wrong.

```ini
[env]
platform = atmelavr
framework = arduino
monitor_speed = 115200

board = diecimilaatmega168
```

`board` tells PlatformIO exactly which Arduino you have. It uses it to choose the chip
(for the right compiler settings and memory sizes) and **how fast to talk to the board's bootloader**
when uploading. Boards that look almost the same need different values.

**The name has two parts: the board model, and the chip on it.** Older Arduinos came with
either an ATmega168 or an ATmega328 chip. The chip's name is printed on the large black rectangle
on the board.

| Your board | ATmega168 | ATmega328 |
|---|---|---|
| Arduino Diecimila or Duemilanove | `diecimilaatmega168` | `diecimilaatmega328` |
| Arduino Nano | `nanoatmega168` | `nanoatmega328` (older bootloader), or `nanoatmega328new` (newer bootloader) |
| Arduino Pro / Pro Mini, 5V 16 MHz | `pro16MHzatmega168` | `pro16MHzatmega328` |
| Arduino Pro / Pro Mini, 3.3V 8 MHz | `pro8MHzatmega168` | `pro8MHzatmega328` |
| Arduino Mini | `miniatmega168` | `miniatmega328` |
| Arduino Uno | (none made) | `uno` |

There are many more boards. List them with `pio boards atmelavr`, or search with, for example,
`pio boards mega`.

**Change it in one place.** The `board =` line is in the shared `[env]` section, so it applies to every
program. Edit that one line and all five programs use it. The comment above it in the file lists the
common choices.

#### What differs between the 168 and the 328

| | ATmega168 | ATmega328 |
|---|---|---|
| Flash memory (for the program) | 14 KB usable | 30 KB usable (Uno: 31.5 KB) |
| RAM (for variables) | 1 KB | 2 KB |
| Upload speed (bootloader) | 19200 baud, on all the 168 boards in the table above | 57600 baud on Diecimila, Duemilanove, Nano, Pro and Mini. 115200 on the Uno and `nanoatmega328new` |

**Memory.** The 168 is small, which is why the programs here are kept short. This is how much of the
168's memory each one uses:

| Program | Flash used (of 14 KB) | RAM used (of 1 KB) |
|---|---|---|
| `led_test` | 4.8 KB | 220 bytes |
| `serial_test` | 3.2 KB | 239 bytes |
| `simple_animation` | 6.0 KB | 286 bytes |
| `animations` | 8.1 KB | 375 bytes |
| `animations_fastled` | 8.8 KB | 487 bytes |

If you add to a program and it stops compiling with "region `text` overflowed", you have run out
of flash. Running out of RAM doesn't give an error; the program just misbehaves. Keep an eye on the
`RAM:` line that PlatformIO prints after a build, and try to stay under about 70%. On a 328 board
you have double the room.

**Upload speed.** The board's bootloader (the small program that receives uploads) only works at one speed,
and PlatformIO picks that speed from the `board` setting. Pick the wrong board and the upload fails with:

```
avrdude: stk500_getsync() attempt 1 of 10: not in sync: resp=0x00
```

That message means "the Arduino did not reply", so the board setting is the first thing to check. It was
what happened with this project: a Diecimila with a 168 chip, uploaded as `uno` (115200 baud) and
then as `diecimilaatmega328` (57600 baud), failed until it was set to `diecimilaatmega168` (19200 baud).

### The rest of the file

```ini
[platformio]
src_dir = .
default_envs = simple_animation
```

- `src_dir = .` — PlatformIO normally looks for code in a folder called `src`. Here each program has its own
  folder, so this tells it to look from the `firmware` folder itself.
- `default_envs` — the program that `pio run` builds and uploads when you don't use `-e`. Only one is listed
  so you don't upload the wrong thing by accident.

```ini
[env]
platform = atmelavr
framework = arduino
monitor_speed = 115200
```

Settings shared by every program. `platform` and `framework` say we are programming an AVR chip with the
Arduino libraries. `monitor_speed` is how fast the serial monitor talks. It must match `Serial.begin(115200)`
in the code, and the `--baud` setting of `audio_stream.py`.

```ini
[env:animations_fastled]
build_src_filter = -<*> +<animations_fastled/>
lib_deps =
    fastled/FastLED@^3.9.0
```

One section per program. The name after `env:` is the name you use with `-e`. It is only a label; it doesn't have to
match the board. Inside it:

- `build_src_filter` — which folder holds this program's code. `-<*>` means "ignore everything", and
  `+<animations_fastled/>` then adds back just that folder. This is how several programs can live in one project
  without being mixed together.
- `lib_deps` — the libraries to download. PlatformIO fetches them the first time you build.

### Choosing the USB port

Normally PlatformIO finds the Arduino by itself. If you have several USB serial devices plugged in and it
picks the wrong one, name the port. In `platformio.ini` there are two lines for this, commented out with `;`:

```ini
; upload_port = COM3
; monitor_port = COM3
```

Remove the `;` and set the port. To find yours, run `pio device list`. It shows entries such as
`/dev/cu.usbserial-A70064SK` or `/dev/cu.usbmodem1101` on macOS, `/dev/ttyUSB0` or `/dev/ttyACM0` on
Linux, and `COM3` on Windows. You can also try it for one upload only:

```
pio run -e led_test -t upload --upload-port /dev/cu.usbserial-A70064SK
```

## If the upload fails

| What you see | What to try |
|---|---|
| `not in sync: resp=0x00` | The wrong `board`. See above. Check the chip name on the board. |
| `Could not open port` or `Resource busy` | Something else is using the port. Close any serial monitor or `audio_stream.py`. |
| No port found | Try another USB cable (some only carry power) and another USB port. Run `pio device list`. Some boards need a USB driver (FTDI or CH340). |
| It still fails | Press the board's reset button just as the upload starts. Unplug anything connected to pins 0 and 1 (TX/RX). On an older Diecimila, check the power jumper next to the USB socket is on the pins marked USB. |
| `region 'text' overflowed` | The program is too big for the chip's flash. See the memory table above. |
| Library download fails | The first build of each program needs an internet connection. |

## Adding your own program

1. Make a new folder here, for example `my_program`, with a `main.cpp` in it that has `setup()` and `loop()`.
2. Add a section to `platformio.ini`:

   ```ini
   [env:my_program]
   build_src_filter = -<*> +<my_program/>
   lib_deps =
       fastled/FastLED@^3.9.0
   ```

   (Leave out `lib_deps` if it doesn't use a library.)
3. Build and upload it with `pio run -e my_program -t upload`.

If you only want to write your own *animation*, you don't need any of this: use `custom.h` in
`animations_fastled` instead.
