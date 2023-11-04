# baby-cry-detector
* Bachelor Thesis: 
[Bachelor Thesis](Baby Sleep Radio Monitor.pdf)


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


# abstract
The Baby Radio Monitoring System (BRMS) is a device designed to address Sudden Infant Death Syndrome (SIDS), the leading cause of death for one-month- to one-year-olds. The design integrates a room temper- ature and sleeping position monitor since both factors are thought to increase the risk of SIDS, according to the Centers for Disease Control and Prevention. The BRMS is furthermore capable of detecting whether a baby is crying. All temperature, position and audio data is captured at the baby’s side, processed by a microcontroller, and then sent wirelessly to the parent’s side.
The temperature monitor was built using a thermistor as its core component. An STM8S105K4 microcon- troller closely measures the voltage across this temperature-dependent resistor. This way, the BRMS is capable of tracking the temperature in the baby’s room with up to 0.5 ◦C precision.
The position monitor was built using a Hall sensor as its core component. In addition, a magnet is placed onto the baby’s chest which interacts with the Hall sensor. The voltage across the sensor changes according to the magnetic field strength applied to it. Again, the STM8 records this voltage and can hence determine in which position the baby is sleeping.
For the cry detector, audio signals arriving from a microphone were processed with an ESP32-WROOM- 32 microcontroller using digital processing techniques. The cry detector achieved an 88.4% accuracy in distinguishing infant cries, showing the effectiveness of the cry detection algorithm.
Whenever crying, flipping or unsafe temperature is detected at the infant’s side of the monitor, the parent is warned through an LCD display present at their side. Hence, the BRMS is an effective baby monitoring system.


Sound processing:

In order for sound detection to be meaningful and useful in the context of baby monitoring, our system should be able to differentiate between crying sounds, babbling sounds, and noise inside the room where the baby lies so that the care person(s) using the monitor can be alerted of the state of the baby in real-time. One way to make that distinction is through fundamental frequency and the frequency spectrum of the harmonics.

Research shows that the mean fundamental frequency related to the speech signal of a baby from birth to 1 year of age ranges from 300 to 600 Hz and that the lower third of that range (frequencies between 300 and 400 Hz) is associated with babbling sounds whereas the other 2 thirds of that range (frequencies between 400 and 600 Hz) relates to crying sounds. According to multiple scientific papers, there is a notable amplitude difference between the frequency spectrum of the signal emitted when a baby cries and when a baby is babbling. Therefore, our monitor should be able to detect speech signals with the amplitude and the frequency spectrum. The exact range of the amplitude differences will need to be studied and verified in later stage. Signals with fundamental frequencies in between 300 and 400 Hz can be classified as babbling, signals with fundamental frequencies between 400 and 600 Hz and amplitude 20dB larger than the ones of babbling can be addressed as crying and all other perceived frequencies (i.e. those of adult males, adult females, children, etc.), given that they lie outside the range of interest, as noise. The frequency band which does not lie in the integer multiples of the interest fundamental frequency will be disregarded as noise.

The sound detection system will collect the sound input with a TDK InvenSense MMICT4086-00-908 microphone connected to an analog-to-digital signal path that includes a four-stage signal processing system, which is consist of analog gain/signal conditioning, anti-aliasing filtering, analog-to-digital conversion (ADC) and digital signal processing. Sound detection of crying and utterance of a baby will be differentiated with respective thresholds. A crying frequency spectrum with its amplitude reaching our threshold will send an alert to the master node, in turns triggering of the rocking actuator.

For each stage in the sound detection block diagram, a short description will be listed below.

In the analog gain and signal conditioning stage, a Butterworth bandpass filter and an instrumental amplifier with unity gain will be connected in series to filter out the non-target frequency and to reduce common environmental noise. Frequency below and above the fundamental frequencies and harmonics of an infant can be disregarded by a bandpass filter.

An anti-aliasing low pass filter following the gain/signal conditioning cell is to keep the DC signal out of the spectrum and to reduce out-of-band frequencies that can degrade the analog-to-digital conversion. Anti-aliasing low pass filter is again a Butterworth 6th order low pass filter, with a cut off frequency at 4000Hz.

Analog-to-digital converter cell will transmit a digital representation of the gained, filtered analog signal to a digital processor. ADC of the system will have a bit depth of at least 8 to ensure the quality of a signal magnitude rating system, with a sampling rate of at least 8,000 Hz, up to 44,100Hz. A sampling frequency of 44100Hz is used in this system.

Digital signal processor, in the next stage, consists of two elements, digital filtering and digital signal processing. Digital filtering is to further reduce noise and smooth the signal output from ADC, such as by a digitized Butterworth low pass filter. Digital signal processing, on the other hand, is to generate the frequency spectrum of filtered digital signal for pattern analysis, and further utilize Continuous Fourier Transform, Fast Fourier Transform, or Finite Impulse Response to develop an algorithm that discerns signals of crying and of babbling.

<img width="832" alt="image" src="https://user-images.githubusercontent.com/100540403/209318564-90f98585-b6ed-4794-b138-9b9138894e79.png">
<img width="821" alt="image" src="https://user-images.githubusercontent.com/100540403/209318594-04a261db-ea62-40ff-a078-a2739c597586.png">
<img width="808" alt="image" src="https://user-images.githubusercontent.com/100540403/209318607-6acb1a21-3493-4f66-bdaa-787df5ae96e9.png">



