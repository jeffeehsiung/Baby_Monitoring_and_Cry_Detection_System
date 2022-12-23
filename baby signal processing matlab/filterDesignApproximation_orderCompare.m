%% anti alisaing filter design approximation
close all; 
clear

wavfile = '139048_babbling-dada-wowowawidawida.wav';
[data,fs] = audioread(wavfile);

% low pass
fc = 4000; %Hz
n_order = 2;
Wn = fc/(fs/2); % since there are two sides

[zl,pl,kl]= butter(n_order,Wn,'low');  %the cutoff frequency Wn must be 0.0 < Wn < 1.0, with 1.0 corresponding to half the sample rate.
[sosl]=zp2sos(zl,pl,kl);    % convert to second order section matrix
h_btlp =fvtool(sosl,'Analysis','freq'); % Plot magnitude response
title('butter lowpass order 2');

% [zl,pl,kl]= besself(n_order,(fc*pi()/180),"low"); 
% [sosl]=zp2sos(zl,pl,kl);   
% h_bslp =fvtool(sosl,'Analysis','freq') 

% bandpass
fc_low = 300;
fc_up = 4000;
n_order = 2;

[zbp,pbp,kbp]= butter(n_order,[fc_low/(fs/2),fc_up/(fs/2)],'bandpass');  
[sosbp]=zp2sos(zbp,pbp,kbp);
h_btbp = fvtool(sosbp,'Analysis','freq');   
title('butter bandpass order 2');

% [zl,pl,kl]= besself(n_order/2,[(fc_low*pi()/180),(fc_up*pi()/180)],"bandpass"); 
% [sosbp]=zp2sos(zl,pl,kl);    
% h_bsbp =fvtool(sosbp,'Analysis','freq') 

% low pass
fc = 4000; %Hz
n_order = 6;
Wn = fc/(fs/2); % since there are two sides

[zl,pl,kl]= butter(n_order,Wn,'low');  %the cutoff frequency Wn must be 0.0 < Wn < 1.0, with 1.0 corresponding to half the sample rate.
[sosl]=zp2sos(zl,pl,kl);    % convert to second order section matrix
h_btlp =fvtool(sosl,'Analysis','freq'); % Plot magnitude response
title('butter lowpass order 6');

% [zl,pl,kl]= besself(n_order,(fc*pi()/180),"low"); 
% [sosl]=zp2sos(zl,pl,kl);   
% h_bslp =fvtool(sosl,'Analysis','freq') 

% bandpass
fc_low = 300;
fc_up = 4000;
n_order = 6;

[zbp,pbp,kbp]= butter(n_order,[fc_low/(fs/2),fc_up/(fs/2)],'bandpass');  
[sosbp]=zp2sos(zbp,pbp,kbp);
h_btbp = fvtool(sosbp,'Analysis','freq');   
title('butter bandpass order 6');

% [zl,pl,kl]= besself(n_order/2,[(fc_low*pi()/180),(fc_up*pi()/180)],"bandpass"); 
% [sosbp]=zp2sos(zl,pl,kl);    
% h_bsbp =fvtool(sosbp,'Analysis','freq') 


