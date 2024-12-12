order = 3;
framelen = 11;

lx = 34;
x = randn(lx,1);

sgf = sgolayfilt(x,order,framelen);

plot(x,':')
hold on
plot(sgf,'-')
legend('signal','sgolay')