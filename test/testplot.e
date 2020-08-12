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

j=1i;TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.05:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
Ta=abs(T);
xplot(f,Ta);

.. xlog plot

load plot
j=1i;TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.01:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
subplot(111);xplot(f,20*log10(abs(T)),"LX,w=2");
subplot(111);xplot(f,20*log10(abs(T)),"LX",lw=2);


. xlog-ylog plot
load plot
subplot(111);
j=1i;TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.01:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
xplot(f,abs(T),"LLc5,w=2");

.. multiple subplots 2x1 layout
load plot
j=1i;
TM=4;f0=3k;m=[1,0.7,0.5,0.3,0.05]';w0=2*pi*f0;f=10^(1:0.005:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
subplot(211);
xplot(f,abs(T),"LL,c1+",lw=2);
subplot(212);
xplot(f,arg(T)/pi*180,"LX,c1+",lw=2,yticks=45*(-2:2));

.. multiple subplots 2x1 layout
load plot
j=1i;
Ti=1;f0=1k;m=[5,1,0.7,0.5,0.3,0.05]';w0=2*pi*f0;f=10^(1:0.005:5);w=2*pi*f;
T=Ti*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
subplot(211);
setplot([10,100k,0.01,10]);xplot(f,abs(T),"LL,c1+",lw=2);xlabel("f [Hz]")ylabel("|T(jw)|");title("band-pass 2nd order filter");
subplot(212);
xplot(f,arg(T)/pi*180,"LX,c1+",lw=2,yticks=45*(-2:2));xlabel("f [Hz]")ylabel("arg T(jw)");

.. multiple subplots 2x2 layout
load plot
j=1i;
TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(2:0.005:5);w=2*pi*f;
T1=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
T2=-TM*(w/w0)^2/(1-(w/w0)^2+j*2*m*w/w0);
subplot(221);
xplot(f,abs(T1),"LL,w=2");xlabel("f [Hz]");ylabel("|T|");
title("2nd order band-pass filter");
subplot(222);
xplot(f,abs(T2),"LL,w=2");xlabel("f [Hz]");ylabel("arg T");
title("2nd order high-pass filter");
subplot(223);
xplot(f,arg(T1)/pi*180,"LX,w=2",grid=0,yticks=45*(-2:2));xlabel("f [Hz]");ylabel("|T|");
subplot(224);
xplot(f,arg(T2)/pi*180,"LX,w=2",yticks=45*(0:4));xlabel("f [Hz]");ylabel("arg T");

load plot
j=1i;
TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(2:0.005:5);w=2*pi*f;
T1=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
T2=-TM*(w/w0)^2/(1-(w/w0)^2+j*2*m*w/w0);
subplot(111);
xplot(f,abs(T1),"LL,w=2");xlabel("f [Hz]");ylabel("|T| Hello World ppp!");
title("2nd order band-pass filter");


function testloop()
	loop 2 to 8
		#
	end
	return 0;
endfunction
testloop();

load plot
subplot(111);
x=-3m:0.01m:4m;y=10*cos(2000*pi*x)*exp(-1000*x);xplot(x,10M*y,lw=2,col=13);

subplot(111);
setplot(-3,4,-70,20);x=-3:0.05:4;y=x^3-4*x^2+2*x+8;xplot(x,y,col=11);

subplot(111);x=0:11;xplot(x,x^2)
subplot(111);setplot([-4,14,-10,130]);x=0:11;xplot(x,x^2)
