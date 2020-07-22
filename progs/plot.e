
... text and frame color to black 
textcolor(1);
framecolor(1);

subwindows=zeros([9,4]);

function xsubplot(rci)
## sets plot layout to r x c plots, sets plot index to the ith one
##   with r, c, i coded on a single digit each, i <= r x c
##   so i<=9 : 2x4, 4*2 or 3x3 max layouts
## returns a 1x3 vector with [r, c, i]
	global subwindows;
	l=subplot(rci);
	if l[3]==1 hold off; clg; endif
	w=window(); wt=textwidth(); ht=textheight();
	hr=floor(1024/l[1]);wc=floor(1024/l[2]);
	for r=1 to l[1]
		for c=1 to l[2]
			w=[8*wt+(c-1)*wc,1.5*ht+(r-1)*hr,c*wc-2*wt,r*hr-3*ht];
			subwindows[(r-1)*l[2]+c]=w;
		end
	end
	hold on
	window(subwindows[l[3]]);	
	return l;
endfunction

function select
## Returns coordinates {x,y} of mouse clicks, until the user clicked
## above the plot window.
	p=plot();
	x=[];
	repeat
		m=mouse();
		if m[2]>p[4]; break; endif;
		h=holding(1); mark(m[1],m[2]); holding(h);
		x=x_m;
	end;
	x=x'; return {x[1],x[2]}
endfunction

... text and labeling

function textheight
## textheight() returns the height of a letter.
	h=textsize();
	return h[2];
endfunction

function textwidth
## textwidth() returns the width of a letter.
	h=textsize();
	return h[1];
endfunction

function title (text)
## title(text) plots a title to the grafik window.
	w=window(); ht=textheight();
	ctext(text,[w[1]+(w[3]-w[1])/2,w[2]-1.5*ht]);
	return text;
endfunction

function xlabel(text,l=1)
## Puts the label text at the x-axis at point l in [0,1].
	w=window();
	ht=textheight();
	return text(text,[(w[1]+w[3])/2,w[4]+2*ht],1);
endfunction

function ylabel(text,l=1)
## Puts the label text at the y-axis at point l in [0,1].
	w=window();wt=textwidth();
	return text(text,[w[1]-8*wt,(w[4]+w[2])/2],4);
endfunction

function text
## text(s,[c,r]) plots the centered string s at the coordinates (c,r).
## Also text(s,c,r).
	if argn()==2 return text(arg1,arg2,0); endif;
	if argn()==3 return text(arg1,[arg2,arg3],0); endif;
	error("Illegal argument number!"),
endfunction

function ctext
## ctext(s,[c,r]) plots the centered string s at the coordinates (c,r).
## Also ctext(s,c,r).
	if argn()==2 return text(arg1,arg2,1); endif;
	if argn()==3 return text(arg1,[arg2,arg3],1); endif;
	error("Illegal argument number!"),
endfunction

function rtext
## rtext(s,[c,r]) plots the centered string s at the coordinates (c,r).
## Also text(s,c,r).
	if argn()==2 return text(arg1,arg2,2); endif;
	if argn()==3 return text(arg1,[arg2,arg3],2); endif;
	error("Illegal argument number!"),
endfunction

function fullwindow
## fullwindow() takes the full size (besides a title) for the
## plots.
	h=textsize();
	w=window();
	window([12,textheight()*1.5,1011,1011]);
	return w;
endfunction

function shrinkwindow
## shrinkwindow() shrinks the window to allow labels.
	h=textheight(); b=textwidth();
	w=window();
	window([8*b,1.5*h,1023-2*b,1023-3*h]);
	return w;
endfunction

function setplot
## setplot([xmin xmax ymin ymax]) sets the plot coordinates and holds
## the plot. Also setplot(xmin,xmax,ymin,ymax).
## setplot() resets it. 
	if argn()==4 return setplot([arg1,arg2,arg3,arg4]);
	else
		if argn()==0 scaling(1); 
		else error("Illegal arguments!");
		endif
	endif;
	return plot();
endfunction

function ticks (a,b,extend=0)
## returns the proper ticks to be used for intervals [a,b] and
## the factor of the ticks.
	fexp0=floor(log(b-a)/log(10));
	if fexp0<-2 || fexp0>2 fexp=3*floor(fexp0/3); else fexp=0; endif
