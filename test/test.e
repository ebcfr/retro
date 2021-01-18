function f(x,y,z)
## test func
  m=x+y+z;
  return m;
endfunction

f(2,3,4)
f(2,3,z=4)

z=2
f(2,3,z==4)

f(2,3,1:6=7)


function f(x,y,z=4)
## test func
  m=x+y+z;
  return m;
endfunction

f(2,3,4)
f(2,3,z=4)

z=2
f(2,3,z==4)

f(2,3,1:6=7)

function f(x)
  if x<-1 return -1;
  elseif x>1 return 1;
  else return 0;
  endif
endfunction

function h(x)
  s=0;
  listvar
  for i=1 to x step 1;
    s=s+i
  end
  return s
endfunction

function g(x)
  loop 1 to 10
    #
  	if #>x break endif
  end
  return x;
endfunction

A=[1,2,3;4,5,6;7,8,9]
A[]


................................................................
.. test passing functions as string parameter

function f(x)
  return 2m*x;
endfunction

function calc(ff,a=0,b=1,n=10)
  t=a:(b-a)/(n-1):b;
  y=ff(t);
  return y;
endfunction

calc("f")

.. calling a function defined by a string
t=0:1/9:1;
s="f";
s(t)

..
eval("f",[1,2,3])
................................................................

num=[7,-2,3,5];den=[3,8,0,-4];
{a,b}=polydiv(num,den)


function f(a,b)
...  memorydump
  listvar
  hexdump a
  hexdump b
  hexdump pi
  return b/a;
endfunction;
f(1,pi)

function getdata
	return pi;
endfunction

function getdata1(x=3)
	listvar
	"..."
	return x+pi;
endfunction

function f(a,b,c,extra1=5,extra2=7)
  d=3*pi;
  listvar
  "..."
  return b/a+c+d;
endfunction;
z=2;f(z,pi,1)
z=2;h=7;f(z,h,1)
z=2;f(z,getdata,1)
z=2;f(z,getdata,getdata1)
z=2;f(z,getdata,getdata1+1)

function setplot
  listvar
  arg1
  arg2
  arg3
  arg4
  if argn==4 return setplot([arg1,arg2,arg3,arg4]);
  else error("bad args")
endfunction
setplot(-pi,pi,-2,2);

