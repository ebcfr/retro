... The Demo. Starts automatically.

"
Demo loading.
"

demodelay=5;

function delay
	global demodelay;
	return demodelay
endfunction

function pause
	"" "<< Press return to continue >>" "",
	return wait(delay());
endfunction


function cplot(z)
## cplot(z) plots a matrix of complex values, which contains the values
## of a function at some rectangular grid.
...	plot(re(z),im(z)); hold on;
...	plot(re(z'),im(z')); hold off;
	w=z';plot(re(z)_re(z'),im(z)_im(z'));
	return 0;
endfunction

function fisch
## plots a fish
	t=0:3;
	ph=interp(t,[0.05,0.3,0.2,1.2]);
	pb=interp(t,[0.05,0.5,0.2,0.2]);
	phi=pi-linspace(0,2*pi,20);
	l=linspace(0,3,40);
	psin=dup(sin(phi),length(l));
	pcos=dup(cos(phi),length(l));
	lh=dup(interpval(t,ph,l),length(phi))';
	lb=dup(interpval(t,pb,l),length(phi))';
	x=pcos*lb; z=psin*lh; y=dup(l-1.5,length(phi))';
	view(2.5,1.5,0.3,0.1);
	solid(x,y,z);
	title("A solid fish (return)"); wait(delay());
	huecolor(1); solidhue(x,y,z,-y,1);
	title("A shaded fish (return)"); wait(delay());
	return 0;
endfunction

function grafikdemo
## grafikdemo shows the grafik capabilities of EULER

	"Demo of EULER's graphics." ""
	"These graphics have been programmed with a few lines."
	"Study demo for more information!"
	pause();
	
## Application from R to R
	t=-pi:pi/20:pi;
	shrinkwindow(); setplot(-pi,pi,-2,2);
	linewidth(4); xplot(t,sin(t),xticks=-3:1:3,yticks=-1:1);
	linewidth(1); hold on;
	color(2); style("l."); plot(t,cos(t)); 
	color(3); style("l--"); plot(t,(sin(t)+cos(t))/2);
	title("2D plots (return)");
	color(1); style(""); hold off;
	wait(delay());
	
## Application from R to R^2 in 4 subplots
	t=-pi:pi/50:pi;
	subplot(221);
	xplot(sin(3*t)/3+cos(t),sin(3*t)/3+sin(t),grid=0);
	title("Parametric curves (return)");
	subplot(222);
	xplot(sin(2*t)/3+cos(t),sin(2*t)/3+sin(t),grid=0);
	subplot(223);
	xplot(sin(2*t),sin(3*t),grid=0);
	subplot(224);
	t=t+2*pi;
	xplot(cos(t)*log(t),sin(t)*log(t),grid=0);
	wait(delay());
	
## x^2+y^3 als mesh und contour.
	fullwindow();
	t=-1:0.1:1; t=t*pi;
	{x,y}=field(t,t);
	mesh(x*x+y*y*y); title("x^2+y^3 (return)"); wait(delay());
	clg;
	contour(x*x+y*y*y,-30:5:30);
	title("Contour plot of this function (return)"); wait(delay());
	clg;
	t=-1:0.04:1; t=t*pi;
	{x,y}=field(sin(2*t),cos(2*t));	
	huecolor(1); density(x*y,1);
	title("Density plot of a function (return)"); wait(delay());
	
## e^-r cos(r) als Drahtmodell.
	clg;
	t=-1:0.1:1; t=t*pi;
	{x,y}=field(t,t);
	r=x*x+y*y; f=exp(-r)*cos(r);
	view(2,1,0,0.5); ## [ Abstand, Zoom, Winkel rechts, Winkel hoch ]
	wire(x/pi,y/pi,f);
    title("Wire frame model (return)"); wait(delay());

## Der Fisch als solides Modell.
	clg;
	fisch();
	view(2,1,0.2,0.3);

## Eine complexe Abbildung w=z^1.3
	shrinkwindow();
	setplot(-1, 2, -0.5, 1.5);
	t=0:0.1:1; z=dup(t,length(t)); z=z+1i*z';
	color(1); style("l."); cplot(z); hold on; 
	color(2); style(""); cplot(z^1.3); color(1);
	title("Complex numbers w=z^1.3"); hold off;
	wait(delay());
	
	"End"
	return 0
endfunction

function normaldemo
## Demo of Retro	
	"                             Wellcome at EULER"
	""
	"EULER has been designed to ease numerical computation."
	"Most of the time one has a tiny problem, to tiny to use a compiler"
	"language."
	""
	"Besides, one may want to play with intermediate results, see a"
	"graphical representation, or simply have readily available"
	"numerical functions."
	"" 
	"All this can be done with EULER."
	"It is like using a numerical laboratory."
	pause();
	
	"But EULER is not a symbolic algebra program. It uses numbers with"
	"finite accuracy as supplied by the C compiler used. It is therefore"
	"faster than a symbolic program, like MATHEMATICA."
	
	"All commands are entered by the keyboard. Old commands can be"
	"recalled with cursor up or down. ESC interrupts a computation,"
	"and HELP shows the graphics screen."
	pause();
	
## Eingabe von Matrizen
	"With EULER one can enter matrices like this:" ""
	">x=[5,6,7]" x=[5,6,7], 
	">A=[1,2,3;5,6,7;8,9,0]" A=[1 2 3;5 6 7;8 9 0],
	"The comma may be omitted."
	"These matrices are stored on the variables x and A."
	pause();
	
	"All computing is done elementwise."
	">x*x" x*x,
	">A+A" A+A,
	pause();
	
## Complexe Zahlen.
	"Complex numbers and complex computations."
	">1+2i", 1+2i,
	">c=[1i 3 4+5i]", c=[1i 3 4+5i],
	"" ">1i*1i", 1i*1i,
	">exp(1i*pi/2)", exp(1i*pi/2),
	pause();
	
## Matrix product.
	"The dot multiplies two matrices."
	">A.x'" A.x'
	"Note, that x had to be transposed!"
	"" ">x", x,
	pause();
	
	"" ">x'", x',
	"" ">x'.x" x'.x,
	pause();
	
## :
	"Generating vectors:"
	">1:10", 1:10,
	">-1:0.3:1", -1:0.3:1,
	pause();
	
	">[1 2 3]|[4 5]", [1 2 3]|[4 5],
	">[1 2 3]_[4 5 8]", [1 2 3]_[4 5 8],
	">dup([1 2 3],4)", dup([1 2 3],4),
	pause();
	
## Functions.
	"Also all functions are applied to all elements of a matrix."
	"" ">sqrt([1,2,3,4])" sqrt([1,2,3,4]),
	""
	"There are all the numerical functions."
	"See an overview with >list."
	pause();
	
	"It is easy to generate a table for a function, and with"
	"the table, one can plot the function."
	"" ">t=0:pi/50:7; s=sin(t); xplot(t,s);"
	"plots the sine function in [0,7]",
	pause();
	t=0:0.05:7; s=sin(t);
	shrinkwindow(); xplot(t,s); title("Please press return!");
	wait(delay());

## Submatrices.
	"Submatrices:" ""
	">x=[11 12 13;21 22 23; 31 32 33]"
	x=[11 12 13;21 22 23; 31 32 33],
	">x[1]" x[1],
	"One can write x[...] or x(...).",
	pause();
	
	">x[1:2,2]", x[1:2,2],
	">x[2:6,2:6]" x[2:6,2:6],
	">x[:,3]", x[:,3],
	">x[1,[2 2 1 3]]", x[1,[2 2 1 3]],
	pause();
	
## Numerical integration.
	"An example:" ""
	">sum(sin((1:100)/100))" sum(sin((1:100)/100)),
	"" 
	"Let us integrate the sine in [0,pi] using the simpson method (but"
	"not the builtin functions). The exact value is 2."
	">t=0:pi/50:pi; n=length(t);", 
	t=0:pi/50:pi; n=length(t);
	">f=2*mod((1:n)+1,2)+2; f[1]=1; f[n]=1;",
	f=2*mod((1:n)+1,2)+2; f[1]=1; f[n]=1;
	">result=sum(sin(t)*f)/3*(t[2]-t[1])", 
	longformat(); result=sum(sin(t)*f)/3*(t[2]-t[1]),
	shortformat();
	"" "; surpresses output of intermediate results."
	pause();
	
	"By the way:"
	">f[1:5]", f[1:5],
	">f[n-4:n]", f[n-4:n],
	pause();
	
## Gleichungssysteme.
	"Solve linear systems:"
	"" ">a=hilb(10); b=a.dup(1,10); a\b" ""
	longformat(); a=hilb(10); b=a.dup(1,10); a\b, shortformat();
	""
	"The inaccuracy is unavoidable for the Hilbert matrix." 
	pause();
	
## Curve discussion.
	"A discussion" ""
	"The function:"
	">x=1:0.02:10; y=log(x)/x;"
	x=1:0.02:10; y=log(x)/x;
	"The function looks like this (press any key)"
	wait(delay());
	shrinkwindow();
	setplot([1,10,-0.1,0.4]);xplot(x,y,"c1,w=2"); title("log(x)/x, maximum, and first derivative (return)"); wait(delay());
	"" "The maximal value is: " ">ym=max(y)",
	ym=max(y),
	"" "it is obtained in" ">xm=x[nonzeros(y>=ym)]",
	xm=x[nonzeros(y>=ym)],
	wait(delay());
	hold on; style("c11,l.,w=1"); plot([1,xm,xm],[ym,ym,0]); style("c11,m#o"); plot(xm,ym); hold off;
	"" "The first derivative (numerically)"
	">n=length(x); a=(y[2:n]-y[1:n-1])/(x[2:n]-x[1:n-1]);"
	n=length(x); a=(y[2:n]-y[1:n-1])/(x[2:n]-x[1:n-1]);
	hold on; style("c2,w=2"); plot((x[2:n]+x[1:n-1])/2,a); hold off;
	wait(delay());
	
	pause();
	return 0
endfunction

... *** Approximations - Demo ***

function apprdemo
	
## cosine developpement in series
	"Cosine developpement Taylor's series of order 2, 4, ...
	
	t=linspace(0,14,500);
	s=(1-t^2/2);
	for i=4 to 30 step 2;
		s=s_(s[rows(s),:]+(-1)^(i/2)*t^i/fak(i));
	end
	"
	pause();
	t=linspace(0,14,500);
	s=(1-t^2/2);
	for i=4 to 30 step 2;
		s=s_(s[rows(s),:]+(-1)^(i/2)*t^i/fak(i));
	end
	subplot(111);setplot(min(t),max(t),-max(t)/2,max(t)/2);
	xplot(t,s,"c1+");
	hold on; linewidth(3); plot(t,cos(t)); linewidth(1); hold off;
	title("Cosine and its truncated series"); wait(delay());

## L_2-Approximation.
	"Some approximation problems."
	"" "First the x-values and the function values are computed:"
	"" ">x=0:0.02:1; y=sqrt(x);", x=0:0.02:1; y=sqrt(x);
	"" "Then the matrix of polynomial values:"
	"" ">n=length(x); p=polyfit(x,y,5),",
	n=length(x); p=polyfit(x,y,5),
	"" 
	">xplot(x,y); hold on; color(3); "| ...
	"plot(x,polyval(p,x)); color(1); hold off;"
	pause();
	subplot(211);
	xplot(x,y); hold on; color(3);
	plot(x,polyval(p,x)); color(1); hold off;
	title("sqrt(x) and its best L_2-approximation (return)");
	subplot(212);
	xplot(x,y-polyval(p,x)); title("The error"); wait(delay());
	"The maximal error in this interval is"
	"" ">max(abs(y-polyval(p,x)))", max(abs(y-polyval(p,x))),
	pause();
	
## Interpolation.
	"Interpolation:" ""
	">t=equispace(0,1,5); s=sin(t); d=interp(t,s);",
	"" ">x=0:0.01:1; max(x,abs(interpval(t,d,x)-sin(x)))",
	"" ">plot(x,interpval(t,d,x)-sin(x)); ",
	pause();
	t=equispace(0,1,5); s=sin(t); d=interp(t,s);
	x=0:0.01:1; max(abs(interpval(t,d,x)-sin(x))),
	subplot(211);xplot(x,sin(x)_interpval(t,d,x));title("interpolation polynom (return)");
	subplot(212);xplot(x,interpval(t,d,x)-sin(x));
	title("Interpolation error"); wait(delay());	
	
## Fourierreihe.
	"Discrete Fourier series:"
	""
	"Calculate the Fourier coefficients corresponding to a signal period"
	">n=64; t=(0:n-1)*2:pi/n; s=(t<pi); w=fft(s);"
	n=64; t=(0:n-1)*2*pi/n; s=(t<pi); w=fft(s); 
	"Here, we know that even harmonics do not exist, so null the coefficients"
	"w[2*(2:n/2)-1]=0;"
	w[2*(2:n/2)-1]=0;
	"Show the time signal reconstructed from a selection of Fourier coefficients."
	">k=10; w[k+2:n-k]=zeros(1,n-2*k-1); s1=re(ifft(w));"
	pause();
	n=64; t=(0:n-1)*2*pi/n; s=(t<pi); w=fft(s); w[2*(2:n/2)-1]=0;

	for k=0 to 32 step 2;
		w1=w;
		w1[k+2:n-k]=0; s1=re(ifft(w1));
		subplot(311);
		setplot(0, 2*pi, -0.3, 1.3);
		xplot(t,s); hold on; color(11); plot(t,s1); color(1); 
		xlabel("1 period of the signal");title("Fourier series (return)");hold off;
		subplot(312);
		setplot([-n/2,n/2,-0.1,0.6]);xplot(-n/2:n/2-1,abs(w1[n/2+1:n]|w1[1:n/2]),style="lc,c11,mo,w=2");
		xlabel("harmonic number n");ylabel("|c_n|");
		subplot(313);
		setplot([-n/2,n/2,-100,100]);xplot(-n/2:n/2-1,180/pi*arg(w1[n/2+1:n]|w1[1:n/2]),style="lc,c11,mo,w=2");
		xlabel("harmonic number n");ylabel("arg c_n"); wait(delay());
	end;

## Fourier analysis.
	"Fourier analysis"
	""
	"The signal:"
	">n=128; fs=100; Ts=1/fs; t=(0:n-1)*Ts; x=sin(2*pi*10*t)+2*cos(2*pi*15*t);"
	"" "Add some noise:"
	">y=x+normal(size(x));"
	""
	pause();
	"Now the Fourier analysis:" 
	n=128; fs=100; Ts=1/fs; t=(0:n-1)*Ts; x=sin(2*pi*10*t)+2*cos(2*pi*15*t);
	y=x+normal(size(x));
	subplot(211);
	setplot(0,(n-1)*Ts,-5, 5); 
	xplot(t,x_y,"c11+");
	title("Signal und signal with noise (return)");
	hold off;
	c=fft(y); p=abs(c)^2;
	subplot(212);
	xplot((-n/2:n/2-1)*fs/n,p[n/2+1:n]|p[1:n/2],"lc,c11");
	title("Spectral analysis"); wait(delay());
	
	return 0;

endfunction

... *** Statistik - Demo ***

function statdemo
## Statistikdemo.
	
## t-Test.
	"Statistics :" ""
	"Let us generate 10 simulated measurements:"
	"" ">x=1000+5*normal(1, 10)",
	x=1000+5*normal(1, 10),
	"" "Mean value:" ">m=sum(x)/10", m=sum(x/10),
	"" "Standart deviation:", ">d=sqrt(sum((x-m)^2)/9),"
	d=sqrt(sum((x-m)^2)/9),
	pause();
	"" "A 95-% confidence interval for the mean value:"
	"" ">t=invtdis(0.975,9)*d/3; [m-t,m+t]",
	t=invtdis(0.975,9)*d/3; [m-t,m+t],
	pause();
	
## xhi^2
	"Statistical evaluation of dice throws."
	""
	">x=random(1, 600); t=count(x*6,6)'",
	x=random(1, 600); t=count(x*6,6),
	"Compute chi^2" ">chi=sum((t-100)^2/100),"
	chi=sum((t-100)^2/100),
	"We decide that the dice is unbiased, since"
	">1-chidis(chi,5)" 1-chidis(chi,5),
	pause();
	"Now we simulate a biased dice:"
	">x=(x<0.95)*x; t=count(x*6,6), chi=sum((t-100)^2/100);"
	x=(x<0.95)*x; t=count(x*6,6), chi=sum((t-100)^2/100);
	"Error probability:"
	">1-chidis(chi,5)"
	1-chidis(chi,5),
	pause();
	
## Binomialverteilung
	"The binomial distribution:"
	"" ">n=20; t=0:n; p=0.4; b=p^t*(1-p)^(n-t)*bin(n,t);"
	n=20; t=0:n; p=0.4; b=p^t*(1-p)^(n-t)*bin(n,t);
	"" ">d=sqrt(n*p*(1-p));"
	">s=exp(-((t-20*p)/d)^2/2)/(d*sqrt(2*pi));"
	">hold on; plot(t,s); hold off;"
	pause();
	d=sqrt(n*p*(1-p));
	s=exp(-((t-20*p)/d)^2/2)/(d*sqrt(2*pi));
	subplot(111);
	xplot(t,b_s,style="lnmxc11+;l-");
	title("Binomial and normal distribution (press return)"); wait(delay());

## Mehrfeldertest
	"chi^2 test:"
	""
	"Given the matrix:"
	">a"
	stdformat(3);
	a=[23,37,43,52,67,74;45,25,53,40,60,83];
	a, shortformat();
	""
	"The matrix of expected values:"
	">{k,l}=field(sum(a')',sum(a)'); s=totalsum(a); b=(k*l)/s;"
	{k,l}=field(sum(a')',sum(a)'); s=totalsum(a); b=(k*l)/s;
	pause();
	"Compute chi^2:"
	">chi=totalsum((a-b)^2/b); 1-chidis(chi,5),"
	chi=totalsum((a-b)^2/b); 1-chidis(chi,5),
	""
	"That means we decide that the columns do not depend of the rows."
	pause();

## Regression.
	"Regression analysis:" ""
	"A linear function with errors: "
	"" ">t=0:0.2:5; s=2*t+3+0.2*normal(size(t));",
	t=0:0.2:5; s=2*t+3+0.2*normal(size(t));
	">p=polyfit(t,s,1),",
	p=polyfit(t,s,1),
	">subplot(111);xplot(t,s_polyval(p,t),style=\"lnm+c11+;l-\");"
	pause();
	subplot(111);xplot(t,s_polyval(p,t),style="lnm+c11+;l-");
	title("The best fit (press return)"); wait(delay());
	
	return 0;
endfunction

function fibo(n)
## fibo(n) liefert einen Vektor mit den ersten n Finonaccizahlen.
	if (n<3) return [1 1]; endif;
	f=ones(1, n);
	for i=3 to n
		f[i]=f[i-1]+f[i-2];
	end
	return f;
endfunction

function itertest(x)
## itertest(x) konvergiert gegen sqrt(2).
	repeat
		x=(x+2/x)/2; x,
		if x*x~=2 break; endif
	end
	return x;
endfunction
	
function multitest
	return {arg1,arg1*arg1}
endfunction

function hut (x,y)
	si=size(x,y); z=zeros(si);
	loop 1 to prod(si)
		r=x{#}^2+y{#}^2;
		if r<1 z{#}=exp(-1/(1-r)); endif
	end
	return exp(1)*z;
endfunction
		
function udfdemo
	"EULER is programmable.",
	"" "As an example we list a function, which computes the first n"
	"fibonacci numbers when called with fibo(n):"
	pause();
	">show fibo",
	show fibo;
	">fibo(5)", fibo(5),
	pause();
	"As you can see, a function starts with" 
	">function name" 
	"and ends with"
	">end"|"function" 
	"Every function must have a return statement, which stops it."
	pause();
	"Arguments are named arg1, arg2, arg3, ...,"
	"unless they are given names in the function header."
	"argn() gives the number of arguments. All variables are local and"
	"cease to exist after function execution is finished."
	"But the command" ">global name"
	"allows acces of a global variable."
	pause();
	
	"There are:" 
	""
	"Conditionals:"
	">if condition ... { else ... } endif"
	""
	"The condition can be any expression, especially a test like < > etc."
	"Logical connections are made with || (or) and && (and)."
	"!condition means: not condition."
	pause();
	"Repetitive statements:"
	">for variable=start to end { step value } ... end"
	">loop start to end ... end"
	">repeat ... end"
	pause();
	"In loop's the index can be accessed with index() or #."
	"All these statements can be left with a break or return ...;"
	pause();
	
	"" "Example:"
	""
	show itertest
	""
	">longformat(); itertest(1.5), shortformat();", 
	longformat(); itertest(1.5), shortformat();
	pause();

	"A function can return multiple values and these values can be"
	"assigned to several variables."
	""
	"Syntax:"
	">return {variable,variable,...}"
	"The brackets { and } are part of the syntax here."
	"Then the multiple assignment looks like:"
	">{a,b}=test(parameter)"
	pause();
	"An example:"
	""
	show multitest
	""
	"{a,b}=multitest(2); a, b,", {a,b}=multitest(2); a, b,
	pause();
	
	"Comments start with #"|"#. Comments are not listed with show."
	"If the first lines of a functions start with #"|"#,"
	"they are considered help text. With help this text can be read."
	""
	">help field" "", help field,
	pause();

	"Since all EULER functions work for vector and matrix input,"
	"user defined functions should be written the same way."
	"Most of the time, this is automatic. But sometimes a function"
	"uses a complicate algorithm or cases. Then there is the {} indexing"
	"which treats the matrix like a vector."
	pause();
	">show hut", show hut
	""
	fullwindow();
	v=view(5,2,0,0.4);
	">{x,y}=field(-1.2:0.2:1.2,-1.2:0.2:1.2); framedsolid(x,y,hut(x,y));",
	{x,y}=field(-1.2:0.2:1.2,-1.2:0.2:1.2); 
	framedsolid(x,y,hut(x,y)); title("(return)");
	view(v);
	wait(delay());
	""
	shrinkwindow();
	">x=-2:0.05:2; xplot(x,hut(x,0));",
	x=-2:0.05:2; xplot(x,hut(x,0)); title("(return)"); wait(delay());
	pause();

	"Functions can be written with an external editor and collected within a file."
	"This file can then be loaded with"
	"" ">load filename" ""

	"You can echo output to a file"
	""
	">dump filename"
	"" 
	"Again, filename must be string expression."
	pause();
	"With"
	">dump"
	"echoing is stopped and the file closed."
	""
	"Dumps are always appended to the file."
	"With"
	">remove filename"
	"the file is removed (killed)."
	""
	"By the way:"
	">forget f; clear a;"
	"removes the function 'f' and the variable 'a'."
	pause();
	
	return 0
endfunction

function f(x)
	return exp(x)-1.5;
endfunction

function dgl(x,y)
	return -2*x*y;
endfunction

function specialdemo
## Simpson
	"Simpson integral : ",
	""
	">show f", show f;
	""
	">t=simpson(\"f\",0,1)", t=simpson("f",0,1),
	""
	"Let's evaluate the error : "
	""
	"t-(exp(1)-2.5)," t-(exp(1)-2.5),
	pause();

## Bisection
	"Solutions of an equation : "
	""
	">show f", show f;
	""
	">t=bisect(\"f\",0,1),", t=bisect("f",0,1),
	""
	"Let's evaluate the error : "
	""
	">t-log(1.5)", t-log(1.5),
	pause();
	
## Differential equations
	"Differential equations : "
	""
	">show dgl", show dgl;
	""
	">t=0:0.05:2; s=heun(\"dgl\",t,1); xplot(t,s,\"c1+\");"
	subplot(111);t=0:0.05:2; s=heun("dgl",t,1); xplot(t,s,"c11+",lw=2); title("(return)"); wait(delay());
	""
	"Maximal error : "
	""
	">max(abs(s-exp(-t^2)))", max(abs(s-exp(-t^2))),
	pause();	

## Eigenvalues and eigenvectors
	"Eigenvalues and eigenvectors : " ""
	"Given the matrix"
	">A=[0,1;1,1]", A=[0,1;1,1],
	""
	"We compute the eigenvalues and eigenvectors of A."
	">{l,V}=eigen(A); l=re(l),"
	{l,V}=eigen(A); l=re(l),
	">V=re(V),"
	V=re(V), 
	pause();
	"Then V'.D.V=A."
	""
	">D=diag(size(A),0,l); V'.D.V,"
	D=diag(size(A),0,l); V'.D.V,
	pause();

	return 0;	
endfunction

function grafiktutor
## Demo of graphical features of Retro
	"Explanation of some graphic functions."
	""
	"First we compute a vector of x-values."
	">x=-1:0.02:1;" x=-1:0.02:1;
	"Then we compute the function at these points."
	">y=sin(4*pi*x)*exp(-x*x);" y=sin(4*pi*x)*exp(-x*x);
	"Finally we can plot the function."
	">xplot(x,y);"
	pause();
	subplot(111); xplot(x,y); wait(delay);
	"The plot can be given a title."
	">title(\"sin(4*pi*x)*exp(-x*x),(return)\");"
	pause();
	title("sin(4*pi*x)*exp(-x*x),(return)"); wait(delay());
	"To draw another plot above it, we use the hold command.",
	">hold on; plot(x,cos(4*pi*x)*exp(-x*x)); hold off;"
	pause();
	hold on; plot(x,cos(4*pi*x)*exp(-x*x)); hold off; wait(delay);

	"One can plot several curves wth a single plot command."
	"",
	">shrinkwindow();" shrinkwindow();
	""
... in the old days, things had to be done like that 
...	">{x,n}=field(linspace(-1,1,50),1:5); T=cos(n*acos(x));"
...	{x,n}=field(linspace(-1,1,50),1:5); T=cos(n*acos(x));
	">n=(1:5)';x=linspace(-1,1,50);T=cos(n*acos(x));"
	n=(1:5)';x=linspace(-1,1,50);T=cos(n*acos(x));
	"This way we computed 5 rows of 5 functions cos(n*acos(x)). n is a parameter."
	"", "First we set the plot region manually.",
	">setplot(-1.5,1.5,-1.5,1.5);"
	"Then we plot all functions, with a diferent color for each one,"
	"starting from color 1, then incrementing the color index for each function."
	">xplot(x,T,\"c1+\"); wait(delay);"
	setplot(-1.5,1.5,-1.5,1.5);
	xplot(x,T,"c1+");  wait(delay);
	"One can also mark a point on the screen with the mouse."
	">title(\"Click the left mouse button.\"); c=mouse(); c,"
	pause();
	title("Click the left mouse button."); c=mouse(); c,
	"These are the coordinates, you choose."
	pause();
	
	"Now a function with a singularity."
	""
	">shrinkwindow(); t=-1:0.01:1; setplot(-1,1,-10,10);"
	shrinkwindow(); t=-1:0.01:1; setplot(-1,1,-10,10);
	">xplot(t,1/t_1/t^2,\"c1+,w=2\"); title(\"1/t and 1/t^2 (return)\"); wait(delay());"
	pause();
	xplot(t,1/t_1/t^2,"c1+,w=2"); title("1/t and 1/t^2 (return)"); wait(delay());
	""

	"Functions of two variables :"
	""
	"First we compute a grid of x- and y-coordinates."
	">{x,y}=field(-1:0.1:1,-1:0.1:1);"
	{x,y}=field(-1:0.1:1,-1:0.1:1);
	"", "Then we compute the function values at these grid points and"
	"draw them."
	">z=x*x+x*y-y*y; mesh(z);"
	pause();
	z=x*x+x*y-y*y; fullwindow(); mesh(z); wait(delay());
	"The contour level lines of these function we can inspect with"
	">contour(z,-3:0.5:3);"
	pause();
	contour(z,-3:0.5:3); wait(delay);
	"A perspective view of the same surface."
	">view(2.5,1,1,0.5); solid(x,y,z);"
	pause();
	view(2.5,1,1,0.5); solid(x,y,z); wait(delay);
	"A wire frame model."
	">wire(x,y,z);"
	pause();
	wire(x,y,z); wait(delay);
	
	return 0
endfunction

function abspann
	"As I think, this demo has given you a good impression. So"
	"EULER is well suited for many tasks in applied mathematics, as"
	"well as for viewing functions of one or two variables."
	""
	"The author wishes you success with mathematics and EULER!"
	return 0
endfunction

function torus
## some torus type bodies.
	view([4,1,0,0.6]);
	x=linspace(0,2*pi,40);
	y=linspace(0,2*pi,20);
	cosphi=dup(cos(x),length(y));
	sinphi=dup(sin(x),length(y));
	cospsi=dup(cos(y'),length(x));
	sinpsi=dup(sin(y'),length(x));
## faster than {phi,psi}=field(x,y); cosphi=cos(phi); ...

## a torus with a thick and a thin side.	
	factor=1.5+cospsi*(cosphi/2+0.6);
	X=cosphi*factor;
	Y=sinphi*factor;
	Z=sinpsi*(cosphi/2+0.6);
	solid(X,Y,Z); title("(Return)"); wait(delay);
	
## a deformed torus
	factor=1.5+cospsi;
	X=cosphi*factor;
	Y=sinphi*factor;
	Z=sinpsi+dup(cos(2*x),length(y));
	solid(X,Y,Z); title("(Return)"); wait(delay);
	
## the Moebius band
	t=dup(linspace(-1,1,20)',length(x));	
	x=linspace(0,2*pi,40);
	cosphi1=dup(cos(x),length(y));
	sinphi1=dup(sin(x),length(y));
	factor=2+cosphi1*t;
	X=cosphi*factor;
	Y=sinphi*factor;
	Z=sinphi1*t;
	solid(X,Y,Z); title("Return"); wait(delay);

	return 0;
endfunction

function tube
## some tube like bodies.
	view([3,1,0,0.1]);
	x=linspace(0,2*pi,40);

## a atomic modell or so.
	y=0.1+(sin(linspace(0,pi,15))| ...
		1.5*sin(linspace(0,pi,10))|sin(linspace(0,pi,15)));
	cosphi=dup(cos(x),length(y));
	sinphi=dup(sin(x),length(y));
	f=dup(y',length(x));
	
	solid(f*cosphi,f*sinphi,dup(linspace(-2,2,length(y)-1)',length(x)));
	title("(Return)");
	wait(delay); 

## a black hole
	t=linspace(0,1,20);
	cosphi=dup(cos(x),length(t));
	sinphi=dup(sin(x),length(t));
	f=dup((t*t+0.2)',length(x));
	view([3,1.5,0,0.7]);
	solid(f*cosphi,f*sinphi,dup(t'*2-1,length(x)));
	title("(Return)"); wait(delay);

	return 0;
endfunction

function minimal (r,phi)
	R=dup(r',length(phi));
	X=R*dup(cos(phi),length(r))+R*R/2*dup(cos(2*phi),length(r));
	Y=-R*dup(sin(phi),length(r))-R*R/2*dup(sin(2*phi),length(r));
	Z=4/3*dup((r^1.5)',length(phi))*dup(cos(1.5*phi),length(r))-1;
	view(9,1,-1.5,0.2);
	solid(X,Y,Z);
	title("A minmal surface (Return)");
	wait(delay);
	return 0;
endfunction

function spiral
	{r,a}=field(0:0.1:1,0:pi()/8:6*pi());
	z=a/8;
	x=r*cos(a)*(1-a/20);
	y=r*sin(a)*(1-a/20);
	z=z-1.5;
	view(4,1.5,0.5,0.3);
	framedsolid(x,y,z); title("(Return)"); wait(delay);
	return 0;
endfunction

function rings
	rr=0.2;
	t=linspace(0,2*pi,10);
	s=linspace(0,2*pi,41); n=length(s);
	r=dup(1+cos(t)*rr,n)'; m=length(t);
	x=dup(cos(s),m)*r; y=dup(sin(s),m)*r;
	z=dup(sin(t)*rr,n)';
	view([4,1.5,0.3,0.3]);
	X=x_(x+1.3)_(x-1.3);
	Y=y_-z_-z;
	Z=z_y_y;
	solid(X,Y,Z,[m,2*m]);
	return 0;
endfunction

function startdemo
	{x,y}=field(-1:0.1:1,-1:0.1:1);
	z=x*x+y*y;
	mesh(z); title("(Return)"); wait(delay);
	return 0;
endfunction

function demo3d
	startdemo();
	torus();
	tube();
	minimal(0.1:0.1:2.5,0:pi()/20:2*pi());
	spiral();
	rings();
	return 0;
endfunction

function demos
	normaldemo();
	grafikdemo();
	apprdemo();
	statdemo();
	udfdemo();
	specialdemo();
	grafiktutor();
	demo3d();
	return 0;
endfunction;

function demo (del=5)
	global demodelay;
	if argn==1
		demodelay=del;
	endif
	repeat
		shortformat();linewidth(1);color(1);
		demos();
		abspann();
		pause();
	end
	return 0
endfunction

""
show demos
""
"Use one of the above, or type demo() or demo(10)."

"<< Demo will start after 10 seconds. Press Esc to stop it. >>"
wait(10);
demo();