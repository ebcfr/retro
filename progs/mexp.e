

function mexp(A)
	B=id(length(A)); S=B; n=1;
	repeat;
		if max(max(abs(B)))~=0; break; endif;
		B=A.B/n; S=S+B; n=n+1;
	end;
	return S
endfunction

function mexp1(A)
	{l,v}=eigen1(A);
	return v.diag(size(A),0,exp(l)).inv(v)
endfunction

"A=[1,1,2;3,4,5;6,7,8]"
A=[1,1,2;3,4,5;6,7,8]
"eigenvalues(A)"
eigenvalues(A)
"mexp(A)"
mexp(A)
"mexp1(A)"
mexp1(A)

function f(a,b=7,c)
  return a+2*b+c;
endfunction

f(5,,11)

f(5,c=11)

f(c=11,a=5)

{x,y,z}={1,2,3}

function g(a)
  return {2*a,3*a,4*a}
endfunction

{x,y,z}=g(1)

.. test global feature
d=2
function h(a,b,c)
  global d;
  c=3;
  return a+2*b+c+d;
endfunction

h(5,7,11)
d

.. test global feature
d=2
function h(a,b=7,c)
  global d;
  return a+2*b+c+d;
endfunction

h(5,,11)
d

.. test global feature
d=2
function h(a,b=7,c)
  global d;
  return a+2*b+c+d;
endfunction

h(5,,11)
d

d=2
function h(a,b=7,c)
  global d;
  d=3;
  return a+2*b+c+d;
endfunction

h(5,,11)
d

d=2
function h(a,b=7,c)
  d=4;
  global d;
  d=3;
  return a+2*b+c+d;
endfunction

h(5,,11)
d

d=2
function h(a,b=7,c)
  d=4;
  global d;
  return a+2*b+c+d;
endfunction

h(5,,11)
d

.. Cannot change type of a non-local variable
d=2
function h(a,b=7,c)
  global d;
  d=1:3;
  return a+2*b+c+d;
endfunction

h(5,,11)
d

.. global d shadowed by local variable d
d=2
function h(a,b=7,c)
  d=0;
  global d;
  d=1:3;
  return a+2*b+c+d;
endfunction

h(5,,11)
d

.. since d global variable exists, d is a reference to it
d=2
function h(a,b=7,c)
  global *
  d=3;
  return a+2*b+c+d;
endfunction

h(5,,11)
d

.. e is a local variable
function h(a,b=7,c)
  global *
  e=4;
  return a+2*b+c+e;
endfunction

h(5,,11)
e

d=2
function h(a,b=7,c)
  global *
  d=3;
  return a+2*b+c+d;
endfunction

h(5,,11)
d

f0=1e3;m=0.2;T0=2;j=1i;
function T(f)
  return T0/(1+j*2*m*f/f0+(j*f/f0)^2);
endfunction
abs(T(1000))

f0=1e3;m=0.2;T0=2;j=1i;
function T(f)
  f0=100;
  global *
  return T0/(1+j*2*m*f/f0+(j*f/f0)^2);
endfunction
abs(T(1000))