...	tick=10^(fexp0-fexp);
	tick=10^fexp0;
	n=ceil((b-a)*10^-fexp0);
	if n<3 tick1=n*tick/10;
	elseif n<5 tick1=tick/2;
	else tick1=tick;
	endif
...	m=a/10^fexp;M=b/10^fexp;
...	return {(floor(m/tick1)+1)*tick1:tick1:(ceil(M/tick1)-1)*tick1,10^fexp}
...	tick1=tick1*10^fexp;
	if extend
		return {(floor(a/tick1+0.05))*tick1:tick1:(ceil(b/tick1-0.05))*tick1,10^fexp};
	endif
	return {(floor(a/tick1)+1)*tick1:tick1:(ceil(b/tick1)-1)*tick1,10^fexp};
endfunction

function logticks(a,b)
...	expi=floor(log(a)/log(10))+1;
...	expf=ceil(log(b)/log(10))-1;
	expi=floor(a)+1;expf=ceil(b)-1;
	tick=expi-1+log(floor(a/10^(expi-1)+1):9)/log(10);
	for k=expi to expf-1
		tick=tick|(k+log(1:9)/log(10));
	end
	tick=tick|expf+log(1:ceil(b/10^(expf))-0.95)/log(10);
	return {tick,1};
endfunction

function logticks2(a,b)
...	expi=floor(log(a)/log(10));
...	expf=ceil(log(b)/log(10));
	expi=floor(a);expf=ceil(b);
	tick=[];
	for k=expi to expf-1
		tick=tick|(k+log(1:9)/log(10));
	end
	tick=tick|expf;
	return {tick,1};
endfunction

function xplot (x,y=0,grid=1,ticks=1,xlog=0,ylog=0)
## xplot(x,y) or xplot(y) works like plot, but shows axis ticks.
## xplot() shows only axis ticks and the grid.
## options:
## - grid, tick: show the grid and ticks
## - xlog, ylog: use a base 10 log scale in x or y directions
## - xticks and yticks: use these as ticks instead of calculate them
## - lw, ls, color: line attribute
## Returns the extent of the plot
	if argn()>0
		if argn()==1
			if iscomplex(x) y=im(x); xh=re(x);
			else y=x; xh=1:cols(y);
			endif
		else xh=x;
		endif
		if xlog xh=log(xh)/log(10); endif
		if ylog y=log(y)/log(10); endif
		p=plotarea(xh,y);
		if !holding() clg; endif
	else
		p=plot();
	endif
	olw=linewidth(1);
	if !xlog
		if isvar(xticks)
			t=xticks; f=1;
			p=setplot([xticks[1],xticks[length(xticks)]]|p[3:4]);
		else
			{t,f}=ticks(p[1],p[2],1);
			p=setplot([t[1],t[length(t)]]|p[3:4]);
		endif
	else
		{t,f}=logticks2(p[1],p[2]);
	endif
	xgrid(t,f,grid,ticks,xlog);
	if !ylog
		if isvar(yticks)
			t=yticks; f=1;
			p=setplot(p[1:2]|[yticks[1],yticks[length(yticks)]]);
		else
			{t,f}=ticks(p[3],p[4],1);
			p=setplot(p[1:2]|[t[1],t[length(t)]]);
		endif
	else
		{t,f}=logticks2(p[3],p[4]);
	endif
	ygrid(t,f,grid,ticks,ylog);
	if isvar(lw) linewidth(lw); else linewidth(olw); endif;
	if argn()>0;
		if isvar(col)
			ocolor=color(col);
		else
			ocolor=color(1);
			color(ocolor);
		endif
		ho=holding(1); plot(xh,y); holding(ho);
		color(ocolor);
	endif;
	linewidth(olw);
...	if !holding() frame(); endif
	frame();
	return p;
endfunction

function xmark (x,y=0,grid=1,ticks=1)
## xmark(x,y) or xmark(y) works like plot, but shows axis ticks.
## xmark() shows only axis ticks and the grid.
	if !holding(); clg; frame(); endif;
	if argn()==1;
		if iscomplex(x); y=im(x); xh=re(x);
		else; y=x; xh=1:cols(y);
		endif;
	else; xh=x;
	endif;
	p=plotarea(xh,y);
	{t,f}=ticks(p[1],p[2]);
	xgrid(t,f,grid,ticks);
	{t,f}=ticks(p[3],p[4]);
	ygrid(t,f,grid,ticks);
	ho=holding(1); p=mark(xh,y); holding(ho);
	if !holding() frame(); endif
	return p;
