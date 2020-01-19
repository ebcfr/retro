

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
