Examples
========

Basic ideas
-----------

Retro can do ordinary arithmetic like

    >(22+5)*(2^4-6)
      270.00000

or

    >exp(-4)
        0.01832

To get a longer output, we simply use the function

    >longformat
      14.000000000000       5.000000000000

defined in UTIL.

    >exp(-4)
       0.018315638889

Note, that the logarithm function computes the natural logarithm

    >log(exp(-4))
      -4.000000000000

and the arcsine, arccosine a.s.o. are called asin,...

    >2*asin(1)-pi
       0.000000000000

Retro can also handle vectors, like

    >[1,2,3,4,5]
    Column 1 to 3:
       1.000000000000       2.000000000000       3.000000000000
    Column 4 to 5:
       4.000000000000       5.000000000000

All data can be assigned to a variable. Its name may consist of up to
15 letters

    >t=[1,2,3,4,5]
    Column 1 to 3:
       1.000000000000       2.000000000000       3.000000000000
    Column 4 to 5:
       4.000000000000       5.000000000000

The important thing to know is that all functions are applied for each
component of the vector. So

    >sqrt(t)
    Column 1 to 3:
       1.000000000000       1.414213562373       1.732050807569
    Column 4 to 5:
       2.000000000000       2.236067977500

computes the square root of all elements of the vector. And

    >sqrt(t)*sqrt(t)
    Column 1 to 3:
       1.000000000000       2.000000000000       3.000000000000
    Column 4 to 5:
       4.000000000000       5.000000000000

multiplies two vectors in each element. If one of the operators is a
scalar (a real or complex number), the operation is applied on the
scalar and each component of the vector.

    >5*t
    Column 1 to 3:
       5.000000000000      10.000000000000      15.000000000000
    Column 4 to 5:
      20.000000000000      25.000000000000
    >t*5
    Column 1 to 3:
       5.000000000000      10.000000000000      15.000000000000
    Column 4 to 5:
      20.000000000000      25.000000000000


It may be confusing to devide vectors, but Retro does it simly deviding
each element of the vector.

    >t/t
    Column 1 to 3:
       1.000000000000       1.000000000000       1.000000000000
    Column 4 to 5:
       1.000000000000       1.000000000000

An expression of the form a:h:b generates the vector [a,a+h,a+2h,...a+nh],
where the n is the biggest n such that a+nh <= b+epsilon. Here, epsilon
is a built in machine constant of small size. Example:

    >t=-1:0.01:1;

The semicolon ";" suppresses the boring output of the vector.

The elements of t are

    >t(1)
      -1.000000000000
    >t(2)
      -0.990000000000

The size of this vector can be asked with

    >size(t)
       1.000000000000     201.000000000000

The size function outputs a 1x2 vector containing the number of rows
and columns of the matrix (we will soon see that vectors are special
matrices).

    >t(201)
       1.000000000000

It is better to use [...], because this notation evaluate faster.

    >t[201]
       1.000000000000

Note, that

   >t[202]

evaluates to a 1x0 vector.

Let us do something useful with a vector. We like to plot a function.
First of all, we use a graphics command to be explained later on.   

    >shrinkwindow
    Column 1 to 3:
      12.000000000000      38.000000000000    1011.000000000000
    Column 4 to 4:
    1011.000000000000

The 14 digit precission becomes boring, so we switch it off

   >shortformat;

Next we plot a function

    >xplot(t,t*t*t-5*t*t+t)
       -1.00000        1.00000       -7.00000        0.05100

xplot(t,s) or plot(t,s) simply plot the points (t[i],s[i]) on the
graphcis screen.

You may even add a title to your plot.

    >title("t^3-5t^2+t");

Another function:

    >xplot(t,sin(2*pi*t))
       -1.00000        1.00000       -1.00000        1.00000
       
We could have stored the values of our function into a vector.

    >s=t^3-5*t^2+t;

Compute the maximum and minimum of this vector

    >max(s)
        0.05100
    >min(s)
       -7.00000
       
Vectors are really special matrices. You can enter a matrix with

    >A=[1,2,3;4,5,6;7,8,9]
        1.00000        2.00000        3.00000
        4.00000        5.00000        6.00000
        7.00000        8.00000        9.00000

