function [frames] = framing(data,fs,f_d)
% do framing
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
end