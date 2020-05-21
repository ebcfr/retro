
N=10000;Te=0.1e-3;f0=100;

t=Te*(0:N-1);s=5*sin(2*pi*f0*t);
xplot(t,s);
fe=1/Te

.. ""
.. "Press a key"
.. wait(5);

f=1/(N*Te)*(0:N-1);
sf=fft(s)*Te;
xplot(f[1:N/2],2/Te*abs(sf[1:N/2]))

.. ""
.. "Press a key"
.. wait();

xplot(f[1:N/20],2/Te*abs(sf[1:N/20]),lw=2)
