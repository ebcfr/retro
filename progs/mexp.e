

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
