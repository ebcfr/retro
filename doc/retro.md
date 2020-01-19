

Documentation
=============

Retro, The numerical Laboratory
===============================

Version 3.04


INTRODUCTION
------------

Retro consists of the following files:

>	retro (the program, which may also have another name)

>	retro.cfg (is loaded after start of the program)

>	util.e (is loaded from retro.cfg)

There are some additional files, which help using the program:

>	HELP.E (help texts)

>	demo.e (a demo file, see the demo section)

>	retro.md (this file)

>	examples.md (some example sessions)

To load Retro, copy the above files in a directory, and start Retro. A
startup screen indicating the version date will appear. The program
will prompt for input, using a DOS-like text screen.

The idea of Retro is a system with the following features:

- Interactive evaluation of numerical expressions with real or complex
  values, vectors and matrices, including use of variables.
- Built in functions that can take vectors as input and are then
  evaluated for each element of the vector or matrix.
- Matrix functions.
- Statistical functions and random numbers.
- 2D- and 3D-plots.
- A built in programming language with parameters and local variables.
- An online help.
- A tracing feature for the Retro programming language.

These features make Retro an ideal tool for the tasks such as:

- Inspecting and discussing functions of one real or complex variable.
- Viewing surfaces in parameter representation.
- Linear algebra and eigenvalue computation.
- Testing numerical algorithms.
- Solving differential equations numerically.
- Computing polynomials.


THE COMMAND LINE
----------------

If Retro is started from a shell, command line parameters for Retro can
be entered. These parameter must be file names and those files are
loaded after the start of Retro (See "loading a file"). Note, that the
file `retro.cfg` is always loaded as first file.

The file `retro.cfg` contains some commands to set up the Retro system,
like defining the default file. It does in turn load the file UTIL.E.
UTIL contains functions which are written in the Retro programming
language and can be considered a part of Retro. But the user should
feel free to change the content of UTIL to his own needs.


THE DEMO AND THE ONLINE HELP
----------------------------

The best way to get a good idea of Retro is to run the demo. To do that
issue the command

```
	>load "demo"
```

Do not enter the prompt ">" and press the Return key. Of course, the
file DEMO must be in the active directory. To get a print of the
output of the demo, enter

	>dump "demo.txt"; load "demo"; dump;

Then the file "demo.txt" will contain the demo texts. If
the demo is interrupted (by the Escape key), the command

	>dump;

must be entered to close the file "demo.txt".

You can also learn to program Retro by studying the demo file.

It is a good idea to make a print of the HELP file, since all
functions are explained in that file. To load the online help, enter

	>load "help";

Then

	>help command

will show the help text of the command (see "programming Retro").


THE LINE EDITOR
---------------

Text can be entered after the prompt (>) using the keyboard. If a
letter is mistyped, it can be cancelled with the Backspace key. The
cursor keys -> and <- position the cursor back and forth in the line.
The line editor is always in insert mode, so any character is inserted
at the cursor position. Shift plus <- positions the cursor to the
beginning of the line, and Shift plus -> to the end of the line.
Clr-Home clears all input (Contrl-Up or Escape on some machines).
Control plus -> or <- position the cursor one word to the right or
left. Finally, the input is entered by pressing Return. Then Retro
will start to interpret the command. Note, that the cursor can be at
any position of the input line, when Return is pressed.

Previous input can be recalled with the cursor-up and cursor-down
keys. If a command is recalled this way and entered with Return,
cursor-down recalls the next command; i.e., cursor-up and cursor-down
are always based on the last recalled command. Pressing Clr-Home (see
above) not only clears the input line, but also makes the last command
the base for recalling. Thus Clr-Home plus cursor-up recalls the
previous command.

Pressing the Help key (or the insert key, depending on the version)
extends incomplete commands. Pressing this key again gives another
extension, if there is one. The search goes through the command
history, the implemented functions, the user defined functions and the
builtin commands in that order.

There are some other special keys. The Tabulator (TAB, or some other key,
depending on the program version) key switches to the Graphics screen
and from there any key switches back. The Esc key stops any running
Retro program and some internal functions, like the 3-dimensional
plots, the linear equation solver and the polynomial root finder.

Input can spread over several lines by the use of "..". The two dots
are allowed at any place, where a space is acceptable. E.g.

	>3+ .. some comment
	>4

is equivalent to

	>3+4

Comments can follow the ".." and are skipped.

The function keys may be programmed by the command

	>setkey(number,"text");

The number must be between 1 and 10 and the "text" may be any Retro
string. Then the corresponding function key will produce the text,
when it is pressed. If the function key text is to contain a ", one
can use char(34) and the Retro string concatenation, as in the example

	>setkey(1,"load "|char(34)|"test"|char(34)|";");

which puts

	>load "test";

on the function key F1.


THE DATA TYPES
--------------

Retro uses the following data types

- real numbers
- complex numbers
- real matrices
- complex matrices
- strings
- references
- programs

The meaning of the first five data types should be clear. Retro tries
to keep an evaluation real. But as soon, as complex data is involved
the computation gets complex. That is why

	>sqrt(-1)

results in NaN (not a number), but

	>sqrt(complex(-1))

gives 0+1*i. complex(x) is a way to turn a number complex. See the
section on expressions for more information on how to enter these
data types.

Strings are only used for explaining output, file names and function
key texts. There are only two string operators, the concetanation |
and the function stringcompare.

References are used internally for parameters of functions (see the
programming section).

Programs are the user defined programs including the functions in
UTIL. 

All these data are kept on the stack. Usually, the user does not have
to worry about the stack, since Retro uses normal mathematical notation
for expressions. Internally, the evaluation of an expression uses the
stack rather heavily. Programs are kept in the lower area of the stack.
The stack is also used to pass parameters to builtin or user defined
functions and to keep the local variables of these functions.

Finally, we remark that the data explained in this section are
different from builtin functions which can be used in the same places
as user defined functions. There are also "commands", which cannot be
used in expressions. Builtin functions and commands are part of the
code of Retro.

A complete list of builtin functions, commands and user defined
functions is printed by

	>list

If more information is needed, the command

	>memorydump

is available. A hexdump of any data can be optained with

	>hexdump name

but it will only be useful for insiders.

	>store("filename")

stores the content of the Retro stack into a file.

	>restore("filename")

