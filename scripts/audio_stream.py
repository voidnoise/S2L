"""Stream FFT band levels over serial to drive Arduino LED animations.

Reads audio from a WAV file or the default microphone, computes an FFT per
block, buckets it into 7 frequency bands via fft.Fft, and writes each
frame's band levels to a serial port as a CSV line:

    12,45,200,88,3,0,9\n

Each value is auto-gained to the 0-255 range using a slowly decaying
running max per band, so no manual calibration is needed before a demo.
"""
import argparse
import sys
import threading
import time

import serial
import sounddevice as sd
import soundfile as sf

from fft import Fft

BAND_COUNT = 7
GAIN_DECAY = 0.999  # per-block decay of each band's running max (auto-gain)
GAIN_FLOOR = 1.0    # minimum running max, avoids divide-by-near-zero on silence


def normalize_bands(raw_bands, running_max):
    """Scale each band to 0-255 against a slowly decaying running max."""
    levels = []
    for i, value in enumerate(raw_bands):
        running_max[i] = max(value, running_max[i] * GAIN_DECAY, GAIN_FLOOR)
        level = 255.0 * value / running_max[i]
        levels.append(int(max(0, min(255, level))))
    return levels


def process_block(fft_engine, block, samplerate, running_max):
    mono = block if block.ndim == 1 else block.mean(axis=1)
    fft_engine.run_fft(mono, len(mono), samplerate)
    if not fft_engine.configured:
        fft_engine.configure_bin_parameters()
    fft_engine.splitLevels()
    return normalize_bands(fft_engine.stats['bin_values'], running_max)


def send_frame(ser, levels):
    line = ",".join(str(v) for v in levels) + "\n"
    ser.write(line.encode("ascii"))


def print_replies(ser, stop):
    """Print every line the Arduino sends back until `stop` is set."""
    while not stop.is_set():
        try:
            line = ser.readline()  # returns b"" after the port timeout
        except serial.SerialException:
            return
        if line:
            print("arduino:", line.decode("ascii", "replace").rstrip())


def stream_file(path, ser, blocksize, running_max, play_audio):
    data, samplerate = sf.read(path, dtype="float32", always_2d=False)
    fft_engine = Fft(frate=samplerate)

    if play_audio:
        sd.play(data, samplerate)

    n_blocks = len(data) // blocksize
    block_duration = blocksize / samplerate
    next_time = time.perf_counter()

    print(f"Streaming {path} ({samplerate:.0f} Hz, {n_blocks} blocks)...")

    for i in range(n_blocks):
        block = data[i * blocksize:(i + 1) * blocksize]
        levels = process_block(fft_engine, block, samplerate, running_max)
        send_frame(ser, levels)

        next_time += block_duration
        sleep_for = next_time - time.perf_counter()
        if sleep_for > 0:
            time.sleep(sleep_for)

    if play_audio:
        sd.wait()

    print("Done.")


def stream_mic(ser, blocksize, samplerate, running_max):
    fft_engine = Fft(frate=samplerate)

    def callback(indata, frames, time_info, status):
        if status:
            print(status, file=sys.stderr)
        levels = process_block(fft_engine, indata[:, 0], samplerate, running_max)
        send_frame(ser, levels)

    print(f"Streaming microphone input at {samplerate:.0f} Hz. Press Ctrl+C to stop.")
    with sd.InputStream(channels=1, samplerate=samplerate,
                         blocksize=blocksize, callback=callback):
        while True:
            time.sleep(0.1)


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                      formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--file", type=str,
                         help="WAV file to analyze. Omit to use the microphone.")
    parser.add_argument("--port", type=str, required=True,
                         help="Serial port, e.g. COM5 or /dev/ttyUSB0.")
    parser.add_argument("--baud", type=int, default=115200,
                         help="Serial baud rate (default 115200).")
    parser.add_argument("--blocksize", type=int, default=1024,
                         help="Samples per FFT window (default 1024).")
    parser.add_argument("--samplerate", type=float, default=44100.0,
                         help="Mic sample rate in Hz (default 44100). Ignored for --file.")
    parser.add_argument("--no-playback", action="store_true",
                         help="Don't play the file's audio out loud while streaming.")
    parser.add_argument("--animation", type=int, choices=[1, 2, 3],
                         help="Pick the animation on firmware that supports it (the 'animations' program): "
                              "1 = vu meter, 2 = shooting lights, 3 = beat ripples.")
    parser.add_argument("--echo", action="store_true",
                         help="Print the replies the Arduino sends back (for testing the firmware).")
    args = parser.parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=1)
    time.sleep(2)  # let the Arduino finish its reset-on-connect before we send data

    stop_replies = threading.Event()
    if args.echo:
        threading.Thread(target=print_replies, args=(ser, stop_replies), daemon=True).start()

    if args.animation:
        ser.write(f"{args.animation}\n".encode("ascii"))

    running_max = [GAIN_FLOOR] * BAND_COUNT

    try:
        if args.file:
            stream_file(args.file, ser, args.blocksize, running_max,
                        play_audio=not args.no_playback)
        else:
            stream_mic(ser, args.blocksize, args.samplerate, running_max)
    except KeyboardInterrupt:
        print("\nStopped.")
    finally:
        if args.echo:
            time.sleep(0.3)  # let the replies to the last frames arrive
        stop_replies.set()
        ser.close()


if __name__ == "__main__":
    main()
