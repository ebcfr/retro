/*
 *	file : spread.h -- spread function elementwise
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include "header.h"
#include "spread.h"

#define isreal(hd) (((hd)->type==s_real || (hd)->type==s_matrix))
#define iscomplex(hd) (((hd)->type==s_complex || (hd)->type==s_cmatrix))

static double (*func) (double);

static void funceval (double *x, double *y)
/* evaluates the function stored in func with pointers. */
{	*y=func(*x);
}

header *map1 (void f(double *, double *),
	void fc(double *, double *, double *, double *),
	header *hd)
/***** map
	do the function elementwise to the value.
	the value may be real or complex!
******/
{	double x,y;
	dims *d;
	header *hd1;
	double *m,*m1;
	long i,n;
	if (hd->type==s_real)
	{	f(realof(hd),&x);
		return new_real(x,"");
	}
	else if (hd->type==s_matrix)
	{	d=dimsof(hd);
		hd1=new_matrix(d->r,d->c,"");
		if (error) return new_string("Fehler",6,"");
		m=matrixof(hd);
		m1=matrixof(hd1);
		n=d->c*d->r;
		for (i=0; i<n; i++)
		{	f(m,m1); m++; m1++;
			if (error) break;
		}
		return hd1;
	}
	else if (fc && hd->type==s_complex)
	{	fc(realof(hd),imagof(hd),&x,&y);
		return new_complex(x,y,"");
	}
	else if (fc && hd->type==s_cmatrix)
	{	d=dimsof(hd);
		hd1=new_cmatrix(d->r,d->c,"");
		if (error) return new_string("Fehler",6,"");
		m=matrixof(hd);
		m1=matrixof(hd1);
		n=d->c*d->r;
		for (i=0; i<n; i++)
		{	fc(m,m+1,m1,m1+1); m+=2; m1+=2;
			if (error) break;
		}
		return hd1;
	}
	output("Illegal operation\n"); error=3;
	return new_string("Fehler",6,"");
}

header *map1r (void f(double *, double *),
	void fc(double *, double *, double *),
	header *hd)
/***** map
	do the function elementwise to the value.
	te value may be real or complex! the result is always real.
******/
{	double x;
	dims *d;
	header *hd1;
	double *m,*m1;
	int i,n;
	if (hd->type==s_real)
	{	f(realof(hd),&x);
		return new_real(x,"");
	}
	else if (hd->type==s_matrix)
	{	d=dimsof(hd);
		hd1=new_matrix(d->r,d->c,"");
		if (error) return new_string("Fehler",6,"");
		m=matrixof(hd);
		m1=matrixof(hd1);
		n=d->c*d->r;
		for (i=0; i<n; i++)
		{	f(m,m1); m++; m1++;
			if (error) break;
		}
		return hd1;
	}
	else if (fc && hd->type==s_complex)
	{	fc(realof(hd),imagof(hd),&x);
		return new_real(x,"");
	}
	else if (fc && hd->type==s_cmatrix)
	{	d=dimsof(hd);
		hd1=new_matrix(d->r,d->c,"");
		if (error) return new_string("Fehler",6,"");
		m=matrixof(hd);
		m1=matrixof(hd1);
		n=d->c*d->r;
		for (i=0; i<n; i++)
		{	fc(m,m+1,m1); m+=2; m1++;
			if (error) break;
		}
		return hd1;
	}
	output("Illegal operation\n"); error=3;
	return new_string("Fehler",6,"");
}

header * map2 (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *, double *),
	header *hd1, header *hd2)
