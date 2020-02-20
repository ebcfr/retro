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
