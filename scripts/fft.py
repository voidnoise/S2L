# coding: utf8

from __future__ import absolute_import
from __future__ import print_function
import copy
import numpy as np
import os

# python -m core.testwav -f=sounds/DaftPunk.wav -d=True -s=2048 

def translate(value, leftMin, leftMax, rightMin, rightMax):
    # Figure out how 'wide' each range is
    leftSpan = leftMax - leftMin
    rightSpan = rightMax - rightMin

    if leftSpan > 0.0:
        # Convert the left range into a 0-1 range (float)
        valueScaled = float(value - leftMin) / float(leftSpan)

        # Convert the 0-1 range into a value in the right range.
        return rightMin + (valueScaled * rightSpan) 
    else:
        return 0.0

class Fft():
    '''
    Class to handle FFT processing of audio data.
    
    Some examples of frequency ranges for common instruments and vocals
    
    Low-End:
        Kick Drum : 60 Hz to 100 Hz
        Bass Guitar : 41 Hz to 350 Hz
    Low-Mid Presence : 65 Hz to 500 Hz
        Male Volcals : 85 Hz to 500 Hz
        Cello : 65 Hz to 880 Hz
    Mid-Range -
        Saxaphone : 130 Hz to 700 Hz
        Snare Drum : 200 Hz to 400 Hz
        Female Vocals : 165 Hz to 1 kHz
    Upper-Mid - 
        Acoustic Guitar : 80 Hz to 1.2 kHz 
        Electric Guitar : 80 Hz to 5 kHz
        Violib : 196 Hz to 3.5 kHz
    High-Frequency - 
        Cymbals : 300 Hz to 17 kHz
    '''


    stats = {
            'raw_power' : 0.0,
            'fft_out'   : None,
            'fft_out_simple' : None,
            'frequencies' : None,
            'freq_bins' : [63, 160, 400, 1000, 2500, 6250, 16000], 
            'freq_bins_max' : [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
            'freq_bins_max_normalized' : [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
            'freq_bin_indexes' : [[0,0], [0,0], [0,0], [0,0], [0,0], [0,0], [0,0]],
            'freq_bin_range' : [0, 0, 0, 0, 0, 0, 0],
            'bandwidth_factor' : 2.5,
            'bin_values' : [0, 0, 0, 0, 0, 0, 0],
            'bin_values_normalized' : [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0],
            'dominant_freq' : 0.0,
            'saturation' : 1024,
            'bin_max_calibration' : [2402788.39313636, 4218139.305255711, 2547538.548324541, 1369974.408750654, 525360.842142844, 161615.50320851896, 98677.28562918777] 
    }

    def __init__(
                self,  
                frate=44100.0,
                saturation_point=1024
                ):

        print("setup args")

        # self.stats is a class-level dict; deep copy it here so each Fft
        # instance gets its own state instead of sharing one across instances.
        self.stats = copy.deepcopy(Fft.stats)

        self.frate=frate


        # cellFRange = self.frate / self.datasize

        self.stats['saturation'] = saturation_point
        self.configured = False

    def configure_fft(self, data, fftsize):
        self.run_fft(data,fftsize,self.frate)
        self.configure_bin_parameters()
        self.configured = True

    ##
    # @brief Identifies the strongest frequency in the fft results.
    #    
    # Keyword arguments:
    # @param fftData -- y values from fft
    # @param return -- Dominant frequency in Hz
    def getDominantF(self):
        idx = np.argmax(self.stats['fft_out']) # return index of maximum value
        freq = self.stats['frequencies'][idx]
        # print('f',idx,freq)
        self.stats['dominant_freq'] = freq


    ##
    # @brief Splits fft data in to regions for approximate instrument detection.    
    def splitLevels(self):

        output = []

        for i in range(len(self.stats['freq_bins'])):

            start_index = self.stats['freq_bin_indexes'][i][0]
            end_index = self.stats['freq_bin_indexes'][i][1]
            level = self.stats['fft_out'][start_index:end_index].sum() / (self.stats['freq_bin_range'][i])

            output.append(level)

        self.stats['bin_values'] = output


    def normalize_bin_values(self):
        ''' multiplies raw fft values by gain, and clamps between 0 & saturation point'''
        for i in range(len(self.stats['bin_values'])):
            self.stats['bin_values_normalized'][i] = 1024*self.stats['bin_values'][i]/self.stats['bin_max_calibration'][i]


    def run_fft(self, data, fftsize, samplerate):

        # Convert raw sound data to Numpy array
        # fmt = "%dH"%(len(data)/2)
        # print(fmt)
        # data = struct.unpack(fmt, data)
        # data = np.array(data, dtype='h')   

        # Check if data size is big enough
        # if len(data) < self.datasize:
        #     raise Exception('Fft error: data less than data size')
        #     return 0

        self.fftsize = fftsize
        self.stats['fft_out'], self.stats['frequencies'] =  self.do_fft(data,samplerate,fftsize)
        # print(self.stats['fft_out'], self.stats['frequencies'])
       

    '''
    applies fft to return the levels for each frequency cell
    '''
    def do_fft(self, data,samplerate, fftsize=1024):
        ''' this calls fft, then splits it in two ready to add both sides 
            together '''
        fft_raw  = np.fft.fft(data,fftsize)#,n=fftsize) 
        fftfreq = np.fft.fftfreq(len(fft_raw))

        ''' correct and align frequency spectrum  '''
        fftfreq = np.abs(fftfreq)
        leftF,rightF = np.split(fftfreq,2) 
        fftfreq=np.add(leftF,rightF[::-1])
        
        fftfreq = fftfreq * (samplerate/2)
        

        ''' correct and alighn fft output '''
        w = np.abs(fft_raw)
        left,right=np.split( w ,2)
        ''' add both sides of fft together but flip one to match other '''
        ffty=np.add(left,right[::-1])


        return ffty, fftfreq


    # def sum_bin_power(self):

    def get_total_power(self, data):
        if isinstance(data, np.ndarray):
            self.stats['raw_power'] = data.sum()
            return self.stats['raw_power']
        else:
            print('Error: get_total_power data argument is not an numpy ndarray')
            return None
        pass

    ##
    # @brief Works out bin start and end f, and number of cells it overs.
    # 
    # Itterates through the stats['freq_bins'], determins start and stop frequency,
    # then associated array cells and range between them.   
    def configure_bin_parameters(self):

        for i in range(len(self.stats['freq_bins'])):

            freq = self.stats['freq_bins'][i]

            start_f = int(freq / self.stats['bandwidth_factor'])
            end_f = int(freq * self.stats['bandwidth_factor'])

            self.stats['freq_bin_indexes'][i][0] = self.get_closest_freq_index(start_f, self.stats['frequencies'], bias='left')
            self.stats['freq_bin_indexes'][i][1] = self.get_closest_freq_index(end_f, self.stats['frequencies'], bias='right')

            self.stats['freq_bin_range'][i] = self.stats['freq_bin_indexes'][i][1] - self.stats['freq_bin_indexes'][i][0]
        self.configured = True



    def set_freq_bins_max(self):
        for i in range(len(self.stats['bin_values'])):
            if self.stats['bin_values'][i] > self.stats['freq_bins_max'][i]:
                self.stats['freq_bins_max'][i] = self.stats['bin_values'][i]
            # print self.stats['freq_bins_max'][i] 
       

    def set_freq_bins_normalized_max(self):
        for i in range(len(self.stats['bin_values'])):
            if self.stats['bin_values_normalized'][i] > self.stats['freq_bins_max_normalized'][i]:
                self.stats['freq_bins_max_normalized'][i] = self.stats['bin_values_normalized'][i]

    ##
    # @brief Search a sorted list of frequencies to find closest index for target
    #       frequency.
    #    
    # Keyword arguments:
    # @param freq -- the frequency to search for cloests index
    # @param freq_list -- freq_list must a sorted list of the fft frequencies
    # @param bias -- if closest freq is not exact, either 'left' or 'right' cell
    #       is returned.
    # @return -- index for closest frequency.
    def get_closest_freq_index(self, freq, data, bias='left'):
        return np.searchsorted(data, freq, side=bias)


        

    def calcVolume(self, data, debug=False):
        # work out rms (amplitude of waveform)
        rms_val = np.sqrt(np.mean(data**2)) / 10000.0
        if not np.isnan(rms_val): 
            if debug:
                # print rms_val
                os.system('clear')
                s = ""
                for i in range(int(rms_val/1.0)):
                    s += 'x'
                print(s)
            return rms_val
        else:
            return 0.0

    def clamp(self, n, minn, maxn):
        return max(min(maxn,n),minn)




if __name__ == '__main__':

    # python fft.py --file ../audio/melody7.wav --debug true --datasize 2048
    # python fft.py --file ../audio/melody7.wav --debug true --noplay   (process as fast as possible, no sound)
    # python fft.py --mic true --debug true

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file", type=str, help="file path and name of wav file.")
    parser.add_argument("-d", "--debug", type=str, help="set true to print each window to terminal.")
    parser.add_argument("-s", "--datasize", type=int, help="data size/window, default 2048.")
    parser.add_argument("-m", "--mic", type=str, help="set true to use microphone input.")
    parser.add_argument("-n", "--noplay", action="store_true", help="don't play the file, just process it as fast as possible.")
    args = parser.parse_args()

    def is_true(value):
        return value is not None and value.lower() in ('true', '1', 'yes', 'y')

    data_size = args.datasize if args.datasize is not None else 2048
    if data_size < 2 or data_size % 2:
        parser.error("--datasize must be an even number (the fft output is split in half)")

    debug = is_true(args.debug)
    mic = is_true(args.mic)
    fname = args.file if args.file is not None else "../audio/test.wav"

    def process_window(fft, window):
        """ run one window of samples through the fft and update the stats """
        fft.run_fft(window, data_size, fft.frate)
        fft.getDominantF()
        fft.splitLevels()
        fft.normalize_bin_values()
        fft.set_freq_bins_max()
        fft.set_freq_bins_normalized_max()
        if debug:
            print("dominant: %7.1f Hz | bins: %s" % (
                fft.stats['dominant_freq'],
                " ".join("%6.1f" % v for v in fft.stats['bin_values_normalized'])))

    def print_summary(fft, windows):
        print("windows processed: %d" % windows)
        print("bin centres (Hz):  %s" % fft.stats['freq_bins'])
        print("max normalized:    %s" % [round(float(v), 1) for v in fft.stats['freq_bins_max_normalized']])

    if mic:
        import sounddevice as sd

        fft = Fft(frate=44100.0)
        windows = 0
        print("listening on the default input, ctrl-c to stop")
        try:
            with sd.InputStream(samplerate=fft.frate, channels=1, dtype='int16', blocksize=data_size) as stream:
                while True:
                    block, _ = stream.read(data_size)
                    window = block[:, 0]
                    if not fft.configured:
                        fft.run_fft(window, data_size, fft.frate)
                        fft.configure_bin_parameters()
                    process_window(fft, window)
                    windows += 1
        except KeyboardInterrupt:
            print()
            print_summary(fft, windows)
    else:
        import soundfile as sf

        if not os.path.isfile(fname):
            parser.error("file not found: %s" % fname)

        # int16 keeps the samples on the scale bin_max_calibration was measured with
        audio, samplerate = sf.read(fname, dtype='int16', always_2d=True)
        samples = audio.mean(axis=1)  # mix down to mono for the fft

        fft = Fft(frate=float(samplerate))

        windows = 0
        n_windows = len(samples) // data_size
        if n_windows == 0:
            parser.error("%s is shorter than one window (%d samples)" % (fname, data_size))

        def handle(window):
            if not fft.configured:
                fft.run_fft(window, data_size, fft.frate)
                fft.configure_bin_parameters()
            process_window(fft, window)

        if args.noplay:
            for i in range(n_windows):
                handle(samples[i * data_size:(i + 1) * data_size])
                windows += 1
        else:
            import queue
            import time
            import sounddevice as sd

            # The audio callback hands one window of samples to the sound card and
            # queues the same window for the fft, so the values shown follow the
            # audio clock. The fft runs here in the main thread, not in the callback.
            q = queue.Queue()
            position = [0]

            def callback(outdata, frames, time_info, status):
                start = position[0]
                chunk = audio[start:start + frames]
                outdata[:len(chunk)] = chunk
                outdata[len(chunk):] = 0
                position[0] = start + frames
                if len(chunk) == frames:
                    q.put(samples[start:start + frames])
                else:
                    q.put(None)  # end of file
                    raise sd.CallbackStop

            print("playing %s, ctrl-c to stop" % fname)
            try:
                with sd.OutputStream(samplerate=samplerate, channels=audio.shape[1], dtype='int16',
                                     blocksize=data_size, callback=callback) as stream:
                    while True:
                        window = q.get()
                        if window is None:
                            break
                        handle(window)
                        windows += 1
                    time.sleep(stream.latency)  # let the last block finish playing
            except KeyboardInterrupt:
                print()

        print_summary(fft, windows)