loads that file. This is a short way of storing a session including
the global variables and functions.


ENTERING INPUT
--------------

Retro input is either a "command", an "expression" or an "assignment".
An example for a command is

	>quit

which quits the Retro system. By and by, we will mention a few others.
Another example is "load".

An expression is a valid Retro expression, which has a resulting
value. If it is entered on its own or followed by a comma, this value
is printed.

	>3+4*5

prints the value 23.00000. The print is surpressed, if the expression
is followed by a ";", like in the setkey example above. This makes
sense, since some functions have side effects and their result is not
needed, of since the user does not want to see the long output of
an assignment (see below).

The printing is done with a format determined by the function

	>format([n,m])

where n is the total width of print and m is the number of digits after
the decimal dot.

	>format(n,m)

does the same thing, but is a little bit slower since it is a function
in UTIL. The output automatically switches to exponential format, if
there is not enough space to display the number in fixed format.

	>longformat()

is a function in UTIL, which sets a longer output format, while

	>shortformat()

sets a shorter one.

An assignment looks like

	>variablename=value

It assigns the value to the variable, which is declared by the
assignment, and prints the value. If the assignment is followed by a
";", then the printing is surpressed. An assignment may be followed by
",", which prints the right hand side of the assignment.

There are also multiple assignments (see below).


Expressions
-----------

The most simple expressions are variables. Their value is the value
of the variable, and their type is the type of the variable. If the
variable is undefined, interpreting the expressions will stop with
an error message.

The basic constant expressions are numbers. Those are entered in the
usual form 100, 100.0, 1e2, or 1.0e+2. The letter "small e" indicates
powers of 10. An appendix "i" indicates multiples of the complex unit
"i". "1+1i" is in fact a sum of 1 and 1i.

A matrix is entered in the brackets "[" and "]" row by row. The columns
are seperated by "," and the rows by ";". Example:

	>A=[1,2,3;4,5;6]

This is equivalent with

	>A=[1,2,3;4,5,0;6,0,0]

The matrix is real, if all entries are real, otherwise it is complex.
If a row is shorter than the others, it is filled with zeros. A matrix
constant can spread over several lines. 

A submatrix is a matrix, which is made up by the entries of another
matrix. The simplest example is a matrix element

	>A[1,1]

which gives the element in the first row and column of A. Note, that
submatrices can be assigned values. Thus

	>A[1,1]=4.5

is a legal statement. If a submatrix gets complex, the matrix gets
complex. If v is a 1xN or Nx1 matrix (i.e., a vector), then v[1] gives
the first element of v; i.e.,

	>v=[1.5,-2,0,4.8]; v[3]

gives 0. Let us now assume, that A is a matrix, and r and c are
vectors. Then A[r,c] results in a matrix, which consists of the rows
r[1],r[2],... of A, and from these rows, only the columns c[1],c[2],...
are taken. Example:

	>A[[1,2],[1,2]]

is the upper left 2x2 submatrix of A. If a row or column does not
exist, it is simply neglected. Thus, if A is a 4x4 matrix, then
A[[4,7],[4,7]] results in the value A[4,4]. A special thing is A[1],
which gives the first row of A. To be precise, if only one index is
present, then the second index is assumed to be ":". A ":" indicates
all rows or columns; i.e., A[:,1] is the first column of A, and A[:,:]
is A itself. Another example:

	>v=[-1,-2,-3,-4,-5]; v[[5,4,3,2,1,1]]

gives the vector [-5,-4,-3,-2,-1,-1]. If A is a 4x4 matrix, then
A[[2,1]] gives a 2x4 matrix, which consists of the second row of A on
top of the first row. Note, that there may be a 0xN or Nx0 matrix.

For compatibility reasons, the square brackets can be replaced by
round brackets. Thus, A(1,1) is the same thing as A[1,1]. But
A[1,1] is faster. Furthermore, if there is a function A, then A(1,1)
will result in a function call to A.

A{i} gives the i-th element of the matrix A, as if the NxM Matrix A
was a vector of length N*M. This is useful for making functions
work for matrices, and is really the quickest way to access a matrix
element. It works also, if the matrix A is to small or a real or
complex variable. Then the result is the last element of A.

String constants are enclosed in double quotes, like in

	>string="This is a text"

The section on the line editor shows how to insert a double quote into
a string. A single character with ASCII code n can be produced by

	>char(n)

If A is a matrix expression (an expression of type matrix), then A'
is the transposed matrix. 

The ":" serves to generate a vector quickly. Thus

	>1:10

generates the vector [1,2,3,4,5,6,7,8,9,10]. A step size may be given
as in the example

	>5:-1.5:1

which yields [5,3.5,2]. By numerical reasons, one cannot expect to
hit 1 exactly with 0:0.1:1. However, the program uses the internal
epsilon to stop generating the vector, so that 0:0.1:1 yields the
desired result. By default, the internal epsilon is set so that
even

	>0:0.0001:1

works correctly. 

The binary operator "|" puts a matrix aside another; i.e., if A is a
NxM matrix and B is a NxK matrix, then A|B is a Nx(M+K) matrix, which
consists of A left of B. Analogously, A_B puts A atop of B. These
operators work also for numbers, which are treated as 1x1 matrices.
They do even work, if A is a Nx0 or 0xN matrix.

The mathematical operators +,-,*,/ work as usual for numbers. For
matrices they work elementwise. The matrix product of A and B is
computed by "A.B". If a is a number and B a matrix, then a+B or
B+a computes the sum of all elements of B with a. This also holds
for the other operators. Of course, -A negates all elements of A.
Retro knows the rule, to compute "*" and "/" before "+" and "-".
One can also write ".*","./" for compatibility with MATLAB. If
A has a different size as B, and neither A or B is a 1x1 matrix or
a number, then A+B results in error.

Of course, one can use the round brackets ( and ) to group expressions
like in

	>(1+5)*(6+7^(1+3))

The power operator can be written "^" or "**" (or ".^"). It computes
the power elementwise, like all the other operatos. So

	>[1,2,3]^2

yields [1,4,9]. The power may also be negative; i.e., the integer
powers of all numbers are defined. For a matrix, inv(A) computes the
inverse of A (not "A^-1"!). Note, that "^" has precedence, so

	>-2^2

gives -4.