Digital signal processor, in the next stage, consists of two elements, digital filtering and digital signal processing. Digital filtering is to further reduce noise and smooth the signal output from ADC, such as by a digitized Butterworth low pass filter. Digital signal processing, on the other hand, is to generate the frequency spectrum of filtered digital signal for pattern analysis, and further utilize Continuous Fourier Transform, Fast Fourier Transform, or Finite Impulse Response to develop an algorithm that discerns signals of crying and of babbling.




| Supported Targets | ESP32 |
| ----------------- | ----- | 

# ESPNOW Example

(See the README.md file in the upper level 'examples' directory for more information about examples.)

This example shows how to use ESPNOW of wifi. Example does the following steps:

* Start WiFi.
* Initialize ESPNOW.
* Register ESPNOW sending or receiving callback function.
* Add ESPNOW peer information.
* Send and receive ESPNOW data.

This example need at least two ESP devices:

* In order to get the MAC address of the other device, Device1 firstly send broadcast ESPNOW data with 'state' set as 0.
* When Device2 receiving broadcast ESPNOW data from Device1 with 'state' as 0, adds Device1 into the peer list.
  Then start sending broadcast ESPNOW data with 'state' set as 1.
* When Device1 receiving broadcast ESPNOW data with 'state' as 1, compares the local magic number with that in the data.
  If the local one is bigger than that one, stop sending broadcast ESPNOW data and starts sending unicast ESPNOW data to Device2.
* If Device2 receives unicast ESPNOW data, also stop sending broadcast ESPNOW data.

In practice, if the MAC address of the other device is known, it's not required to send/receive broadcast ESPNOW data first,
just add the device into the peer list and send/receive unicast ESPNOW data.

There are a lot of "extras" on top of ESPNOW data, such as type, state, sequence number, CRC and magic in this example. These "extras" are
not required to use ESPNOW. They are only used to make this example to run correctly. However, it is recommended that users add some "extras"
to make ESPNOW data more safe and more reliable.

## How to use example

### Code Initialize
* for reciever:
* in config.h, set macro RECV to 1

### Hardware Required

* A development board with ESP32 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for power supply and programming
* A microphone (with amplifier) and one or two speaker(s) for testing.

The following is the hardware connection:

|hardware|module|GPIO|
|:---:|:---:|:---:|
|Microphone|ADC1_CH0|GPIO35|
|speaker(R)|DAC1|GPIO25|
|speaker(L)|DAC2|GPIO26|

### Configure the project

```
. $HOME/esp/esp-idf/export.sh
idf.py set-target esp32
idf.py menuconfig
```

* Set WiFi mode (station or SoftAP) under Example Configuration Options.
* Set ESPNOW primary master key under Example Configuration Options.
  This parameter must be set to the same value for sending and recving devices.
* Set ESPNOW local master key under Example Configuration Options.
  This parameter must be set to the same value for sending and recving devices.
* Set Channel under Example Configuration Options.
  The sending device and the recving device must be on the same channel.
* Set Send count and Send delay under Example Configuration Options.
* Set Send len under Example Configuration Options.
* Set Enable Long Range Options.
  When this parameter is enabled, the ESP32 device will send data at the PHY rate of 512Kbps or 256Kbps
  then the data can be transmitted over long range between two ESP32 devices.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

Here is the example of ESPNOW receiving device console output.

```
initializing i2s spk
Received 3264 packets in 10 seconds
Received 3392 packets in 10 seconds
Received 3264 packets in 10 seconds
Received 3328 packets in 10 seconds
Received 3200 packets in 10 seconds
Received 3328 packets in 10 seconds
Received 3264 packets in 10 seconds
```

Here is the example of ESPNOW sending device console output.

```
Init transport!
initializing i2s mic
Bytes available in stream buffer: 16384 
peak 1 at frequency 430.000000 Hz with amplitude 2.408174 
peak 2 at frequency 1892.000000 Hz with amplitude 2.283813 
peak 1 at frequency 1505.000000 Hz with amplitude 2.708347 
peak 2 at frequency 301.000000 Hz with amplitude 2.529614 
peak 1 at frequency 645.000000 Hz with amplitude 3.665064 
peak 2 at frequency 387.000000 Hz with amplitude 2.986069 
cry detected at f0 387.000000 Hz with amplitude 3.617141 and f2 1376.000000 with amplitude 2.395091
cry detected at f0 387.000000 Hz with amplitude 2.040233 and f2 1376.000000 with amplitude 1.812771
cry detected at f0 387.000000 Hz with amplitude 2.650546 and f2 1376.000000 with amplitude 2.321801
cry detected at f0 387.000000 Hz with amplitude 3.742941 and f2 1376.000000 with amplitude 2.673561
cry detected at f0 387.000000 Hz with amplitude 3.999676 and f2 1376.000000 with amplitude 2.550040
cry detected at f0 387.000000 Hz with amplitude 2.740722 and f2 1376.000000 with amplitude 1.979644
```

## Troubleshooting

If ESPNOW data can not be received from another device, maybe the two devices are not
on the same channel or the primary key and local key are different.

In real application, if the receiving device is in station mode only and it connects to an AP,
modem sleep should be disabled. Otherwise, it may fail to revceive ESPNOW data from other devices.


## Steps to merging branch into master
* git fetch.
* git rebase origin/master.
* git checkout master.
* git pull origin master.
* git merge test.
* git push origin master.
