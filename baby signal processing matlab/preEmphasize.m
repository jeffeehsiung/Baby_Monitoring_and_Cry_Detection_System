% This file talks about framing, windowing and pre-emphasis

close all; clear all;

% load sound files
[data,fs] = audioread('10a07La_boredom.wav');

% take frame of 0.025s (25 msec)
f_d = 0.025; % frame duration
f_size = f_d * fs;

fr = data(2001:2001+f_size);

% windowing (hamming window)
fr_win = fr .* hamming(length(fr));

% plot between normal frame and after windowning
plot(fr);hold on;plot(fr_win,'r');

% fft plot
figure;
plot(abs(fft(fr))); hold on; plot(abs(fft(fr_win)),'r');

%% pre-emphasis
% all-zero filter
preemph = [1 -0.95];
figure; bode(preemph,1);
x1 = filter(preemph, 1, fr_win);

figure;
plot(abs(fft(x1)));