% This file calculates the Magnitude Spectrum and Power Spectrum Density of
% the speech signal


close all; clear all;

% %% Read signal
% wavfile = 'big_dogs_can_be_dangerous_slide6.wav';
% [data,fs] = audioread(wavfile);

%% or record data
fs = 44100;
rec = audiorecorder(fs, 16, 1);
% start recording
disp('start speaking');
record(rec, 5); % will record for 5sec. for this command pause needs to be added
recordblocking(rec, 2);

% stop recording
disp('Stop recording');

% get data
data = getaudiodata(rec);

%% take fourier transform and plot single sided spectrum
l = length(data);
NFFT = 2^nextpow2(l);   % the next power of 2 that's greater than our sample data length
f = fs/2*linspace(0,1,NFFT/2+1);    % generates NFFT/2+1 points between 0 and 1
xf = abs(fft(data, NFFT));  %fft

subplot(2,1,1);plot(data);title('Input Laughing Signal');
xlabel('time (sec)');
subplot(2,1,2);plot(f, xf(1:NFFT/2+1));
xlabel('frequency (Hz)');
title('Single Sided Spectrum of the Laughing Signal');

%% ZCR to distinct voice and unvoice signal
rate = zerocrossrate(xf);

%% PSD with pwelch, default window Hanning
pwelch(data,[],[],[],fs);

%%
% plot PSD, power spectrum density (using welch method)
h = spectrum.welch; % create welch spectrum object
d = psd(h, data,'Fs', fs);
figure;
plot(d);

%% Plot PSD (From FFT)
% single sided PSD
Hpsd = dspdata.psd(xf(1:length(xf)/2),'Fs',fs);
figure;plot(Hpsd);

% Periodogram Based PSD estimate
figure;
[psdestx,Fxx] = periodogram(data,rectwin(length(data)),length(data),fs);
plot(Fxx,10*log10(psdestx)); grid on;
xlabel('Hz'); ylabel('Power/Frequency (dB/Hz)');
title('Periodogram Power Spectral Density Estimate');

