j=1i;

function log10(x)
	return log(x)/log(10);
endfunction

TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.05:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
Ta=abs(T);
xplot(f,Ta);

.. xlog plot
load plot
TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.01:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
xplot(f,20*log10(abs(T)),xlog=1);

.. xlog-ylog plot
load plot
TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.01:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
xplot(f,abs(T),xlog=1,ylog=1);

.. multiple subplots 2x1 layout
load plot
j=1i;
TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.005:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
xsubplot(211);
xplot(f,abs(T),xlog=1,ylog=1,lw=2);
xsubplot(212);
xplot(f,arg(T)/pi*180,xlog=1,lw=2,yticks=45*(-2:2));

.. multiple subplots 2x2 layout
load plot
j=1i;
TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(2:0.005:5);w=2*pi*f;
T1=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
T2=-TM*(w/w0)^2/(1-(w/w0)^2+j*2*m*w/w0);
xsubplot(221);
xplot(f,abs(T1),xlog=1,ylog=1,lw=2);xlabel("f [Hz]");
xsubplot(222);
xplot(f,abs(T2),xlog=1,ylog=1,lw=2);xlabel("f [Hz]");
xsubplot(223);
xplot(f,arg(T1)/pi*180,xlog=1,lw=2,yticks=45*(-2:2));xlabel("f [Hz]");
xsubplot(224);
xplot(f,arg(T2)/pi*180,xlog=1,lw=2,yticks=45*(0:4));xlabel("f [Hz]");
title("2nd order transfer functions");

function testloop()
	loop 2 to 8
		#
	end
	return 0;
endfunction
testloop();

load plot
x=-3m:0.01m:4m;y=10*cos(2000*pi*x)*exp(-1000*x);xplot(x,10M*y,lw=2,col=16);

setplot(-3,4,-70,20);x=-3:0.05:4;y=x^3-4*x^2+2*x+8;xplot(x,y,col=11);
