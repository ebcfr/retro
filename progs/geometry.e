... k=homothetie factor
a=pi/4;k=1;R=[k*cos(a),k*sin(a),0;-k*sin(a),k*cos(a),0;0,0,1]
... translation of vector [M,N]
M=4,N=4;M1=[1,0,0;0,1,0;-M,-N,1],M2=[1,0,0;0,1,0;M,N,1]
coord=[6,4,1;9,4,1;6,6,1;6,4,1];
trcoord=coord.M1.R.M2
xc=coord[:,1]';yc=coord[:,2]';
xt=trcoord[:,1]';yt=trcoord[:,2]';

xsubplot(111);
setplot([0,20,0,20]);
xplot(xc,yc);plot(xt,yt);