Comparison of values can be performed with  >,	>=, <, <=, != (not
equal) or == (equal). They result in 1 or 0, where 1 is TRUE. Again,
these operators work elementwise; i.e,

	>[1,2,3,4]>2

yields [0,0,1,1].

	>!A

(not A) gives a matrix, which is 1 on all zero elements of A, and 0 on
all nonzero elements of A. 

	>A && B

gives a matrix, which is 1 whenever the corresponding elements of A
and B are nonzero. 

	>A || B

is 1 whenever the corresponding element of A is nonzero or the
corresponding element of B is nonzero.

	>any(A)

yields 1 if any element of A is nonzero.

If S and T are strings, then S|T consists of the string T appended to
S.


GENERATING A MATRIX
-------------------

There are several builtin functions which generate matrices. The most
elementary ones are zeros([N,M]) and ones([N,M]), which can also be
written zeros(N,M) and ones(N,M). They produce a NxM matrix, which is
filled with ones or zeros respectively. The first form, which gives
the size in a 1x2 vector, is faster, since the second form is not a
builtin function, but a user defined function from UTIL. The input for
zeros and ones is the output of the "size" function, which gives [N,M]
for size(A), if A is an NxM matrix. Note, that one can also generate
0xN and Nx0 matrices. So

	>zeros[0,5]_v_v

is a legal statement, if v is a 1x5 vector. It is also possible to
give size several arguments. Then

	>size(A,B,...)

results in the size of the largest matrix of A,B,... However, all
matrixes in the list must have the same size, unless their size is
1x1. The use of this feature will become apparent later on. Also

	>cols(A)
	>rows(A)

return the number of columns and rows of a matrix A.

	>length(A)

it the maximum of the number of columns and rows.

More generally,

	>matrix([N,M],x)

or matrix(N,M,x) returns a NxM matrix filled with x, which may be real
or complex.

	>diag([N,M],K,v)

produces a NxM matrix, which has the vector v on its K-th diagonal and
is 0 everywhere else. If v is not long enough, the last element of v
is taken for the rest of the diagonal. The 0-th diagonal is the main
diagonal, the 1-st the one above, and the -1-st the one below. So

	>diag([5,5],0,1)

produces the 5x5 identity matrix. The same can be achieved with

	>id(5) 

from UTIL. One can also write diag(N,M,K,v), as this is defined in UTIL.

	>diag(A,K)

gives a vector, which is the K-th diagonal of A.

	>dup(v,N)

duplicates the 1xM vector N times, such that a NxM matrix is generated,
which has v in each row. If v is an Mx1 vector, then v is duplicated
into the N columns of a MxN matrix. dup works also, if v is a number.
Then it generates a column vector.

	>B=band(A,N,M)

sets all elements of A[i,j] to 0, unless N <= i-j <= M.

	>B=setdiag(A,N,v)

sets the N-th diagonal of A to v. v may be a number or a vector.

	>bandmult(A,B)

multiplies to matrices A and B (like A.B), but is considerably faster,
if A and B contain lots of zeros.

	>symmult(A,B)

multiplies symmetric matrices A and B and saves half of the time.

Furthermore,

	>flipx(A)

flips the matrix, such that the last column becomes the first, the first
column the last.

	>flipy(A)

does the same to the rows.


LOADING A FILE
--------------

To abbreviate tedious input one may generate a file containing Retro
commands. This file can be generated with any editor (like 1STWORD,
TEMPUS or so). To load that file enter

	>load "filename"

All lines of that file are then interpreted just as any other input
from the keyboard. Also a loaded file may itself contain a load command.
If an error occurs, the loading is stopped and an error message is
displayed. There is a default extension ".e", which you should use for
your files. You need not specify this extension in the load command.

The best use of a load file is to define functions in that file (see
"programming Retro").


BUILTIN FUNCTIONS
-----------------

In this section some basic builtin functions are explained. There are
the usual functions:

abs, sqrt, exp, log, sin, cos, tan, asin, acos, atan, re, im, conj.

They all work for complex values. In this case they yield the
principle value. There are some functions which make sense only
for real values:

floor, ceil, sign, fak, bin.

floor and ceil give integer approximations to a real number. "bin(n,m)"
computes the binomial coefficient of n and m.

	>pi()

(or simply ">pi") is a builtin constant.

	>epsilon()

is an internal epsilon, used by many functions and the operator ~=
which compares two values and returns 1 if the absolute difference is
smaller than epsilon. This epsilon can be changed with the statement

	>setepsilon(value)

	>round(x,n)

rounds x to n digits after the decimal dot. It also works for complex
numbers. x may be a matrix.

	>mod(x,y)

return x modulus y.

	>max(x,y)

and min(x,y) return the maximum (minimum resp.) of x and y.

	>max(A)

and min(A) return a column vector containting the maxima (minima resp.)
of the rows of A.

If A is a NxM matrix, then

	>extrema(A)

is a Nx4 matrix, which contains in each row a vector of the form
[min imin max imax], where min and max are the minima and maxima of
the corresponding row of A, and imin and imax are the indices, where
those are obtained.

If v is a 1xN vector, then

	>nonzeros(v)

returns a vector, containing all indices i, where v[i] is not zero.
Furthermore,

	>count(v,M)

returns a 1xM vector, the i-th component of which contains the number
of v[i] in the interval [i-1,i).

	>find(v,x)

assumes that the elements of v are ordered. It returns the index (or
indices, if x is a vector) i  such that v[i] <= x < v[i+1], or 0 if
there is no such i.

	>sort(v)

sorts the elements of v with the quicksort algorithm.

If A is NxM matrix

	>sum(A)

returns a column vector containing the sums of the rows of A.
Analoguously,

	>prod(A)

returns the products.

	>cumsum(A)

returns a NxM matrix containing the cumulative sums of the columns of A.

	>cumprod(A)

works the same way. E.g.,

	>cumprod(1:20)

returns a vector with faculty function at 1 to 20.

The only string function in Retro is

	>stringcompare("string1","string2")

which returns 0, if the strings are equal, -1 if string1 is
alphabetically prior to string2, and 1 else.

	>time()

returns a timer in seconds. It is useful for benchmarks etc.

	>wait(n)

waits for n seconds or until a key was pressed.

	>key()