endfunction

function setplotm
## The user may choose the plotting coordinates with the mouse.
## Returns the plot coordinates.
	h=holding(1);
	k1=mouse(); mark(k1[1],k1[2]);
	k2=mouse(); mark(k2[1],k2[2]);
	kl=min(k1,k2); ku=max(k1,k2);
	c=color(2);
	plot([kl[1],kl[1],ku[1],ku[1],kl[1]],[kl[2],ku[2],ku[2],kl[2],kl[2]]);
	color(c);
	setplot(kl[1],ku[1],kl[2],ku[2]);
	holding(h);
	return plot();
endfunction

function fplot (ffunction,a=0,b=0,n=200)
## fplot("f",a,b,n,...) plots the function f in [a,b].
## The arguments ... are passed to f.
## fplot("f") or fplot("f",,,n,...) plots f in the old interval.
	if a~=b; s=plot(); a=s[1]; b=s[2]; endif;
	t=linspace(a,b,n);
	s=ffunction(t,args(4));
	return xplot(t,s)
endfunction

function histogram (d,n=10,grid=0,ticks=1,width=4,integer=0)
## Plots a histogram of the data d with n intervals.
## d must be a vector.
	mi=min(d); ma=max(d);
	if mi~=ma; ma=mi+1; endif;
	if integer; n=floor(ma-mi)+1; ma=ma+0.9999; endif;
	x=zeros(1,2*n+2); y=zeros(1,2*n+2);
	h=(d-mi)/(ma-mi)*n;
	c=count(h,n+1); c[n]=c[n]+c[n+1]; c=c[1:n];
	y[2:2:2*n]=c; y[3:2:2*n+1]=c;
	xx=linspace(mi,ma,n);
	x[2:2:2*n]=xx[1:n]; x[3:2:2*n+1]=xx[2:n+1];
	x[1]=mi; x[2*n+2]=ma;
	w=linewidth(width); xplot(x,y,grid,ticks); linewidth(w);
	return plot;
endfunction

function niceform (x)
## Return a string, containing a nice print of x.
	y=round(x,10);
	return printf("%g",y);
endfunction

