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
