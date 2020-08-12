load signal

... Use of the fft as an approximation of a signal with exp(j*w*t)

function fseries()
	n=128; t=(0:n-1)*2*pi/n; s=0.5*(1+sqrwave(t,1/(2*pi),0.5)); w=fft(s); ... w[2*(2:n/2)-1]=0;
	for k=0 to n/2
		w1=w;
		w1[k+2:n-k]=0; s1=re(ifft(w1));
		subplot(311);
		setplot(0, 2*pi, -0.3, 1.3);
		xplot(t,s); hold on; color(11); plot(t,s1); color(1); hold off;
		subplot(312);
...		setplot([-n/2,n/2,-0.1,0.6]);
		xplot(-n/2:n/2-1,abs(w1[n/2+1:n]|w1[1:n/2]),style="lc,c11,mo,w=2");
		subplot(313);
		setplot([-n/2,n/2,-180,180]);xplot(-n/2:n/2-1,180/pi*arg(w1[n/2+1:n]|w1[1:n/2]),style="lc,c11,mo,w=2");
		wait(0.5);
	end
	return 0;
endfunction
fseries

	n=256;fs=100;Ts=1/fs;t=(0:n-1)*Ts; x=sin(2*pi*10*t)+2*cos(2*pi*15*t);
	y=x+normal(size(x));
	subplot(211);
	setplot(0,(n-1)*Ts,-5, 5); 
	xplot(t,x_y,"c11+");
	... hold on; color(3); plot(t,y); color(1);
	title("Signal und signal with noise (return)");
	hold off;
	c=fft(y); p=abs(c);
	subplot(212);
	xplot((-n/2:n/2-1)*fs/n,p[n/2+1:n]|p[1:n/2],"lc,c11");
	title("Spectral analysis (return)");

comment
"*** sinusoide spectrum ***"
N=1000;Ts=0.1m;fs=1/Ts;f0=100;
t=Ts*(0:N-1);s=5*sin(2*pi*f0*t);

f=1/(N*Ts)*(0:N-1);
sf=fft(s);

subplot(311);
xplot(t,s,lw=2);
subplot(312);
xplot(f[1:N/2],2*abs(sf[1:N/2]),"lc,mo,c11",lw=2);
subplot(313);
xplot(f[1:N/20],2*abs(sf[1:N/20]),"lc,mo,c11",lw=2);

... ""
... "Press a key"
... wait(5);

load signal

"*** square wave spectrum ***"
N=1000;Ts=0.1m;fs=1/Ts;f0=100;
t=Ts*(0:N-1);s=sqrwave(t,f0);
f=1/(N*Ts)*(0:N-1);
sf=fft(s);
clg
subplot(311);
xplot(t,s,lw=2);
subplot(312);
xplot(f[1:N/2],2*abs(sf[1:N/2]),lw=2);
subplot(313);
xplot(f[1:N/20],2*abs(sf[1:N/20]),lw=2);

"*** amplitude modulation ***"
"* without carrier"
N=500;fs=500k;f0=50k;fm=4k
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
xm=2*sin(2*pi*fm*t);xc=5*sin(2*pi*f0*t);x=xc*xm;
xf=fft(x);
clg
subplot(311);
xplot(t,xm_x,lw=2);
subplot(312);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);
subplot(313);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);

"version1: define the 3 sinusoides"
N=2000;fs=500k;f0=50k;fm1=2k;fm2=4k;fm3=5k;
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
xm=2*sin(2*pi*fm1*t)+1*sin(2*pi*fm2*t)+1.5*sin(2*pi*fm3*t);xc=5*sin(2*pi*f0*t);x=xc*xm;
xf=fft(x);
clg
subplot(311);
xplot(t,xm_x,lw=2);
subplot(312);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);
subplot(313);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);

"version2: sum of the sinusoides through a matrix product"
N=2000;fs=500k;f0=50k;fm=[2k,4k,5k]';
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
... Xm=[2,1,1.5]';xm=Xm*sin(2*pi*fm*t);xm=(sum(xm'))';xc=5*sin(2*pi*f0*t);x=xc*xm;
Xm=[2,1,1.5];xm=Xm.sin(2*pi*fm*t);xc=5*sin(2*pi*f0*t);x=xc*xm;
xf=fft(x);
clg
subplot(311);
xplot(t,xm_x,lw=2);
subplot(312);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);
subplot(313);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);

