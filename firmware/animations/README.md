# Animations

Three light animations for a strip of 12 WS2812 LEDs, driven by music. The Arduino
doesn't listen to anything itself: the computer runs `scripts/audio_stream.py`,
splits the sound into 7 frequency bands and sends the levels over USB. The
Arduino turns those levels into light.

## Wiring

| Strip | Arduino |
|---|---|
| DIN | pin 6, through a 330 ohm resistor |
| 5V | 5V |
| GND | GND |

A 470 uF capacitor across 5V and GND at the strip is a good idea (but not required).
To use another pin or a different number of LEDs, change `LED_PIN` or `LED_COUNT` in `config.h`.
(Beat Ripples needs an even number of LEDs.)

## Running it

1. Upload the program: `pio run -d firmware -e animations -t upload`
2. Start the music. Pick an animation with `--animation`:

```
python scripts/audio_stream.py --port /dev/cu.usbserial-A70064SK --file audio/melody7.wav --animation 2
```

You can also switch while it runs: open a serial monitor (`pio device monitor -d firmware`)
and type `1`, `2` or `3`. (Close the monitor before you run `audio_stream.py`, because only
one program can use the port at a time.)

## The animations

### 1. VU meter (`vu_meter.h`)

A bar that grows with the volume. Its colour comes from all 7 bands: each band pushes
the hue round the colour wheel, and higher bands push harder. Quiet is red, and as
the music gets louder (or higher in pitch) the colour moves through yellow, green,
cyan, blue and violet.

Try changing: `VU_HUE_PER_UNIT` (how quickly the colours change), `VU_FALL_SPEED`
(how slowly the bar falls).

### 2. Shooting lights (`shooting_lights.h`)

A light shoots along the strip whenever a band gets louder than a threshold. Once it
has fired, it has to wait before it can fire again. The colour is a mix of the
colours of all the bands that were over the threshold: bass (red) plus treble
(violet) gives purple.

Try changing: `SHOT_THRESHOLD` (lower = fires more often), `SHOT_COOLDOWN_MS` (the
wait), `SHOT_STEP_MS` (speed), `SHOT_TRAIL` (length of the tail).

### 3. Beat ripples (`beat_ripples.h`)

On every drum beat a ripple spreads out from the middle of the strip, each in a new
colour. A beat is a sudden jump in the bass compared with its recent average. High notes
add white-blue sparkles.

Try changing: `BEAT_MARGIN` (lower = detects softer beats), `RIPPLE_STEP_MS` (speed),
`HUE_CHANGE_PER_BEAT` (how different each colour is), `SPARKLE_THRESHOLD`.

## How the code is organised

| File | What it does |
|---|---|
| `main.cpp` | Reads the serial port, turns each line into 7 band levels, runs the chosen animation, and shows it on the strip. No animation code here. |
| `config.h` | Settings shared by all the animations (pin, number of LEDs, brightness, band colours). |
| `vu_meter.h` | Animation 1. |
| `shooting_lights.h` | Animation 2. |
| `beat_ripples.h` | Animation 3. |

Each animation file is laid out the same way: a comment that explains the idea, then the
settings you can change, then the things it remembers between frames, then one function
that draws a frame. `main.cpp` calls that function about 43 times a second, each time with
the newest band levels, so a good way to start learning is to read `vu_meter.h` first.

## Writing your own animation

1. Copy `vu_meter.h` to a new file, for example `my_animation.h`, and rename the functions.
2. In `main.cpp`: `#include` the new file, add a name and number next to the others, add a `case` in
   `drawFrame()` and in `chooseAnimation()`, call your reset function in `chooseAnimation()`, and
   increase `ANIMATION_COUNT`.
3. Also add the new number to `choices` for `--animation` in `scripts/audio_stream.py`.

An animation gets `levels[0]` to `levels[6]` (0 to 255, bass to treble) and draws with
`strip.setPixelColor(led, colour)`. `main.cpp` shows the result.