/**** map2
    calculate the result of a binary operator applied to hd1 and
    hd2, selecting the right operator (in R or C). If hd1 or hd2 
    are matrices, the operator is applied elementwise
 ****/
{	int t1,t2,t,r1,c1,r2,c2,rr,cr,r,c; /* means real */
	double *m1,*m2,*m,x,y,null=0.0,*l1,*l2;
	header *result;
	if (isreal(hd1)) t1=0;
	else if (iscomplex(hd1)) t1=1;
	else
	{	output("Illegal Argument.\n"); error=1; return 0;
	}
	if (isreal(hd2)) t2=0;
	else if (iscomplex(hd2)) t2=1;
	else
	{	output("Illegal Argument.\n"); error=1; return 0;
	}
	if ( (t1==0 && t2==0 && !f) || (!fc && (t1==1 || t2==1)) )
	{	output("Cannot evaluate this operation.\n");
		error=1; return 0;
	}
	getmatrix(hd1,&r1,&c1,&m1); l1=m1;
	getmatrix(hd2,&r2,&c2,&m2); l2=m2;
	if ((r1>1 && r2>1 && (r1!=r2)) ||
	 (c1>1 && c2>1 && (c1!=c2)))
	{   output("Cannot combine these matrices!\n");
		error=1; return 0;
	}
	rr=r1; if (rr<r2) rr=r2;
	cr=c1; if (cr<c2) cr=c2;
	t=t1; if (t2!=0) t=t2;
	switch (t)
	{	case 0 :
			if (rr==1 && cr==1)
			{	f(m1,m2,&x);
				return new_real(x,"");
			}
			result=new_matrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	f(m1,m2,m);
					if (error) break;
					if (c1>1) m1++;
					if (c2>1) m2++;
					m++;
				}
				if (r1==1) m1=l1;
				else if (c1==1) m1++;
				if (r2==1) m2=l2;
				else if (c2==1) m2++;
			}
			return result;
		case 1 :
			if (rr==1 && cr==1)
			{	if (t1==0) fc(m1,&null,m2,m2+1,&x,&y);
				else if (t2==0) fc(m1,m1+1,m2,&null,&x,&y);
				else fc(m1,m1+1,m2,m2+1,&x,&y);
				return new_complex(x,y,"");
			}
			result=new_cmatrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	if (t1==0)
					{	fc(m1,&null,m2,m2+1,m,m+1);
						if (c1>1) m1++;
						if (c2>1) m2+=2;
					}
					else if (t2==0)
					{	fc(m1,m1+1,m2,&null,m,m+1);
						if (c1>1) m1+=2;
						if (c2>1) m2++;
					}
					else
					{	fc(m1,m1+1,m2,m2+1,m,m+1);
						if (c1>1) m1+=2;
						if (c2>1) m2+=2;
					}
					if (error) break;
					m+=2;
				}
				if (r1==1) m1=l1;
				else if (c1==1)
				{	if (t1==0) m1++;
					else m1+=2;
				}
				if (r2==1) m2=l2;
				else if (c2==1)
				{	if (t2==0) m2++;
					else m2+=2;
				}
			}
			return result;
	}
	return 0;
}

