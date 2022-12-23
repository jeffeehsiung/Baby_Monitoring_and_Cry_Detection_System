function [fr_win] = windowing(fr)
% windowing (hamming window)
fr_win = fr .* hamming(length(fr));

% plot between normal frame and after windowning
plot(fr);hold on;plot(fr_win,'r');title('before and after windowing in time domain');

% fft plot
figure;
plot(abs(fft(fr))); hold on; plot(abs(fft(fr_win)),'r');title('before and after windowing in frequency domain');


end