waits for a keypress and returns the internal scan code.


LINEAR ALGEBRA
--------------

There are lots of builtin functions to perform linear algebra. The
matrix product has already been discussed. The operation

	>A\b

takes a NxN matrix A and a Nx1 vector b and returns the vector x such
that Ax=b. If in

	>A\B

B is a NxM matrix, then the systems A\B[:,i] are solved simultanuously.
An error is issued if the determinant of A turns out to be to small
relative to the internal epsilon.

	>inv(A)

computes the invers of A. This is a function in UTIL defined as

	>A\id(cols(A))
 
There are also more primitive functions, like

	>lu(A)

for NxM matrices A. It returns multiple values, which needs explaining.
You can assign its return values to variables with

	>{Res,ri,ci,det}=lu(A)

If you use only

	>Res=lu(A)

all other output values are dropped. To explain the output of lu, lets
start with Res. Res is a NxM matrix containing the LU-decomposition of
A; i.e., L.U=A with a lower triangle matrix L and an upper triangle
matrix U. L has ones in the diagonal, which are omitted so that L and
U can be stored in Res. det is of course the determinant of A. ri
contains the indices of the rows of lu, since during the algorithm the
rows may have been swept. ci is not important if A is nonsingular. If
A is singular, however, Res contains the result of the Gauss algorithm.
ri contains 1 and 0 such that the columns with 1 form a basis for the
columns of A.

lu is used by several functions in UTIL.

	>kernel(A)

gives a basis of the kernel of A; i.e., the vectors x with Ax=0.

	>image(A)

gives a basis of the vectors Ax.

The primitive function for computing eigenvalues currently implemented
is

	>charpoly(A)

which computes the characteristic polynomial of A. This is used by

	>eigenvalues(A)

to compute the eigenvalues of A. Then

	>eigenspace(A,l)

computes a basis of the eigenspace of l.

	>{l,X}=eigen(A)

returns the eigenvalues of A in l and the eigenvectors in X.


STATISTICAL FUNCTIONS
---------------------

Retro supports statistical functions, such as distribution integrals
and random variables.

First of all,

	>random(N,M)

or random([N,M]) generates an NxM random matrix with uniformly
distributed values in [0,1]. It uses the internal random number
generator of ANSI-C. The quality of this generator may be doubtful;
but for most purposes it should suffice. The function

	>normal(N,M)

or normal([N,M]) returns normally distributed random variables with
mean value 0 and standart deviation 1. You should scale the function
for other mean values or deviations.

A function for the mean value or the standart deviation has not been
implemented, since it is easily defined in Retro; e.g,

	>m=sum(x)/cols(x)

is the mean value of the vector x, and

	>d=sqrt(sum((x-m)^2)/(cols(x)-1))

the standart deviation.

Rather, some distributions are implemented.

	>normaldis(x)

returns the probability of a normally dstributed random variable being
less than x.

	>invnormaldis(p)

is the inverse to the above function. These functions are only accurate
to about 4 digits. However, this is enough for practical purposes and
an improved version is easily implemented with the Romberg or Gauss
integration method.

Another distribution is

	>tdis(x,n)

It the T-distrution of x with n degrees of freedom; i.e., the
probability that n the sum of normally distributed random variables
scaled with their mean value and standart deviation are less than x.

	>invtdis(p,n)

returns the inverse of this function.

	>chidis(x,n)

returns the chi^2 distribution; i.e., the distribution of the sum of
the squares n normally distributed random variables.

	>fdis(x,n,m)

returns the f-distribution with n and m degrees of freedom.

Other functions have been mentioned above, like bin, fak, count, etc.,
which may be useful for statistical purposes.


POLYNOMIALS
-----------

Retro stores a polynomial a+bx+...+cx^n in the form [a,b,...,c]. It
can evaluate a polynomial with the Horner scheme

	>polyval(p,x)

where x can be a matrix, of course. It can multiply polynomials with

	>polymult(p,q)

or add them with

	>polyadd(p,q)

Of course, the polynomials need not have the same degree.

	>polydiv(p,q)

divides p by q and returns {result,remainder}.

	>polytrunc(p)

truncates a polynomial to its true degree (using epsilon). In UTIL

	>polydif(p)

is defined. It differentiates the polynomial once. To construct a
polynomial with prescribed zeros z=[z1,...,zn]

	>polycons(z)

is used. The reverse is obtained with

	>polysolve(p)

This function uses the Bauhuber method, which converges very stabelly
to the zeros. However, there is always the problem with multiple zeros
destroying the accuracy (but not the speed of convergence). Another
problem is the scaling of the polynomial, which can improve the
stability and accuracy of the method considerably.

Polynomial interpolation can be done by

	>d=interp(t,s)

where t and s are vectors. The result is a polynomial in divided
differences (Newton) form, and can be evaluated by

	>interpval(t,d,x)

at x. To transform the Newton form to the usual polynomial form

	>polytrans(t,d)

may be used. Interpolation in the roots of unity can be done with
the fast Fourier transform

	>p=fft(s)

Then p(exp(I*2*pi*i/n))=s[i+1], 0<=i<n-1. For maximal speed, n should
be a power of 2. The reverse evaluates a polynomial at the roots of
unity simultanuously

	>s=ifft(p)



GRAPHICS
--------

The best part of Retro is its graphic capability. There are two
screens, the text screen and the graphic screen. Text output
automatically displays the text screen, and graphic output displays
the graphic screen. One may also switch to the graphic screen by
pressing TAB on input or with the command

	>shg;

There are two coordinate systems. The screen coordinates are a
1024x1024 grid with (0,0) in the upper left corner. Furthermore, each
plot generates plot coordinates mapping a rectangle of the plane on
the screen with the smallest x value left and the smallest y value at
the bottom. To be precise, the rectangle is mapped to the screen
window, which may only cover a part of the display area. See below for
more information on windows.

If x and y are 1xN vectors, the function

	>plot(x,y)

connects the points (x[i],y[i]) with straight lines and plots these
lines. It first clears the screen and draws a frame around the screen
window. The plot coordinates are chosen such that the plot fits
exactly into the screen window. This behaviour is called autoscaling.
You can set your own plot coordinates with

	>setplot(xmin,xmax,ymin,ymax)

(or setplot([...])). The autoscaling is then turned off. To turn it on
again,

	>setplot()

