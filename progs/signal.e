function signal
## A signal processing package with functions to:
## - generate signals: 
##   dirac, sha, ustep, ramp, pulse, triangle, sawwave, sqrwave, triwave
## - deal with LTI analog systems
##   * simulate: tf (Transfer function), ss (state space), zp (zeros, poles, gain) models supported
##     impulse, step, lsim, freqs, unwrap
##   * convert from a model to another: tf2zp, zp2tf, tf2ss
##   * make filter design: outputs zpk model (zeros, poles, gain)
##     butter, bessel, cheb1, ellip, ellipord
##   * special plots: bode, nyquist, black/nichols [TODO]
## - deal with LTI digital systems [TODO]
## - compute special polynoms: be, cheb
## - get quick asymptotical frequency response for standard small order filters
##   alp1, ahp1, alp2, ahp2
	error("Illegal argument number!");
endfunction

...............................................................
... signals
...............................................................

function dirac(t)
## returns a vector containing a dirac = 1 when t==0, else 0
##
## t     : time
	return(t==0);
endfunction

function sha(t,f)
## returns a vector containing a dirac comb
##
## t     : time
## f     : frequency
	x=zeros(size(t));
	dt=t[2]-t[1];dn=1/(f*dt);
	for  i=0 to cols(t)/dn-1;
		x[1+i*dn]=1;
	end
	return x;
endfunction

function ustep(t)
## returns the heaviside step function
##
## t     : time
	return (t>=0);
endfunction

function ramp(t)
## returns a vector containing a unit ramp signal: r(t) = t when t>=0,  0 else
##
## t     : time
	return t*(t>=0);
endfunction

function sawwave(t,f)
## returns a vector containing a unit saw wave
##
## t     : time
## f     : frequency
	return t*f-floor(t*f);
endfunction

function sqrwave(t,f,alpha=0.5)
## returns a vector containing a square signal
##
## t     : time
## f     : frequency
## alpha : duty cycle
##
	return 2*(sawwave(t,f)<alpha)-1;
endfunction

function triwave(t,f,alpha=0.5)
## returns a vector containing a triangle signal
##
## t     : time
## f     : frequency
## alpha : period ratio
##
	s=(t*f-floor(t*f));
	return 2*(s/alpha*(s<=alpha)+(1-s)/(1-alpha)*(s>alpha))-1;
endfunction

function trapwave(t,f,E,tr)
## returns a vector containing a trapezoidal signal
##
## t     : time
## f     : frequency
## E     : amplitude
## tr    : rise time [s]. must be < 1/(4f) = T/4
	v=E/(4*f*tr)*triwave(t+1/(4*f),f);
	i=nonzeros(v>E);v[i]=E;
	i=nonzeros(v<-E);v[i]=-E;
	return v;
endfunction

function pulse(t,theta=1)
## returns a signal containing a centered pulse = 1 for |t|<theta/2, else 0
##
## t     : time
## theta : pulse width
	return (t>=-theta/2)-(t>=theta/2);
endfunction

function triangle(t,theta=1)
## returns a signal containing a centered pulse = 1 - |t|/theta for |t|<theta, else 0
##
## t     : time
## theta : half triangle width
	return (1-abs(t)/theta)*pulse(t,2*theta);
endfunction

function sinc(x)
## cardinal sinus = sin(x)/x
## x: a matrix
	return sin(x)/(x+(x==0))+(x==0);
endfunction

function fftshift(x)
## returns the spectrum vector calculated with 'fft' reordered to 
## correspond to the frequency scale (-n/2:n/2-1)*fs/N where 'fs'
## is the sampling frequency.
	n=length(x);
	return x[n/2+1:n]|x[1:n/2];
endfunction

...............................................................
... convert between LTI models
...............................................................
function tf2zp(num,den)
## {z,p,k}=tf2zp(num,den)
## Convert transfer function filter parameter to zero-pole-gain form
## See also : zp2tf, ss2tf, tf2ss, ss2zp, zp2ss.
	return {polysolve(num),polysolve(den),num(length(num))/den(length(den))};
endfunction


function zp2tf(z,p,k)
## {num,den}=zp2tf(z,p,k)
## Convert zero-pole-gain filter parameters to transfer function form.
## See also : tf2zp, ss2tf, tf2ss, ss2zp, zp2ss.
  if length(z)==0 b=1; else b=re(polycons(z)); endif
  return {k*b,re(polycons(p))};
endfunction


