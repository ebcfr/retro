j=1i;

function log10(x)
	return log(x)/log(10);
endfunction

function ticks (a,b)
## returns the proper ticks to be used for intervals [a,b] and
## the factor of the ticks.
	fexp0=floor(log(b-a)/log(10));
	if fexp0<-2 || fexp0>2 fexp=3*floor(fexp0/3); else fexp=0; endif
	tick=10^(fexp0-fexp)
	n=ceil((b-a)*10^-fexp0)
	if n<3 tick1=n*tick/10;
	elseif n<5 tick1=tick/2;
	else tick1=tick;
	endif
	tick1=tick1*10^fexp;
  return {(floor(a/tick1+0.05))*tick1:tick1:(ceil(b/tick1-0.05))*tick1,10^fexp};
endfunction
{t,f}=ticks(0,1);t,f
{t,f}=ticks(0,10);t,f
{t,f}=ticks(0,100k);t,f
{t,f}=ticks(0,2);t,f
{t,f}=ticks(0,40);t,f
{t,f}=ticks(0,600k);t,f
{t,f}=ticks(-2,2);t,f
{t,f}=ticks(-3,2);t,f
{t,f}=ticks(-4,4);t,f
{t,f}=ticks(-4,5);t,f
{t,f}=ticks(-5,5);t,f
{t,f}=ticks(-4,6);t,f
{t,f}=ticks(-5,6);t,f
{t,f}=ticks(-7,8);t,f

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
xplot(f,abs(T1),xlog=1,ylog=1,lw=2);xlabel("f [Hz]");ylabel("|T|");
title("2nd order band-pass filter");
xsubplot(222);
xplot(f,abs(T2),xlog=1,ylog=1,lw=2);xlabel("f [Hz]");ylabel("arg T");
title("2nd order high-pass filter");
xsubplot(223);
xplot(f,arg(T1)/pi*180,xlog=1,lw=2,yticks=45*(-2:2));xlabel("f [Hz]");ylabel("|T|");
xsubplot(224);
xplot(f,arg(T2)/pi*180,xlog=1,lw=2,yticks=45*(0:4));xlabel("f [Hz]");ylabel("arg T");

load plot
j=1i;
TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(2:0.005:5);w=2*pi*f;
T1=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
T2=-TM*(w/w0)^2/(1-(w/w0)^2+j*2*m*w/w0);
xsubplot(111);
xplot(f,abs(T1),xlog=1,ylog=1,lw=2);xlabel("f [Hz]");ylabel("|T| Hello World ppp!");
title("2nd order band-pass filter");


function testloop()
	loop 2 to 8
		#
	end
	return 0;
endfunction
testloop();

load plot
xsubplot(111);
x=-3m:0.01m:4m;y=10*cos(2000*pi*x)*exp(-1000*x);xplot(x,10M*y,lw=2,col=16);

xsubplot(111);
setplot(-3,4,-70,20);x=-3:0.05:4;y=x^3-4*x^2+2*x+8;xplot(x,y,col=11);
