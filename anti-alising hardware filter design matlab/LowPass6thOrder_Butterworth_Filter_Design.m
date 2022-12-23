close all;
clear;
clc;

%% 6th order butterworth lowpass filter
s = tf('s');
fc = 4000;
wc = 2*pi()*fc;
coeff = 1/wc;
%% three transfer functoins
num1 = [1];
denum1 = [1*((coeff)^2), 0.518*coeff, 1];

num2 = [1];
denum2 = [1*((coeff)^2), 1.414*coeff, 1];

num3 = [1];
denum3 = [1*((coeff)^2), 1.932*coeff, 1];

H1 = tf(num1,denum1); 
H2 = tf(num2,denum2);
H3 = tf(num3,denum3);
H = H1*H2*H3;

bode(H);

%% Butterworth lowpass filter RC calcualtion: all sallen key
fontable = 1;
fneeded = fontable * fc;

Q1 = 0.518;
Q2 = 0.707;
Q3 = 1.932;

% for a second order low pass section, implemented with unity gain sallen

m = 1;

disp('n')
n1 =((Q1)^2)*((m+1)^2)/m; %1.0733
n2 =((Q2)^2)*((m+1)^2)/m; %1.994
n3 =((Q3)^2)*((m+1)^2)/m; %14.9305

disp('C in F')
C = 10*(10^(-9));
C1 = n1*C; %10.7 nF
C2 = n2*C; %19.99nF
C3 = n3*C; %149.3 nF

disp('R in Ω')
R1 = 1/(2*pi()*((m*n1)^0.5)*C1*fneeded); %3578.334 ohm
R2 = 1/(2*pi()*((m*n2)^0.5)*C2*fneeded); %1407 ohm
R3 = 1/(2*pi()*((m*n3)^0.5)*C3*fneeded); %68.968 ohm