header * map2r (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *),
	header *hd1, header *hd2)
{	int t1,t2,t,r1,c1,r2,c2,r,c,rr,cr; /* means real */
	double *m1,*m2,*m,x,null=0.0,*l1,*l2;
	header *result;
	if (isreal(hd1)) t1=0;
	else if (iscomplex(hd1)) t1=1;
	else
	{	output("Illegal Argument.\n"); error=1; return 0;
	}
	if (isreal(hd2)) t2=0;
	else if (iscomplex(hd2)) t2=1;
	else
	{	output("Illegal Argument.\n"); error=1; return 0;
	}
	if ( (t1==0 && t2==0 && !f) || (!fc && (t1==1 || t2==1)) )
	{	output("Cannot evaluate this operation.\n");
		error=1; return 0;
	}
	getmatrix(hd1,&r1,&c1,&m1); l1=m1;
	getmatrix(hd2,&r2,&c2,&m2); l2=m2;
	if ((r1>1 && r2>1 && (r1!=r2)) ||
	 (c1>1 && c2>1 && (c1!=c2)))
	{   output("Cannot combine these matrices!\n");
		error=1; return 0;
	}
	rr=r1; if (rr<r2) rr=r2;
	cr=c1; if (cr<c2) cr=c2;
	t=t1; if (t2!=0) t=t2;
	switch (t)
	{	case 0 :
			if (rr==1 && cr==1)
			{	f(m1,m2,&x);
				return new_real(x,"");
			}
			result=new_matrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	f(m1,m2,m);
					if (error) break;
					if (c1>1) m1++;
					if (c2>1) m2++;
					m++;
				}
				if (r1==1) m1=l1;
				else if (c1==1) m1++;
				if (r2==1) m2=l2;
				else if (c2==1) m2++;
			}
			return result;
		case 1 :
			if (rr==1 && cr==1)
			{	if (t1==0) fc(m1,&null,m2,m2+1,&x);
				else if (t2==0) fc(m1,m1+1,m2,&null,&x);
				else fc(m1,m1+1,m2,m2+1,&x);
				return new_real(x,"");
			}
			result=new_matrix(rr,cr,"");
			if (error) return 0;
			m=matrixof(result);
			for (r=0; r<rr; r++)
			{	for (c=0; c<cr; c++)
				{	if (t1==0)
					{	fc(m1,&null,m2,m2+1,m);
						if (c1>1) m1++;
						if (c2>1) m2+=2;
					}
					else if (t2==0)
					{	fc(m1,m1+1,m2,&null,m);
						if (c1>1) m1+=2;
						if (c2>1) m2++;
					}
					else
					{	fc(m1,m1+1,m2,m2+1,m);
						if (c1>1) m1+=2;
						if (c2>1) m2+=2;
					}
					if (error) break;
					m++;
				}
				if (r1==1) m1=l1;
				else if (c1==1)
				{	if (t1==0) m1++;
					else m1+=2;
				}
				if (r2==1) m2=l2;
				else if (c2==1)
				{	if (t2==0) m2++;
					else m2+=2;
				}
			}
			return result;
	}
	return 0;
}

void spread2 (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *, double *),
	header *hd)
{	header *result,*st=hd,*hd1;
	hd1=next_param(st); if (!hd1 || error) return;
	if (next_param(hd1))
	{	output("Too many parameters for operator! (Multiple returns?)\n");
		error=1; return;
	}
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	result=map2(f,fc,hd,hd1);
	if (!error) moveresult(st,result);
}

void spread2r (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *),
	header *hd)
{	header *result,*st=hd,*hd1;
	hd1=next_param(st); if (!hd1 || error) return;
	if (next_param(hd1))
	{	output("Too many parameters for operator! (Multiple returns?)\n");
		error=1; return;
	}
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	result=map2r(f,fc,hd,hd1);
	if (!error) moveresult(st,result);
}

void spread1 (double f (double),
	void fc (double *, double *, double *, double *),
	header *hd)
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	func=f;
	result=map1(funceval,fc,hd);
	if (!error) moveresult(st,result);
}

void spread1r (double f (double),
	void fc (double *, double *, double *),
	header *hd)
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	func=f;
	result=map1r(funceval,fc,hd);
	if (!error) moveresult(st,result);
}