"* Amplitude modulation with carrier"
N=500;fs=500k;f0=50k;fm=4k
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
xm=2*sin(2*pi*fm*t);xc=5*sin(2*pi*f0*t);m=0.4;x=xc*(1+m*xm);
xf=fft(x);
clg
subplot(411);
title("Amplitude Modulation (AM)");
xplot(t,xm,lw=2);xlabel("t[s]");ylabel("x_m(t)");
subplot(412);
xplot(t,x,lw=2);xlabel("t [s]");ylabel("modulated signal x(t)");
subplot(413);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);xlabel("f [Hz]");ylabel("|X(f)|");
subplot(414);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);xlabel("f [Hz]");ylabel("|X(f)|");

"3 sinusoides"
N=2000;fs=500k;f0=50k;fm=[2k,4k,5k]';
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
... Xm=[2,1,1.5]';xm=Xm*sin(2*pi*fm*t);xm=(sum(xm'))';xc=5*sin(2*pi*f0*t);x=xc*xm;
Xm=[2,1,1.5];xm=Xm.sin(2*pi*fm*t);xc=5*sin(2*pi*f0*t);m=0.4;x=xc*(1+m*xm);
xf=fft(x);
clg
subplot(411);
title("Amplitude Modulation (AM)");
xplot(t,xm,lw=2);xlabel("t[s]");ylabel("x_m(t)");
subplot(412);
xplot(t,x,lw=2);xlabel("t [s]");ylabel("modulated signal x(t)");
subplot(413);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);xlabel("f [Hz]");ylabel("|X(f)|");
subplot(414);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);xlabel("f [Hz]");ylabel("|X(f)|");


...
... intégration numérique
...
... primitive de x(t)=sin(2*pi*f0*t) --> -1/w0*cos(w0*t)+K = cumsum(x)*dt
f0=1k;T0=1/f0;t=0:T0/10000:3*T0;x=sin(2*pi*f0*t);dt=t[2]-t[1];
N=length(t);
clg;
subplot(211);
xplot(t,x,lw=2);
subplot(212);
y=1-2*pi*f0*cumsum(x)*dt;
xplot(t,y,lw=2);

... primitive de y=a*x --> y=a/2*x^2+K
clg;shrinkwindow();
a=1;b=0;
x=-2:0.01:2;dx=x[2]-x[1],y=a*x+b;
z=cumsum(y)*dx;
xplot(x,y_z,lw=2)
endcomment

cls
delay=5;

"
============== Discrete Fourier Transform =============


Let's start with an analog pulse signal and display its spectrum

				       |sin(pi theta f)| 
				A theta|---------------| 
				       |  pi theta f   | 

"
"A theta\left|\frac{\sin(pi theta f)}{pi theta f}\right|";

wait(delay);

t=[-1,-0.1,-0.1,0.1,0.1,1];x=[0,0,5,5,0,0];
theta=0.2;A=5;
f=-100:0.1:100;X=A*theta*abs(sinc(pi*theta*f));
subplot(211);title("a pulse analog signal, and its spectrum");
xplot(t,x,lw=2);xlabel("t [s]");ylabel("x(t)");
subplot(212);
xplot(f,X,lw=2);xlabel("f [Hz]");ylabel("|X(f)|");wait(delay);

"Sampling a signal is, in fact, multiplying it by a Dirac comb with a period Ts.

          __ +oo                                       1  __ +oo               k 
x(t)  =  \            x(t) delta(t - n T ) ~> X(f)  =  -- \             X(f - --)
         /__ n = -oo                    s              T  /__ k = -oo         T  
                                                        s                      s 
"
"x(t) = \sum_{n=-\infty}^{+\infty}x(t) delta(t-n T_s) \leadsto X(f) = \frac{1}{T_s} \sum_{k=-\infty}^{+\infty} X(f-\frac{k}{T_s})";
"

which leads to a periodisation of the spectrum.

"

wait(delay);

fs=100;Ts=1/fs;
t=-1:Ts:1;
f=-100:0.1:100;X=A*theta*abs(sinc(pi*theta*f));
X=abs(A*theta*abs(sinc(pi*theta*(f+fs)))+A*theta*abs(sinc(pi*theta*f))+A*theta*abs(sinc(pi*theta*(f-fs))));

subplot(211);title("a pulse analog signal, and its spectrum");
style("m[]");xmark(t,A*pulse(t,theta),lw=2);xlabel("t [s]");ylabel("x(t)");
subplot(212);
plot(f,X,lw=2);xlabel("f [Hz]");ylabel("|X(f)|");wait(delay);
