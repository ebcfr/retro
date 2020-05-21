j=1i;

function log10(x)
	return log(x)/log(10);
endfunction

TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.05:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
Ta=abs(T);
xplot(f,Ta);

load plot
TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.01:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
Ta=abs(T);
xplot(f,20*log10(Ta),xlog=1);

load plot
j=1i;
TM=4;f0=3k;m=0.05;w0=2*pi*f0;f=10^(1:0.01:5);w=2*pi*f;
T=TM*2*m*j*w/w0/(1-(w/w0)^2+j*2*m*w/w0);
xplot(f,abs(T),xlog=1,ylog=1,lw=2);

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
