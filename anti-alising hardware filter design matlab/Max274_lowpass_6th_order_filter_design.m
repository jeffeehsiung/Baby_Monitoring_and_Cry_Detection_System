% %% maxim Q1 section: trasnflating calcuated f0 and Q pairs into max274
% % % calculate R2 R4 where they set the center frequency
% % Ra2 = (2*(10^9))/fneeded;           % 500000
% % Ra4 = Ra2-5000;                     % 495000
% % % calculate R3 as it sets Q
% % Ra3 = (Q1*(2*(10^9))/fneeded)/(5);  % 51800
% % % calculate R1 where R1 is inversely proportional to LP gain
% % Ra1 = ((2*(10^9))/fneeded)/5;       % 100000
% 
% %% maxim Q2 section: trasnflating calcuated f0 and Q pairs into max274
% % calculate R2 R4 where they set the center frequency
% Rb2 = (2*(10^9))/fneeded;
% Rb4 = Ra2-5000;
% % calculate R3 as it sets Q
% Rb3 = (Q2*(2*(10^9))/fneeded)/(5);  % 70700
% % calculate R1 where R1 is inversely proportional to LP gain
% Rb1 = ((2*(10^9))/fneeded)/5;
% 
% %% maxim Q3 section: trasnflating calcuated f0 and Q pairs into max274
% % calculate R2 R4 where they set the center frequency
% Rc2 = (2*(10^9))/fneeded;
% Rc4 = Ra2-5000;
% % calculate R3 as it sets Q
% Rc3 = (Q3*(2*(10^9))/fneeded)/5;    % 193200
% % calculate R1 where R1 is inversely proportional to LP gain
% Rc1 = ((2*(10^9))/fneeded)/5;

%% maxim 274 low pass filter design
Q1 = 1;
Q2 = 1;
Q3 = 1;
q = (1-(1/(2*(Q1^2))));
F0 = fc/(q+(q^2+1)^0.5)^0.5
Rxy = 1/5
Holp = 1/((1/Q1)*((1-(1/(4*(Q1^2))))^0.5))
R21 = 0.5*1/(Holp)

%% maxim Q1,Q2,Q3 section: trasnflating calcuated f0 and Q pairs into max274
% calculate R2 R4 where they set the center frequency
Ra2 = (2*(10^9))/F0;                % 636010
Ra4 = Ra2-5000;                     % 631010
% calculate R3 as it sets Q
Ra3 = (Q1*(2*(10^9))/F0)*Rxy;      % 127200
% calculate R1 where R1 is inversely proportional to LP gain
Ra1 = ((2*(10^9))/(F0*Holp))*Rxy;  % 110150