or

	>scaling(1)

is used.

	>s=scaling(0)

does also turn off autoscaling and returns the old state of the scaling
flag to s. By the way, the plot command returns the plot coordinates;
i.e., a vector [xmin,xmax,ymin,ymax].

The screen window is a rectangle on the screen which can be set by

	>window(cmin,cmax,rmin,rmax)

(or window([...])) in screen coordinates.

If x is a 1xN vector and A MxN matrix,

	>plot(x,A)

plots several functions. The same is true with

	>plot(B,A)

if B is a MxN matrix. In this case, the plot is done for corresponding
rows of A and B.

The graphic screen is cleared by the plot command. This can be turned
off with

	>hold on;

or

	>holding(1);

To turn holding off,

	>hold off;

or

	>holding(0);

is used. The function holding returns the old state of the holding
flag. This is a way to draw several plots into the same frame.
Combining window and holding is a way to draw several plots in several
windows on the screen.

	>xplot(x,y)

works like plot but does also show axis grids. Actually, xplot has
default parameters grid=1 and ticks=1, which determine, if a grid is
plotted and axis laballing is done. Thus

	>xplot(x,y,1,0)

does no ticks. Also

	>xplot(x,y,ticks=0)

may be used for the same purpose (see below in the section about Retro
programming for details on default parameters).

	>xplot()

plots the axis and ticks only.

You should use

	>shrinkwindow()

if you intend to use labels (ticks) on the axis. This leaves some
space around the plot.

	>fullwindow()

resizes the plots to the full size. You may also set the x axis grids
on your own with

	>xgrid([x1,x2,x3,...])

or

	>xgrid([x1,x2,x3,...],f)

the latter producing digits at the edge of the plot (ygrid(...) works
respectively). Actually, these functions are defined in UTIL.

	>plotarea(x,y)

sets the plot area for the next plot like plot(x,y), but it does not
actually plot.

	>title("text")

draws a title above the plot. It is a function in UTIL using

	>ctext("text",[col,row])

which plots the text centered at screen coordinates (col,row).

	>text("text",[col,row])

plots the text left justified. The width and height of the charcters can
be asked with

	>textsize()

returning the vector [width,height] in screen coordinates.

If a color monitor is used, Retro can produce colored plots. The plot
color can be modified with

	>color(n)

where n=1 is black, and n=0 is white. Other colors depend on your
system settings. The textcolor and framecolor can be set the same way.

There is the possibility to use dotted and dashed lines, or even
invisible lines erasing the background. This is done with the style
one of the commands

	>style(".")
	>linestyle(".")

("-","--","i"). The function linestyle, also

	>linestyle("")

returns the previous line style. The lines can have a width greater than
0. This is set with

	>linewidth(n)

The function returns the previous setting.

The command

	>mark(x,y)

works like plot. But it does not connect the points but plots single
markers at (x[i],y[i]). The style of the marker can be set with one
of the commands

	>style("mx")
	>markerstyle("mx")

for a cross. Other styles are "m<>" for diamonds, "m." for dots, "m+"
for plus signs, "m[]" for rectangles and "m*" for stars. The function
returns the previous marker style.

	>style("")

resets the plot and marker styles. There is also the command

	>xmark(x,y)

The easiest way to produce 3D-plots is

	>mesh(Z)

If Z is a NxM matrix, its elements are interpreted as z-values of a
function defined on a grid of points (i,j). The plot is a three
dimensional plot with hidden lines of the points (i,j,Z[i,j]). It is
autoscaled to fit onto the screen window. The point (1,1,z[1,1]) is
the left foremost point. To improve the look of the plot, one uses

	>triangles(1)

However, this way is considerably slower. Of course,

	>triangles(0)

turns this feature off again. One can also turn off the different fill
styles mesh uses for the two sides of the plot with

	>twosides(0)

This function works also for solid plots described below. It is a faster
way than the use of triangles to avoid the errors, that the mesh plot
frequently makes. By the way, both functions return the old values of
the flags.

There is a function which produces matrices X and Y such that X[i,j]
and Y[i,j] are the coordinates in the plane of a point in a
rectangular grid parametrized by (i,j). This function is

	>{X,Y}=field(x,y)

where x and y are row vectors. Then X[i,j] is equal to x[i] and Y[i,j]
is equal to y[j]. So you can easily generate a matrix of function
values; e.g.,

	>Z=X*X+Y*Y

Note that this is not the most economical way. The above Z would be
generated by

	>Z=dup(x*x,cols(y))+dup(y*y,cols(x))'

much more rapidly.

A nicer way to plot a surface is

	>solid(x,y,z)

or

	>framedsolid(x,y,z)

where x, y and z are NxM matrices. The surface parameters are then
(i,j) and (x[i,j],y[i,j],z[i,j]) are points on the surface. The
surface should not self intersect; or else plot errors will occur. The
surface is projected onto the screen in central projection, with the
view centered to (0,0,0). You can set the viewing distance, a zooming
parameter, the angles of the eye from the negative y- to the positive
x-axis, and the height of the eye on the x-y-plane, by

	>view(distance,tele,angle,height)

(or view([...])). view returns the previous values and view() merely
returns the old values. framedsolid has a default parameter scale,
which scales the image to fit into a cube with side length 2*scale,
centered at 0, unless scale=0, which is the default value (no
scaling). Thus

	>framedsolid(x,y,z,2)

will scale the plot so that |x|,|y|,|z|<=2.

	>wire(x,y,z)

and

	>framedwire(x,y,z)

work the same way, but the plotting is not solid. If x, y and z are
vectors, then a path in three dimensions is drawn. The color of the
wires is set by

	>wirecolor(c)

If you add an extra value to framedwire or framedsolid like in

	>framedwire(x,y,z,scale)

the plot is scaled to fit into a cube of side length 2scale. The
function

	>{x1,y1}=project(x,y,z)

projects the coordinates x,y,z to the screen and is useful for 3D
path plots. There is a function

	>scalematrix(A)

which scale the matrix A linearly so that its entries are between
0 and 1.

	>solid(x,y,z,i)

is a special form of solid. If i=[i1,...,in], then the ij-th row
of (x,y,z) is not connected to the ij+1-st row. I.e., the plot
consists of n+1 parts.

