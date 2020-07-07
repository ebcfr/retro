... The Utilities.

"Loading utilities, Version 3.04, Apr 16, 1993."

Epsilon=epsilon();
ResetEpsilon=epsilon();
ResetView=view();
Pi=pi();
E=exp(1);
I=1i;

function reset
## reset() resets espilon() and some other things
	global ResetEpsilon,ResetView;
	setepsilon(ResetEpsilon);
	style(""); hold off;
	color(1);
	shrinkwindow();
	shortformat();
	linewidth(1);
	view(ResetView);
	huecolor(1);
	return 0;
endfunction

function wait
## waits for a key (at most 5 min)
	return wait(300);
endfunction

function longformat
## longformat() sets a long format for numbers
	return format("STD",[0,16]);
endfunction

function shortformat
## shortformat() sets a short format for numbers
	return format("STD",[0,6]);
endfunction;

function stdformat(digits=6)
## use a smart format to print litterals with 'digits' significant digits.
##  ex: 123.45  3.34567E-14
	return format("STD",[0,digits]);
endfunction

function sciformat(digits=6)
## use a scientific format to print litterals with 'digits' significant digits.
##  ex: 1.2345E+2  3.34567E-14
	return format("SCI",[0,digits]);
endfunction

function fixedformat(digits=6)
## use a fixed format to print litterals with 'digits' digits for the fractionnal part.
##  ex: 123.450000  0.003346
	return format("FIXED",[0,digits]);
endfunction

function eng1format(digits=6)
## use a engineering format to print litterals with 'digits' significant digits.
## set the printing format to "ENG1".
##  ex: 123.45E+6, 0.056E-9
	return format("ENG1",[0,digits]);
endfunction

function eng2format(digits=6)
## use a plain text engineering format to print litterals with 'digits' significant digits.
##  ex: 123.45M, 0.056n
	return format("ENG2",[0,digits]);
endfunction

function linspace (a,b,n)
## linspace(a,b,n) generates n+1 linear spaced points in [a,b].
	if a~=b; return a; endif;
	r=a:(b-a)/n:b;
	r[n+1]=b;
	return r;
endfunction

function equispace (a,b,n)
## equispace(a,b,n) generates n+1 euqidistribution (acos) spaced values
## in [a,b].
	m=(1-cos(0:pi()/n:pi()))/2;
	return a+(b-a)*m;
endfunction

function length (v)
## length(v) returns the length of a vector
	return max(size(v));
endfunction

function writeform (x)
	if isreal(x); return printf("%25.16e",x); endif;
	if iscomplex(x);
		return printf("%25.16e",re(x))|printf("+%25.16ei",im(x));
	endif;
	if isstring(x); return x; endif;
	error("Cannot print this!");
endfunction

function write (x,s)
	if argn()==1; s=name(x); endif;
	si=size(x);
	if max(si)==1; s|" = "|writeform(x)|";", return 0; endif;
	s|" = [ ..."
	for i=1 to si[1];
		for j=1 to si[2]-1;
			writeform(x[i,j])|",",
		end;
		if i==si[1]; writeform(x[i,si[2]])|"];",
		else; writeform(x[i,si[2]])|";",
		endif;
	end;
	return 0;
endfunction

... ### use zeros,... the usual way ###

function diag
## diag([n,m],k,v) returns a nxm matrix A, containing v on its k-th
## diagonal. v may be a vector or a real number. Also diag(n,m,k,v);
## diag(A,k) returns the k-th diagonal of A.
	if argn()==4; return diag([arg1,arg2],arg3,arg4); endif;
	error("Illegal argument number!"),
endfunction

function normal
## normal([n,m]) returns a nxm matrix of unit normal distributed random
## values. Also normal(n,m).
	if argn()==2; return normal([arg1,arg2]); endif;
	error("Illegal argument number!"),
endfunction

function random
## random([n,m]) returns a nxm matrix of uniformly distributed random 
## values in [0,1]. Also random(n,m).
	if argn()==2; return random([arg1,arg2]); endif;
	error("Illegal argument number!"),
endfunction

function ones
## ones([n,m]) returns a nxm matrix with all elements set to 1.
## Also ones(n,m).
	if argn()==2; return ones([arg1,arg2]); endif;
	error("Illegal argument number!"),
endfunction

function zeros
## zeros([n,m]) returns a nxm matrix with all elements set to 0.
## Also zeros(n,m).
	if argn()==2; return zeros([arg1,arg2]); endif;
	error("Illegal argument number!"),