Compute the determinant of A.

    >det(A)
        0.00000

A is singular. So let us change it a little bit

    >A[3,3]=10
        1.00000        2.00000        3.00000
        4.00000        5.00000        6.00000
        7.00000        8.00000       10.00000
    >det(A)
       -3.00000
       
Now we solve a linear system.

    >x=[1;1;1]
        1.00000
        1.00000
        1.00000

gives a column vector, which can be multiplied with A.

    >b=A.x
        6.00000
       15.00000
       25.00000

Then

    >A\b
        1.00000
        1.00000
        1.00000

solves the linear system.

The eigenvalues of A are all real.

    >eigenvalues(A)
    Column 1 to 2:
        0.19825        0.00000       -0.90574        0.00000
    Column 3 to 3:
       16.70749       -0.00000

This is a complex vector, which we can make real with

    >re(eigenvalues(A))
        0.19825       -0.90574       16.70749

Zeros of a real-valued function of one variable
-----------------------------------------------

The task is to compute the smallest positive zero of a function.
First we define f.

    >function xi (g)
    udf>  return (2-3*g^2)/(1-2*g^2)*g
    udf>endfunction
    >function l (g)
    udf>  xi=xi(g);
    udf>  return log(g)+8*xi/(1-xi^2)
    udf>endfunction

In this case, we used an interactive definition. Of course, the functions
could also be defined in a file. Let us print the functions.

    >type xi
    function xi (g)
      return (2-3*g^2)/(1-2*g^2)*g
    endfunction
    >type l
    function l (g)
      xi=xi(g);
      return log(g)+8*xi/(1-xi^2);
    endfunction

Now we plot the functions, to see where the zeros are.

    >t=0:0.01:1; s=l(t);
    >setplot(0,1,-5,5); shrinkwindow();
    >xplot(t,s); ygrid(0); xgrid(0:0.1:1); hold off;

To locate the zero we use the mouse.

    >mouse()
        0.11912       -0.15416

There should be a zero close to 0.12. We use bisection to compute it.

    >longformat(); bisect("l",0.1,0.2)
         0.122366058965495

That solves our problem.


The fibonacci numbers
---------------------

Lets investigate the fibonacci numbers defined by f(1)=1, f(2)=1 and
f(n+1)=f(n)+f(n-1). First we write a function computing the first
n fibonacci numbers.

    >function fibo (n)
    udf>## computes the first n fibonacci numbers.
    udf>  r=ones(1,n);
    udf>  for i=3 to n; r[i]=r[i-1]+r[i-2]; end;
    udf>  return r
    udf>endfunction
 
Test:

    >fibo(5)
        1.00000        1.00000        2.00000        3.00000        5.00000

Now we compute a 1000 of them.

    >f=fibo(1000);
    >plot(f)
        1.00000     1000.00000        1.00000  4.34666e+0208

The plot shows a sharp increase of the numbers, probably exponetially.

    >plot(log(f))
        1.00000     1000.00000        0.00000      480.40711
        
Indeed the growth is of exponential nature. The growth rate is about
0.48 as can be seen with

    >plot(log(f)/(1:length(f)))
        1.00000     1000.00000        0.00000        0.48041

The reason is the largest eigenvalue of the matrix

    >A=[0,1;1,1]
        0.00000        1.00000
        1.00000        1.00000

This matrix maps [f(n);f(n+1)] to [f(n+2);f(n+3)].

Let us compute the eigenvalues.

    >{l,V}=eigen(A); D=diag(size(A),0,l);

Thus

    >V.D.V'
    5.42101e-0020        0.00000        1.00000        0.00000
          1.00000        0.00000        1.00000        0.00000

is A. V is a unitary matrix, since all eigenvectors are normed.

Thus A^n = V . D^n . V':

    >V.(D^998).V'.[1;1]
      2.68638e+0208  5.71486e-0226
      4.34666e+0208 -3.53198e-0226
    >f[1000]
      4.34666e+0208

See!

Thus the growth rate is

    >log(max(abs(l)))
        0.48121


The exponential function for matrices
-------------------------------------

