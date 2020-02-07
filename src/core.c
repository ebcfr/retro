
#include <stdlib.h>
#include <string.h>

#include "header.h"
#include "funcs.h"
#include "core.h"

/* functions that manipulate the stack */
char *ramstart,*ramend,*udfend,*startlocal,*endlocal,*newram,
     *startglobal, *endglobal;

/* Organization of the stack
	----------------------------- <-- ramend
	   
	   
	            free
	   
	
	----------------------------- <-- newram = endlocal (top of stack)
	running function local scope
	----------------------------- <-- startlocal = endglobal   [endlocal]
	global variables
	----------------------------- <-- udfend     = startglobal [startlocal]
	udf (user defined functions)                                    ^
	----------------------------- <-- ramstart                      |
                                                               when global
	                                                              scope
   
   the running function local is transient and exists only while
   the function is executed.
   
   startlocal and endlocal relates either to the current running
   local scope or to the global scope when an expression is
   evaluated at the global level of the interpreter.
   
 */

int xor (char *n)
/***** xor
	compute a hashcode for the name n.
*****/
{	int r=0;
	while (*n) r^=*n++;
	return r;
}

static void *make_header (stacktyp type, ULONG size, char *name)
/***** make_header
	pushes a new element on the stack.
	return the position after the header.
******/
{	header *hd;
	char *erg;
#ifdef SPECIAL_ALIGNMENT
	size=(((size-1)/8)+1)*8;
#endif
	hd=(header *)(newram);
	if (newram+size>ramend)
	{	output("Stack overflow!\n"); error=2;
		return 0;
	}
	hd=(header *)newram;
	hd->size=size;
	hd->type=type;
	hd->flags=0;
	if (*name)
	{	strcpy(hd->name,name);
		hd->xor=xor(name);
	}
	else
	{	*(hd->name)=0;
		hd->xor=0;
	}
	erg=newram+sizeof(header);
	newram+=size;
	return erg;
}

header *new_matrix (int rows, int columns, char *name)
/***** new_matrix
	pops a new matrix on the stack.
*****/
{	ULONG size;
	dims *d;
	header *hd=(header *)newram;
	size=matrixsize(rows,columns);
	d=(dims *)make_header(s_matrix,size,name);
	if (d) { d->c=columns; d->r=rows; }
	return hd;
}

header *new_cmatrix (int rows, int columns, char *name)
/***** new_matrix
	pops a new matrix on the stack.
*****/
{	ULONG size;
	dims *d;
	header *hd=(header *)newram;
	size=matrixsize(rows,2*columns);
	d=(dims *)make_header(s_cmatrix,size,name);
	if (d) { d->c=columns; d->r=rows; }
	return hd;
}

header *new_command (int no)
/***** new_command
	pops a command on stack.
*****/
{	ULONG size;
	int *d;
	header *hd=(header *)newram;
	size=sizeof(header)+sizeof(int);
	d=(int *)make_header(s_command,size,"");
	if (d) *d=no;
	return hd;
}

header *new_real (double x, char *name)
/***** new real
	pops a double on stack.
*****/
{	ULONG size;
	double *d;
	header *hd=(header *)newram;
	size=sizeof(header)+sizeof(double);
	d=(double *)make_header(s_real,size,name);
	if (d) *d=x;
	return hd;
}

header *new_string (char *s, ULONG length, char *name)
/***** new real
	pops a string on stack.
*****/
{	ULONG size;
	char *d;
	header *hd=(header *)newram;
	size=sizeof(header)+((int)(length+1)/2+1)*2;
	d=(char *)make_header(s_string,size,name);
	if (d) {strncpy(d,s,length); d[length]=0;}
	return hd;
}

header *new_udf (char *name)
/***** new real
	pops a udf on stack.
*****/
{	ULONG size;
	ULONG *d;
	header *hd=(header *)newram;
//    size=sizeof(header)+sizeof(ULONG)+(ULONG)2;
    size=sizeof(header)+sizeof(ULONG);
	d=(ULONG *)make_header(s_udf,size,name);
	if (d) { *d=sizeof(header)+sizeof(ULONG); *((char *)(d+1))=0; }
	return hd;
}

header *new_complex (double x, double y, char *name)
/***** new real
	pushes a complex on stack.
*****/
{	ULONG size;
	double *d;
	header *hd=(header *)newram;
	size=sizeof(header)+2*sizeof(double);
	d=(double *)make_header(s_complex,size,name);
	if (d) {*d=x; *(d+1)=y;}
	return hd;
}