A contour plot of a matrix is produced by

	>contour(A,[v1,...,vn])

The contour lines are then at the heights v1 to vn. The interpretation
of A is the same as in mesh(A).

A density plot is a plot of a matrix, that uses shades to make the
values visible. It is produced with

	>density(A)

The integer part of the values is cut off. So the shades run through
the available shades, if A runs from 0 to 1. A can be scaled to 0 to
f with

	>density(A,f)

f=1 is the most important value. The shades can be controlled with

	>huecolor(color)

if color=0, then the function uses the available colors for shading.
One should set these colors to give a nice color scale from dark to
light. This option is not available on B/W screens. Any positive color
produces shades in that color using dithering. Any negative color
cycles through colors 4 to 16 plus dithering.

	>solidhue(x,y,z,h)

makes a solid view with shades. h are the shading values as in density.

	>framedsolidhue(x,y,z,h,scale,f)

works like a mixture of framedsolid and solidhue. scale=0 and f=1 are
the default values.

The plotting of a user defined function f(x,...) can be done with

	>fplot("f",a,b)
	>fplot("f",a,b,n)
	>fplot("f",a,b,,...)

The extra parameters ... are passed to f. n is the number of
subintervals. Note the extra ',', if n is omitted. If a and b
are omitted, as in

	>fplot("f",,,,...)

or

	>fplot("f")

then the plot coordinates of the last plot are used. You can zoom
in with the mouse by

	>setplotm(); fplot("f");

This lets you select a square region of the screen. In any case,
setplotm() sets the plot coordinates to the user selected square.

It is possible to sample coordinates from the graphics screen with
the mouse.

	>mouse()

displays the graphics screen and a mouse pointer and waits for a mouse
click. It returns a vector [x,y] which are the plot coordinates of the
clicked spot. Using

	>{x,y}=select()

returns several x coordinates and y coordinates at the points, which
the user selected by the mouse. The selection stops, if the user clicks
above the plot window.


DIRECTORIES
-----------

The active directory can be changed with

	>cd("path")

where path is the new directory and may include a drive letter, like

	>cd("a:\progs")

The command

	>dir("*.e")

displays all files in the active directory, which fit with the pattern.
This is not available on UNIX machines.


THE META FILE
-------------

For special purposes, you can keep a record of your graphic output
on an external file called meta file. You turn on recording with

	>meta "filename";

and turn it off again with

	>meta;

The content of the meta file looks like this. All commands start with
a command number (a byte) and the number of arguments (a byte). Then
several arguments follow (int or long or a string). The screen
coordinates are multiplied by 1000 and are stored long, as is hue
and scale. Note that int may be 16-bit or 32-bit depending on
your machine.