This job will be solved in two versions. The first one evaluates
  exp(A) = I + A + A/2 + A/3! + A/4! + ...

    >type mexp
    function mexp (A)
      B=id(length(A)); S=B; n=1;
      repeat;
        if max(max(abs(B)))~=0; break; endif;
        B=A.B/n; S=S+B; n=n+1;
       end;
       return S
     endfunction

The stopping criterion max(max(abs(B)))~=0 tests, if all elements of
B are smaller than epsilon().

The second version uses eigenvalue decomposition.

    >type mexp1
    function mexp1 (A)
      {l,v}=eigen1(A);
      return v.diag(size(A),0,exp(l)).inv(v)
    endfunction

Let us test these functions.

    >A=[1,1,2;3,4,5;6,7,8]
        1.00000        1.00000        2.00000
        3.00000        4.00000        5.00000
        6.00000        7.00000        8.00000

    >eigenvalues(A)
    Column 1 to 2:
        0.30420        0.00000       -0.73432        0.00000
    Column 3 to 3:
       13.43012       -0.00000

which means that A can be diagonalized.

    >mexp(A)
       59292.72131    71084.03394    87292.04912
      169351.28682   203035.19937   249326.53344
      283826.55487   340276.98381   417861.71137
    >mexp1(A)
    Column 1 to 2:
       59292.72131        0.00000    71084.03394        0.00000
      169351.28682        0.00000   203035.19937        0.00000
      283826.55487        0.00000   340276.98381        0.00000
    Column 3 to 3:
       87292.04912        0.00000
      249326.53344        0.00000
      417861.71137        0.00000

Both functions work also for complex arguments.

Solving a two point boundary equation
-------------------------------------

We wish to solve the following differential equation

>   y''(t)=t*y(t)*(y(t)-t)+1, y(0)=0, y(1)=0.

First we define the differential equation; i.e., the system

>   y1'=y2,

>   y2'=ty1(y1-t)+1,

    >type dgl
    function dgl (t,x)
      return [x[2],t*x[1]*(x[1]-t)+1]
    endfunction