header *new_reference (header *ref, char *name)
{	ULONG size;
	header **d;
	header *hd=(header *)newram;
	size=sizeof(header)+sizeof(header *);
	d=(header **)make_header(s_reference,size,name);
	if (d) *d=ref;
	return hd;
}

header *new_subm (header *var, ULONG l, char *name)
/* makes a new submatrix, which is a single element */
{	ULONG size;
	header **d,*hd=(header *)newram;
	dims *dim;
	int *n,r,c;
	size=sizeof(header)+sizeof(header *)+
		sizeof(dims)+2*sizeof(int);
	d=(header **)make_header(s_submatrix,size,name);
	if (d) *d=var;
	else return hd;
	dim=(dims *)(d+1);
	dim->r=1; dim->c=1;
	n=(int *)(dim+1);
	c=dimsof(var)->c;
	if (c==0 || dimsof(var)->r==0)
	{	output("Matrix is empty!\n"); error=1031; return hd;
	}
	else r=(int)(l/c);
	*n++=r;
	*n=(int)(l-(ULONG)r*c-1);
	return hd;
}

header *new_csubm (header *var, ULONG l, char *name)
/* makes a new submatrix, which is a single element */
{	ULONG size;
	header **d,*hd=(header *)newram;
	dims *dim;
	int *n,r,c;
	size=sizeof(header)+sizeof(header *)+
		sizeof(dims)+2*sizeof(int);
	d=(header **)make_header(s_csubmatrix,size,name);
	if (d) *d=var;
	else return hd;
	dim=(dims *)(d+1);
	dim->r=1; dim->c=1;
	n=(int *)(dim+1);
	c=dimsof(var)->c;
	if (c==0 || dimsof(var)->r==0)
	{	output("Matrix is empty!\n"); error=1031; return hd;
	}
	else r=(int)(l/c);
	*n++=r;
	*n=(int)(l-r*c-1);
	return hd;
}

