"*** sinusoide spectrum ***"
N=1000;Ts=0.1m;fs=1/Ts;f0=100;
t=Ts*(0:N-1);s=5*sin(2*pi*f0*t);

f=1/(N*Ts)*(0:N-1);
sf=fft(s);

clg
xsubplot(311);
xplot(t,s,lw=2);
xsubplot(312);
xplot(f[1:N/2],2*abs(sf[1:N/2]),lw=2);
xsubplot(313);
xplot(f[1:N/20],2*abs(sf[1:N/20]),lw=2);

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
xsubplot(311);
xplot(t,s,lw=2);
xsubplot(312);
xplot(f[1:N/2],2*abs(sf[1:N/2]),lw=2);
xsubplot(313);
xplot(f[1:N/20],2*abs(sf[1:N/20]),lw=2);

"*** amplitude modulation ***"
"* without carrier"
N=500;fs=500k;f0=50k;fm=4k
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
xm=2*sin(2*pi*fm*t);xc=5*sin(2*pi*f0*t);x=xc*xm;
xf=fft(x);
clg
xsubplot(311);
xplot(t,xm_x,lw=2);
xsubplot(312);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);
xsubplot(313);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);

"version1: define the 3 sinusoides"
N=2000;fs=500k;f0=50k;fm1=2k;fm2=4k;fm3=5k;
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
xm=2*sin(2*pi*fm1*t)+1*sin(2*pi*fm2*t)+1.5*sin(2*pi*fm3*t);xc=5*sin(2*pi*f0*t);x=xc*xm;
xf=fft(x);
clg
xsubplot(311);
xplot(t,xm_x,lw=2);
xsubplot(312);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);
xsubplot(313);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);

"version2: sum of the sinusoides through a matrix product"
N=2000;fs=500k;f0=50k;fm=[2k,4k,5k]';
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
... Xm=[2,1,1.5]';xm=Xm*sin(2*pi*fm*t);xm=(sum(xm'))';xc=5*sin(2*pi*f0*t);x=xc*xm;
Xm=[2,1,1.5];xm=Xm.sin(2*pi*fm*t);xc=5*sin(2*pi*f0*t);x=xc*xm;
xf=fft(x);
clg
xsubplot(311);
xplot(t,xm_x,lw=2);
xsubplot(312);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);
xsubplot(313);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);

"* Amplitude modulation with carrier"
N=500;fs=500k;f0=50k;fm=4k
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
xm=2*sin(2*pi*fm*t);xc=5*sin(2*pi*f0*t);m=0.4;x=xc*(1+m*xm);
xf=fft(x);
clg
xsubplot(411);
xplot(t,xm,lw=2);
xsubplot(412);
xplot(t,x,lw=2);
xsubplot(413);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);
xsubplot(414);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);

"3 sinusoides"
N=2000;fs=500k;f0=50k;fm=[2k,4k,5k]';
t=(0:N-1)/fs;f=(0:N-1)*fs/N;
... Xm=[2,1,1.5]';xm=Xm*sin(2*pi*fm*t);xm=(sum(xm'))';xc=5*sin(2*pi*f0*t);x=xc*xm;
Xm=[2,1,1.5];xm=Xm.sin(2*pi*fm*t);xc=5*sin(2*pi*f0*t);m=0.4;x=xc*(1+m*xm);
xf=fft(x);
clg
xsubplot(411);
xplot(t,xm,lw=2);
xsubplot(412);
xplot(t,x,lw=2);
xsubplot(413);
xplot(f[1:N/2],2*abs(xf[1:N/2]),lw=2);
xsubplot(414);
i=N*100k/fs+1;
xplot(f[1:i],2*abs(xf[1:i]),lw=2);

sp=(1|zeros([1,9]))';s=(dup(sp,10))';
xplot(s,lw=2,color=2);

function sha(t,f)
	x=zeros(size(t));
	dt=t[2]-t[1];dn=1/(f*dt);
	for  i=0 to cols(t)/dn-1;
		x[1+i*dn]=1;
	end
	return x;
endfunction

... sample and hold
N=5000;fs=50k;Te=100/fs;fe=1/Te;t=(0:N-1)/fs;dt=1/fs;
sh=sha(t,fe);
function xf(t,fe)
	return 5*sin(2*pi*fe/15*t);
endfunction
xsubplot(311);
xplot(t,xf(t,fe)*sha(t,fe)_xf(t,fe),lw=2);
xsubplot(312);
x=(xf(t,fe)-xf(t-Te,fe))*sha(t,fe);x[1]=0;
xplot(t,x,lw=2);
xr=cumsum(x);
xsubplot(313);
xplot(t,xr_xf(t,fe),lw=2);

function SH(ff,t,fe)
	Te=1/fe;
	x=ff(t,args(4))*sha(t,fe);
	xt=(ff(t,args(4))-ff(t-1/fe,args(4)))*sha(t,fe);xt[1]=0;
	xh=cumsum(xt);
	return {x,xh};
endfunction

function quantize(x,PE=3.3,n=12)
	return 2^(n-1)+floor(2^(n-1)*x/PE+0.5);
endfunction

N=5000;fs=50k;Te=100/fs;fe=1/Te;t=(0:N-1)/fs;dt=1/fs;
function xf(t,fe)
	return 3*sin(2*pi*fe/15*t);
endfunction
{x,xh}=SH("xf",t,fe,fe);
n=4;xq=quantize(xh,3.3,n);
clg;
xsubplot(311);
xplot(t,x_xf(t,fe),lw=2);
xsubplot(312);
xplot(t,xh_xf(t,fe),lw=2);
xsubplot(313);
p=setplot();
setplot(p[1:2]|[0,2^n]);
xplot(t,xq,lw=2);

...
... intégration numérique
...
... primitive de x(t)=sin(2*pi*f0*t) --> -1/w0*cos(w0*t)+K = cumsum(x)*dt
f0=1k;T0=1/f0;t=0:T0/100:3*T0;x=sin(2*pi*f0*t);dt=t[2]-t[1];
N=length(t);
clg;
xsubplot(211);
xplot(t,x,lw=2);
xsubplot(212);
y=1-2*pi*f0*cumsum(x)*dt;
xplot(t,y,lw=2);

... primitive de y=a*x --> y=a/2*x^2+K
clg;shrinkwindow();
a=1;b=0;
x=-2:0.01:2;dx=x[2]-x[1],y=a*x+b;
z=cumsum(y)*dx;
xplot(x,y_z,lw=2)