function xgrid(xx,f=1,grid=1,ticks=1,xlog=0,color=3)
## xgrid([x0,x1,...]) draws vertical grid lines on the plot window at
## x0,x1,...
## xgrid([x0,x1,...],f) additionally writes x0/f to the axis.
	c=plot(); n=cols(xx); s=scaling(0); h=holding(1);
	w=window();
	st=linestyle("."); color(color);
	ht=textheight();wt=textwidth();
	if argn()<2 ticks=0; endif
	loop 1 to n;
		x=xx[index()];
		if (x<=c[2])&&(x>=c[1])
			if grid plot([x,x],[c[3],c[4]]); endif
			if ticks && !xlog
				col=w[1]+(x-c[1])/(c[2]-c[1])*(w[3]-w[1]);
				ctext(niceform(x/f),[col,w[4]+0.2*ht]);
			endif
		endif
	end
	if xlog
		loop floor(c[1]) to ceil(c[2])
			col=w[1]+(#-c[1])/(c[2]-c[1])*(w[3]-w[1]);
...			ctext(printf("10^%g",#),[col,w[4]+0.2*ht]);
			rtext("10",[col+wt,w[4]+0.75*ht]);
			text(printf("%g",#),[col+wt,w[4]+ht*0.25]);
		end
	endif
	if ticks && !(f~=1)
		rtext(printf("*%g",f),[w[3],w[4]+1.5*ht]);
	endif
	linestyle(st); color(1); holding(h); scaling(s);
	return 0;
endfunction

function ygrid(yy,f=1,grid=1,ticks=1,ylog=0,color=3)
## ygrid([x0,x1,...]) draws horizontal grid lines on the plot window at
## x0,x1,...
## ygrid([x0,x1,...],f) additionally writes x0/f to the axis.
	c=plot(); n=cols(yy); s=scaling(0); h=holding(1);
	st=linestyle("."); color(color);
	w=window(); wt=textwidth(); ht=textheight();
	if argn()<2 ticks=0; endif
	loop 1 to n
		y=yy[index()];
		if (y>=c[3])&&(y<=c[4])
			if ticks && !ylog
				row=w[4]-(y-c[3])/(c[4]-c[3])*(w[4]-w[2]);
				rtext(niceform(y/f),[w[1]-wt/2,row-ht/2]);
			endif
			if grid plot([c[1],c[2]],[y,y]); endif
		endif
	end;
	if ylog
		loop ceil(c[3]) to floor(c[4])
			row=w[4]-(#-c[3])/(c[4]-c[3])*(w[4]-w[2]);
...			rtext(printf("10^%g",#),[w[1]-wt/2,row-h/2]);
			rtext("10",[w[1]-3*wt,row-ht/2]);
			text(printf("%g",#),[w[1]-3*wt,row-ht]);
		end
	endif
	if ticks && !(f~=1)
		text(printf("*%g",f),[w[1]-6*wt,w[2]-1.5*ht]);
	endif
	linestyle(st); color(1); holding(h); scaling(s);
	return 0;
endfunction

function plot (x,y)
## plot(x,y) plots the values (x(i),y(i)) with the current style.
## If x is a matrix, y must be a matrix of the same size.
## The plot is then drawn for all rows of x and y.
## The plot is scaled automatically, unless hold is on.
## plot(x,y) and plot() return [x1,x2,y1,y2], where [x1,x2] is the range
## of the x-values and [y1,y2] of the y-values.
## plot(x) is the same as plot(1:cols(x),x).
	if argn()==1;
		if iscomplex(x); return plot(re(x),im(x));
		else return plot(1:cols(x),x);
		endif;
	endif;
	error("Illegal argument number!"),
endfunction

function mark (x,y)
## mark(x,y) plots markers at (x(i),y(i)) according the the actual style.
## Works like plot.
	if argn()==1;
		if iscomplex(x); return mark(re(x),im(x));
		else return mark(1:cols(x),x);
		endif;
	endif;
	error("Illegal argument number!"),
endfunction

function cplot (z)
## cplot(z) plots a grid of complex numbers.
	plot(re(z),im(z)); s=scaling(0); h=holding(1);
	w=z'; plot(re(w),im(w)); holding(h); scaling(s);
	return plot();
endfunction

function plotwindow
## plotwindow() sets the plot window to the screen coordinates.
	w=window();
	setplot(w[1],w[3],w[2],w[4]);
	return plot()
endfunction

function window
## window([c1,r1,c2,r2]) sets a plotting window. The cooridnates must
## be screen coordimates. Also window(c1,r1,c2,r2).
## window() returns the active window coordinates.
	if argn()==4; return window([arg1,arg2,arg3,arg4]); endif;
	error("Illegal argument number!"),
endfunction

... ###################### 3d plots #########################

function view
## view([distance, tele, angle1, angle2]) sets the perspective for
## solid and view. distance is the eye distance, tele a zooming factor.
## angle1 is the angle from the negativ y-axis to the positive x-axis.
## angle2 is the angle to the positive z-axis (the height of the eye).
## Also view(d,t,a1,a2).
## view() returns the values of view.
	if argn()==4; return view([arg1,arg2,arg3,arg4]); endif;
	error("Illegal argument number!"),
endfunction

function getframe (x,y,z)
## gets a box around all points in (x,y,z).
	ex=extrema(x); ey=extrema(y); ez=extrema(z);
	return [min(ex[:,1]'),max(ex[:,3]'),
		min(ey[:,1]'),max(ey[:,3]'),min(ez[:,1]'),max(ez[:,3]')]
endfunction

function framez0 (f)
	wire([f[1],f[2],f[2],f[1],f[1]], ...
		[f[3],f[3],f[4],f[4],f[3]],dup(f[5],5)');
	return 0	
endfunction

function framez1 (f)
	wire([f[1],f[2],f[2],f[1],f[1]], ...
		[f[3],f[3],f[4],f[4],f[3]],dup(f[6],5)');
	return 0
endfunction

function framexpyp (f)
	wire([f[2],f[2]],[f[4],f[4]],[f[5],f[6]]);
	return 0
endfunction

function framexpym (f)
	wire([f[2],f[2]],[f[3],f[3]],[f[5],f[6]]);
	return 0
endfunction

function framexmyp (f)
	wire([f[1],f[1]],[f[4],f[4]],[f[5],f[6]]);
	return 0
endfunction

function framexmym (f)
	wire([f[1],f[1]],[f[3],f[3]],[f[5],f[6]]);
	return 0
endfunction

function frame1 (f)
## draws the back part of the box (considering view)
	v=view();
	y=sin(v[4])*v[1];
	if y>f[5]; framez0(f); endif;
	if y<f[6]; framez1(f); endif;
	x=sin(v[3])*v[1]; y=-cos(v[3])*v[1];
	if x<=f[2]; framexpyp(f); framexpym(f); endif;
	if x>=f[1]; framexmyp(f); framexmym(f); endif;
	if y<=f[4]; framexmyp(f); framexpyp(f); endif;
	if y>=f[3]; framexmym(f); framexpym(f); endif;
	return 0
endfunction

function frame2 (f)
## draws the front part of the box (considering view).
	v=view();
	y=sin(v[4])*v[1];
	if y<=f[5]; framez0(f); endif;
	if y>=f[6]; framez1(f); endif;
	x=sin(v[3])*v[1]; y=-cos(v[3])*v[1];
	if x>=f[2]; framexpyp(f); framexpym(f); endif;
	if x<=f[1]; framexmyp(f); framexmym(f); endif;
	if y>=f[4]; framexmyp(f); framexpyp(f); endif;
	if y<=f[3]; framexmym(f); framexpym(f); endif;
	return 0
endfunction

function scaleframe (x,y,z,f,m)
	s=max([f[2]-f[1],f[4]-f[3],f[6]-f[5]]);
	xm=(f[2]+f[1])/2;
	ym=(f[4]+f[3])/2;
	zm=(f[6]+f[5])/2;
	ff=m/s*2;
	f=[f[1]-xm,f[2]-xm,f[3]-ym,f[4]-ym,f[5]-zm,f[6]-zm]*ff;
	return {(x-xm)*ff,(y-ym)*ff,(z-zm)*ff}
endfunction

function framedsolid (x,y,z,scale=0)
## works like solid and draws a frame around the plot
## if scale is specified, then the plot is scaled to fit into a cube of
## side length 2f centered at 0
	frame=getframe(x,y,z);
	if !holding(); clg; endif;
	h=holding(1);
	if scale; {x1,y1,z1}=scaleframe(x,y,z,frame,scale);
	else; {x1,y1,z1}={x,y,z}; endif;
	color(2); frame1(frame);
	color(1); solid(x1,y1,z1);
	color(2); frame2(frame);
	color(1);
	holding(h);
	return frame
endfunction

function framedsolidhue (x,y,z,hue,scale=0,f=1)
## works like solidhue and draws a frame around the plot
## if scale is specified, then the plot is scaled to fit into a cube of
## side length 2f centered at 0
	frame=getframe(x,y,z);
	if !holding(); clg; endif;
	h=holding(1);
	if scale; {x1,y1,z1}=scaleframe(x,y,z,frame,scale);
	else; {x1,y1,z1}={x,y,z}; endif;
	color(2); frame1(frame);
	color(1);
	if f; solidhue(x1,y1,z1,hue,f);
	else; solidhue(x1,y1,z1,hue);
	endif;
	color(2); frame2(frame);
	color(1);
	holding(h);
	return frame
endfunction

function framedwire (x,y,z,scale=0)
## works like framedsolid
	frame=getframe(x,y,z);
	if !holding(); clg; endif;
	h=holding(1);
	if scale; {x1,y1,z1}=scaleframe(x,y,z,frame,scale);
	else; {x1,y1,z1}={x,y,z}; endif;
	color(2); frame1(frame);
	color(1); wire(x1,y1,z1);
	color(2); frame2(frame);
	color(1);
	holding(h);
	return frame
endfunction

function density (x,f)
## density(x,1) makes density plot of the values in the matrix x
## scaled to fit into [0,f].
	ma=max(max(x)'); mi=min(min(x)'); h=ma-mi;
	if h~=0; h=1; endif;
	xx=(x-mi)/h*f;
	density(xx);
	return xx;
endfunction

function solidhue (x,y,z,h,f)
## solidhue(x,y,z,h) makes a shaded solid 3D-plot of x,y,z.
## h is the shading and should run between 0 and 1.
## f determines, if h is scaled to fit in between 0 and f.
	ma=max(max(h)'); mi=min(min(h)'); delta=ma-mi;
	if delta~=0; delta=1; endif;
	hh=(h-mi)/delta*f*0.9999;
	return solidhue(x,y,z,hh);
endfunction