header *hnew_submatrix (header *var, header *rows, header *cols, 
	char *name, int type)
{	ULONG size;
	header **d;
	double *mr,*mc=0,x,*mvar;
	dims *dim;
	int c,r,*n,i,c0,r0,cvar,rvar,allc=0,allr=0;
	header *hd=(header *)newram;
	getmatrix(var,&rvar,&cvar,&mvar);
	if (rows->type==s_matrix)
	{	if (dimsof(rows)->r==1) r=dimsof(rows)->c;
		else if (dimsof(rows)->c==1) r=dimsof(rows)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mr=matrixof(rows);
	}
	else if (rows->type==s_real)
	{	r=1; mr=realof(rows);
	}
	else if (rows->type==s_command && *commandof(rows)==c_allv)
	{	allr=1; r=rvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	if (cols->type==s_matrix)
	{	if (dimsof(cols)->r==1) c=dimsof(cols)->c;
		else if (dimsof(cols)->c==1) c=dimsof(cols)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mc=matrixof(cols);
	}
	else if (cols->type==s_real)
	{	c=1; mc=realof(cols);
	}
	else if (cols->type==s_command && *commandof(cols)==c_allv)
	{	allc=1; c=cvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	size=sizeof(header)+sizeof(header *)+
		sizeof(dims)+((ULONG)r+c)*sizeof(int);
	d=(header **)make_header(type,size,name);
	if (d) *d=var;
	else return hd;
	dim = (dims *)(d+1);
	n=(int *)(dim+1);
	r0=0;
	if (allr)
	{	for (i=0; i<rvar; i++) *n++=i;
		r0=rvar;
	}
	else for (i=0; i<r; i++)
	{	x=(*mr++)-1;
		if (!((x<0.0) || (x>=rvar)) )
		{	*n++=(int)x; r0++;
		}
	}
	c0=0;
	if (allc)
	{	for (i=0; i<cvar; i++) *n++=i;
		c0=cvar;
	}
	else for (i=0; i<c; i++) 
	{	x=(*mc++)-1;
		if (!((x<0.0) || (x>=cvar))) 
		{	*n++=(int)x; c0++;
		}
	}
	dim->r=r0; dim->c=c0;
	size=(char *)n-(char *)hd;
#ifdef SPECIAL_ALIGNMENT
	size=((size-1)/8+1)*8;
#endif
	newram=(char *)hd;
	hd->size=size;
	return hd;
}

static header *built_csmatrix (header *var, header *rows, header *cols)
/***** built_csmatrix
	built a complex submatrix from the matrix hd on the stack.
*****/
{	double *mr,*mc=0,*mvar,*mh,*m;
	int n,c,r,c0,r0,i,j,cvar,rvar,allc=0,allr=0,*pc,*pr,*nc,*nr;
	header *hd;
	char *ram;
	getmatrix(var,&rvar,&cvar,&mvar);
	if (rows->type==s_matrix)
	{	if (dimsof(rows)->r==1) r=dimsof(rows)->c;
		else if (dimsof(rows)->c==1) r=dimsof(rows)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mr=matrixof(rows);
	}
	else if (rows->type==s_real)
	{	r=1; mr=realof(rows);
	}
	else if (rows->type==s_command && *commandof(rows)==c_allv)
	{	allr=1; r=rvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	if (cols->type==s_matrix)
	{	if (dimsof(cols)->r==1) c=dimsof(cols)->c;
		else if (dimsof(cols)->c==1) c=dimsof(cols)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mc=matrixof(cols);
	}
	else if (cols->type==s_real)
	{	c=1; mc=realof(cols);
	}
	else if (cols->type==s_command && *commandof(cols)==c_allv)
	{	allc=1; c=cvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	ram=newram;
	if (ram+((ULONG)(c)+(ULONG)(r))*sizeof(int)>ramend)
	{	output("Out of memory!\n"); error=710; return 0;
	}
	nr=pr=(int *)ram; nc=pc=pr+r; newram=(char *)(pc+c);
	c0=0; r0=0;
	if (allc) { for (i=0; i<c; i++) pc[i]=i; c0=c; }
	else for (i=0; i<c; i++)
	{	n=(int)(*mc++)-1;
		if (n>=0 && n<cvar) { *nc++=n; c0++; }
	}
	if (allr) { for (i=0; i<r; i++) pr[i]=i; r0=r; }
	else for (i=0; i<r; i++) 
	{	n=(int)(*mr++)-1;
		if (n>=0 && n<rvar) { *nr++=n; r0++; }
	}
	if (c0==1 && r0==1)
	{	m=cmat(mvar,cvar,pr[0],pc[0]);
		return new_complex(*m,*(m+1),"");
	}
	hd=new_cmatrix(r0,c0,""); if (error) return 0;
	m=matrixof(hd);
	for (i=0; i<r0; i++)
		for (j=0; j<c0; j++)
		{	mh=cmat(mvar,cvar,pr[i],pc[j]);
			*m++=*mh++;
			*m++=*mh;
		}
	return hd;
}

static header *built_smatrix (header *var, header *rows, header *cols)
/***** built_smatrix
	built a submatrix from the matrix hd on the stack.
*****/
{	double *mr,*mc=0,*mvar,*m;
	int n,c,r,c0,r0,i,j,cvar,rvar,allc=0,allr=0,*pr,*pc,*nc,*nr;
	header *hd;
	char *ram;
	getmatrix(var,&rvar,&cvar,&mvar);
	if (rows->type==s_matrix)
	{	if (dimsof(rows)->r==1) r=dimsof(rows)->c;
		else if (dimsof(rows)->c==1) r=dimsof(rows)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mr=matrixof(rows);
	}
	else if (rows->type==s_real)
	{	r=1; mr=realof(rows);
	}
	else if (rows->type==s_command && *commandof(rows)==c_allv)
	{	allr=1; r=rvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	if (cols->type==s_matrix)
	{	if (dimsof(cols)->r==1) c=dimsof(cols)->c;
		else if (dimsof(cols)->c==1) c=dimsof(cols)->r;
		else
		{	output("Illegal index!\n"); error=41; return 0;
		}
		mc=matrixof(cols);
	}
	else if (cols->type==s_real)
	{	c=1; mc=realof(cols);
	}
	else if (cols->type==s_command && *commandof(cols)==c_allv)
	{	allc=1; c=cvar;
	}
	else
	{	output("Illegal index!\n"); error=41; return 0;
	}
	ram=newram;
	if (ram+((ULONG)(c)+(ULONG)(r))*sizeof(int)>ramend)
	{	output("Out of memory!\n"); error=710; return 0;
	}
	nr=pr=(int *)ram; nc=pc=pr+r; newram=(char *)(pc+c);
	c0=0; r0=0;
	if (allc) { for (i=0; i<c; i++) pc[i]=i; c0=c; }
	else for (i=0; i<c; i++)
	{	n=(int)(*mc++)-1;
		if (n>=0 && n<cvar) { *nc++=n; c0++; }
	}
	if (allr) { for (i=0; i<r; i++) pr[i]=i; r0=r; }
	else for (i=0; i<r; i++) 
	{	n=(int)(*mr++)-1;
		if (n>=0 && n<rvar) { *nr++=n; r0++; }
	}
	if (c0==1 && r0==1)
	{	return new_real(*mat(mvar,cvar,pr[0],pc[0]),"");
	}
	hd=new_matrix(r0,c0,""); if (error) return 0;
	m=matrixof(hd);
	for (i=0; i<r0; i++)
		for (j=0; j<c0; j++)
			*m++=*mat(mvar,cvar,pr[i],pc[j]);
	return hd;
}

header *new_submatrix (header *hd, header *rows, header *cols, 
	char *name)
{	if (nosubmref) return built_smatrix(hd,rows,cols);
	return hnew_submatrix(hd,rows,cols,name,s_submatrix);
}

header *new_csubmatrix (header *hd, header *rows, header *cols, 
	char *name)
{	if (nosubmref) return built_csmatrix(hd,rows,cols);
	return hnew_submatrix(hd,rows,cols,name,s_csubmatrix);
}

/**************** builtin operators ****************/

void getmatrix (header *hd, int *r, int *c, double **m)
/***** getmatrix
	get rows and columns from a matrix.
*****/
{	dims *d;
	if (hd->type==s_real || hd->type==s_complex)
	{	*r=*c=1;
		*m=realof(hd);
	}
	else
	{	d=dimsof(hd);
		*m=matrixof(hd);
		*r=d->r; *c=d->c;
	}
}

int searchglobal=0;

header *searchvar (char *name)
/***** searchvar
	search a local variable, named "name".
	return 0, if not found.
*****/
{/*	int r;
	header *hd=(header *)startlocal;
	r=xor(name);
	while ((char *)hd<endlocal)
	{	if (r==hd->xor && !strcmp(hd->name,name)) return hd;
		hd=nextof(hd);
	}
	return 0;*/
	int r;
	header *hd=(header *)startlocal;
	r=xor(name);
	while ((char *)hd<endlocal)
	{	if (r==hd->xor && !strcmp(hd->name,name)) return hd;
		hd=nextof(hd);
	}
	if (startglobal!=startlocal && searchglobal)
	{   hd=(header *)startglobal;
		while ((char *)hd<endglobal)
		{	if (r==hd->xor && !strcmp(hd->name,name)) return hd;
			hd=nextof(hd);
		}
	}
	return 0;
}

header *searchudf (char *name)
/***** searchudf
	search a udf, named "name".
	return 0, if not found.
*****/
{	header *hd;
	int r;
	r=xor(name);
	hd=(header *)ramstart;
	while ((char *)hd<udfend && hd->type==s_udf)
	{	if (r==hd->xor && !strcmp(hd->name,name)) return hd;
		hd=nextof(hd);
	}
	return 0;
}

void kill_local (char *name)
/***** kill_local
	kill a local variable name, if there is one.
*****/
{	ULONG size,rest;
	header *hd=(header *)startlocal;
	while ((char *)hd<endlocal)
	{	if (!strcmp(hd->name,name)) /* found! */
		{	size=hd->size;
			rest=newram-(char *)hd-size;
			if (size) memmove((char *)hd,(char *)hd+size,rest);
			endlocal-=size; newram-=size;
			return;
		}
		hd=(header *)((char *)hd+hd->size);
	}
}

void kill_udf (char *name)
/***** kill_udf
	kill a local variable name, if there is one.
*****/
{	ULONG size,rest;
	header *hd=(header *)ramstart;
	while ((char *)hd<udfend)
	{	if (!strcmp(hd->name,name)) /* found! */
		{	size=hd->size;
			rest=newram-(char *)hd-size;
			if (size && rest) memmove((char *)hd,(char *)hd+size,rest);
			endlocal-=size; startlocal-=size; newram-=size;
			udfend-=size;
			return;
		}
		hd=(header *)((char *)hd+hd->size);
	}
}

int sametype (header *hd1, header *hd2)
/***** sametype
	returns true, if hd1 and hd2 have the same type and dimensions.
*****/
{	dims *d1,*d2;
	if (hd1->type!=hd2->type || hd1->size!=hd2->size) return 0;
	if (hd1->type==s_matrix)
	{	d1=dimsof(hd1); d2=dimsof(hd2);
			if (d1->r!=d2->r) return 0;
	}
	return 1;
}

header *assign (header *var, header *value)
/***** assign
	assign the value to the variable.
*****/
{	char name[MAXNAME],*nextvar;
	LONG size,dif;
	double *m,*mv,*m1,*m2;
	int i,j,c,r,cv,rv,*rind,*cind;
	dims *d;
	header *help,*orig;
	if (error) return 0;
	size=value->size;
	if (var->type==s_reference && !referenceof(var))
		/* seems to be a new variable */
	{	strcpy(name,var->name);
		if (value->type==s_udf)
		{	strcpy(value->name,name);
			value->xor=xor(name);
			if (newram+size>ramend)
			{	output("Memory overflow.\n"); error=500; return value;
			}
			memmove(ramstart+size,ramstart,newram-ramstart);
			newram+=size; endlocal+=size; startlocal+=size;
			value=(header *)((char *)value+size);
			udfend+=size;
			memmove(ramstart,(char *)value,size);
			return (header *)ramstart;
		}
		memmove(endlocal+size,endlocal,newram-endlocal);
		value=(header *)((char *)value+size);
		newram+=size;
		memmove(endlocal,(char *)value,size);
		strcpy(((header *)endlocal)->name,name);
		((header *)endlocal)->xor=xor(name);
		value=(header *)endlocal;
		endlocal+=size;
		return value;
	}
	else
	{	while (var && var->type==s_reference) var=referenceof(var);
		if (!var)
		{	error=43; output("Internal variable error!\n"); return 0;
		}
		if (var->type!=s_udf && value->type==s_udf)
		{	output("Cannot assign a UDF to a variable!\n"); error=320;
			return var;
		}
		if (var->type==s_submatrix)
		{	d=submdimsof(var);
			if (value->type==s_complex || value->type==s_cmatrix)
			{	orig=submrefof(var);
				help=new_reference(orig,""); 
				if (error) return 0;
					mcomplex(help); if (error) return 0;
				var->type=s_csubmatrix;
				submrefof(var)=help;
				assign(var,value); if (error) return 0;
				submrefof(var)=orig;
				assign(orig,help); 
				return orig;
			}
			else if (value->type!=s_real && value->type!=s_matrix)
			{	output("Illegal assignment!\n"); error=45; return 0;
			}
			getmatrix(value,&rv,&cv,&mv);
			getmatrix(submrefof(var),&r,&c,&m);
			if (d->r!=rv || d->c!=cv)
			{	output("Illegal assignment!\n"); error=45; return 0;
			}
			rind=rowsof(var); cind=colsof(var);
			for (i=0; i<d->r; i++)
			{	m1=mat(m,c,rind[i],0);
				m2=mat(mv,cv,i,0);
				for (j=0; j<d->c; j++)
				{	m1[cind[j]]=*m2++;
				}
			}
			return submrefof(var);
		}
		else if (var->type==s_csubmatrix)
		{	d=submdimsof(var);
			if (value->type==s_real || value->type==s_matrix)
			{	help=new_reference(value,""); if (error) return 0;
				mcomplex(help); if (error) return 0;
				assign(var,help);
				return submrefof(var);
			}
			if (value->type!=s_complex && value->type!=s_cmatrix)
			{	output("Illegal assignment!\n"); error=45; return 0;
			}
			getmatrix(value,&rv,&cv,&mv);
			getmatrix(submrefof(var),&r,&c,&m);
			if (d->r!=rv || d->c!=cv)
			{	output("Illegal assignment!\n"); error=45; return 0;
			}
			rind=rowsof(var); cind=colsof(var);
			for (i=0; i<d->r; i++)
			{	m1=cmat(m,c,rind[i],0);
				m2=cmat(mv,cv,i,0);
				for (j=0; j<d->c; j++)
                {   copy_complex(m1+(ULONG)2*cind[j],m2); m2+=2;
				}
			}
			return submrefof(var);
		}		
		else 
		{	if ((char *)var<startlocal || (char *)var>endlocal) 
			/* its not a local variable! */
			{	if (!sametype(var,value))
				{	output1("Cannot change type of non-local variable %s!\n",
						var->name);
					error=12; return 0;
				}
				memcpy((char *)(var+1),(char *)(value+1),
					value->size-sizeof(header));
				return var;
			}
			dif=value->size-var->size;
//fprintf(stderr,"var->name: %s\n",var->name);
//fprintf(stderr,"var->size: %d\nval->size: %d\n\n",var->size, value->size);
			if (newram+dif>ramend)
			{	output("Memory overflow\n"); error=501; return value;
			}
			nextvar=(char *)var+var->size;
			if (dif!=0)
				memmove(nextvar+dif,nextvar,newram-nextvar);
			newram+=dif; endlocal+=dif;
			value=(header *)((char *)value+dif);
			strcpy(value->name,var->name);
			value->xor=var->xor;
			memmove((char *)var,(char *)value,value->size);
		}
	}
	return var;
}

header *getvalue (header *hd)
/***** getvalue
	get an actual value of a reference.
    references to functions with no arguments (e.g. pi) should be
    executed
    submatrices should be resolved to matrices
*****/
{	header *old=hd,*mhd,*result;
	dims *d;
	double *m,*mr,*m1,*m2,*m3;
	int r,c,*rind,*cind,*cind1,i,j;
	
	while (hd && hd->type==s_reference)
		hd=referenceof(hd);
		
	if (!hd) 					
	{	/* points nowhere, try to see if it's a function
		   (builtin or udf) without parameter */
		mhd=(header *)newram;
		if (exec_builtin(old->name,0,mhd)) return mhd;
		hd=searchudf(old->name);
		if (hd)
		{	interpret_udf(hd,mhd,0);
			return mhd;
		}
		/* no variable, no function, so error */
		output1("Variable %s not defined!\n",old->name);
		error=10; return NULL;
	}
	
	/* there is a variable
	   resolve submatrices to matrices */
	if (hd->type==s_submatrix)
	{	mhd=submrefof(hd); d=submdimsof(hd);
		rind=rowsof(hd); cind=colsof(hd);
		getmatrix(mhd,&r,&c,&m);
		if (d->r==1 && d->c==1)
			return new_real(*mat(m,c,*rind,*cind),"");
		result=new_matrix(d->r,d->c,"");
		mr=matrixof(result);
		for (i=0; i<d->r; i++)
		{	cind1=cind;
			m1=mat(mr,d->c,i,0);
			m2=mat(m,c,*rind,0);
			for (j=0; j<d->c; j++)
			{	m1[j]=m2[*cind1];
				cind1++;
			}
			rind++;
		}
		return result;
	}
	if (hd->type==s_csubmatrix)
	{	mhd=submrefof(hd); d=submdimsof(hd);
		rind=rowsof(hd); cind=colsof(hd);
		getmatrix(mhd,&r,&c,&m);
		if (d->r==1 && d->c==1)
		{	m=cmat(m,c,*rind,*cind);
			return new_complex(*m,*(m+1),"");
		}
		result=new_cmatrix(d->r,d->c,"");
		mr=matrixof(result);
		for (i=0; i<d->r; i++)
		{	cind1=cind;
			m1=cmat(mr,d->c,i,0);
			m2=cmat(m,c,*rind,0);
			for (j=0; j<d->c; j++)
            {   m3=m2+(ULONG)2*(*cind1);
				*m1++=*m3++; *m1++=*m3;
				cind1++;
			}
			rind++;
		}
		return result;
	}
	/* resolve 1x1 matrices to scalars */
	if (hd->type==s_matrix && dimsof(hd)->c==1 && dimsof(hd)->r==1)
	{	return new_real(*matrixof(hd),"");
	}
	if (hd->type==s_cmatrix && dimsof(hd)->c==1 && dimsof(hd)->r==1)
	{	return new_complex(*matrixof(hd),*(matrixof(hd)+1),"");
	}
	/* just return the variable */
	return hd;
}

header *getvariable (header *hd)
/***** getvariable
	get an actual variable of a reference.
*****/
{	header *old=hd;
	while (hd && hd->type==s_reference)
		hd=referenceof(hd);
	if (!hd)
	{	output1("Variable %s not defined!\n",old->name);
		error=10; return new_string("Fehler",6,"");
	}
	return hd;
}

header *next_param (header *hd)
/***** next_param
	get the next value on stack, if there is one
*****/
{	hd=(header *)((char *)hd+hd->size);
	if ((char *)hd>=newram) return 0;
	else return hd;
}

#if 0
header* getparam(header* hd)
/***** getparam
	get the next parameter on the stack, if there is one,
	dereference up to the value.
*****/
{
	hd=nextof(hd);
	if ((char *)hd>=newram) return NULL; /* nothing above top of stack */
	/* get the actual value */
	hd=getvalue(hd);
}
#endif

void moveresult (header *stack, header *result)
/***** moveresult
	move the result to the start of stack.
*****/
{	if (stack==result) return;
	memmove((char *)stack,(char *)result,result->size);
	newram=(char *)stack+stack->size;
}

void moveresult1 (header *stack, header *result)
/***** moveresult
	move several results to the start of stack.
*****/
{	ULONG size;
	if (stack==result) return;
	size=newram-(char *)result;
	memmove((char *)stack,(char *)result,size);
	newram=(char *)stack+size;
}

void get_element (int nargs, header *var, header *hd)
/***** get_elements
	get the element of the matrix.
*****/
{	header *st=hd,*result,*hd1;
	var=getvalue(var); if (error) return;
	if (var->type==s_string) /* interpret the string as a function */
	{	if (exec_builtin(stringof(var),nargs,hd));
		else
		{	hd1=searchudf(stringof(var));
			if (hd1) interpret_udf(hd1,hd,nargs);
			else
			{	output1("%s is not function name!\n",stringof(var));
				error=2020; return;
			}
		}
		return;
	}
	hd=getvalue(hd); if (error) return;
	if (nargs<1 || nargs>2) 
	{ 	error=30; output("Illegal matrix reference!\n"); return; }
	if (nargs==2) 
	{	hd1=getvalue(next_param(st)); if (error) return;
	}
	else 
	{	if (dimsof(var)->r==1) { hd1=hd; hd=new_real(1.0,""); }
		else hd1=new_command(c_allv);
		if (error) return;
	}
	if (var->type==s_matrix || var->type==s_real)
	{	result=new_submatrix(var,hd,hd1,"");
	}
	else if (var->type==s_cmatrix || var->type==s_complex)
	{	result=new_csubmatrix(var,hd,hd1,"");
	}
	else
	{	error=31; output1("Not a matrix %s!\n",var->name); return;
	}
	if (error) return;
	moveresult(st,result);
}

void get_element1 (char *name, header *hd)
/* get an element of a matrix, referenced by *realof(hd),
   where the matrix is dentified with a vector of same length
*/
{	header *st=hd,*result,*var;
	ULONG n,l;
	int r,c;
	double *m;
	hd=getvalue(hd);
	var=searchvar(name);
	if (!var)
	{	output1("%s not a variable!\n",name);
		error=1012; return;
	}
	var=getvalue(var); if (error) return;
	if (hd->type!=s_real)
	{	output("Index must be a number!\n");
		error=1013; return;
	}
	if (error) return;
	if (var->type==s_real)
	{	result=new_reference(var,"");
	}
	else if (var->type==s_complex)
	{	result=new_reference(var,"");
	}
	else if (var->type==s_matrix)
	{	getmatrix(var,&r,&c,&m);
		l=(ULONG)(*realof(hd));
		n=(ULONG)r*c;
		if (l>n) l=n;
		if (l<1) l=1;
		if (nosubmref) result=new_real(*(m+l-1),"");
		else result=new_subm(var,l,"");
	}
	else if (var->type==s_cmatrix)
	{	getmatrix(var,&r,&c,&m);
		l=(ULONG)(*realof(hd));
		n=(ULONG)r*c;
		if (n==0)
		{	output("Matrix is empty!\n"); error=1030; return;
		}
		if (l>n) l=n;
		if (l<1) l=1;
		if (nosubmref)
        {   m+=(ULONG)2*(l-1);
			result=new_complex(*m,*(m+1),"");
		}
		else result=new_csubm(var,l,"");
	}
	else
	{	output1("%s not a variable of proper type for {}!\n");
		error=1011; return;
	}
	moveresult(st,result);
}
