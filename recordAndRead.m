% This file record sound from microphone
% 
% Lecture Series: Speech & Audio Signal Processing
% website: www.jcbrolabs.org
% mail: jcbrolabs@gmail.com
close all; clear all;

% get device information
dev = audiodevinfo;

% create recorder object
rec = audiorecorder(44100, 16, 1);

% start recording
disp('start speaking');
% record(rec, 5); % will record for 5sec. for this command pause needs to be added
recordblocking(rec, 5);

% stop recording
disp('Stop recording');

% Play recorded sound
play(rec);

% get audio data
y = getaudiodata(rec);

% plot the sound
plot(y);




