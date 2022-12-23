% This file read audio files in various formats
% 
% Lecture Series: Speech & Audio Signal Processing
% website: www.jcbrolabs.org
% mail: jcbrolabs@gmail.com
close all; clear all;

% read .wav file
wavfile = 'continuous cry.wav';
[x,fs] = audioread(wavfile);

% % read .mp3 file
% mp3file = ''
% [x1, fs1] = audioread(mp3file);

% % play audio files
% sound(x,fs);   %one method

% % play audio files by another method
player = audioplayer(x,fs);
play(player);
pause(5);
pause(player);
pause(5);
stop(player);

% plot signal
plot([0:length(x)-1]/fs,20*log10(x)); 
xlabel('time (sec)');
ylabel('Amplitude (dB)');
title('Input Sound File');

