% This file do separation of voiced/un-voiced/silence regions of an speech

close all; clear all;

% read sound 
wavfile = '139048_babbling-dada-wowowawidawida.wav';
[data, fs] = audioread(wavfile);

% normalize data that the maximum amplitude of the system is one
data = data / abs(max(data));

%%
% do framing
f_d = 0.025;
f_size = round(f_d * fs); % total samples in a frame = duration . samples per second
n = length(data);
n_f = floor(n/f_size);  %no. of frames, floor to get an integer
% loop to create frames to number of frames. each row being one frame
temp = 0;
for i = 1 : n_f
   % index in matlab starts from 1, so 0 + 1
   frames(i,:) = data(temp + 1 : temp + f_size);
   temp = temp + f_size;
end
%% look at the pattern of frames
% voiced sample (number 110 frame, all columns in that row inlcuded)
plot(frames(110,:));
xlabel('samples in this frame');
ylabel('normalized amplitude');
figure; 
autocorr(frames(110,:),length(frames(110,:)) - 1);
figure; plot(abs(fft(frames(110,:))));

%%
% unvoices or silence part
figure; plot(frames(40,:));title('unvoiced/silence part');
xlabel('samples in this frame');
ylabel('normalized amplitude');
figure;
autocorr(frames(40,:),length(frames(40,:)) -1);
figure; plot(abs(fft(frames(50,:))));
%%
% silence removal based on max amplitude
m_amp = abs(max(frames,[],2)); % find maximum of each frame
id = find(m_amp > 0.03); % finding ID of frames with max amp > 0.03
fr_ws = frames(id,:); % frames without silence

%%
% reconstruct signal
data_r = reshape(fr_ws',1,[]);
plot(data_r); title('speech without silence');
