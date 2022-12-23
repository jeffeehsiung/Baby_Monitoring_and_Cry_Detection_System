% This file do separation of voiced/un-voiced/silence regions of an speech
% signal

close all; clear all;

% read sound 
[data, fs] = audioread('intro.wma');
% normalize data
data = data / abs(max(data));
% do framing
f_d = 0.025;
f_size = round(f_d * fs);
n = length(data);
n_f = floor(n/f_size);  %no. of frames
temp = 0;
for i = 1 : n_f
    
   frames(i,:) = data(temp + 1 : temp + f_size);
   temp = temp + f_size;
end

% voiced sample (110 frame)
plot(frames(110,:));
figure; 
autocorr(frames(110,:),length(frames(110,:)) - 1);
figure; plot(abs(fft(frames(110,:))));

% unvoices or silence part
figure; plot(frames(40,:));title('unvoiced/silence part');
figure;
autocorr(frames(40,:),length(frames(40,:)) -1);
figure; plot(abs(fft(frames(50,:))));

% silence removal based on max amplitude
m_amp = abs(max(frames,[],2)); % find maximum of each frame
id = find(m_amp > 0.03); % finding ID of frames with max amp > 0.03
fr_ws = frames(id,:); % frames without silence

% reconstruct signal
data_r = reshape(fr_ws',1,[]);
plot(data_r); title('speech without silence');