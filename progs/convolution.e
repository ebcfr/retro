... ############### Convolution ################
load signal

function weiter
	"
	<< Press return to continue >>
	" wait(5);
	return 120;
endfunction
"
                                          (i)                          
                               __ oo     f   (0) i                     
                     f(x)  =  \          -------x                      
                              /__ i = 0    i!                          

"
"1. Convolution: an introduction"
... pulse signal
function d(t, dt)
	return ustep(t) - ustep(t-dt);
endfunction

"
1.1. A system can be characterized by its impulse response
"
... impulse response
function h(t, tau)
## h(t,tau)
##  1st order impulse response
##   = 1/tau*exp(-t/tau)
	return 1/tau*exp(-t/tau)*ustep(t);
endfunction

weiter();

"
1.2. Principle of time invariance of a system
"
N=20;T=10e-3;tau=5e-3;dt=2*T/N;
t=-T:T/100:2*T;ti=[0;10*dt];ei=d(t-ti,dt);
xi=h(t-ti, tau);
clg;
xsubplot(221);xplot(t,ei(1,:),lw=2);xlabel("t [s]");title("impulse at 0");
xsubplot(222);xplot(t,ei(2,:),lw=2);xlabel("t [s]");title("impulse at 10ms");
xsubplot(223);xplot(t,xi(1,:),lw=2);xlabel("t [s]");
xsubplot(224);xplot(t,xi(2,:),lw=2);xlabel("t [s]");wait(5);

weiter();

... step response
"1.3. step response as the pulse narrows"
N=20;T=10e-3;tau=5e-3;dt=2*T/N;
t=-T:T/100:2*T;ti=(0:dt:2*T)';xi=h(t-ti, tau);
clg;
xsubplot(211);xplot(t,xi,lw=2);xlabel("t [s]");ylabel("e(t_i) x h(t-t_i)");title("1st order system: step response");
xsubplot(212);xplot(t,dt*sum(xi')',lw=2);xlabel("t [s]");ylabel("step response");

function stepresp()
	for N=20 to 200 step 10
		T=10e-3;tau=5e-3;dt=2*T/N
		t=-T:T/100:2*T;ti=(0:dt:2*T)';xi=h(t-ti, tau);
		xsubplot(211);xplot(t,xi,lw=2);xlabel("t [s]");ylabel("e(t_i) x h(t-t_i)");title("1st order system: step response");
		xsubplot(212);xplot(t,dt*sum(xi')',lw=2);xlabel("t [s]");ylabel("step response");
		wait(0.2);
	end
	return 0;
endfunction

stepresp();

weiter();

... harmonic response
"
1.4. harmonic response as the pulse narrows
"
... input signal
function e(t,f)
	return sin(2*pi*f*t)*ustep(t);
endfunction

N=20;T=10e-3;tau=5e-3;dt=2*T/N;f0=1/T
t=-T:T/100:2*T;ti=(0:dt:2*T)';xi=h(t-ti, tau)*e(ti,f0);
clg;
xsubplot(211);xplot(t,xi,lw=2);xlabel("t [s]");ylabel("e(t_i) x h(t-t_i)");title("1st order system: sinus response");
xsubplot(212);xplot(t,e(t,f0)_dt*sum(xi')',lw=2);xlabel("t [s]");ylabel("sinus response");

function freqresp()
	for N=20 to 200 step 10
		T=10e-3;tau=5e-3;dt=2*T/N,f0=1/T;
		t=-T:T/100:2*T;ti=(0:dt:2*T)';xi=h(t-ti, tau)*e(ti,f0);
		xsubplot(211);xplot(t,xi,lw=2);xlabel("t [s]");ylabel("e(t_i) x h(t-t_i)");title("1st order system: sinus response");
		xsubplot(212);xplot(t,e(t,f0)_dt*sum(xi')',lw=2);xlabel("t [s]");ylabel("sinus response");
		wait(0.2);
	end
	return 0;
endfunction

freqresp();

... Note: the "large" dt used for the pulse width infers a sampling
... of the signals with a sample period Te=dt, and a 1/Te factor in
... spectrum expression
...
... ∑ δ(t-k.Te) = ∑ cn exp j2π.n.fe.t  ⇌  1/Te ∑ δ(f-n.fe)
... k             n                            n
...
... To compensate the 1/Te factor in the spectrum, the result of 
... the convolution is multiplied by Te.

weiter();

"
2. Effective methods to compute convolution
"

"2.1. Different algorithm to calculate the convolution
"
function pad(x,L)
  return x|zeros(1,L-length(x));
endfunction

... brute-force convolution (O(n^2) algorithm)
function conv(x,h)
## brute-force convolution (O(n^2) algorithm)
## returns x*h
## size(x): [1,n] ; size(h): [1,m] --> size(x*h): [1,n+m-1]
  n=length(x); m=length(h);
  y=zeros([1,n+m-1]);
  for i=1 to n
    for k=1 to m
      y[i+k-1]=y[i+k-1]+x[i]*h[k];
    end
  end
  return y;
endfunction

function conv2(x,h)
## returns x*h
##   builds a band (toeplitz) matrix
  n=length(x); m=length(h);
  T=zeros([n,n+m-1]);
...  t1=time();
  for k=1 to m
  	T=T+diag([n,n+m-1],k-1,h[k]);
  end
...  t2=time();t2-t1
...  result of time measurement the for loop is catastrophic!!
  return bandmult(x,T);
...  return symmult(x,T);
...  return x.T;
endfunction

x=random([1,7])+1i*random([1,7]);
h=random([1,3])+1i*random([1,3]);
L=length(x)+length(h)-1;

"let's test all the methods, they should give the same result!
* brute-force"
y1=conv(h,x)
""
"* fft"
y2=L*ifft(fft(pad(x,L))*fft(pad(h,L)))
"
* polynom multiplication"
y3=polymult(x,h)
"
* toeplitz band matrix"
y4=conv2(x,h)

"
2.2 Some standard response to a first order low pass system"
shrinkwindow();clg;
tau=10m;N=4000;t=(0:N-1)*20*tau/N;dt=t[2]-t[1];printf("tau/dt=%g",tau/dt)
x=ustep(t-dt)-ustep(t-8*tau);
h=1/tau*exp(-t/tau)*ustep(t);
"get timings for:
* brute-force method"
... t1=time();y=conv(x,h);t2=time();t2-t1
... xplot(t,y[1:length(t)]*dt,lw=2);wait(5);
"* toeplitz band matrix"
... t1=time();y=conv2(x,h);t2=time();t2-t1
... xplot(t,y[1:length(t)]*dt,lw=2);wait(5);
"* polynom multiplication"
t1=time();y=polymult(x,h);t2=time();t2-t1
xplot(t,y[1:length(t)]*dt,lw=2);wait(5);
"* fft"
... t1=time();L=length(x)+length(h)-1;y=re(L*ifft(fft(pad(x,L))*fft(pad(h,L))));t2=time();t2-t1
... xplot(t,y[1:length(t)]*dt,lw=2);wait(5);



tau=10m;N=4000;t=(-N:N-1)*20*tau/N;dt=t[2]-t[1];printf("tau/dt=%g",tau/dt)
x=ustep(t-2*tau)-ustep(t-8*tau);
h=1/tau*exp(-t/tau)*ustep(t);
t1=time();y=polymult(x,h);t2=time();t2-t1;
clg;
xsubplot(211);xplot(t,h,lw=2);
xsubplot(212);xplot(t,y[N:N-1+length(t)]*dt_x,lw=2);wait(5);