Next we define a function which computes y(1) in dependence of y'(0).

    >type f
    function f (a,h)
      si=size(a); r=zeros(si);
      loop 1 to prod(si);
        l=heun("dgl",h,[0,a{#}]);
        r{#}=l[1,length(l)];
      end;
      return r
    endfunction

Let us get an idea, how solutions of this equation look like.

    >h=0:0.05:1;
    >xplot(h,heun("dgl",h,[0,0]));
    >xplot(h,heun("dgl",h,[0,1]));
    >xplot(h,heun("dgl",h,[0,-1]));
    
Note, that heun returns a vector of function values and a vector of
first derivatives.

Now we solve the equation f(s)=0, which is equivalent to the shooting
method.

    >a=-1:0.1:1; xplot(a,f(a,h));
    >mouse
       -0.51456        0.01060
       
Thus the solution is close to -0.51.

    >s=secant("f",-0.52,-0.51,h)
       -0.51056
       
This is the solution.

    >xplot(h,heun("dgl",h,[0,s]));
    
finally plots the solution and the derivative.


Interest rates
--------------

If you buy an investement for 108 (thousand or so) at year 0 and get
payments of 8 at the end of the next 8 years. At the end of the 8th
year you sell the investment for 100. The problem is to determine
the interest rate of this investment.

First of all, we enter the payments in a vector

    >K=[-108,8,8,8,8,8,8,8,108]
    Column 1 to 5:
     -108.00000        8.00000        8.00000        8.00000        8.00000
    Column 6 to 9:
        8.00000        8.00000        8.00000      108.00000

If the interest rate was i=(1+p/100), then a payment of a in n years
is a/i^n today. Thus the interest rate is the solution of the equation
  K[0] + K[]/i + K[2]/i^2 + ... K[n]/i^n = 0.
If we replace i with 1/x this becomes a polynomial. And we have polysolve
at our hands to get the zeros of polynomials.
        
    >z=polysolve(K)
    Column 1 to 2:
        0.93741        0.00000       -0.71379       -0.71358
    Column 3 to 4:
       -0.71379        0.71358        0.71304       -0.71421
    Column 5 to 6:
        0.71304        0.71421       -0.00035        1.00929
    Column 7 to 8:
       -0.00035       -1.00929       -1.00930  1.28498e-0019
       
Of these zeros the first one is the solution to our problem

    >r=(1/re(z[1])-1)*100
        6.67698

Thus this investment has about 6.7% interest.        

However, this way is quite slow and therefore we should use another
method. In this case the Newton method works very well. The following
functions implements is.

    >type rendite
    function rendite (K)
      K1=polydif(K);
      x=1;
      repeat;
        xn=x-polyval(K,x)/polyval(K1,x);
        if xn~=x; return (1/xn-1)*1e+0002; endif;
        x=xn;
      end;
    endfunction

Note, that the result is rediculously accurate. Instead of xn~=x,
abs(xn-x)<0.0001 would fully suffice.

Lets test it.

    >rendite(K)
        6.67698

Now we can solve a lot of similar questions. A loan is a single payment
made at year 0, and periodic payments over some years. E.g.,
 
    >K=-104|dup(12,10)'
    Column 1 to 5:
     -104.00000       12.00000       12.00000       12.00000       12.00000
    Column 6 to 10:
       12.00000       12.00000       12.00000       12.00000       12.00000
    Column 11 to 11:
       12.00000
    >rendite(K)
        2.69021

is the interest rat of that loan. Lets test some other rates

    >K=-104|dup(13,10)'; rendite(K)
        4.27750
    >K=-104|dup(14,10)'; rendite(K)
        5.80498
    >K=-104|dup(15,10)'; rendite(K)
        7.28074

We could make a function of it

    >type loanrate
    function loanrate (K,R,n)
      return rendite(-K|dup(R,n)')
    endfunction

Including some help text helps.

    >help loanrate
    function loanrate (K,R,n)
    ## loanrate(K,R,n) computes the interest rate of a loan of K (including
    ## disaggio) and n periodic payments of R.
    endfunction

    >loanrate(104,12,10)
        2.69021

But we are better of, if we make the function work for vector input.

    >type loanrate
    function loanrate (K,R,n)
      si=size(K,R,n); x=zeros(si);
      loop 1 to prod(si);
        x{#}=rendite(-K{#}|dup(R{#},n{#})');
      end;
      return x
    endfunction

Then we can plot various interest rates depending on the payments. 

    >r=loanrate(104,12:0.1:17,10); shrinkwindow(); xplot(12:0.1:17,r);


The Fast Fourier Transform
--------------------------

The fast Fourier transform has many applications. Let us start with
a first example involving complex numbers.

We define the 512-th roots of unity with

    >z=exp(I*2*pi*(0:511)/512);

and take the exponential function a these points

    >w=exp(z);
   
To plot the image of the unit circle after the exp function has been
applied, we use

    >shrinkwindow
       12.00000       38.00000     1011.00000     1011.00000
    >xplot(re(w),im(w))
        0.36788        2.71828       -1.32115        1.32115

This gives an oval shape.        

Of course,

    >xplot(re(z),im(z))
       -1.00000        1.00000       -1.00000        1.00000

is the unit circle, which is a little deformed on the screen.

Now,       

    >p=fft(w);

computes the interpolating polynomial; i.e. p(z[i])=w[i]. "ifft(p)"
then evaluates this polynomial at the roots of unity z simultanuously.
There is an error of course, but it is small.

    >max(abs(ifft(p)-w))
      1.38951e-0018
  
Let us truncate p to the first 16 coefficients

    >q=p[1:16]
    Column 1 to 2:
            1.00000 -2.78697e-0020        1.00000 -3.27627e-0020
    Column 3 to 4:
            0.50000 -1.87865e-0020        0.16667 -6.79627e-0020
    Column 5 to 6:
            0.04167 -3.04213e-0020        0.00833 -3.16973e-0020
    Column 7 to 8:
            0.00139 -2.07105e-0020        0.00020 -2.05299e-0020
    Column 9 to 10:
            0.00002 -4.69085e-0020  2.75573e-0006 -3.18317e-0020
    Column 11 to 12:
      2.75573e-0007 -2.58004e-0020  2.50521e-0008 -9.53105e-0021
    Column 13 to 14:
      2.08768e-0009 -9.94706e-0021  1.60590e-0010 -8.97525e-0021
    Column 15 to 16:
      1.14707e-0011 -7.15237e-0021  7.64716e-0013  2.48211e-0021

Since q is a real polynomial, we may as well use  

    >q=re(p[1:16])
    Column 1 to 5:
            1.00000        1.00000        0.50000        0.16667        0.04167
    Column 6 to 10:
            0.00833        0.00139        0.00020        0.00002  2.75573e-0006
    Column 11 to 15:
      2.75573e-0007  2.50521e-0008  2.08768e-0009  1.60590e-0010  1.14707e-0011
    Column 16 to 16:
      7.64716e-0013
  
These coefficients are in fact close to the Taylor series of exp

    >max(abs(q-1/fak(0:15)))
      2.16840e-0019
  
The error of the approximating polynomial q is not very big

    >max(abs(polyval(q,z)-w))
      5.07712e-0014
    >xplot(abs(polyval(q,z)-w))
      1.00000      512.00000  4.51316e-0014  5.07712e-0014

This is a typical error curve.

Another use is signal processing. Let us first define equispaces values
in [0,2pi]

    >t=0:pi/256:2*pi-pi/256;
    >size(t)
        1.00000      512.00000

For the Fourier transform, optimal speed is achieved when the size is
a power of 2.

The signal is the mountain shaped function

    >s=pi-abs(t-pi);
    >xplot(t,s)
        0.00000        6.27091        0.00000        3.14159
        
Then we add a little bit of noise

    >sn=s+normal(size(s))*0.05;
    >style("m."); mark(t,sn);

This gives a cloud of values around the function.

Then

    >xplot(abs(fft(s)));
    >fsn=fft(sn);
    >xplot(abs(fsn));

compares the Fourier transforms of the two functions. The noise can
be seen clearly.
   
To remove the noise, we might try

    >fsn1=fsn*(abs(fsn)>max(abs(fsn))/50);
    >xplot(t,re(ifft(fsn1)))
        0.00000        6.27091        0.15441        2.99504

or even

    >fsn1=fsn*(abs(fsn)>max(abs(fsn))/100);
    >xplot(t,re(ifft(fsn1)))
        0.00000        6.27091        0.10004        3.04940

This looks like the function and gives a smooth curve. However,
it may sound quite different from the function.

We could also remove high frequences with

    >fsn1=fsn[1:32]|zeros(1,512-64)|fsn(512-31:512);
    >xplot(t,re(ifft(fsn1)))
        0.00000        6.27091        0.02219        3.14823


The apple set
-------------

We like to plot the fractal set, defined as the set of all points z,
such that the iteration w -> z+w*w is bounded when started at z. First
we define a function, which performs this iteration 10 times.

    >type apple
    function apple (z)
      w=z;
      loop 1 to 10; w=z+w*w; end;
      return w;
    endfunction

Next a rectangle in the complex plane is iterated

    >{x,y}=field(-2.5:0.05:1,-1.5:0.05:1.5);
    >z=x+I*y;
    >w=apple(z);

This takes some time.

To view the result, we use

    >view(5,1.5,0.5,0.5); twosides(0);
    >framedsolid(x,y,abs(w)<2,2); twosides(1);

This looks quite nice, since we stopped Retro's backside detection.

    >contour(abs(w)<2,0.5);

When abs(w[i,j])<2 is 1, we decide that the point z[i,j] should belong
to the apple set.


Iteration
---------

Let us explore the function "iterate" defined in UTIL

    >help iterate
    function iterate (ffunction,x,n)
    ## iterate("f",x,n,...) iterate the function f(x,...) n times,
    ## starting with the point x.
    ## if n is missing or n is 0, then the iteration stops at a fixed point.
    ## Returns the value after n iterations, and its history.

We try it out with the cosine function, which is a builtin function.
However, we could as well use any user defined function.

    >iterate("cos",0)
        0.73909

This is the fixed point of the cosine.

    >x=iterate("cos",0.73); x-cos(x)
     -2.83953e-0016

We can also iterate a vector x. 

    >x=0:0.01:1;
    >{y,Y}=iterate("cos",x,10);
    >shrinkwindow(); xplot(x,Y')
        0.00000        1.00000        0.00000        1.00000

You see a plot of the iterateted cosine functions, which approach a
constant function with geometrical speed.


Splines
-------

To demonstrate the use of splines, we solve the differential equation

>   y'' + sin(x)^2 *y' + y = cos(x)^2

with a very rough step size.

The function f is defined as

    >type f
    function f (t,y)
      return [y[2],cos(t)^2-y[1]-sin(t)^2*y[2]]
    endfunction

It represents the differential equation and maps (y,y') to its
derivative.

    >t=0:20;
    >s=heun("f",t,[1,0]);
    >plot(t,s[1])
        0.00000       20.00000        0.21585        1.00000

This does not look smoothly. We smooth it out with spline
interpolation

    >sp=spline(t,s[1]);
    >plot(x,splineval(t,s[1],sp,x))
        0.00000       20.00000        0.21224        1.00118

which looks much better. Since "sp" contains the second derivatives
of the spline at the itnerpolation knots, we could as well compute
"sp" ourselves from the differential equation.

    >sp=cos(t)^2-sin(t)^2*s[2]-s[1];
    >plot(x,splineval(t,s[1],sp,x))
        0.00000       20.00000        0.21585        1.00022

There is not much difference.

Now let us see if our grid size was to large.

    >t=0:0.1:20;
    >s=heun("f",t,[1,0]);
    >plot(t,s[1])
        0.00000       20.00000        0.18316        1.00000

This is the same picture, we already had. In fact, there is some
difference. But the behaviour of the solution can be seen with
the rough step size too.


Complex numbers
---------------

To visualize a complex mapping, there is the function cplot in UTIL.
this functions takes a grid of complex values and connects them.

    >help cplot
    function cplot (z)
    ## cplot(z) plots a grid of complex numbers.

To use this function, define a erctangular grid of complex numbers.

    >{x,y}=field(linspace(0,pi(),20),linspace(-1,1,20));
    >z=x+1i*y;

Now

    >cplot(z)
        0.00000        3.14159       -1.00000        1.00000

plots just a rectangular pattern of lines. But

    >cplot(cos(z))
       -1.54308        1.54308       -1.17520        1.17520

shows what the function cos does to the rectangle [0,pi]x[-1,1]. A
pattern of elipses and hyperbola appears. Let us show some grids.

    >xgrid(-2:2)
        0.00000
    >ygrid(-2:2)
        0.00000

To see the effect of the exponential function to the unit circle, we
define

    >{r,phi}=field(0:0.1:1,0:pi()/10:2*pi());
    >z=r*exp(I*phi);
    >cplot(z)
       -1.00000        1.00000       -1.00000        1.00000

This is just the unit circle a little distorted.

    >cplot(exp(z))
        0.36788        2.71828       -1.30249        1.30249

shows the image of the unit circle. With grids:

    >xgrid(1:2)
        0.00000
    >ygrid(-1:1)
        0.00000

To see the real part of the function, i.e. a harmonic function,
we use

    >twosides(0)
        0.00000
    >framedsolid(re(z),im(z),im(exp(z)),2)
    Column 1 to 5:
       -1.53552        1.53552       -1.53552        1.53552       -2.00000
    Column 6 to 6:
        2.00000

Also in Retro you need not be afraid of singularities. All one has to
do is set the plot area accordingly.

    >{x,y}=field(-1:0.1:1,-1:0.1:1);
    >z=x+1i*y;
    >w=1/z;
    >setplot(-5,5,-5,5);
    >cplot(w)
       -5.00000        5.00000       -5.00000        5.00000

gives a nice picture of the inversion with respect to the unit circle.

Another example:

    >{r,phi}=field(linspace(0.001,5,20),linspace(0,2*pi(),30));
    >z=r*exp(1i*phi);
    >setplot(-2.5,5.5,-3,3.5);

Now we can display the following M”bius transform

    >cplot((2*z-1i)/(z-1))
       -2.50000        5.50000       -3.00000        3.50000
    >xgrid(0); ygrid(0);
