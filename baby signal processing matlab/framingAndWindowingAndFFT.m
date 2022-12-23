% This file talks about framing, windowing

close all; clear all;

% load sound files
wavfile = 'continuous cry.wav';
[data,fs] = audioread(wavfile);

% take frame of 0.025s (25 msec)
f_d = 0.025; % frame duration
f_size = f_d * fs;

% take one frame
fr = data(2001:2001+f_size); 

% windowing (hamming window)
fr_win = fr .* hamming(length(fr));

% plot between normal frame and after windowning
plot(fr);hold on;plot(fr_win,'r');

% fft plot
figure;
plot(abs(fft(fr))); hold on; plot(abs(fft(fr_win)),'r');
