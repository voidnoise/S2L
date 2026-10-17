# Animations (FastLED version)

Three light animations (plus an empty one to write yourself) for a strip of 12 WS2812 LEDs,
driven by music. This is the same as the `animations` program, but written with the **FastLED**
library. It is meant to be the
easiest one to read, so start here if you are new to this.

The Arduino doesn't listen to anything itself. The computer runs `scripts/audio_stream.py`,
splits the sound into 7 frequency bands, and sends the levels over USB. The Arduino turns
those levels into light.

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

1. Upload the program: `pio run -d firmware -e animations_fastled -t upload`
2. Start the music. Pick an animation with `--animation`:

```
python scripts/audio_stream.py --port /dev/cu.usbserial-A70064SK --file audio/melody7.wav --animation 2
```

You can also switch while it runs: open a serial monitor (`pio device monitor -d firmware`)
and type `1`, `2`, `3` or `4`. (Close the monitor before you run `audio_stream.py`, because only
one program can use the port at a time.)

## FastLED in 60 seconds

You only need to know five things to read the animations.

| Word | What it is |
|---|---|
| `leds[]` | A list with one colour per LED. `leds[3] = CRGB::Red;` makes LED number 3 red. The first LED is number 0. |
| `CRGB` | A colour made from Red, Green and Blue amounts, 0 to 255 each. `CRGB(255, 0, 0)` is red. `CRGB::Black` is off. |
| `CHSV` | A colour made from Hue, Saturation and Value (brightness). The hue is a place on the colour wheel (0 = red, 96 = green, 160 = blue), so changing only the hue changes only the colour. |
| `FastLED.show()` | Sends `leds[]` to the real LEDs. Nothing lights up until this is called. `main.cpp` calls it once per frame. |
| `fadeToBlackBy(leds, count, amount)` | Makes every LED a bit darker. Do it every frame and the old lights fade away behind the new ones. That is how the trails work. |

Two more you will see: `mix += colour` adds colours like shining torches together (red + green =
yellow), and `colour.nscale8(brightness)` dims a colour.

## The animations

### 1. VU meter (`vu_meter.h`)

A bar that grows with the volume. Its colour comes from all 7 bands: each band pushes
the hue round the colour wheel, and higher bands push harder. Quiet is red, and as
the music gets louder (or higher in pitch) the colour moves through yellow, green,
aqua, blue and purple.

Try changing: `VU_FALL` (higher = the bar falls more slowly), `VU_COLOUR_SPEED`
(higher = the colour reacts faster).

### 2. Shooting lights (`shooting_lights.h`)

A light shoots along the strip whenever a band gets louder than a threshold. Once it
has fired, it has to wait before it can fire again. The colour is a mix of the
colours of all the bands that were over the threshold.

Try changing: `SHOT_THRESHOLD` (lower = fires more often), `SHOT_COOLDOWN_MS` (the
wait), `SHOT_STEP_MS` (speed), `SHOT_FADE` (higher = shorter trail).

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
| `config.h` | Settings shared by all the animations (pin, number of LEDs, brightness, band colours), and the `leds[]` list. |
| `vu_meter.h` | Animation 1. |
| `shooting_lights.h` | Animation 2. |
| `beat_ripples.h` | Animation 3. |
| `custom.h` | Animation 4: a template to write your own. |

Each animation file is laid out the same way: a comment that explains the idea, then the
settings you can change, then the things it remembers between frames, then one function
that draws a frame. `main.cpp` calls that function about 43 times a second, each time with
the newest band levels, so a good way to start learning is to read `vu_meter.h` first.

## Writing your own animation

Animation 4 is a template for you. Open `custom.h`: it is already wired in, so you only
need to edit that one file.

1. Upload the program, then run `audio_stream.py` with `--animation 4`. You will see the whole
   strip pulse green with the bass. That is the starting point.
2. Change the code inside `custom()`. The comments in the file list everything you can use
   and some ideas to try.
3. Upload again to see your changes.

An animation gets `levels[0]` to `levels[6]` (0 to 255, bass to treble) and the time `now`,
and draws by setting `leds[...]`. `main.cpp` shows the result.

Want a fifth animation as well? Copy `custom.h` to a new file, rename the two functions, then in
`main.cpp` add an `#include`, a name and number next to `ANIMATION_CUSTOM`, a `case` in
`drawFrame()` and `chooseAnimation()`, a reset call in `chooseAnimation()`, and raise
`ANIMATION_COUNT`. Also add the new number to `choices` for `--animation` in
`scripts/audio_stream.py`.

## Differences from the NeoPixel version

- Colours are `CRGB` and `CHSV` values, not packed numbers, so there is no bit-shifting.
- Hues run 0 to 255 (not 0 to 65535), and the band colours have names (`HUE_RED`, `HUE_ORANGE`...).
- Trails are made by dimming the whole strip each frame, rather than drawing each trail LED by hand.
- There is no gamma correction, so dim colours look a little brighter than in the NeoPixel version.
- It uses a little more memory: about 490 bytes of RAM and 8.8 KB of flash, against 375 bytes and 8 KB.
