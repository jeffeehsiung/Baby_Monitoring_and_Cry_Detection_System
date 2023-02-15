# baby-cry-detector
repository for my bachelor thesis project: a baby cry detector
languages used:
  1. Matlab for audio signal ananlysis, filter design, async signal processing simulatoin. 
    Tasks involved such as extracting the signal length and sampling frequency of audio files from online open-resources databse , filter design and transfer function calculation, bode plot, audio feature extraction, fft, dtft, welch psd, ZCR, STE
    Result: baby audio samples can be categorized into cooing, babbling, laughing, and crying where its freuqnecy band, pitch, energy, and ZCR can be clearly identified to recongizned the type of the signal based on above matlab analysis.
  2. LabVIEW for real-time signla analysing and processing simulation and NRF24 interfacing and data transmission simulations
    Tasks involved such as interfacing MIC with LabVIEW, build waveform, zero padding, windowing,FFT, and PSD to extract audio features for cry detection.
    Result: tested with five baby babbling, cooing, laughing and crying signals. identifications were all succesfull. yet it may not success with samples that has not been analyzied yet and to be honest some baby shows a slightly different pattern in crying than other babies.
  3. C in microcontorller esp32, STM32, and NRF24 for practical implementation.
    Tasks: ongoing


<img width="792" alt="image" src="https://user-images.githubusercontent.com/100540403/209315718-fb5747c2-73e3-40da-b5c8-a8222c0cca0c.png">

# Sound processing:

In order for sound detection to be meaningful and useful in the context of baby monitoring, our system should be able to differentiate between crying sounds, babbling sounds, and noise inside the room where the baby lies so that the care person(s) using the monitor can be alerted of the state of the baby in real-time. One way to make that distinction is through fundamental frequency and the frequency spectrum of the harmonics.

Research shows that the mean fundamental frequency related to the speech signal of a baby from birth to 1 year of age ranges from 300 to 600 Hz and that the lower third of that range (frequencies between 300 and 400 Hz) is associated with babbling sounds whereas the other 2 thirds of that range (frequencies between 400 and 600 Hz) relates to crying sounds. According to multiple scientific papers, there is a notable amplitude difference between the frequency spectrum of the signal emitted when a baby cries and when a baby is babbling. Therefore, our monitor should be able to detect speech signals with the amplitude and the frequency spectrum. The exact range of the amplitude differences will need to be studied and verified in later stage. Signals with fundamental frequencies in between 300 and 400 Hz can be classified as babbling, signals with fundamental frequencies between 400 and 600 Hz and amplitude 20dB larger than the ones of babbling can be addressed as crying and all other perceived frequencies (i.e. those of adult males, adult females, children, etc.), given that they lie outside the range of interest, as noise. The frequency band which does not lie in the integer multiples of the interest fundamental frequency will be disregarded as noise.

The sound detection system will collect the sound input with a TDK InvenSense MMICT4086-00-908 microphone connected to an analog-to-digital signal path that includes a four-stage signal processing system, which is consist of analog gain/signal conditioning, anti-aliasing filtering, analog-to-digital conversion (ADC) and digital signal processing. Sound detection of crying and utterance of a baby will be differentiated with respective thresholds. A crying frequency spectrum with its amplitude reaching our threshold will send an alert to the master node, in turns triggering of the rocking actuator.

For each stage in the sound detection block diagram, a short description will be listed below.

In the analog gain and signal conditioning stage, a Bessel or Butterworth bandpass filter and an instrumental amplifier with unity gain will be connected in series to filter out the non-target frequency and to reduce common environmental noise. Frequency below and above the fundamental frequencies and harmonics of an infant can be disregarded by a bandpass filter.

An anti-aliasing low pass filter following the gain/signal conditioning cell is to keep the DC signal out of the spectrum and to reduce out-of-band frequencies that can degrade the analog-to-digital conversion. Anti-aliasing low pass filter will be implemented either in Chebyshev-I or a Bessel low pass filter, with a cut off frequency equal to half of the ADC sampling frequency,

Analog-to-digital converter cell will transmit a digital representation of the gained, filtered analog signal to a digital processor. ADC of the system will have a bit depth of at least 8 to ensure the quality of a signal magnitude rating system, with a sampling rate of at least 8,000 Hz, up to 16,000Hz.

Digital signal processor, in the next stage, consists of two elements, digital filtering and digital signal processing. Digital filtering is to further reduce noise and smooth the signal output from ADC, such as by a digitized Butterworth low pass filter. Digital signal processing, on the other hand, is to generate the frequency spectrum of filtered digital signal for pattern analysis, and further utilize Continuous Fourier Transform, Fast Fourier Transform, or Finite Impulse Response to develop an algorithm that discerns signals of crying and of babbling.

<img width="832" alt="image" src="https://user-images.githubusercontent.com/100540403/209318564-90f98585-b6ed-4794-b138-9b9138894e79.png">
<img width="821" alt="image" src="https://user-images.githubusercontent.com/100540403/209318594-04a261db-ea62-40ff-a078-a2739c597586.png">
<img width="808" alt="image" src="https://user-images.githubusercontent.com/100540403/209318607-6acb1a21-3493-4f66-bdaa-787df5ae96e9.png">



Digital signal processor, in the next stage, consists of two elements, digital filtering and digital signal processing. Digital filtering is to further reduce noise and smooth the signal output from ADC, such as by a digitized Butterworth low pass filter. Digital signal processing, on the other hand, is to generate the frequency spectrum of filtered digital signal for pattern analysis, and further utilize Continuous Fourier Transform, Fast Fourier Transform, or Finite Impulse Response to develop an algorithm that discerns signals of crying and of babbling.


# how to use:
1. digitzied signal from ADC output is send to fast fourier transform, whcih is done in fftPeak.c
    signal SAMPLES size (N) in fftPeak.c fft in each frame is 1024, default SAMPLING_RATE is 16,000;
    fftw library is used to achieve faster speed and lower power consumption
    with frequency spectrum calculated in fft, peaks can be found absed on power comparison and are saved into freuqency array
    frequency array contains the f0 and its harmonics
2. vowels and consonants can be distinguish in a cepstrum domain, yet as phonectics recognition is not the main goal for the project,
    we implement a simplied version that only counts the occurance of consonants to avoid false cry detection triggered by closely related vowels

3. to be optimized: 
    Overlap-save fft w/ zero-padded FIR filter implementation
    check the aperture time and sampling rate is safe
4. implement the cry identification algorithm
    by using the same logic demonstrated in labview: identifying the pitch, number of peaks, and the range
5. integrate ZCR and STE into the algorithm as relative parameter features