function tf2ss(num,den)
## {A,B,C,D} = tf2ss(num,den)
## transforms the transfer function coefficient to state space matrices
## See also : ss2tf, impulse, step.
	k=den[length(den)];
	de=den/k;
	nu=num/k;
	n=length(de)-1;
	if length(nu)>length(de);
		error("the degree of numerator must be less or equal to the denominator's one.");
	endif;
	if length(nu)==length(de);
		{q,r}=polydiv(nu,de);
		D=q;
		nu=r[1:n];
	else
		D=0;
	endif;
	A=zeros([n,n]);
	loop 1 to n-1;
		A[#,#+1]=1;
	end;
	A[n,:]=-de[1:n];
	B=zeros([n,1]);
	B[n,1]=1;
	C=zeros([1,n]);
	C[1:length(nu)]=nu;
	return {A,B,C,D};
endfunction

...............................................................
... simulate LTI models
...............................................................

function impulse()
## {y,x}=impulse(A,B,C,D,t)
## {y,x}=impulse(num,den,t)
## computes the impulse response of the linear system defined in the state
## space (D = 0) or in transfer function form.
## y : response matrix (one row per output)
## x : state variable evolution (one row per state variable)
## For muti-input system, you'll have to consider each input at a time,
## selecting for C one column of the original input matrix each time.
## See also : tf2ss, step.
	if argn()<5; .. assume it is in transfer function form
		{A,B,C,D}=tf2ss(arg1,arg2);t=arg3;
	else;
		A=arg1;B=arg2;C=arg3;D=arg4;t=arg5;
	endif;
	dt=t[2]-t[1];
	s=size(A);n=s[1];s=size(C);m=s[1];
	eAdt=inv(id(n)-A*dt+0.5*(A*dt).(A*dt)); .. Pade approximation
	x=zeros(n,length(t));
	y=zeros(m,length(t));
	x[:,1]=B;
	y[:,1]=C.x[:,1];
	loop 2 to length(t);
		x[:,#]=eAdt.x[:,#-1];
		y[:,#]=C.x[:,#];
	end;
	return {y,x};
endfunction


function step()
## {y,x}=step(A,B,C,D,t,U=1)
## {y,x}=step(num,den,t,U=1)
## computes the unit step response of the linear system defined in the state
## space or in transfer function form.
## U : input step value
## y : response matrix (one row per output)
## x : state variable evolution (one row per state variable)
## For muti-input system, you'll have to consider each input at a time,
## selecting for C one column of the original input matrix each time.
## See also : tf2ss, impulse.
	if argn()<5; .. assume it is in transfer function form
		{A,B,C,D}=tf2ss(arg1,arg2);t=arg3;
		if argn()==3;
			U=1;
		else;
			U=arg4;
		endif;
	else;
		A=arg1;B=arg2;C=arg3;D=arg4;t=arg5;
		if argn()==5;
			U=1;
		else;
			U=arg6;
		endif;
	endif;
	dt=t[2]-t[1];
	s=size(A);n=s[1];s=size(C);m=s[1];
	eAdt=inv(id(n)-A*dt+0.5*(A*dt).(A*dt)); .. Pade approximation
	M=inv(A).(eAdt-id(n)).(B*U);
	x=zeros(n,length(t));
	y=zeros(m,length(t));
	loop 2 to length(t);
		x[:,#]=eAdt.x[:,#-1]+M;
		y[:,#]=C.x[:,#]+D*U;
	end;
	return {y,x};
endfunction


function lsim()
## {y,x}=lsim(A,B,C,D,u,t,[x0])
## {y,x}=lsim(num,den,u,t)
## computes the unit step response of the linear system defined in the state
## space form or in the transfer function form.
## u : input vector
## y : response matrix (one row per output)
## x : state variable evolution (one row per state variable)
## For muti-input system, you'll have to consider each input at a time,
## selecting for C one column of the original input matrix each time.
## See also : tf2ss, impulse, step.
	if argn()<6; .. assume it is in transfer function form
		{A,B,C,D}=tf2ss(arg1,arg2);u=arg3;t=arg4;
		x0=zeros(rows(A),1);
	else;
		A=arg1;B=arg2;C=arg3;D=arg4;u=arg5;t=arg6;
		if argn()==7;
			x0=arg7;
		else;
			x0=zeros(rows(A),1);
		endif;
	endif;
	dt=t[2]-t[1];
	s=size(A);n=s[1];s=size(C);m=s[1];
	eAdt=inv(id(n)-A*dt+0.5*(A*dt).(A*dt)); .. Pade approximation
	M=inv(A).(eAdt-id(n));
	x=zeros(n,length(t));x[:,1]=x0;
	y=zeros(m,length(t));y[:,1]=C.x[:,1]+D*u[:,1];
	loop 2 to length(t);
		x[:,#]=eAdt.x[:,#-1]+M.(B*u[#]);
		y[:,#]=C.x[:,#]+D*u[#];
	end;
	return {y,x};
endfunction

...............................................................
... filter design
...............................................................
function cheb(x,n)
	signum=-mod(n,2)*2+1;
	w1=matrix([rows(n),0],0);
	w2=matrix([rows(n),0],0);
	w3=matrix([rows(n),0],0);
	if x>1
		i=nonzeros(x>1);
		w1=(x[i]+sqrt(x[i]^2-1))^n;
		w1=(w1+1/w1)/2;
	endif
	if x<-1
		i=nonzeros(x<-1);
		w3=(-x[i]+sqrt(x[i]^2-1))^n;
		w3=signum*(w3+1/w3)/2;
	endif
	if x>=-1 && x<=1
		i=nonzeros(x>=-1 && x<=1);
		w2=cos(n*acos(x[i]));
	endif
	return w3|w2|w1;
endfunction

function alp1(f,fo)
## Passe bas asymptotique 1er ordre (asymptotical low pass)
## fo : fréquence de cassure
## f : vecteur fréquence
	g=ones(size(f));
	.. Calcul des fréquences après la fréquence de coupure
	i=nonzeros(f>fo);
	g[i]=-1i*fo/f[i];
	return g;
endfunction

function ahp1(f,fo)
## Passe haut asymptotique 1er ordre (asymptotical high pass)
## fo : fréquence de cassure
## f  : vecteur fréquence
	g=ones(size(f));
	.. Calcul des fréquences après la fréquence de coupure
	i=nonzeros(f<fo);
	g[i]=1i*f[i]/fo;
	return g;
endfunction

function alp2(f,fo)
## Passe bas asymptotique 2e ordre (asymptotical low pass)
## fo : fréquence de cassure
## f  : vecteur fréquence
	g=ones(size(f));
	.. Calcul des valeurs après la fréquence de coupure
	i=nonzeros(f>=fo);
	g[i]=-(fo/f[i])^2;
	return g;
endfunction

function ahp2(f,fo)
## Passe haut asymptotique 2e ordre (asymptotical high pass)
## fo : fréquence de cassure
## f  : vecteur fréquence
	g=ones(size(f));
	.. Calcul des valeurs avant la fréquence de coupure
	i=nonzeros(f<fo);
	g[i]=-(f[i]/fo)^2;
	return g;
endfunction


function butterord()

endfunction

function butter(n,wc)
## butter(n,wc)
##  returns the Butterworth transfert function in zpk form
  return {[],wc*exp(-1i*pi*(2*(0:n-1)+n+1)/(2*n)),wc^n};
endfunction


function cheb1(n,wc,rp)
  eps=sqrt(10^(rp/10)-1);
  g=((1+sqrt(1+eps^2))/eps)^(1/n);
  i=1:n;
  p=wc*((1/g-g)/2*sin((2*i-1)*pi/(2*n))+1i*(1/g+g)/2*cos((2*i-1)*pi/(2*n)));
  k=re(prod(-p));
  if mod(n,2)==0 k=k*10^(-rp/20); endif
  return {[],p,k};
endfunction


function ellipord(wp,ws,Ap,As)
  k=wp/ws;  .. Selectivity factor k
  u=(1-(1-k^2)^0.25)/(2*(1+(1-k^2)^0.25));
  q=u+2*u^5+15*u^9+150*u^13;       .. modular constant q
  D=(10^(As/10)-1)/(10^(Ap/10)-1); .. discrimimnation factor D
  n=ceil(log(16*D)/log(1/q));      .. minimum filter order
  Ts=10*log10(1+(10^(Ap/10)-1)/(16*q^n)); .. effective minimum stopband loss
  return {n,Ts}
endfunction

function ellip(n,wp,ws,Ap)
  k=wp/ws;alpha=sqrt(wp*ws);
  u=(1-(1-k^2)^0.25)/(2*(1+(1-k^2)^0.25));
  q=u+2*u^5+15*u^9+150*u^13;       .. modular constant q
  N=4;
  V=1/(2*n)*log((10^(Ap/20)+1)/(10^(Ap/20)-1));
  m=0:N;l=1:N;
  p0=abs(q^0.25*sum((-1)^m*q^(m*(m+1))*sinh((2*m+1)*V))/(0.5+sum((-1)^l*q^(l^2)*cosh(2*l*V))));
  W=sqrt((1+p0^2/k)*(1+k*p0^2));
  r=floor(n/2);
  t=(2*r~=n);
  mu=((1:r)*(1-t)+((1:r)-0.5)*t)';
  m=0:N;l=1:N;
  X=2*q^0.25*sum((-1)^m*q^(m*(m+1))*sin((2*m+1)*mu*pi/n))/(1+2*sum((-1)^l*q^(l^2)*cos(2*l*mu*pi/n)));
  Y=sqrt((1-X^2/k)*(1-k*X^2));
  a=1/X^2;
  b=2*p0*Y/(1+(p0*X)^2);
  c=((p0*Y)^2+(X*W)^2)/(1+(p0*X)^2)^2;
  H0=prod((c')/(a'))*(10^(-Ap/20)*t+p0*(1-t));
  den=c|b|ones(size(b));
  p=[];z=[];
  if t~=0 p=p|(-p0); endif
  for i=1 to r
    p=p|polysolve(den[i,:]);
    z=z|[-1i*sqrt(a[i]), 1i*sqrt(a[i])];
  end
  z=z*alpha;p=p*alpha;H0=H0*(alpha*(length(p)-length(z))*(1-t)+t);
  return {z,p,H0};
endfunction

function be(n)
  if n==1 return [1,1]; endif
  qn0=1;qn1=[1,1,0];
  loop 2 to n;
    qn=(2*#-1)*qn1+([0,0]|qn0);
    qn0=qn1[1:#];
    qn1=qn|0;
  end;
  return qn;
endfunction

function renorm(b)
  a=[1,0.72675,0.57145,0.46946,0.41322,0.37038,0.33898,0.31546];
  n=length(b)-1;
  if n<=8;
    loop 1 to n;
      b[#]=a[n]^(n-#+1)*b[#];
    end;
  endif;
  return b;  
endfunction

function bessel(n,wc,norm="delay")
  normfactor=[1,0.72675,0.57145,0.46946,0.41322,0.37038,0.33898,0.31546];
  b=be(n);k=0;
  p=polysolve(b);p=p*wc;
  k=b[1]*wc^n;
  if norm=="mag"
    p=p*normfactor[n];
    k=k*(normfactor[n])^n;
  endif
  return {[],p,k};
endfunction

function reverse(a)
## reverse(p)
##   reverse a vector
  v=a;
  loop 1 to length(a);
    v[#]=a[length(a)-#+1];
  end;
  return v;
endfunction

function freqs()
## computes the frequency response (w=pulsation)
## H=freqs(b,a,w) 
##   transfer function in the form b/a
## H=freqs(z,p,k,w)
##   transfer function in the form zeros/poles/gain
  if argn==3
    return polyval(arg1,1i*arg3)/polyval(arg2,1i*arg3);
  elseif argn()==4
  	if length(arg1)==0
  	  num=1;
  	else
      num=re(polycons(arg1));
    endif
    den=re(polycons(arg2));
    return arg3*polyval(num,1i*arg4)/polyval(den,1i*arg4);
  endif
  error("bad parameters");
  return 0;
endfunction

function unwrap2(ph)
	dph=ph-(ph[1]|ph[1:length(ph)-1]);
	return ph-2*pi*cumsum(dph>1.5*pi);
endfunction

function unwrap(ph)
	dph=ph-(ph[1]|ph[1:length(ph)-1]);
	return ph-2*pi*(cumsum(dph>1.5*pi)-cumsum(dph<-0.5*pi));
endfunction

...............................................................
... digital
...............................................................
function c2d(t,ff,fs)
## continous to digital world. it realizes the sample and hold function.
##   t : time vector
##   ff: function or vector of the signal to sample
##   fs: sampling frequency
## returns {xa,xh,xs}
##   xa: the analog sampled signal
##   xh: the analog sample & hold signal
##   xs: the discretized analog signal
	sh=sha(t,fs);i=nonzeros(sh);
	if isfunction(ff) 
		xa=ff(t,args(4))*sh;
	else
		xa=zeros(size(t));xa[i]=ff[i];
	endif
	xs=xa[i];
	f=zeros(size(xs));f[2:cols(f)]=xs[2:cols(f)]-xs[1:cols(f)-1];f[1]=xs[1];
	xt=zeros(size(t));xt[i]=f;
	xh=cumsum(xt);
	return {xa,xh,xs};
endfunction

function d2c()

endfunction

function ADC(x,FS=3.3,n=12)
## quantifies the signal according to the ADC parameters
##   x : the discretized analog signal
##   FS: ADC full scale
##   n : ADC resolution
## returns the digital signal
	return 2^(n-1)+floor(2^(n-1)*x/FS+0.5);
endfunction

function DAC(x,FS=3.3,n=12)
## set the signal back to the analog world
##   x : the digital signal
##   FS: DAC full scale
##   n : DAC resolution
## returns a discretized analog signal
	return FS*(x-2^(n-1))/2^(n-1);
endfunction