- 1 c0 r0 c1 r1 color style width
  Draws a line from (c0,r0 to (r1,c1). The style is none=0, solid=1,
  dotted=2, dashed=3.

- 2 c r color type
  A marker at (c,r). dot=0, plus=1, start=2, square=3, cross=4,
  diamond=5.

- 3 n c1 r1 f1 c2 r2 f2 c3 r3 f3 ... cn rn style
  A filled rectangle with the specified corners. fill style is solid=0,
  dotted=1. The corners are (ci,ri) and fi is a flag which determines
  if the point is to be connected to the next one on the boundary. n
  is the number of points.

- 4 c r color centered Text+0(+0)
  A text at (c,r). The extra 0 makes the text have an even length.

- 5 scale
  scales the plot window to 1 : (scale/1024). A 0 means rescaling
  to full screen.

- 6
  clears the screen.

- 7 c1 r1 c2 r2 hue color
  Draws a rectangle with upper left corner (c1,r1) and lower right
  corner (c2,r2). hue is a value from 0 to 1 multiplied by 1000.
  color is as in the function huecolor().

- 8 c1 r1 c2 r2 c3 r3 c4 r4 hue color
  Draws a filled polygon with corner (ci,ri).

The author has no use for the meta file right now. But clearly someone
might write a program for printer output, film sequences etc. using
the meta file. Sorry, but I was unable to use POSTSCRIPT commands. It
should be trival to translate the above commands however.


PROGRAMMING RETRO
----------------

Retro would not be as powerful as it is, if there wasn't the
possibility to extend it with user defined functions. A function can
be entered from the keyboard or better from a load file. Since there
may be errors in a function, it is best to call an editor, edit the
function in a file and then load that file.

Loading a file is done with

	>load "filename"

as explained above.


FUNCTIONS
---------

A function is declared by the following commands

	>function name (parameter,...,parameter)
	>...
	>endfunction

It can have several parameters or none. If the function is entered
from the keyboard, the prompt changes to "udf>". "endfunction"
finishes the function definition. An example:

	>function cosh (x)
    udf>  return (exp(x)+exp(-x))/2
    udf>endfunction

Every function must have a return statement, which ends the execution
of the function and defines the value it returns. A function can be
used in any expression, just as the builtin functions. If a function
is not used in an assignment and with surpressed output (followed by
";"), it is used like a procedure and its result is evidently lost.
However, the function may have had some side effect.

Inside a function one cannot access global variables or variables of
the function which called the function. To use global variables use
the command

	>global variablename

in the function body. Of course, one can use other functions in
expressions inside a function, one can even use the function inside
itself recursively. All variables defined inside a function are local
to that function and undefined after the return from the function.
There is no way to define global variables or change the type or size
of global variables from within a function, even if one defines these
variables in a "global" statement.

All parameters are passed as references. That means that a change
of a parameter results in the change of the variable, which was
passed as the parameter. For example

	>function test(x)
	>  x=3;
	>  return x
	>endfunction
	>a=5;
	>test(a);
	>a

gives the value 3. There is an exeption to this. A submatrix is not
passed by value. Thus

	>a=[1,2,3];
	>test(a(1));
	>a(1)

gives the value 1. 

A function can have a variable number of parameters. The number of
parameters passed to a function can be determined with the builtin
function "argn()". If the parameters are not named in the function
definition, they are given the names arg1, arg2 and so on.

Parameters can have default values. The syntax ist parameter=value in
the parameter definition; e.g.,

	>function f(x=3,y,z=1:10)

assigns the default value 3 to x and the vector 1 to 10 to z, if the
function is called in the form

	>f(,4,)

If the function is called

	>f(1,4)

x has the value 1, y the value 4, and z the value 1:10.

The function can even be given a named parameter. Consider the
function

	>function f(x,y,z=4,w=4,t=5)

Then

	>f(1,2,t=7)

calls the function, as if

	>f(1,2,4,4,7)

was entered. Actually the name needs not be a parameter name. Thus

	>f(1,2,s=7)

defines a local variable s with value 7.

A function can return multiple values. This is done with the return
statement

	>  return {x,y,...}

You can assign all the return values of a function to variables, using

	>{a,b,...}=function(...);

If the result of such a function is assigned to a number of variables
smaller than the number of returned values, only the first values are
used. If is is assigned to a larger number of variables, the last value
is used more than once. Some builtin functions return multiple values.

Comments can be included, starting with ##. Comments are not entered
in the function definition and do not appear, when you type the
function with

	>type functionname

If the lines immediately after the function header start with ##,
then those lines are considered to be help text. They can be displayed
with

	>help functionname

This is a good way to remember the parameters of the function. Also

	>list

can be used to display the names of the user defined functions.

A function or several functions can be removed with

	>forget name,...

By the way, a variable can be removed with

	>clear name,...



RETRO PROGRAMMING LANGUAGE
-------------------------

Like in any other programming language, in Retro there are commands
for changing the flow of evaluation. These commands can only be used
inside user defined functions.

First there is the "if" command.

	>if expression; ...; else; ....; endif;

The expression is any Retro numerical expression. If it is a matrix,
and all its entries are different from zero, then the part from the
";" to the "else;" is evaluated. Else the part from "else;" to "endif"
is evaluated. Of course "..." may spread over several lines. To work
correctly, keywords like "if", "else", "endif" and others should be
the first nonblank characters in a line, or should be preceded by "," or
";" (plus blanks or TABs). The "else" may be omitted. In this case the
evaluation skips behind the "endif", if the matrix contains nonzero
elements (resp. the number is nonzero).

There is the function "any(A)", which yields 1, if there is any
nonzero element in A, 0 otherwise. The function is useful in
connection with the if statement.

Next, there are several loops.

	>repeat; ...; end;
	>loop a to b; ...; end;
	>for i=a to b step c; ...; end;

All loops can be aborted by the break command (usually inside an
"if"), especially the seemingly infinite "repeat". "loop" loops are
fast long integer loops. The looping index can be accessed with the
function "index()" or with "#". In a "for" loop the looping index is
the variable left of the "=". The step size can be omitted. Then it is
assumed to be 1. As an example, the following loops count from 1 to
10:

	>  i=1;
	>  repeat;
	>    i, i=i+1;
	>    if i>10; break; endif;
	>  end;

and

	>  loop 1 to 10; #, end;

and

	>  for i=1 to 10; i, end;



DEBUGGING RETRO PROGRAMS
------------------------

The command

	>trace on

sets tracing of all functions on. Then any new line in a user defined
function will be printed with the function name before it is executed.
The uses has to press a key, before execution continues:

	F1	debugs every function.
	
	F2	debugs no function called in the current line.

	F3	will stop debugging until return of the current function.

	F4	will prompt for an expression and evaluate it.

	F9	aborts execution of the program.

	F10	ends tracing for this command.

Any other key will display the available keys.

	>trace off

switches tracing off.

Note, that with F4 you can evaluate any expression, even if it contains
local variables or subroutine calls. Tracing is switched off during
evaluation of the expression.

A single function can be traced with

	>trace function

or

	>trace "function"

Execution will stop only in this function. The same command switches
the trace bit of this function off.

	>trace alloff

switches tracing for all functions off.


PROGRAMMING STYLE
-----------------

All internal Retro functions can handle vector or matrix input. And so
should user defined functions. To achieve this, sometimes nothing
special needs to be done. E.g., the function

	>function kap (r,i,n)
	>  p=1+i/100;
	>  return p*r*(p^n-1)/(p-1)
	>endfunction

is automatically capable to handle matrix intput. Thus

	>kap(1000,5:0.1:10,10)

will produce a vector of values. However, if the function uses a more
complicated algorithm, one needs to take extra care. E.g.,

	>function lambda (a,b)
	>  si=size(a,b); l=zeros(si);
	>  loop 1 to prod(si);
	>    l{#}=max(abs(polysolve([1,a{#},b{#},1])));
	>  end;
	>  return l
	>endfunction

shows the fastest way to achieve the aim. "si" will contain the size,
of the matrix "a" or "b", and will yield an error message, if the
sizes are different. However, if "a" is a real and "b" a matrix, "si"
will contain the size of "b" and the program will still run properly.
The loop will then be over all the elements of "a" or "b". the "{#}"
indexing works correctly, if the vector is smaller than the index. It
then yields the last element of a vector, which is OK in the case of a
real or complex values. See also the function "map" in UTIL explained
below.

Forthermore, as a matter of good style, one should use the help lines
extensively. One should explain all parameters of the function and its
result. This is a good way to remember what the function really does.


PASSING FUNCTIONS AS PARAMETERS
-------------------------------

It is possible to pass functions to a function. E.g., the function
"bisect" in UTIL finds the zero of a function using bisection. One
uses this function in the following way:

	>function f(x)
	>  return x*x-2
	>endfunction
	>bisect("f",1,2)

The result will be sqrt(2). If "f" needs extra paramters, those can
also be passed to "bisect":

	>funtion f(x,a)
	>  return x*x-a
	>endfunction
	>bisect("f",0,a,a)

will result in sqrt(a) (for a>=0). The search interval is set to [0,a].

The way to write a function like "bisect" is to use the "args" function.

	>function bisect (function,a,b)
	>...
	>  y=function(x,args(4));
	>...
	>endfunction

Then "function" will be called with the parameter "x" and all paramters
from the 4-th on (if any) which have been passed to "bisect". Of course,
"function" should be assigned a string, containing the name of the
function which we want the zero of.


USER INTERFACE
--------------

Clearly, Retro is designed to run interactively. The user loads a file
containing the functions he needs. The file may inform the user of its
content by simply printing some messages with commands like

	>"The content of this file is:",
	>...

Then the user can use the help facility to retrieve further information
on the functions, its parameters and so on. He (or she) then calls the
particular function with the parameters he desires.

However, it is also possible to run a file as a standalone program.
If you start Retro from a shell simply put the file into the command
line.

If you wish a standalone application, the user will have to enter data.
You can prompt him with

	>data=input("prompt");

The prompt

   prompt? >

will appear and the user may enter any valid Retro expression, even if
it uses variables. Errors are catched and force the user to reenter
the input. If you wish the user to enter a string, use

	>string=lineinput("prompt");

The string may then be evaluated with

	>data=interpret(string);

and if it does not consist of a valid Retro expression the result is the
string "error".

Output is printed to screen. All expressions and assignments produce
output unless followed by ";". If formated output is wanted, use

	>printf("formatstring",realdata)

The format string obeys the C syntax; e.g., "%15.10f" prints the data
on 15 places with 10 digits after decimal dot, and "%20.10e" produces
the exponential format. You can concatenate strings with | to longer
output in a single line.

Output is surpressed globally with

	>output off;

and

	>output on;

turns the output on again. This is useful if a dump file is defined by

	>dump "filename";

Then all output is echoed into the dump file. The command

	>dump;

turns the dump off. Note that the dump is always appended to the file.
Furthermore, that file may not be edited while dump is on! In UTIL the
function

	>write(x,"x")

is defined, which writes x in a format readable by Retro on input.
If you omit the name "x", the name of x is used automatically. This
is done with the help of the function

	>name(x)

which gives a string containing the name of x.

	>cls;

clears the screen.

	>clg;

clears the graphics. Also to show graphics to the user, use

	>shg;

Subsequent output will switch back to the text screen.

Finally, an error can be issued with the function

	>error("error text")



UTILITIES
---------

In UTIL some fuctions are defined, which will to be of general use.
First there are some functions for solving equations.

	>bisect("f",a,b,...)

uses the bisetion method to solve f(x,...)=0. f must be a function
of the real variable x which may have additional parameters "...".
These parameters can be passed to bisect and are in turn passed to f.

	>secant("f",a,b,...)

uses the secant method for the same purpose.

	>broyden("f",x)

or

	>broyden("f",x,J,...)

uses the Broyden method to find the roots of f(x,...)=0. This time, f
may be a function f : R^n -> R^n. Then x is a vector. J is an
approximation of the Jacobian at x, the starting point. If J==0 or J
is missing, then the function computes J. Again, additional parameters
are passed to f.

	>simpson("f",a,b)

or

	>simpson("f",a,b,n,...)

computes the Simpson integral with of f in [a,b]. n is the discretization
and additional parameters are passed to f(x,...). f must be able to
handle vector input for x.

	>gauss("f",a,b)

or

	>gauss("f",a,b,n,...)

performs gauss integration with 10 knots. If n is specified, then
the interval is subdivided into n subintervals. f(x,...) must be able
to handle vector intput for x.

	>romberg("f",a,b)

or

	>romberg("f",a,b,n,...)

uses the Romberg method for integration. n is the starting discretization.
All other parameters are as in "simpson". The iteration goes on until
the accuracy rombergeps is reached, which is epsilon() initially.

There are some approximation tools. Polynomial interpolation has been
discussed above. There is also spline interpolation

	>sp=spline(t,s)

which returns the second derivatives of the natural cubic spline through
(t[i],s[i]). To evaluate this spline at x,

	>splineval(t,s,sp,x)

is available.

	>polyfit(t,s,n)

fits a polynomial of n-th degree to (t[i],s[i]) in least square mode.
This is an application of

	>fit(A,b)

which computes x such that the L_2-norm of Ax-b is minimal.

	>iterate("f",x)

or

	>iterate("f",x,n,...)

iterates f starting at x n times and returns the vector of iterated
values. If n is missing or 0, then the iteration stops at a fixed
point and only this fixed point is returned.

	>map("f",x,...)

evaluates the function f(x,...) at all elements of x, if f(x,...)
does not work because the function f does not accept vectors x.


HINTS
-----

One can make all sorts of mistakes in Retro. This section tries to
warn you of the more common ones, most os which the author has some
experience with.

As already mentioned, you should not assign to the parameter of a
function. This will generally produce strange errors, which are
difficult to debug. If you need default values for parameters, use a
construction like

	>function f(a,x)
	>  if argn()==2; xx=x; else xx=4; endif
	>  ...

which gives xx the value of the second parameter, with default 4.

The next mistake is to produce matrices with 0 rows or columns. Retro
can not do any computation with these matrices. Make sure that every
index you use is in range. And use special handling, if there is
nothing to do.

Another subtlety concerns the use of multiple return values. The
following simply does not work:

	>x=random(1,10); sin(sort(x))

The reason is that sort returns not only the sorted array but also the
indices of the sorted elements. This works as if sin was passed two
parameters and Retro will not recognize that use of sin. To work
around this either assign the sorted array to a variable or put extra
brackets around it:

	>x=random(1,10); sin((sort(x)))

Also a return statement like

	>return {sort(x),y}

really returns 3 (or more) values! Use

	>return {(sort(x)),y}

One further misfortune results from the use of strings as functions,
like in

	>function test(f,x)
	>	return f(x*x)
	>endfunction
	>test("sin",4)

This works well as long as there is no function by the name of "f". If
there is, this function is called rather than the sine function. The
only way to avoid this is to use really strange names for function
parameters. I prefer "test(ffunction,x)" and used it throughout UTIL.


VERSION CHANGES
---------------

Here is a list of recent changes to Retro. The new version should be
downward compatible in most cases. Minor error corrections are not
listed here.

- Named parameters and default values for paramters have been
introduced. Many functions in UTIL.E have been redesigned using the
new features.

- Graphic primitives have been changed. Lines can have a width now.
The density, huecolor and solidhue functions have been introduced.
This changed the content of the meta file, which has also been
simplified.

- New functions in UTIL.E: scalematrix, select, framedsolidhue,
setplotm.

- The key function is defined. It waits for a key stroke.

- The help/insert key extends incomplete commands.


FINAL WORDS
-----------

The author is interested in applications of Retro and will collect
them. These applications will then be included in program updates
(maybe in packed form). Some interesting applications are in the
folder PROGS. The file EXAMPLES.DOC contains sample sessions with
Retro and shows how to solve problems with this programming system.

The author wishes anyone success in using Retro.