#if 0
void mmap1 (header *hd)
/* map a function to all matrix arguments (of mixed type) */
{   header *st=hd,*result,*hds[16],*hq,*hdudf=0;
	int i,n=0,t[16],row[16],col[16],rescol=1,resrow=1,r,c,mt=0;
	int foundudf=0;
	double *m[16],*mr=0;
	if (!hd || hd==(header *)newram)
	{	output("Map needs arguments.\n");
		error=1; return;
	}
	hq=nextof(hd);
	while (n<16)
	{	if (hq>=(header *)newram) break;
		hds[n++]=hq;
		hq=nextof(hq);
	}
	if (n<1 || n>=16) wrong_arg_in("map");
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
	{	output("Map needs a string as first argument.\n");
		error=1; return;
	}
	for (i=0; i<n; i++)
	{	hds[i]=getvalue(hds[i]); if (error) return;
		switch (hds[i]->type)
		{	case s_real :
				row[i]=1; col[i]=1; t[i]=1; m[i]=realof(hds[i]);
				break;
			case s_complex :
				row[i]=1; col[i]=1; t[i]=2; m[i]=realof(hds[i]);
				break;
#ifdef INTERVAL
			case s_interval :
				row[i]=1; col[i]=1; t[i]=3; m[i]=realof(hds[i]);
				break;
#endif
			case s_matrix :
				t[i]=1;
				mat :
				row[i]=dimsof(hds[i])->r;
				if (row[i]!=1 && resrow!=1 && resrow!=row[i])
				{	output("Rows do not match in map!\n");
					error=1; return;
				}
				if (row[i]!=1) resrow=row[i];
				col[i]=dimsof(hds[i])->c;
				if (col[i]!=1 && rescol!=1 && rescol!=col[i])
				{	output("Colums do not match in map!\n");
					error=1; return;
				}
				if (col[i]!=1) rescol=col[i];
				m[i]=matrixof(hds[i]);
				break;
			case s_cmatrix :
				t[i]=2; goto mat;
#ifdef INTERVAL
			case s_imatrix :
				t[i]=3; goto mat;
#endif
			default :
				wrong_arg_in("map");
		}
	}
	result=0;
	for (r=0; r<resrow; r++)
	{	for (c=0; c<rescol; c++)
		{	hq=(header *)newram;
			for (i=0; i<n; i++)
			{	switch (t[i])
				{	case 1 :
						new_real(*(m[i]),"");
						break;
					case 2 :
						new_complex(*(m[i]),*(m[i]+1),"");
						break;
#ifdef INTERVAL
					case 3 :
						new_interval(*(m[i]),*(m[i]+1),"");
						break;
#endif
					default :
						output("Error!\n");
				}
			}
			if (foundudf)
			{	interpret_udf(hdudf,hq,n,0);
			}
			else
			{	if (!exec_builtin(stringof(hd),n,hq))
				{	hdudf=searchudf(stringof(hd));
					if (error || !hdudf) error=1;
					else foundudf=1;
					if (foundudf) interpret_udf(hdudf,hq,n,0);
				}
			}
			if (error) { output("Error in map.\n"); return; }
			if (!result)
			{   if (resrow==1 && rescol==1)
				{   moveresult(st,hq); return;
				}
				switch (hq->type)
				{	case s_real :
						result=new_matrix(resrow,rescol,"");
						if (error) return;
						mr=matrixof(result); mt=1;
						*mr++=*realof(hq);
						break;
					case s_complex :
						result=new_cmatrix(resrow,rescol,"");
						if (error) return;
						mr=matrixof(result); mt=2;
						*mr++=*realof(hq);
						*mr++=*(realof(hq)+1);
						break;
#ifdef INTERVAL
					case s_interval :
						result=new_imatrix(resrow,rescol,"");
						if (error) return;
						mr=matrixof(result); mt=3;
						*mr++=*realof(hq);
						*mr++=*(realof(hq)+1);
						break;
#endif
					default :
						output("Illegal function result in map.\n");
						error=1; return;
				}
			}
			else
			{	switch (hq->type)
				{	case s_real :
						if (mt!=1) goto err1;
						*mr++=*realof(hq); break;
					case s_complex :
						if (mt!=2) goto err1;
						*mr++=*realof(hq); *mr++=*(realof(hq)+1);
						break;
#ifdef INTERVAL
					case s_interval :
						if (mt!=3) goto err1;
						*mr++=*realof(hq); *mr++=*(realof(hq)+1);
						break;
#endif
					default :
						err1 :
						output("Illegal function result in map.\n");
						error=1; return;
				}
				newram=(char *)hq;
			}
			for (i=0; i<n; i++)
				if (col[i]>1)
				{	if (t[i]==1) m[i]++;
					else m[i]+=2;
				}
		}
		for (i=0; i<n; i++)
		{   if (t[i]==1)
			{	if (col[i]==1) m[i]++;
				if (row[i]==1) m[i]-=col[i];
			}
			else
			{	if (col[i]==1) m[i]+=2;
				if (row[i]==1) m[i]-=2*col[i];
			}
		}
	}
	if (!result) result=new_matrix(resrow,rescol,"");
	moveresult(st,result);
}
#endif