endfunction

function matrix
## matrix([n,m],x) returns a nxm matrix with all elements set to x.
## Also matrix(n,m,x).
	if argn()==3; return matrix([arg1,arg2],arg3); endif;
	error("Illegal argument number!"),
endfunction


... ### linear algebra things ###

function id (n)
## id(n) creates a nxn identity matrix.
	return diag([n n],0,1);
endfunction

function inv (a)
## inv(a) produces the inverse of a matrix.
	return a\id(cols(a));
endfunction

function fit (a,b)
## fit(a,b) computes x such that ||a.x-b||_2 is minimal.
	A=conj(a');
	return symmult(A,a)\(A.b')
endfunction

function kernel (a)
## kernel(a) computes the kernel of the quadratic matrix a.
	{aa,r,c,d}=lu(a);
	n=size(a); nr=size(r);
	if nr[2]==n[2]; return zeros([1,n[2]])'; endif;
	if nr[2]==0; return id(n[2]); endif;
	c1=nonzeros(c); c2=nonzeros(!c);
	b=lusolve(aa[r,c1],a[r,c2]);
	m=size(b);
	x=zeros([n[2] m[2]]);
	x[c1,:]=-b;
	x[c2,:]=id(cols(c2));
	return x
endfunction

function image (a)
## image(a) computes the image of the quadratic matrix a.
	{aa,r,c,d}=lu(a);
	return a[:,nonzeros(c)];
endfunction

function det (a)
## det(A) returns the determinant of A
	{aa,r,c,d}=lu(a);
	return d;
endfunction

function eigenvalues (a)
## eigenvalues(A) returns the eigenvalues of A.
	return polysolve(charpoly(a));
endfunction

function eigenspace (a,l)
## eigenspace(A,l) returns the eigenspace of A to the eigenvaue l.
	k=kernel(a-l*id(cols(a)));
	if k==0; error("No eigenvalue found!"); endif;
	si=size(k);
	loop 1 to si[2];
		x=k[:,index()];
		k[:,index()]=x/sqrt(x'.x);
	end;
	return k;
endfunction

function eigen (A)
## eigen(A) returns the eigenvectors and a basis of eigenvectors of A.
## {l,x,ll}=eigen(A), where l is a vector of eigenvalues,
## x is a basis of eigenvectors,
## and ll is a vector of distinct eigenvalues.
	l=eigenvalues(A);
	s=eigenspace(A,l[1]);
	si=size(s); v=dup(l[1],si[2])'; vv=l[1];
	l=eigenremove(l,si[2]);
	repeat;
		if min(size(l))==0; break; endif;
		ll=l[1]; sp=eigenspace(A,ll);
		si=size(sp); l=eigenremove(l,si[2]);
		s=s|sp; v=v|dup(ll,si[2])'; vv=vv|ll;
	end;
	return {v,s,vv}
endfunction


function eigenspace1
## eigenspace1(A,l) returns the eigenspace of A to the eigenvalue l.
## returns {x,l1}, where l1 should be an improvement over l, and
## x contains the eigenvectors as columns.
	eps=epsilon();
	repeat;
		k=kernel(arg1-arg2*id(cols(arg1)));
		if k==0; else; break; endif;
		if epsilon()>1 break; endif;
		setepsilon(100*epsilon());
	end;
	if k==0; error("No eigenvalue found!"); endif;
	setepsilon(eps);
	{dummy,l}=eigennewton(arg1,k[:,1],arg2);
	eps=epsilon();
	repeat;
		k=kernel(arg1-l*id(cols(arg1)));
		if k==0; else; break; endif;
		if epsilon()>1 break; endif;
		setepsilon(100*epsilon());
	end;
	if k==0; error("No eigenvalue found!"); endif;
	setepsilon(eps);
	si=size(k);
	loop 1 to si[2];
		x=k[:,index()];
		k[:,index()]=x/sqrt(x'.x);
	end;
	return {k,l};
endfunction

function eigenremove(l)
## helping function.
	return l(nonzeros(!(l[1]~=l)))
endfunction

function eigen1(A)
## eigen(A) returns the eigenvectors and a basis of eigenvectors of A.
## {l,x,ll}=eigen(A), where l is a vector of eigenvalues,
## x is a basis of eigenvectors,
## and ll is a vector of distinct eigenvalues.
## Improved version of eigen.
	l=eigenvalues(A);
	{s,li}=eigenspace1(A,l[1]);
	si=size(s); v=dup(li,si[2])'; vv=li;
	l=eigenremove(l,si[2]);
	repeat;
		if min(size(l))==0; break; endif;
		{sp,li}=eigenspace1(A,l[1]);
		si=size(sp); l=eigenremove(l,si[2]);
		s=s|sp; v=v|dup(li,si[2])'; vv=vv|li;
	end;
	return {v,s,vv}
endfunction

function eigennewton
## eigennewton(a,x,l) does a newton step to improve the eigenvalue l
## of a and the eigenvector x.
## returns {x1,l1}.
	a=arg1; x=arg2; x=x/sqrt(x'.x); n=cols(a);
	d=((a-arg3*id(n))|-x)_(2*x'|0);
	b=d\((a.x-arg3*x)_0);
	return {x-b[1:n],arg3-b[n+1]};
endfunction

function hilb (n)
## hilb(n) produces the nxn-Hilbert matrix.
    {i,j}=field(1:n,1:n);
    return 1/(i+j-1);
endfunction

... ### polynomial fit ##

function polydif (p)
## polydif(p) returns the polynomial p'
	n=cols(p);
	if (n==1); return 0; endif;
	return p[2:n]*(1:n-1);
endfunction

function polyfit (xx,yy,n)
## fit(x,y,degree) fits a polynomial in L_2-norm to (x,y).
	A=ones(size(xx))_dup(xx,n); A=cumprod(A');
	return fit(A,yy)';
endfunction
 
function field (x,y)
## field(x,y) returns {X,Y} such that the X+i*Y is a rectangular
## grid in C containing the points x(n)+i*y(m). x and y must be
## 1xN and 1xM vectors.
	return {dup(x,cols(y)),dup(y',cols(x))};
endfunction

function totalsum (A)
## totalsum(a) computes the sum of all elements of a
	return sum(sum(A)');
endfunction

function totalmin (A)
## Returns the total minimum of the elements of a
	return min(min(A)')'
endfunction

function totalmax (A)
## Returns the total maximum of the elements of a
	return max(max(A)')'
endfunction

function log10(x)
# base 10 logaithm
  return log(x)/log(10);
endfunction

function sinh
## sinh(x) = (exp(x)-exp(-x))/2
	h=exp(arg1);
	return (h-1/h)/2;
endfunction

function cosh
## cosh(x) = (exp(x)+exp(-x))/2
	h=exp(arg1);
	return (h+1/h)/2;
endfunction

function arsinh
## arsinh(z) = log(z+sqrt(z^2+1))
	return log(arg1+sqrt(arg1*arg1+1))
endfunction

function arcosh
## arcosh(z) = log(z+(z^2-1))
	return log(arg1+sqrt(arg1*arg1-1))
endfunction

function heun (ffunction,t,y0)
## y=heun("f",t,y0,...) solves the differential equation y'=f(t,y).
## f(t,y,...) must be a function. 
## y0 is the starting value.
	n=cols(t);
	y=dup(y0,n);
	loop 1 to n-1;
		h=t[#+1]-t[#];
		yh=y[#]; xh=t[#];
		k1=ffunction(xh,yh,args(4));
		k2=ffunction(xh+h/2,yh+h/2*k1,args(4));
		k3=ffunction(xh+h,yh+2*h*k2-h*k1,args(4));
		y[#+1]=yh+h/6*(k1+4*k2+k3);
	end;
	return y';
endfunction

solveeps=epsilon();

function bisect (ffunction,a,b)
## bisect("f",a,b,...) uses the bisection method to find a root of 
## f in [a,b].
	global solveeps;
	if ffunction(b,args(4))<0;
		b1=a; a1=b;
	else;
		a1=a; b1=b;
	endif;
	if ffunction(b1,args(4))<0 error("No zero in interval"); endif;
	repeat
		m=(a1+b1)/2;
		if abs(a1-b1)<solveeps; return m; endif;
		if ffunction(m,args(4))>0; b1=m; else a1=m; endif;
	end;
endfunction

function secant (ffunction,a,b)
## secant("f",a,b,...) uses the secant method to find a root of f in [a,b]
	global solveeps;
	x0=a; x1=b; y0=ffunction(x0,args(4)); y1=ffunction(x1,args(4));
	repeat
		x2=x1-y1*(x1-x0)/(y1-y0);
		if abs(x2-x1)<solveeps; break; endif;
		x0=x1; y0=y1; x1=x2; y1=ffunction(x2,args(4));
	end;
	return x2
endfunction

function simpson (ffunction,a,b,n=50)
## simpson("f",a,b) or simpson("f",a,b,n,...) integrates f in [a,b] with
## the simpson method. f must be able to evaluate a vector. ... is passed
## to f.
	t=linspace(a,b,2*n);
	s=ffunction(t,args(5));
	ff=4-mod(1:2*n+1,2)*2; ff[1]=1; ff[2*n+1]=1;
	return sum(ff*s)/3*(t[2]-t[1]);
endfunction

rombergeps=epsilon();

function romberg(ffunction,a,b,m=10)
## romberg(f,a,b) computes the Romberg integral of f in [a,b].
## romberg(f,a,b,m,...) specifies h=(b-a)/m/2^k for k=1,...
## ... is passed to f(x,...)
	global rombergeps;
	y=ffunction(linspace(a,b,m),args(5)); h=(b-a)/m;
	y[1]=y[1]/2; y[m+1]=y[m+1]/2; I=sum(y);
	S=I*h; H=h^2; Intalt=S;
	repeat;
		I=I+sum(ffunction(a+h/2:h:b,args(5))); h=h/2;
		S=S|I*h;
		H=H|h^2;
		Int=interpval(H,interp(H,S),0);
		if abs(Int-Intalt)<rombergeps; break; endif;
		Intalt=Int;
	end;
	return Intalt
endfunction

gaussz = [ ...
-9.739065285171717e-0001,
-8.650633666889845e-0001,
-6.794095682990244e-0001,
-4.333953941292472e-0001,
-1.488743389816312e-0001,
 1.488743389816312e-0001,
 4.333953941292472e-0001,
 6.794095682990244e-0001,
 8.650633666889845e-0001,
 9.739065285171717e-0001];
gaussa = [ ...
 6.667134430868814e-0002,
 1.494513491505806e-0001,
 2.190863625159821e-0001,
 2.692667193099964e-0001,
 2.955242247147529e-0001,
 2.955242247147529e-0001,
 2.692667193099964e-0001,
 2.190863625159821e-0001,
 1.494513491505806e-0001,
 6.667134430868814e-0002];

function gauss10 (ffunction,a,b)
## gauss10("f",a,b,...)
## evaluates the gauss integration with 10 knots an [a,b]
	global gaussa,gaussz;
	z=a+(gaussz+1)*(b-a)/2;
	return sum(gaussa*ffunction(z,args(4)))*(b-a)/2;
endfunction

function gauss (ffunction,a,b,n=1)
## gauss("f",a,b) gauss integration with 10 knots
## gauss("f",a,b,n,...) subdivision into n subintervals.
## a and b may be vectors.
	si=size(a,b); R=zeros(si);
	if n==1;
		loop 1 to prod(si);
			sum=0;
			sum=sum+gauss10(ffunction,a{#},b{#});
			R{#}=sum;
		end;
	else;
		loop 1 to prod(si);
			h=linspace(a{#},b{#},n); sum=0;
			loop 1 to n;
				sum=sum+gauss10(ffunction,h{#},h{#+1},args(5));
			end;
			R{#}=sum;
		end;
	endif;
	return R
endfunction

... ### Broyden ###

broydenmax=50;

function broyden (ffunction,xstart,A=0)
## broyden("f",x) or broyden("f",x,a,...) finds a zero of f.
## x is the starting value and a is a approximation of the jacobian.
## if a is 0, it is neglected.
## ... is passed to f(x,...)
	global solveeps;
	x=xstart; n=cols(x);
	global broydenmax;
	delta=sqrt(solveeps);
	if A==0;
		A=zeros([n n]);
		y=ffunction(x,args(4));
		loop 1 to n;
			x0=x; x0[#]=x0[#]+delta;
			A[:,#]=(ffunction(x0,args(4))-y)'/delta;
		end;
	endif;
	itercount=0;
	y=ffunction(x,args(4));
	repeat
		d=-A\y'; x=x+d';
		y1=ffunction(x,args(4)); q=y1-y;
		A=A+((q'-A.d).d')/(d'.d);
		if (max(abs(d))<solveeps || itercount>broydenmax) break; endif;
		y=y1;
		itercount=itercount+1;
	end;
	if (itercount>broydenmax) error("Iteration failed"); endif;
	return x;
endfunction

function map (ffunction,t)
## map("f",t,...) evaluates the function f at all points of the vector t.
## usually this is the same as f(t,...), but sometimes functions do not
## accept vectors as input.
	si=size(t); s=zeros(si);
	loop 1 to prod(si); s{#}=ffunction(t{#},args(3)); end;
	return s;
endfunction

itereps=epsilon();

function iterate (ffunction,x,n=0)
## iterate("f",x,n,...) iterate the function f(x,...) n times,
## starting with the point x.
## if n is missing or n is 0, then the iteration stops at a fixed point.
## Returns the value after n iterations, and its history.
	global itereps;
	if n==0;
		R=x; Rold=x;
		repeat
			Rnew=ffunction(Rold,args(4));
			R=R_Rnew;
			if max(abs(Rold-Rnew))<itereps; return Rnew; endif;
			Rold=Rnew;
		end;
	else;
		R=zeros(n,cols(x));
		R[1]=x;
		loop 2 to n; R[#]=ffunction(R[#-1],args(4)); end;
		return {R[n],R'}
	endif;
endfunction	

... ### Remez algorithm ###

function remezhelp (x,y,n)
## {t,d,h}=remezhelp(x,y,deg) is the case, where x are exactly deg+2
## points.
	d1=interp(x,y); d2=interp(x,mod(1:n+2,2)*2-1);
	h=d1[n+2]/d2[n+2];
	d=d1-d2*h;
	return {x[1:n+1],d[1:n+1],h};
endfunction

remezeps=0.00001;

function remez(x,y,deg,trace=0)
## {t,d,h,r}=remez(x,y,deg) computes the divided difference form
## of the polynomial best approximation to y at x of degree deg.
## remez(x,y,deg,1) does the same thing with tracing.
## h is the discrete error (with sign), and x the rightmost point,
## which is missing in t.
	global remezeps;
	n=cols(x);
	ind=linspace(1,n,deg+1); h=0;
	repeat
		{t,d,hnew}=remezhelp(x[ind],y[ind],deg);
		r=y-interpval(t,d,x); hh=hnew;
		if trace;
			xind=x[ind];
			plot(x,r); xgrid(xind); ygrid([-h,0,h]);
			title("Weiter mit Taste"); wait(180);
		endif;
		indnew=ind; rr=ind;
		rr[1]=r[ind[1]];
		rr[2]=r[ind[deg+2]];
		loop 2 to deg+1;
			e=extrema(r[ind[index()-1]:ind[index()+1]]);
			if (hh>0);
				indnew[index()]=e[2]+ind[index()-1]; rr[index()]=e[1];
			else
				indnew[index()]=e[4]+ind[index()-1]; rr[index()]=e[3];
			endif;
			hh=-hh;
		end;
		ind=indnew;
		if (abs(hnew)<(1+remezeps)*abs(h)) break; endif;
		h=hnew;
	end;
	{t,d,h}=remezhelp(x[ind],y[ind],deg);
	if trace;
		xind=x[ind];
		plot(x,y-interpval(t,d,x)); xgrid(xind); ygrid([-h,0,h]);
		title("Weiter mit Taste"); wait(180);
	endif;
	return {t,d,h,x[ind[deg+2]]};
endfunction

... ### Natural spline ###

function spline (x,y)
## spline(x,y) defines the natural Spline at points x(i) with
## values y(i). It returns the second derivatives at these points.
	n=cols(x);
	h=x[2:n]-x[1:n-1];
	s=h[2:n-1]+h[1:n-2];
	l=h[2:n-1]/s;
	A=diag([n,n],0,2);
	A=setdiag(A,1,0|l);
	A=setdiag(A,-1,1-l|0);
	b=6/s*((y[3:n]-y[2:n-1])/h[2:n-1]-(y[2:n-1]-y[1:n-2])/h[1:n-2]);
	return (A\(0|b|0)')';
endfunction

function splineval (x,y,h,t)
## splineval(x,y,h,t) evaluates the interpolation spline for
## (x(i),y(i)) with second derivatives h(i) at t.
	p1=find(x,t); p2=p1+1;
	y1=y[p1]; y2=y[p2];
	x1=x[p1]; x2=x[p2]; f=(x2-x1)^2;
	h1=h[p1]*f; h2=h[p2]*f;
	b=h1/2; c=(h2-h1)/6;
	a=y2-y1-b-c;
	d=(t-x1)/(x2-x1);
	return y1+d*(a+d*(b+c*d));
endfunction

