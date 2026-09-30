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
            'bin_max_calibration' : [2402788.39313636, 4218139.305255711, 2547538.548324541, 1369974.408750654, 525360.842142844, 161615.50320851896, 98677.28562918777] #odroic
            # 'bin_max_calibration' : [57.166831055783405, 143.45927914460194, 108.30046088176863, 126.65308397404952, 105.80798720353737, 46.998874956479725, 20.175332649576898]
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
        self.run_fft(data,fftsize)
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

    # python -m core.testwav -f=sounds/DaftPunk.wav -d=True -s=2048 

    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--file", type=str, help="file path and name of wav file.")
    parser.add_argument("-d", "--debug", type=str, help="set true to print to terminal.")
    parser.add_argument("-s", "--datasize", type=int, help="data size/window, default 2048.")
    parser.add_argument("-m", "--mic", type=str, help="set true to print to use microphone input.")
    args = parser.parse_args()


    if args.datasize != None: data_size = args.datasize
    else: data_size = 2048

    if args.file != None: fname = args.file
    else: fname = "../sounds/test.wav"
    frate = 44100.0

    if args.debug != None: debug = True
    else: debug = False

    if args.mic != None:
        mic=True
    else: mic = None

    mic = False

    fft = Fft(datasize=data_size,mic=mic,debug=debug,fname=fname,frate=44100.0, output=True)


    fft.run()

    while True:
        pass

    exit()