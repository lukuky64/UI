Fs = 1000; % sampling frequnecy
cutoff_frequnecy = 10;

t = (0:1000)'/Fs; 
x = sin(2*pi*[50 250].*t);
x = sum(x,2) + 0.001*randn(size(t));

x = lowpass(x,cutoff_frequnecy,Fs);

plot(x)