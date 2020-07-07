#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <limits.h>

#include "sysdep.h"
#include "core.h"
#include "funcs.h"
#include "matheh.h"
#include "spread.h"

//#define wrong_arg() { error=26; output("Wrong argument\n"); return; }

/****************************************************************
 *	const
 ****************************************************************/
void mpi (header *hd)
{	new_real(M_PI,"");
}

void mepsilon (header *hd)
{	new_real(epsilon,"");
}

void msetepsilon (header *hd)
{	header *stack=hd,*hd1,*result;
	hd1=getvalue(hd); if (error) return;
	if (hd1->type!=s_real) wrong_arg("real value expected");
	result=new_real(epsilon,"");
	epsilon=*realof(hd1);
	moveresult(stack,result);
}	

/****************************************************************
 *	language features
 ****************************************************************/

/*********************** time, string io ************************/
void mtime (header *hd)
{	hd=new_real(myclock(),"");
}

void mwait (header *hd)
{	header *st=hd,*result;
	double now;
	int h;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) wrong_arg("real value expected");
	now=myclock();
	sys_wait(*realof(hd),&h);
	if (h==escape) { error=1; return; }
	result=new_real(myclock()-now,"");
	if (error) return;
	moveresult(st,result);
}

void mkey (header *hd)
{	scantyp scan;
	wait_key(&scan);
	new_real(scan,"");
}

void minput (header *hd)
{	header *st=hd,*result;
	char input[1024],*oldnext;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) wrong_arg("string expected");
	do {
		output(stringof(hd)); output("? ");
		
		// !! BUG: LIMIT THE INPUT LENGTH
		edit(input);
		error=0;
		stringon=1;
		oldnext=next; next=input; result=scan_value(); next=oldnext;
		stringon=0;
	
		if (error) {
			output("Error in input!\n");
		}
	} while (error);
	
	moveresult(st,result);
}

void mlineinput (header *hd)
{	header *st=hd,*result;
	char input[1024];
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) wrong_arg("string expected");
	output(stringof(hd)); output("? ");
	edit(input);
	result=new_string(input,strlen(input),"");
	moveresult(st,result);
}

void mchar (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real) wrong_arg("ascii code expected");
	result=new_string("a",1,""); if (error) return;
	*stringof(result)=(char)*realof(hd);
	moveresult(st,result);
}

void mprintf (header *hd)
{	header *st=hd,*hd1,*result;
	char string[1024];
	hd1=next_param(hd);
	hd=getvalue(hd);
	hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_string || hd1->type!=s_real)
		wrong_arg("printf(\"format string\",value)");
	snprintf(string,1024,stringof(hd),*realof(hd1));
	result=new_string(string,strlen(string),""); if (error) return;
	moveresult(st,result);
}

void msetkey (header *hd)
/*****
	set a function key
*****/
{	header *st=hd,*hd1,*result;
	char *p;
	int n;
	hd=getvalue(hd); if (error) return;
	hd1=nextof(st); hd1=getvalue(hd1); if (error) return;
	if (hd->type!=s_real || hd1->type!=s_string) wrong_arg("setkey(F key number,\"str\")");
	n=(int)(*realof(hd))-1; p=stringof(hd1);
	if (n<0 || n>=10 || strlen(p)>63) wrong_arg("too long");
	result=new_string(fktext[n],strlen(fktext[n]),"");
	if (error) return;
	strcpy(fktext[n],p);
	moveresult(st,result);
}

/***************** number display mode handling *****************/
/* Display modes :
   - standard formats
     mode FRAC: display result as aproximated fraction (?), fallback to STD
                mode if the aproximation is too long to converge.
       params: eps, nb of digits to display a number
     mode STD: display smartly the result as standard or exponentiated number
       params: nb of significant digits, nb of digits to display a number
     mode SCI: display in loating point scientific notation m*1Eexp
     	params: nb of significant digits, nb of digits to display a number
     mode FIXED: display the result with a fixed point format (may be bad
                 with floating point data)
       params: number of dits after the decimal point, nb of digits to display a number
   - ingineering formats
     mode ENG1: use exponentiation format (incr/decr of the power of 10 by 3)
     mode ENG2: use suffixes
       params: nb of digits to display a number
 */

/***** mformat
   format(
     "STD"|"FIXED"|"ENG1"|"ENG2"|"SCI"|"FIXED"|"FRAC",
     [extend, digits]
   )
 *****/
void mformat (header *hd)
{	header *st=hd,*result, *hd1=next_param(hd);
	int l,d;
	int oldl=disp_fieldw,oldd=disp_digits;
	int dmode;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_string) {
		if (strcmp(stringof(hd),"STD")==0) {
			dmode=0;
		} else if (strcmp(stringof(hd),"ENG1")==0) {
			dmode=1;
		} else if (strcmp(stringof(hd),"ENG2")==0) {
			dmode=2;
		} else if (strcmp(stringof(hd),"SCI")==0) {
			dmode=3;
		} else if (strcmp(stringof(hd),"FIXED")==0) {
			dmode=4;
		} else if (strcmp(stringof(hd),"FRAC")==0) {
			dmode=5;
		} else {
			wrong_arg("Display mode expected");
		}
	}
	if (hd1) {
		hd1=getvalue(hd1); if (error) return;
		if (hd1->type!=s_matrix || dimsof(hd1)->r!=1 || dimsof(hd1)->c!=2)
			wrong_arg("1x2 real vector expected");
		l=(int)*matrixof(hd1); d=(int)*(matrixof(hd1)+1);
		if (l<0 || l>2*DBL_DIG+18 || d<0 || d>DBL_DIG+1) wrong_arg("bad value");
		
		disp_eng_sym=0;		/*  no multiple/submultiple symbols */
		
		switch (dmode) {
		case 0: /* smart STD */
			if (l<d+8) l=d+8;
			sprintf(fixedformat,"%%0.%dG",d);
			sprintf(expoformat,"%%0.%dE",d-1);
			minexpo=pow(10,(-d+1>-4) ? -d+1 : -4);
			maxexpo=pow(10,d-1)-0.1;
			break;
		case 1:		/* ENG1 */
			if (d<3) {
				d=3;
				output("warning: \"ENG1\" display mode can't be set to less than 3 digits (3 digits enforced)\n");
			}
			if (l<d+8) l=d+8; 
			disp_eng_sym=0;
			break;
		case 2:		/* ENG2 */
			if (d<3) {
				d=3;
				output("warning: \"ENG2\" display mode can't be set to less than 3 digits (3 digits enforced)\n");
			}
			if (l<d+4) l=d+4;
			disp_eng_sym=1;
			break;
		case 3:		/* SCI */
			if (l<d+8) l=d+8;
			sprintf(expoformat,"%%0.%dE",d-1);
			break;
		case 4:		/* FIXED */
			if (l<DBL_DIG+3) l=DBL_DIG+3;
			sprintf(fixedformat,"%%0.%dF",d);
			break;
		case 5:		/* FRAC */
		default:	/* never used */
			wrong_arg("mode not supported");
			break;
		}
		disp_mode=dmode;
		disp_digits=d;
		disp_fieldw=l;
	}
	result=new_matrix(1,2,""); if (error) return;
	*matrixof(result)=oldl;
	*(matrixof(result)+1)=oldd;
	moveresult(st,result);
}

/**************** user defined function handling ****************/
char *argname[] =
	{ "arg1","arg2","arg3","arg4","arg5","arg6","arg7","arg8","arg9",
		"arg10" } ;
char xors[MAXARGS];

void make_xors (void)
{	int i;
	for (i=0; i<MAXARGS; i++) xors[i]=xor(argname[i]);
}

header *running;
int udfon=0;

static int actargn=0;

void interpret_udf (header *var, header *args, int argn)
/**** interpret_udf
	interpret a user defined function.
	context:
	- udfline
	- udfon status: 0=in global frame
	                1=in a udf
	                2=in the return statement
	- searchglobal: search variables in the global frame as well?
	- actargn     : actual number of arguments pushed on the stack
	- startlocal  : beginning of the function frame used to access
	                its arguments and local variables, on the stack.
	- endlocal    : end of the function frame.
	- running     : the current running function (header of the 
	                function on the stack).
	Parameters:
	
	Parameters are passed as references, but they sould not be 
	writable.
	
	TODO: add a 'copy on write' policy scheme for parameters.
	
	- formal parameters are defined from address helpof(var)
	  the number of formal parameters is given by
	  int nargu = *(int *)helpof(var)
	  
	- the 'argn' actual parameters are pushed on the stack by the
	  calling env and are available from address 'args'
****/
{	int udfold,nargu,i,oldargn,oldtrace, oldsearchglobal;
	char *oldnext=next,*oldstartlocal,*oldendlocal,*udflineold,*p;
	header *result,*st=args,*hd=args,*oldrunning;
	unsigned int arg_bitmap=0;

	/* set p to point to the start of the formal parameter block */
	p=helpof(var);
	nargu=*((int *)p); p+=sizeof(int);
	unsigned int def_bitmap = *(unsigned int*)p; p+=sizeof(unsigned int);
	
	/* name actual parameters according to the formal ones defined
	   in the function parameter list */
	for (i=0; i<argn; i++) {
		if (i<nargu) {	/* standard parameters */
			if (hd->type==s_reference && !referenceof(hd) && hd->name[0]==0) {
				/* empty reference parameter: skip for now
				   look later for named parameter or default val */
			} else {
				udf_arg* arg=(udf_arg*)p;
				strcpy(hd->name,arg->name); hd->xor=arg->xor;
				arg_bitmap |= 1<<i;
			}
			p=next_arg(p, def_bitmap & (1<<i));
		} else {		/* extra parameters */
			if (hd->type==s_reference && !referenceof(hd) && hd->name[0]==0) {
				output1("Empty extra parameter in function %s (useless!)\n", var->name);
				error=701; return;
			} else {
				/* valid parameter: rename it 'arg#' with # the
				   position in the actual parameter list */
				strcpy(hd->name,argname[i]);
				hd->xor=xors[i];
			}
		}
		hd=nextof(hd);
	}

	/* try to see if named parameters set on the stack correspond
	   to unset parameters. Alert on duplicate parameter setups */
	while (hd!=(header*)newram) {
		p=helpof(var)+sizeof(int)+sizeof(unsigned int);
		for (i=0; i<nargu; i++) {
			udf_arg* arg=(udf_arg*)p;
			if ( (hd->xor==arg->xor) && (strcmp(hd->name,arg->name)==0) ) {
				if (arg_bitmap & (1<<i)) {	/* error! parameter set twice */
					output1("parameter %s already set by standard parameter\n",hd->name);
					error=701; return;
				} else {					/* named parameter used */
					arg_bitmap|=1<<i;
				}
			}
			/* next arg in the function formal parameter list */
			p=next_arg(p, def_bitmap & (1<<i));
		}	
		hd=nextof(hd);
	}
	
	/* check if all required parameters have values, try to use
	   the default value, if any, for those unset. */
	p=helpof(var)+sizeof(int)+sizeof(unsigned int);
	for (i=0; i<nargu; i++) {
		
		if ((arg_bitmap & (1<<i))==0) {
			/* this has no value, try to put the default value */
			if (def_bitmap & (1<<i)) {
				moveresult((header *)newram,(header *)p);
				arg_bitmap |= 1<<i;
#ifndef STRICT
			}	/* allows required parameter to not be set */
#else
			} else {
				/* checks that all required parameters are set */
				udf_arg* arg=(udf_arg*)p;
				output1("Argument %s undefined.\n", arg->name);
				error=1; return;
			}
#endif
		}	/* else, this parameter has a value set, skip it */
		p=next_arg(p, def_bitmap & (1<<i));
	}
	
	/* Save context of the caller */
	udflineold=udfline; udfold=udfon;
	oldargn=actargn;
	oldstartlocal=startlocal; oldendlocal=endlocal;
	oldsearchglobal=searchglobal; searchglobal=0;
	oldrunning=running; 
	/* setup the new scope */
	startlocal=(char *)args; endlocal=newram; running=var;
	actargn=argn;
	udfline=next=udfof(var); udfon=1;
	
	if ((oldtrace=trace)>0)
	{	if (trace==2) trace=0;
		if (trace>0) trace_udfline(next);
	}
	else if (var->flags&1)
	{	trace=1;
		if (trace>0) trace_udfline(next);
	}
	/* interpret the udf code */
	while (!error && udfon==1)
	{	command();
		if (udfon==2) /* handling of the return of multiple values */
		{	result=scan_value(); 
			if (error) 
			{	output1("Error in function %s\n",var->name);
				print_error(udfline);
				break;
			}
			moveresult1(st,result);
			break;
		}
		if (test_key()==escape) 
		{	output("User interrupted!\n"); error=58; break; 
		}
	}
	/* function finished, restore the context of the caller */
	endlocal=oldendlocal; startlocal=oldstartlocal;
	running=oldrunning;
	if (trace>=0) trace=oldtrace;
	if (error) output1("Error in function %s\n",var->name);
	if (udfon==0)
	{	output1("Return missing in %s!\n",var->name); error=55; }
	udfon=udfold; next=oldnext; udfline=udflineold;
	actargn=oldargn;
	searchglobal=oldsearchglobal;
}

void mdo (header *hd)
{	header *st=hd,*hd1,*result;
	int count=0;
	ULONG size;
	if (!hd) wrong_arg("parameter required");
	hd=getvalue(hd);
	result=hd1=next_param(st);
	if (hd->type!=s_string) wrong_arg("1st arg: string expected");
	if (error) return;
	hd=searchudf(stringof(hd));
	if (!hd || hd->type!=s_udf) wrong_arg("1st arg not a user defined function");
	while (hd1 && count<=MAXARGS) 
	{	strcpy(hd1->name,argname[count]);
		hd1->xor=xors[count];
		hd1=next_param(hd1); count++; 
	}
	if (result)
	{	size=(char *)result-(char *)st;
		if (size>0 && newram!=(char *)result) 
			memmove((char *)st,(char *)result,newram-(char *)result);
		newram-=size;
	}
	interpret_udf(hd,st,count);
}

void minterpret (header *hd)
{	header *st=hd,*result;
	char *oldnext;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) wrong_arg("string to be interpreted expected");
	stringon=1;
	oldnext=next; next=stringof(hd); result=scan(); next=oldnext;
	stringon=0;
	if (error) { result=new_string("Syntax error!",5,""); error=0; }
	moveresult(st,result);
}

void margn (header *hd)
{	new_real(actargn,"");
}

void margs (header *hd)
/* return all args from realof(hd)-st argument on */
{	header *st=hd,*hd1,*result;
	int i,n;
	ULONG size;
	hd=getvalue(hd);
	if (hd->type!=s_real) wrong_arg("real value expected");
	n=(int)*realof(hd);
	if (n<1) wrong_arg("must be >= 1");
	if (n>actargn)
	{	newram=(char *)st; return;
	}
	result=(header *)startlocal; i=1;
	while (i<n && result<(header *)endlocal)
	{	result=nextof(result); i++;
	}
	hd1=result;
	while (i<actargn+1 && hd1<(header *)endlocal)
	{	hd1=nextof(hd1); i++;
	}
	size=(char *)hd1-(char *)result;
	if (size<=0)
	{	output("Error in args!\n"); error=2021; return;
	}
	memmove((char *)st,(char *)result,size);
	newram=(char *)st+size;
}

void mindex (header *hd)
{	new_real((double)loopindex,"");
}

/******************** type introspection ***********************/
void mname (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	result=new_string(hd->name,strlen(hd->name),"");
	moveresult(st,result);
}

void miscomplex (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd);
	if (hd->type==s_complex || hd->type==s_cmatrix)
		result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void misreal (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
		result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void misstring (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_string)
		result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void misfunction (header *hd)
{	header *st=hd,*result;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_string
		&& (searchudf(stringof(hd))!=0 || find_builtin(stringof(hd))!=0))
			result=new_real(1.0,"");
	else result=new_real(0.0,"");
	if (error) return;
	moveresult(st,result);
}

void misvar (header *hd)
{
	header *st=hd,*result;
	if (hd->type==s_reference && searchvar(hd->name)!=0) {
		result=new_real(1.0,"");
	} else {
		result=new_real(0.0,"");
	}
	if (error) return;
	moveresult(st,result);
}

/************************* error handling ***********************/
void merror (header *hd)
{	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) wrong_arg("string expected");
	output1("Error: %s\n",stringof(hd));
	error=301;
}

void merrlevel (header *hd)
{	header *st=hd,*res;
	char *oldnext;
	int en;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) wrong_arg("string expected");
	stringon=1;
	oldnext=next; next=stringof(hd); scan(); next=oldnext;
	stringon=0;
	en=error; error=0;
	res=new_real(en,""); if (error) return;
	moveresult(st,res);
}


/*************************** file IO ****************************/
#if 0
void mcd (header *hd)
{	header *st=hd,*result;
	char *path;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) wrong_arg("path expected");
	path=cd(stringof(hd));
	result=new_string(path,strlen(path),"");
	moveresult(st,result);
}

void mdir (header *hd)
{	header *st=hd,*result;
	char *name;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string) wrong_arg("path expected");
	name=dir(stringof(hd));
	if (name) result=new_string(name,strlen(name),"");
	else result=new_string("",0,"");
	if (error) return;
	moveresult(st,result);
}

void mdir0 (header *hd)
{	char *name;
	name=dir(0);
	if (name) new_string(name,strlen(name),"");
	else new_string("",0,"");
}
#endif
/************************ Stack handling ************************/
void mfree (header *hd)
{	new_real(ramend-endlocal,"");
}

typedef struct { intptr_t udfend,startlocal,endlocal,newram; }
	ptyp;

void mstore (header *hd)
{	FILE *file;
	ptyp p;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
	{	output("Expect file name.\n");
		error=1100; return;
	}
	p.udfend=udfend-ramstart;
	p.startlocal=startlocal-ramstart;
	p.endlocal=endlocal-ramstart;
	p.newram=newram-ramstart;
	file=fopen(stringof(hd),"wb");
	if (!file)
	{	output1("Could not open %s.\n",stringof(hd));
		error=1101; return;
	}
	fwrite(&p,sizeof(ptyp),1,file);
	fwrite(ramstart,1,newram-ramstart,file);
	if (ferror(file))
	{	output("Write error.\n");
		error=1102; return;
	}
	fclose(file);
}
	
void mrestore (header *hd)
{	FILE *file;
	ptyp p;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_string)
	{	output("Expect file name.\n");
		error=1100; return;
	}
	file=fopen(stringof(hd),"rb");
	if (!file)
	{	output1("Could not open %s.\n",stringof(hd));
		error=1103; return;
	}
	fread(&p,sizeof(ptyp),1,file);
	if (ferror(file))
	{	output("Read error.\n");
		error=1104; return;
	}
	fread(ramstart,1,p.newram,file);
	if (ferror(file))
	{	output("Read error (fatal for EULER).\n");
		error=1104; return;
	}
	fclose(file);
	udfend=ramstart+p.udfend;
	startlocal=ramstart+p.startlocal;
	endlocal=ramstart+p.endlocal;
	newram=ramstart+p.newram;
}

/****************************************************************
 *	basic operators
 ****************************************************************/
static void real_add (double *x, double *y, double *z)
{	*z=*x+*y;
}

static void complex_add (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	*z=*x+*y;
	*zi=*xi+*yi;
}

void add (header *hd, header *hd1)
/***** add
	add the values.
*****/
{	header *result,*st=hd;
	hd=getvalue(hd); hd1=getvalue(hd1);
	if (error) return;
	result=map2(real_add,complex_add,hd,hd1);
	if (!error) moveresult(st,result);
}

static void real_subtract (double *x, double *y, double *z)
{	*z=*x-*y;
}

static void complex_subtract (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	*z=*x-*y;
	*zi=*xi-*yi;
}

void subtract (header *hd, header *hd1)
/***** add
	add the values.
*****/
{	header *result,*st=hd;
	hd=getvalue(hd); hd1=getvalue(hd1);
	if (error) return;
	result=map2(real_subtract,complex_subtract,hd,hd1);
	if (!error) moveresult(st,result);
}

static void real_multiply (double *x, double *y, double *z)
{	*z=*x*(*y);
}

void complex_multiply (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	*z=*x * *y - *xi * *yi;
	*zi=*x * *yi + *xi * *y;
}

void dotmultiply (header *hd, header *hd1)
/***** add
	add the values.
*****/
{	header *result,*st=hd;
	hd=getvalue(hd); hd1=getvalue(hd1);
	if (error) return;
	result=map2(real_multiply,complex_multiply,hd,hd1);
	if (!error) moveresult(st,result);
}

static void real_divide (double *x, double *y, double *z)
{	
#ifdef FLOAT_TEST
	if (*y==0.0) { *y=1e-10; error=1; }
#endif
	*z=*x/(*y);
}

void complex_divide (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	double r;
	r=*y * *y + *yi * *yi;
#ifdef FLOAT_TEST
	if (r==0) { r=1e-10; error=1; }
#endif
	*z=(*x * *y + *xi * *yi)/r;
	*zi=(*xi * *y - *x * *yi)/r;
}

void dotdivide (header *hd, header *hd1)
/***** add
	add the values.
*****/
{	header *result,*st=hd;
	hd=getvalue(hd); hd1=getvalue(hd1);
	if (error) return;
	result=map2(real_divide,complex_divide,hd,hd1);
	if (!error) moveresult(st,result);
}

void cscalp (double *s, double *si, double *x, double *xi,
	double *y, double *yi)
{	*s += *x * *y - *xi * *yi;
	*si += *x * *yi + *xi * *y;
}

void ccopy (double *y, double *x, double *xi)
{	*y++=*x; *y=*xi;
}

void multiply (header *hd, header *hd1)
/***** multiply
	matrix multiplication.
*****/
{	header *result,*st=hd;
	dims *d,*d1;
	double *m,*m1,*m2,*mm1,*mm2,x,y,null=0.0;
	int i,j,c,r,k;
	hd=getvalue(hd); hd1=getvalue(hd1);
	if (error) return;
	if (hd->type==s_matrix && hd1->type==s_matrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_matrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	mm1=mat(m1,d->c,i,0); mm2=m2+j;
				x=0.0;
				for (k=0; k<d->c; k++)
				{	x+=(*mm1)*(*mm2);
					mm1++; mm2+=d1->c;
				}
				*mat(m,c,i,j)=x;
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_matrix && hd1->type==s_cmatrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
            {   mm1=mat(m1,d->c,i,0); mm2=m2+(ULONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	cscalp(&x,&y,mm1,&null,mm2,mm2+1);
					mm1++; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_matrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	mm1=cmat(m1,d->c,i,0); mm2=m2+j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	cscalp(&x,&y,mm1,mm1+1,mm2,&null);
					mm1+=2; mm2+=d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_cmatrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
            {   mm1=cmat(m1,d->c,i,0); mm2=m2+(ULONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	cscalp(&x,&y,mm1,mm1+1,mm2,mm2+1);
					mm1+=2; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	else dotmultiply(st,nextof(st));
}

void divide (header *hd, header *hd1)
{	dotdivide(hd,hd1);
}

static void real_invert (double *x, double *y)
{	*y= -*x;
}

static void complex_invert (double *x, double *xi, double *y, double *yi)
{	*y= -*x;
	*yi= -*xi;
}

void invert (header *hd)
/***** invert
	compute -matrix.
*****/
{	header *result,*st=hd;
	hd=getvalue(hd);
	if (error) return;
	result=map1(real_invert,complex_invert,hd);
	if (!error) moveresult(st,result);
}


/****************************************************************
 *	compare and boolean operator
 ****************************************************************/
static void rgreater (double *x, double *y, double *z)
{	if (*x>*y) *z=1.0;
	else *z=0.0;
}

void mgreater (header *hd)
{	
	header *st=hd,*hd1=nextof(hd),*result;
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string) {
		if (hd1->type!=s_string) wrong_arg("can only compare a string with another one");
		result=new_real(strcmp(stringof(hd),stringof(hd1))>0,"");
		moveresult(st,result);
	} else if (hd1->type!=s_string) {
		result=map2(rgreater,0,hd,hd1);
		if (!error) moveresult(st,result);
	}
}

static void rless (double *x, double *y, double *z)
{	if (*x<*y) *z=1.0;
	else *z=0.0;
}

void mless (header *hd)
{
	header *st=hd,*hd1=nextof(hd),*result;
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string) {
		if (hd1->type!=s_string) wrong_arg("can only compare a string with another one");
		result=new_real(strcmp(stringof(hd),stringof(hd1))<0,"");
		moveresult(st,result);
	} else if (hd1->type!=s_string) {
		result=map2(rless,0,hd,hd1);
		if (!error) moveresult(st,result);
	}
}

static void rgreatereq (double *x, double *y, double *z)
{	if (*x>=*y) *z=1.0;
	else *z=0.0;
}

void mgreatereq (header *hd)
{
	header *st=hd,*hd1=nextof(hd),*result;
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string) {
		if (hd1->type!=s_string) wrong_arg("can only compare a string with another one");
		result=new_real(strcmp(stringof(hd),stringof(hd1))>=0,"");
		moveresult(st,result);
	} else if (hd1->type!=s_string) {
		result=map2(rgreatereq,0,hd,hd1);
		if (!error) moveresult(st,result);
	}
}

static void rlesseq (double *x, double *y, double *z)
{	if (*x<=*y) *z=1.0;
	else *z=0.0;
}

void mlesseq (header *hd)
{
	header *st=hd,*hd1=nextof(hd),*result;
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string) {
		if (hd1->type!=s_string) wrong_arg("can only compare a string with another one");
		result=new_real(strcmp(stringof(hd),stringof(hd1))<=0,"");
		moveresult(st,result);
	} else if (hd1->type!=s_string) {
		result=map2(rlesseq,0,hd,hd1);
		if (!error) moveresult(st,result);
	}
}

static void requal (double *x, double *y, double *z)
{	if (*x==*y) *z=1.0;
	else *z=0.0;
}

static void cequal (double *x, double *xi, double *y, double *yi, double *z)
{	if (*x==*xi && *y==*yi) *z=1.0;
	else *z=0.0;
}

void mequal (header *hd)
{
	header *st=hd,*hd1=nextof(hd),*result;
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string) {
		if (hd1->type!=s_string) wrong_arg("can only compare a string with another one");
		result=new_real(strcmp(stringof(hd),stringof(hd1))==0,"");
		moveresult(st,result);
	} else if (hd1->type!=s_string) {
		result=map2r(requal,cequal,hd,hd1);
		if (!error) moveresult(st,result);
	}
}

static void runequal (double *x, double *y, double *z)
{	if (*x!=*y) *z=1.0;
	else *z=0.0;
}

static void cunequal (double *x, double *xi, double *y, double *yi, double *z)
{	if (*x!=*y || *xi!=*yi) *z=1.0;
	else *z=0.0;
}

void munequal (header *hd)
{
	header *st=hd,*hd1=nextof(hd),*result;
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string) {
		if (hd1->type!=s_string) wrong_arg("can only compare a string with another one");
		result=new_real(strcmp(stringof(hd),stringof(hd1))!=0,"");
		moveresult(st,result);
	} else if (hd1->type!=s_string) {
		result=map2r(runequal,cunequal,hd,hd1);
		if (!error) moveresult(st,result);
	}
}

static void raboutequal (double *x, double *y, double *z)
{	if (fabs(*x-*y)<epsilon) *z=1.0;
	else *z=0.0;
}

static void caboutequal 
	(double *x, double *xi, double *y, double *yi, double *z)
{	if (fabs(*x-*y)<epsilon && fabs(*xi-*yi)<epsilon) *z=1.0;
	else *z=0.0;
}

void maboutequal (header *hd)
{	header *st=hd,*hd1=nextof(hd),*result;
	hd=getvalue(hd); if (error) return;
	hd1=getvalue(hd1); if (error) return;
	result=map2r(raboutequal,caboutequal,hd,hd1);
	if (!error) moveresult(st,result);
}

/****************************************************************
 *	boolean operator
 ****************************************************************/
static double rnot (double x)
{	if (x!=0.0) return 0.0;
	else return 1.0;
}

static void cnot (double *x, double *xi, double *r)
{	if (*x==0.0 && *xi==0.0) *r=1.0;
	else *r=0.0;
}

void mnot (header *hd)
{	spread1r(rnot,cnot,hd);
}

static void ror (double *x, double *y, double *z)
{	if (*x!=0.0 || *y!=0.0) *z=1.0;
	else *z=0.0;
}

void mor (header *hd)
{	spread2(ror,0,hd);
}

static void rrand (double *x, double *y, double *z)
{	if (*x!=0.0 && *y!=0.0) *z=1.0;
	else *z=0.0;
}

void mand (header *hd)
{	spread2(rrand,0,hd);
}

/****************************************************************
 *	math funcs: exp, log, sqr, sin, cos, tan, asin, acos, atan,
 *              atan2, abs, mod, pow
 ****************************************************************/
static void c_sin (double *x, double *xi, double *z, double *zi)
{	*z=cosh(*xi)*sin(*x);
	*zi=sinh(*xi)*cos(*x);
}

void msin (header *hd) 
{	spread1(sin,c_sin,hd);
}

static void c_cos (double *x, double *xi, double *z, double *zi)
{	*z=cosh(*xi)*cos(*x);
	*zi=-sinh(*xi)*sin(*x);
}

void mcos (header *hd)
{	spread1(cos,c_cos,hd);
}

static void c_tan (double *x, double *xi, double *z, double *zi)
{	double s,si,c,ci;
	c_sin(x,xi,&s,&si); c_cos(x,xi,&c,&ci);
	complex_divide(&s,&si,&c,&ci,z,zi);
}

#ifdef FLOAT_TEST
static double rtan (double x)
{	if (cos(x)==0.0) return 1e10;
	return tan(x);
}
#endif

void mtan (header *hd)
{	spread1(
#ifdef FLOAT_TEST
	rtan,
#else
	tan,
#endif
	c_tan,hd);
}

static void c_arg (double *x, double *xi, double *z)
{	
#ifdef FLOAT_TEST
	if (*x==0.0 && *xi==0.0) *z=0.0;
#endif
	*z = atan2(*xi,*x);
}

#ifdef FLOAT_TEST
static double rlog (double x)
{	if (x<=0) { error=1; return 0; }
	else return log(x);
}
#endif

void cclog (double *x, double *xi, double *z, double *zi)
{	
#ifdef FLOAT_TEST
	*z=rlog(sqrt(*x * *x + *xi * *xi));
#else
	*z=log(sqrt(*x * *x + *xi * *xi));
#endif
	c_arg(x,xi,zi);
}

#ifdef FLOAT_TEST
static double ratan (double x)
{	if (x<=-M_PI && x>=M_PI) return 1e10;
	else return atan(x);
}
#endif

static void c_atan (double *x, double *xi, double *y, double *yi)
{	double h,hi,g,gi,t,ti;
	h=1-*xi; hi=*x; g=1+*xi; gi=-*x;
	complex_divide(&h,&hi,&g,&gi,&t,&ti);
	cclog(&t,&ti,&h,&hi);
	*y=hi/2; *yi=-h/2;
}

void matan (header *hd)
{	spread1(
#ifdef FLOAT_TEST
	ratan,
#else
	atan,
#endif
	c_atan,hd);
}

#ifdef FLOAT_TEST
static double rasin (double x)
{	if (x<-1 || x>1) { error=1; return 0; }
	else return asin(x);
}
#endif

static void c_sqrt (double *x, double *xi, double *z, double *zi)
{	double a,r;
	c_arg(x,xi,&a); a=a/2.0;
	r=sqrt(sqrt(*x * *x + *xi * *xi));
	*z=r*cos(a);
	*zi=r*sin(a);
}

static void c_asin (double *x, double *xi, double *y, double *yi)
{	double h,hi,g,gi;
	complex_multiply(x,xi,x,xi,&h,&hi);
	h=1-h; hi=-hi;
	c_sqrt(&h,&hi,&g,&gi);
	h=-*xi+g; hi=*x+gi;
	cclog(&h,&hi,yi,y);
	*yi=-*yi;
}

void masin (header *hd)
{	spread1(
#ifdef FLOAT_TEST
	rasin,
#else
	asin,
#endif
	c_asin,hd);
}

#ifdef FLOAT_TEST
static double racos (double x)
{	if (x<-1 || x>1) { error=1; return 0; }
	else return acos(x);
}
#endif

static void c_acos (double *x, double *xi, double *y, double *yi)
{	double h,hi,g,gi;
	complex_multiply(x,xi,x,xi,&h,&hi);
	h=1-h; hi=-hi;
	c_sqrt(&h,&hi,&g,&gi);
	hi=*xi+g; h=*x-gi;
	cclog(&h,&hi,yi,y);
	*yi=-*yi;
}

void macos (header *hd)
{	spread1(
#ifdef FLOAT_TEST
	racos,
#else
	acos,
#endif
	c_acos,hd);
}

static void c_exp (double *x, double *xi, double *z, double *zi)
{	double r=exp(*x);
	*z=cos(*xi)*r;
	*zi=sin(*xi)*r;
}

void mexp (header *hd)
{	spread1(exp,c_exp,hd);
}

void mlog (header *hd)
{	spread1(log,cclog,hd);
}

#ifdef FLOAT_TEST
static double rsqrt (double x)
{	if (x<0.0) { error=1; return 1e10; }
	else return sqrt(x);
}
#endif

void msqrt (header *hd)
{	spread1(
#ifdef FLOAT_TEST
	rsqrt,
#else
	sqrt,
#endif
	c_sqrt,hd);
}

static void c_abs (double *x, double *xi, double *z)
{	*z=sqrt(*x * *x + *xi * *xi);
}

void mabs (header *hd)
{	spread1r(fabs,c_abs,hd);
}

static void rmod (double *x, double *n, double *y)
{	*y=fmod(*x,*n);
}

void mmod (header *hd)
{	spread2(rmod,0,hd);
}

static void c_pow (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	double l,li,w,wi;
	if (fabs(*x)<epsilon && fabs(*xi)<epsilon)
	{	*z=*zi=0.0; return;
	}
	cclog(x,xi,&l,&li);
	complex_multiply(y,yi,&l,&li,&w,&wi);
	c_exp(&w,&wi,z,zi);
}

static void rpow (double *x, double *y, double *z)
{	int n;
	if (*x>0.0) *z=pow(*x,*y);
	else if (*x==0.0) if (*y==0.0) *z=1.0; else *z=0.0;
	else
	{	n=(int)*y;
		if (n%2) *z=-pow(-*x,n);
		else *z=pow(-*x,n);
	}
}

void mpower (header *hd)
{	spread2(rpow,c_pow,hd);
}

/****************************************************************
 *	sign, ceil, floor, round functions
 ****************************************************************/
static double rsign (double x)
{	if (x<0) return -1;
	else if (x<=0) return 0;
	else return 1;
}

void msign (header *hd)
{	spread1(rsign,0,hd);
}

void mceil (header *hd)
{	spread1(ceil,0,hd);
}

void mfloor (header *hd)
{	spread1(floor,0,hd);
}

static double rounder;

static double rround (double x)
{	x*=rounder;
	if (x>0) x=floor(x+0.5);
	else x=-floor(-x+0.5);
	return x/rounder;
}

static void cround (double *x, double *xi, double *z, double *zi)
{	*z=rround(*x);
	*zi=rround(*xi);
}

static double frounder[]={1.0,10.0,100.0,1000.0,10000.0,100000.0,1000000.0,
10000000.0,100000000.0,1000000000.0,10000000000.0};

void mround (header *hd)
{	header *hd1;
	int n;
	hd1=next_param(hd);
	if (hd1) {hd1=getvalue(hd1); if (error) return;}
	if (hd1->type!=s_real) wrong_arg("2nd arg: real value expected");
	n=(int)(*realof(hd1));
	if (n>0 && n<11) rounder=frounder[n];
	else rounder=pow(10.0,n);
	spread1(rround,cround,hd);
}

/****************************************************************
 *	complex specific functions
 ****************************************************************/
void mcomplex (header *hd)
{	header *st=hd,*result;
	double *m,*mr;
	ULONG i,n;
	int c,r;
	hd=getvalue(hd);
	if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		n=(ULONG)r*c;
        mr=matrixof(result)+(ULONG)2*(n-1);
		m+=n-1;
		for (i=0; i<n; i++)
		{	*mr=*m--; *(mr+1)=0.0; mr-=2;
		}
		moveresult(st,result);
	}
	else if (hd->type==s_real)
	{	result=new_complex(*realof(hd),0.0,""); if (error) return;
		moveresult(st,result);
	}
}

static void cconj (double *x, double *xi, double *z, double *zi)
{	*zi=-*xi; *z=*x;
}

static double ident (double x)
{	return x;
}

void mconj (header *hd)
{	spread1(ident,cconj,hd);
}

static void crealpart (double *x, double *xi, double *z)
{	*z=*x;
}

void mre (header *hd)
{	spread1r(ident,crealpart,hd);
}

static double zero (double x)
{	return 0.0;
}

static void cimagpart (double *x, double *xi, double *z)
{	*z=*xi;
}

void mim (header *hd)
{	spread1r(zero,cimagpart,hd);
}

static double rarg (double x)
{	if (x>=0) return 0.0;
	else return M_PI;
}

void marg (header *hd)
{	spread1r(rarg,c_arg,hd);
}

/****************************************************************
 *	matrix ops
 ****************************************************************/
/************************* object handling **********************/
void msum (header *hd)
{	header *st=hd,*result;
	int c,r,i,j;
	double *m,*mr,s,si;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	s=0.0;
			for (j=0; j<c; j++) s+=*m++;
			*mr++=s;
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	s=0.0; si=0.0;
			for (j=0; j<c; j++) { s+=*m++; si+=*m++; }
			*mr++=s; *mr++=si;
		}
	}
	else wrong_arg("real or complex value or matrix expected");
	moveresult(st,result);
}

void mprod (header *hd)
{	header *st=hd,*result;
	int c,r,i,j;
	double *m,*mr,s,si,h,hi;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	s=1.0;
			for (j=0; j<c; j++) s*=*m++;
			*mr++=s;
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	s=1.0; si=0.0;
			for (j=0; j<c; j++) 
			{	complex_multiply(&s,&si,m,m+1,&h,&hi);
				s=h; si=hi; m+=2; 
			}
			*mr++=s; *mr++=si;
		}
	}
	else wrong_arg("real or complex value or matrix expected");
	moveresult(st,result);
}

void msize (header *hd)
{	header *result,*st=hd,*hd1=hd,*end=(header *)newram;
	int r,c,r0=0,c0=0;
//	if (!hd) wrong_arg();
	result=new_matrix(1,2,""); if (error) return;
	while (end>hd)
	{	hd1=getvariable(hd); if (error) return;
		if (hd1->type==s_matrix || hd1->type==s_cmatrix)
		{	r=dimsof(hd1)->r;
			c=dimsof(hd1)->c;
		}
		else if (hd1->type==s_real || hd1->type==s_complex)
		{	r=c=1;
		}
		else if (hd1->type==s_submatrix || hd1->type==s_csubmatrix)
		{	r=submdimsof(hd1)->r;
			c=submdimsof(hd1)->c;
		}
		else wrong_arg("bad type");
		if ((r>1 && r0>1 && r!=r0) || (c>1 && c0>1 && c!=c0))
		{	if (r0!=r && c0!=c)
			{	output("Matrix dimensions must agree!\n");
				error=1021; return;
			}
		}
		else
		{	if (r>r0) r0=r;
			if (c>c0) c0=c;
		}
        hd=nextof(hd);
	}
	*matrixof(result)=r0;
	*(matrixof(result)+1)=c0;
	moveresult(st,result);
}

void mcols (header *hd)
{	header *st=hd,*res;
	int n;
	hd=getvalue(hd); if (error) return;
	switch (hd->type)
	{	case s_matrix :
		case s_cmatrix : n=dimsof(hd)->c; break;
		case s_submatrix :
		case s_csubmatrix : n=submdimsof(hd)->c; break;
		case s_real :
		case s_complex : n=1; break;
		case s_string : n=(int)strlen(stringof(hd)); break;
		default : wrong_arg("bad type");
	}
	res=new_real(n,""); if (error) return;
	moveresult(st,res);
}

void mrows (header *hd)
{	header *st=hd,*res;
	int n;
	hd=getvalue(hd); if (error) return;
	switch (hd->type)
	{	case s_matrix :
		case s_cmatrix : n=dimsof(hd)->r; break;
		case s_submatrix :
		case s_csubmatrix : n=submdimsof(hd)->r; break;
		case s_real :
		case s_complex : n=1; break;
		default : wrong_arg("bad type");
	}
	res=new_real(n,""); if (error) return;
	moveresult(st,res);
}

void mmatrix (header *hd)
{	header *st=hd,*hd1,*result;
	long i,n;
	double x,xi;
	double *m,*mr;
	int c,r,c1,r1;
	hd1=nextof(hd);
	hd=getvalue(hd);
	if (error) return;
	hd1=getvalue(hd1);
	if (error) return;
	if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		if (*m<0 || *m>INT_MAX || *(m+1)<0 || *(m+1)>INT_MAX)
			wrong_arg("bad matrix size required");
		r1=(int)*m; c1=(int)*(m+1);
		if (hd1->type==s_real)
		{	result=new_matrix(r1,c1,"");
			mr=matrixof(result);
			x=*realof(hd1);
			n=(long)c1*r1;
			for (i=0; i<n; i++) *mr++=x;
		}
		else if (hd1->type==s_complex)
		{	result=new_cmatrix(r1,c1,"");
			mr=matrixof(result);
			x=*realof(hd1); xi=*(realof(hd1)+1);
			n=(long)c1*r1;
			for (i=0; i<n; i++) 
			{	*mr++=x; *mr++=xi;
			}
		}
		else wrong_arg("2nd arg: real or complex value required");
	}
	else wrong_arg("1st arg: 1x2 real vector required");
	moveresult(st,result);
}

void mzerosmat (header *hd)
{	header *result,*st=hd;
	double rows,cols,*m;
	int r,c;
	ULONG i,n;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
		wrong_arg("1x2 real matrix expected");
	rows=*matrixof(hd); cols=*(matrixof(hd)+1);
	if (rows<0 || rows>=INT_MAX || cols<0 || cols>=INT_MAX)
		wrong_arg("bad size required");
	r=(int)rows; c=(int)cols;
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=c*r;
	for (i=0; i<n; i++) *m++=0.0;
	moveresult(st,result);
}

void mones (header *hd)
{	header *result,*st=hd;
	double rows,cols,*m;
	int r,c;
	ULONG i,n;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
		wrong_arg("1x2 real matrix expected");
	rows=*matrixof(hd); cols=*(matrixof(hd)+1);
	if (rows<0 || rows>=INT_MAX || cols<0 || cols>=INT_MAX)
		wrong_arg("bad size required");
	r=(int)rows; c=(int)cols;
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=c*r;
	for (i=0; i<n; i++) *m++=1.0;
	moveresult(st,result);
}

void mdiag (header *hd)
{	header *result,*st=hd,*hd1,*hd2=0;
	double rows,cols,*m,*md;
	int r,c,i,ik=0,k,rd,cd;
	ULONG l,n;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2)
		wrong_arg("1st arg: 1x2 real matrix expected");
	rows=*matrixof(hd); cols=*(matrixof(hd)+1);
	if (rows<0 || rows>=INT_MAX || cols<0 || cols>=INT_MAX)
		wrong_arg("bad size required");
	r=(int)rows; c=(int)cols;
	hd1=next_param(st); if (hd1) hd2=next_param(hd1);
	if (hd1) hd1=getvalue(hd1);
	if (hd2) hd2=getvalue(hd2);
	if (error) return;
	if	(hd1->type!=s_real) wrong_arg("2nd arg: real value expected");
	k=(int)*realof(hd1);
	if (hd2->type==s_matrix || hd2->type==s_real)
	{	result=new_matrix(r,c,""); if (error) return;
		m=matrixof(result);
		n=(ULONG)c*r;
		for (l=0; l<n; l++) *m++=0.0;
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1) wrong_arg("3rd arg: row vector expected");
		m=matrixof(result);
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c) 
			{	*mat(m,c,i,i+k)=*md;
				ik++; if (ik<cd) md++;
			}
		}
	}
	else if (hd2->type==s_cmatrix || hd2->type==s_complex)
	{	result=new_cmatrix(r,c,""); if (error) return;
		m=matrixof(result);
        n=(ULONG)2*(ULONG)c*r;
		for (l=0; l<n; l++) *m++=0.0;
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1) wrong_arg("3rd arg: row vector expected");
		m=matrixof(result);
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c) 
			{	*cmat(m,c,i,i+k)=*md;
				*(cmat(m,c,i,i+k)+1)=*(md+1);
				ik++; if (ik<cd) md+=2;
			}
		}
	}
	else wrong_arg("3rd arg: bad type");
	moveresult(st,result);
}

void msetdiag (header *hd)
{	header *result,*st=hd,*hd1,*hd2=0;
	double *m,*md,*mhd;
	int r,c,i,ik=0,k,rd,cd;
	hd=getvalue(st); if (error) return;
	if (hd->type!=s_matrix && hd->type!=s_cmatrix)
		wrong_arg("1st arg: matrix expected");
	getmatrix(hd,&c,&r,&mhd);
	hd1=next_param(st); if (hd1) hd2=next_param(hd1);
	if (hd1) hd1=getvalue(hd1);
	if (hd2) hd2=getvalue(hd2);
	if (error) return;
	if	(hd1->type!=s_real) wrong_arg("2nd arg: real value expected");
	k=(int)*realof(hd1);
	if (hd->type==s_matrix && 
			(hd2->type==s_complex || hd2->type==s_cmatrix))
		{	make_complex(st); msetdiag(st); return;
		}
	else if (hd->type==s_cmatrix &&
			(hd2->type==s_real || hd2->type==s_matrix))
		{	make_complex(nextof(nextof(st))); msetdiag(st); return;
		}
	if (hd->type==s_matrix)
	{	result=new_matrix(r,c,""); if (error) return;
		m=matrixof(result);
		memmove((char *)m,(char *)mhd,(ULONG)c*r*sizeof(double));
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1) wrong_arg("3rd arg: row vector expected");
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c) 
			{	*mat(m,c,i,i+k)=*md;
				ik++; if (ik<cd) md++;
			}
		}
	}
	else if (hd->type==s_cmatrix)
	{	result=new_cmatrix(r,c,""); if (error) return;
		m=matrixof(result);
        memmove((char *)m,(char *)mhd,(ULONG)c*r*(ULONG)2*sizeof(double));
		getmatrix(hd2,&rd,&cd,&md);
		if (rd!=1 || cd<1) wrong_arg("3rd arg: row vector expected");
		m=matrixof(result);
		for (i=0; i<r; i++)
		{	if (i+k>=0 && i+k<c) 
			{	*cmat(m,c,i,i+k)=*md;
				*(cmat(m,c,i,i+k)+1)=*(md+1);
				ik++; if (ik<cd) md+=2;
			}
		}
	}
	else wrong_arg("3rd arg: bad type");
	moveresult(st,result);
}

void mnonzeros (header *hd)
{	header *st=hd,*result;
	double *m,*mr;
	int r,c,i,k;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix) wrong_arg("real value or matrix expected");
	getmatrix(hd,&r,&c,&m);
	if (r!=1 && c!=1) wrong_arg("row or colum vector expected");
	if (c==1) c=r;
	result=new_matrix(1,c,""); if (error) return;
	k=0; mr=matrixof(result);
	for (i=0; i<c; i++)
	{	if (*m++!=0.0)
		{	*mr++=i+1; k++;
		}
	}
	dimsof(result)->c=k;
	result->size=matrixsize(1,k);
	moveresult(st,result);
}

void many (header *hd)
{	header *st=hd,*result;
	int c,r,res=0;
	ULONG i,n;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		n=(ULONG)(c)*r;
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
        n=(ULONG)2*(ULONG)(c)*r;
	}
	else wrong_arg("bad type");
	for (i=0; i<n; i++)
		if (*m++!=0.0) { res=1; break; }
	result=new_real(res,""); if (error) return;
	moveresult(st,result);
}

void mextrema (header *hd)
{	header *result,*st=hd;
	double x,*m,*mr,min,max;
	int r,c,i,j,imin,imax;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,4,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	min=max=*m; imin=imax=0; m++;
			for (j=1; j<c; j++) 
			{	x=*m++;
				if (x<min) { min=x; imin=j; }
				if (x>max) { max=x; imax=j; }
			}
			*mr++=min; *mr++=imin+1; *mr++=max; *mr++=imax+1;
		}
	}
	else wrong_arg("real value or matrix expected");
	moveresult(st,result);
}

void mcumsum (header *hd)
{	header *result,*st=hd;
	double *m,*mr,sum=0,sumr=0,sumi=0;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_matrix(r,1,"");
		else result=new_matrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	if (c>=1) sum=*m++;
			*mr++=sum;
			for (j=1; j<c; j++) 
			{	sum+=*m++;
				*mr++=sum;
			}
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_cmatrix(r,1,"");
		else result=new_cmatrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	if (c>=1) { sumr=*m++; sumi=*m++; }
			*mr++=sumr; *mr++=sumi;
			for (j=1; j<c; j++) 
			{	sumr+=*m++; *mr++=sumr;
				sumi+=*m++; *mr++=sumi;
			}
		}
	}
	else wrong_arg("bad type");
	moveresult(st,result);
}

void mcumprod (header *hd)
{	header *result,*st=hd;
	double *m,*mr,sum=1,sumi=1,sumr=0;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_matrix(r,1,"");
		else result=new_matrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	if (c>=1) sum=*m++; 
			*mr++=sum;
			for (j=1; j<c; j++) 
			{	sum*=*m++;
				*mr++=sum;
			}
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		if (c<1) result=new_cmatrix(r,1,"");
		else result=new_cmatrix(r,c,"");
		if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	if (c>=1) { sumr=*m++; sumi=*m++; }
			*mr++=sumr; *mr++=sumi;
			for (j=1; j<c; j++)
			{	sum=sumr*(*m)-sumi*(*(m+1));
				sumi=sumr*(*(m+1))+sumi*(*m);
				sumr=sum;
				m+=2;
				*mr++=sumr;
				*mr++=sumi;
			}
		}
	}
	else wrong_arg("bad type");
	moveresult(st,result);
}

void mflipx (header *hd)
{	header *st=hd,*result;
	double *m,*mr,*mr1;
	int i,j,c,r;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_complex)
	{	moveresult(st,hd); return;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=mr+(c-1);
			for (j=0; j<c; j++) *mr1--=*m++;
			mr+=c;
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
		{	mr1=mr+(2l*(c-1)+1);
			for (j=0; j<c; j++)
			{	*mr1--=*m++; *mr1--=*m++;
			}
			mr+=2l*c;
		}
	}
	else wrong_arg("bad type");
	moveresult(st,result);
}

void mflipy (header *hd)
{	header *st=hd,*result;
	double *m,*mr;
	int i,c,r;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_complex)
	{	moveresult(st,hd); return;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		mr+=(long)(r-1)*c;
		for (i=0; i<r; i++)
		{	memmove((char *)mr,(char *)m,c*sizeof(double));
			m+=c; mr-=c;
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		mr+=2l*(long)(r-1)*c;
		for (i=0; i<r; i++)
		{	memmove((char *)mr,(char *)m,2l*c*sizeof(double));
			m+=2l*c; mr-=2l*c;
		}
	}
	else wrong_arg("bad type");
	moveresult(st,result);
}

void mlu (header *hd)
{	header *st=hd,*result,*res1,*res2,*res3;
	double *m,*mr,*m1,*m2,det,deti;
	int r,c,*rows,*cols,rank,i;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		if (r<1) wrong_arg("not a 0-sized matrix expected");
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		memmove((char *)mr,(char *)m,(ULONG)r*c*sizeof(double));
		make_lu(mr,r,c,&rows,&cols,&rank,&det); if (error) return;
		res1=new_matrix(1,rank,""); if (error) return;
		res2=new_matrix(1,c,""); if (error) return;
		res3=new_real(det,""); if (error) return;
		m1=matrixof(res1);
		for (i=0; i<rank; i++)
		{	*m1++=*rows+1;
			rows++;
		}
		m2=matrixof(res2);
		for (i=0; i<c; i++)
		{	*m2++=*cols++;
		}
		moveresult(st,getvalue(result)); st=nextof(st);
		moveresult(st,getvalue(res1)); st=nextof(st);
		moveresult(st,getvalue(res2)); st=nextof(st);
		moveresult(st,getvalue(res3));
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		if (r<1) wrong_arg("not a 0-sized matrix expected");
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
        memmove((char *)mr,(char *)m,(ULONG)r*c*(ULONG)2*sizeof(double));
		cmake_lu(mr,r,c,&rows,&cols,&rank,&det,&deti); 
			if (error) return;
		res1=new_matrix(1,rank,""); if (error) return;
		res2=new_matrix(1,c,""); if (error) return;
		res3=new_complex(det,deti,""); if (error) return;
		m1=matrixof(res1);
		for (i=0; i<rank; i++)
		{	*m1++=*rows+1;
			rows++;
		}
		m2=matrixof(res2);
		for (i=0; i<c; i++)
		{	*m2++=*cols++;
		}
		moveresult(st,getvalue(result)); st=nextof(st);
		moveresult(st,getvalue(res1)); st=nextof(st);
		moveresult(st,getvalue(res2)); st=nextof(st);
		moveresult(st,getvalue(res3));
	}
	else wrong_arg("bad type");
}

void mlusolve (header *hd)
{	header *st=hd,*hd1,*result;
	double *m,*m1;
	int r,c,r1,c1;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (hd1) hd1=getvalue(hd1);
	if (error) return;
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		if (hd1->type==s_cmatrix)
		{	make_complex(st);
			mlusolve(st); return;	
		}
		if (hd1->type!=s_matrix && hd1->type!=s_real) wrong_arg("real value or matrix expected");
		getmatrix(hd1,&r1,&c1,&m1);
		if (c!=r || c<1 || r!=r1) wrong_arg("bad size");
		result=new_matrix(r,c1,""); if (error) return;
		lu_solve(m,r,m1,c1,matrixof(result));
		if (error) return;
		moveresult(st,result);
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		if (hd1->type==s_matrix || hd1->type==s_real)
		{	make_complex(next_param(st));
			mlusolve(st); return;
		}
		if (hd1->type!=s_cmatrix && hd1->type!=s_complex) wrong_arg("complex value or matrix expected");
		getmatrix(hd1,&r1,&c1,&m1);
		if (c!=r || c<1 || r!=r1) wrong_arg("bad size");
		result=new_cmatrix(r,c1,""); if (error) return;
		clu_solve(m,r,m1,c1,matrixof(result));
		if (error) return;
		moveresult(st,result);
	}
	else wrong_arg("real or complex value or matrix expected");
}

void msolve (header *hd)
{	header *st=hd,*hd1,*result;
	double *m,*m1;
	int r,c,r1,c1;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (hd1) hd1=getvalue(hd1);
	if (error) return;
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		if (hd1->type==s_cmatrix)
		{	make_complex(st);
			msolve(st); return;	
		}
		if (hd1->type!=s_matrix && hd1->type!=s_real) wrong_arg("real value or matrix expected");
		getmatrix(hd1,&r1,&c1,&m1);
		if (c!=r || c<1 || r!=r1) wrong_arg("bad size");
		result=new_matrix(r,c1,""); if (error) return;
		solvesim(m,r,m1,c1,matrixof(result));
		if (error) return;
		moveresult(st,result);
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		if (hd1->type==s_matrix || hd1->type==s_real)
		{	make_complex(next_param(st));
			msolve(st); return;
		}
		if (hd1->type!=s_cmatrix && hd1->type!=s_complex) wrong_arg("complex value or matrix expected");
		getmatrix(hd1,&r1,&c1,&m1);
		if (c!=r || c<1 || r!=r1) wrong_arg("bad size");
		result=new_cmatrix(r,c1,""); if (error) return;
		c_solvesim(m,r,m1,c1,matrixof(result));
		if (error) return;
		moveresult(st,result);
	}
	else wrong_arg("real or complex value or matrix expected");
}

void mrandom (header *hd)
{	header *st=hd,*result;
	double *m;
	int r,c;
	ULONG k,n;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2
		|| *(m=matrixof(hd))<0 || *m>=INT_MAX 
		|| *(m+1)<0 || *(m+1)>INT_MAX)
		wrong_arg("1x2 real vector expected or bad values");
	r=(int)*m;
	c=(int)*(m+1);
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=(ULONG)c*r;
	for (k=0; k<n; k++) *m++=(double)rand()/(double)RAND_MAX;
	moveresult(st,result);
}

void mnormal (header *hd)
{	header *st=hd,*result;
	double *m,r1,r2;
	int r,c;
	ULONG k,n;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_matrix || dimsof(hd)->r!=1 || dimsof(hd)->c!=2
		|| *(m=matrixof(hd))<0 || *m>=INT_MAX 
		|| *(m+1)<0 || *(m+1)>INT_MAX)
		wrong_arg("1x2 real vector expected or bad values");
	r=(int)*m;
	c=(int)*(m+1);
	result=new_matrix(r,c,""); if (error) return;
	m=matrixof(result);
	n=(ULONG)c*r;
	for (k=0; k<n; k++) 
	{	r1=(double)rand()/(double)RAND_MAX;
		if (r1==0.0) *m++=0.0;
		else
		{	r2=(double)rand()/(double)RAND_MAX;
			*m++=sqrt(-2*log(r1))*cos(2*M_PI*r2);
		}
	}
	moveresult(st,result);
}

static double gauss (double z)
{	double x,w;
	x=1/(0.2316419*fabs(z)+1);
	w=x*(0.31938153+x*(-0.356556382+x*(1.781477937+x*(
		-1.821255978+x*1.330274429))));
	w=w*exp(-z*z/2)/sqrt(2*M_PI);
	if (z<0) return w;
	else return 1-w;
}

void mgauss (header *hd)
{	spread1(gauss,0,hd);
}

static double invgauss (double a)
{	double t,c,d;
	int flag=0;
	if (a<0.5) { a=1-a; flag=1; }
	t=sqrt(-2*log(fabs(1-a)));
	c=2.515517+t*(0.802853+t*0.010328);
	d=1+t*(1.432788+t*(0.189269+t*0.001308));
	if (flag) return (c/d-t);
	else return t-c/d;
}

void minvgauss (header *hd)
{	spread1(invgauss,0,hd);
}

static double rfak (double x)
{	int i,n;
	double res=1;
	if (x<2 || x>INT_MAX) return 1.0;
	n=(int)x;
	for (i=2; i<=n; i++) res=res*i;
	return res;
}

void mfak (header *hd)
{	spread1(rfak,0,hd);
}

static void rbin (double *x, double *y, double *z)
{   long i,n,m,k;
	double res;
	n=(long)*x; m=(long)*y;
	if (m<=0) *z=1.0;
	else
	{	res=k=(n-m+1);
		for (i=2; i<=m; i++) { k++; res=(res*k)/i; }
		*z=res;
	}
}

void mbin (header *hd)
{	spread2(rbin,0,hd);
}

static void rtd (double *xa, double *yf, double *zres)
{	double t,t1,a,b,h,z,p,y,x;
	int flag=0;
	if (fabs(*xa)<epsilon) { *zres=0.5; return; }
	if (*xa<0) flag=1;
	t=*xa * *xa;
	if (t>=1) { a=1; b=*yf; t1=t; }
	else { a=*yf; b=1; t1=1/t; }
	y=2/(9*a); z=2/(9*b);
	h=pow(t1,1.0/3);
	x=fabs((1-z)*h-1+y)/sqrt(z*h*h+y);
	if (b<4) x=x*(1+0.08*x*x*x*x/(b*b*b));
	h=1+x*(0.196854+x*(0.115194+x*(0.000344+x*0.019527)));
	p=0.5/(h*h*h*h);
	if (t<0.5) *zres=p/2+0.5;
	else *zres=1-p/2;
	if (flag) *zres=1-*zres;
}

void mtd (header *hd)
{	spread2(rtd,0,hd);
}

static void invrtd (double *x, double *y, double *zres)
{	double z=*x,f=*y,g1,g2,g3,g4,z2;
	int flag=0;
	if (z<0.5) { flag=1; z=1-z; }
	z=invgauss(z);
	z2=z*z;
	g1=z*(1+z2)/4.0;
	g2=z*(3+z2*(16+5*z2))/96.0;
	g3=z*(-15+z2*(17+z2*(19+z2*3)))/384.0;
	g4=z*(-945+z2*(-1920+z2*(1482+z2*(776+z2*79))))/92160.0;
	*zres=(((g4/f+g3)/f+g2)/f+g1)/f+z;
	if (flag) *zres=-*zres;
}

void minvtd (header *hd)
{	spread2(invrtd,0,hd);
}

static void chi (double *xa, double *yf, double *zres)
{	double ch=*xa,x,y,s,t,g,j=1;
	long i=1,p,f;
	f=(long)*yf;
	if (ch<epsilon) { *zres=0.0; return; }
	p=(f+1)/2;
	for (i=f; i>=2; i-=2) j=j*i;
	x=pow(ch,p)*exp(-(ch/2))/j;
	if (f%2==0) y=1;
	else y=sqrt(2/(ch*M_PI));
	s=1; t=1; g=f;
	while (t>1e-9)
	{	g=g+2;
		t=t*ch/g;
		s=s+t;
	}
	*zres=x*y*s;
}

void mchi (header *hd)
{	spread2(chi,0,hd);
}

static double f1,f2;

static double rfd (double F)
{	double f0,a,b,h,z,p,y,x;
	if (F<epsilon) return 0.0;
	if (F<1) { a=f2; b=f1; f0=1/F; }
	else { a=f1; b=f2; f0=F; }
	y=2/(9*a); z=2/(9*b);
	h=pow(f0,1.0/3);
	x=fabs((1-z)*h-1+y)/sqrt(z*h*h+y);
	if (b<4) x=x*(1+0.08*x*x*x*x/(b*b*b));
	h=1+x*(0.196854+x*(0.115194+x*(0.000344+x*0.019527)));
	p=0.5/(h*h*h*h);
	if (F>=1) return 1-p;
	else return p;
}

void mfdis (header *hd)
{	header *st=hd,*hd1,*hd2=0;
	hd1=next_param(st);
	if (hd1)
	{	hd2=next_param(hd1);
		hd1=getvalue(hd1);
	}
	if (hd2) hd2=getvalue(hd2);
	if (error) return;
	if (hd1->type!=s_real || hd2->type!=s_real) wrong_arg("real values expected");
	f1=*realof(hd1);
	f2=*realof(hd2);
	spread1(rfd,0,hd);
}

static void rmax (double *x, double *y, double *z)
{	if (*x>*y) *z=*x;
	else *z=*y;
}

void mmax (header *hd)
{	spread2(rmax,0,hd);
}

static void rmin (double *x, double *y, double *z)
{	if (*x>*y) *z=*y;
	else *z=*x;
}

void mmin (header *hd)
{	spread2(rmin,0,hd);
}

typedef struct { double val; int ind; } sorttyp;

static int sorttyp_compare (const sorttyp *x, const sorttyp *y)
{	if (x->val>y->val) return 1;
	else if (x->val==y->val) return 0;
	else return -1;
}

void msort (header *hd)
{	header *st=hd,*result,*result1;
	double *m,*m1;
	sorttyp *t;
	int r,c,i;
	hd=getvalue(hd); if (error) return;
	if (hd->type!=s_real && hd->type!=s_matrix) wrong_arg("real value or matrix expected");
	getmatrix(hd,&r,&c,&m);
	if (c==1 || r==1) result=new_matrix(r,c,"");
	else wrong_arg("row or colum vector expected");
	if (error) return;
	result1=new_matrix(r,c,"");
	if (error) return;
	if (c==1) c=r;
	if (c==0) wrong_arg("can't sort a 0-sized vector");
	if (newram+c*sizeof(sorttyp)>ramend)
	{	output("Out of memory!\n"); error=600; return; 
	}
	t=(sorttyp *)newram;
	for (i=0; i<c; i++)
	{	t->val=*m++; t->ind=i; t++;
	}
	qsort(newram,c,sizeof(sorttyp),
		(int (*) (const void *, const void *))sorttyp_compare);
	m=matrixof(result); m1=matrixof(result1);
	t=(sorttyp *)newram;
	for (i=0; i<c; i++)
	{	*m++=t->val; *m1++=t->ind+1; t++;
	}
	moveresult(st,result);
	moveresult(nextof(st),result1);
}

void mstatistics (header *hd)
{	header *st=hd,*hd1,*result;
	int i,n,r,c,k;
	double *m,*mr;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (hd1) {hd1=getvalue(hd1); if (error) return;}
	if (hd1->type!=s_real || hd->type!=s_matrix) wrong_arg("1st arg: real value or matrix expected");
	if (*realof(hd1)>INT_MAX || *realof(hd1)<2) wrong_arg("2nd arg >= 2");
	n=(int)*realof(hd1);
	getmatrix(hd,&r,&c,&m);
	if (r!=1 && c!=1) wrong_arg("1st arg: real row or column vector expected");
	if (c==1) c=r;
	result=new_matrix(1,n,""); if (error) return;
	mr=matrixof(result); for (i=0; i<n; i++) *mr++=0.0;
	mr=matrixof(result);
	for (i=0; i<c; i++)
	{	if (*m>=0 && *m<n)
		{	k=floor(*m);
			mr[k]+=1.0;
		}
		m++;
	}
	moveresult(st,result);
}

void mmax1 (header *hd)
{	header *result,*st=hd;
	double x,*m,*mr,max;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	max=*m; m++;
			for (j=1; j<c; j++) 
			{	x=*m++;
				if (x>max) max=x;
			}
			*mr++=max;
		}
	}
	else wrong_arg("real value or matrix expected");
	moveresult(st,result);
}

void mmin1 (header *hd)
{	header *result,*st=hd;
	double x,*m,*mr,max;
	int r,c,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,1,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++) 
		{	max=*m; m++;
			for (j=1; j<c; j++) 
			{	x=*m++;
				if (x<max) max=x;
			}
			*mr++=max;
		}
	}
	else wrong_arg("real value or matrix expected");
	moveresult(st,result);
}

void minmax (double *x, ULONG n, double *min, double *max, 
	int *imin, int *imax)
/***** minmax
	compute the total minimum and maximum of n double numbers.
*****/
{	ULONG i;
	if (n==0)
	{	*min=0; *max=0; *imin=0; *imax=0; return; }
	*min=*x; *max=*x; *imin=0; *imax=0; x++;
	for (i=1; i<n; i++)
	{	if (*x<*min) { *min=*x; *imin=(int)i; }
		else if (*x>*max) { *max=*x; *imax=(int)i; }
		x++;
	}
}

void transpose (header *hd)
/***** transpose 
	transpose a matrix
*****/
{	header *hd1,*st=hd;
	double *m,*m1,*mh;
	int c,r,i,j;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		hd1=new_matrix(c,r,""); if (error) return;
		m1=matrixof(hd1);
		for (i=0; i<r; i++)
		{	mh=m1+i;
			for (j=0; j<c; j++)
			{	*mh=*m++; mh+=r;
			}
		}
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m);
		hd1=new_cmatrix(c,r,""); if (error) return;
		m1=matrixof(hd1);
		for (i=0; i<r; i++)
        {   mh=m1+(ULONG)2*i;
			for (j=0; j<c; j++)
			{	*mh=*m++; *(mh+1)=*m++;
                mh+=(ULONG)2*r;
			}
		}
	}
	else if (hd->type==s_real || hd->type==s_complex)
	{	hd1=hd;
	}
	else
	{	error=24; output("\' not defined for this value!\n");
		return;
	}
	moveresult(st,hd1);
}

void vectorize (header *init, header *step, header *end)
{	double vinit,vstep,vend,*m;
	int count;
	header *result,*st=init;
	init=getvalue(init); step=getvalue(step); end=getvalue(end);
	if (error) return;
	if (init->type!=s_real || step->type!=s_real || end->type!=s_real)
	{	output("The : allows only real arguments!\n");
		error=15; return;
	}
	vinit=*realof(init); vstep=*realof(step); vend=*realof(end);
	if (vstep==0)
	{	output("A step size of 0 is not allowed in : ");
		error=401; return;
	}
	if (fabs(vend-vinit)/fabs(vstep)+1+epsilon>INT_MAX)
	{	output1("A vector can only have %d elements",INT_MAX);
		error=402; return;
	}
	count=(int)(floor(fabs(vend-vinit)/fabs(vstep)+1+epsilon));
	if ((vend>vinit && vstep<0) || (vend<vinit && vstep>0))
		count=0;
	result=new_matrix(1,count,""); if (error) return;
	m=matrixof(result);
	while (count>=0)
	{	*m++=vinit;
		vinit+=vstep;
		count--;
	}
	moveresult(st,result);
}

void mfft (header *hd)
{	header *st=hd,*result;
	double *m,*mr;
	int r,c;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	make_complex(st); hd=st; }
	getmatrix(hd,&r,&c,&m);
	if (r!=1) wrong_arg("row vector expected");
	result=new_cmatrix(1,c,"");
	mr=matrixof(result);
    memmove((char *)mr,(char *)m,(ULONG)2*c*sizeof(double));
	fft(mr,c,-1);
	moveresult(st,result);
}

void mifft (header *hd)
{	header *st=hd,*result;
	double *m,*mr;
	int r,c;
	hd=getvalue(hd);
	if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	make_complex(st); hd=st; }
	getmatrix(hd,&r,&c,&m);
	if (r!=1) wrong_arg("row vector expected");
	result=new_cmatrix(1,c,"");
	mr=matrixof(result);
    memmove((char *)mr,(char *)m,(ULONG)2*c*sizeof(double));
	fft(mr,c,1);
	moveresult(st,result);
}

void mtridiag (header *hd)
{	header *result,*st=hd,*result1;
	double *m,*mr;
	int r,c,*rows,i;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix)
	{	getmatrix(hd,&c,&r,&m);
		if (c!=r || c==0) wrong_arg("non 0-sized square matrix expected");
		result=new_matrix(c,c,""); if (error) return;
		result1=new_matrix(1,c,""); if (error) return;
		mr=matrixof(result);
		memmove(mr,m,(ULONG)c*c*sizeof(double));
		tridiag(mr,c,&rows);
		mr=matrixof(result1);
		for (i=0; i<c; i++) *mr++=rows[i]+1;
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&c,&r,&m);
		if (c!=r || c==0) wrong_arg("non 0-sized square matrix expected");
		result=new_cmatrix(c,c,""); if (error) return;
		result1=new_matrix(1,c,""); if (error) return;
		mr=matrixof(result);
        memmove(mr,m,(ULONG)c*c*(ULONG)2*sizeof(double));
		ctridiag(mr,c,&rows);
		mr=matrixof(result1);
		for (i=0; i<c; i++) *mr++=rows[i]+1;
	}
	else wrong_arg("matrix expected");
	moveresult(st,result);
	moveresult((header *)newram,result1);
}

void mcharpoly (header *hd)
{	header *result,*st=hd,*result1;
	double *m,*mr;
	int r,c;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix)
	{	getmatrix(hd,&c,&r,&m);
		if (c!=r || c==0) wrong_arg("non 0-sized square matrix expected");
		result=new_matrix(c,c,""); if (error) return;
		result1=new_matrix(1,c+1,""); if (error) return;
		mr=matrixof(result);
		memmove(mr,m,(ULONG)c*c*sizeof(double));
		charpoly(mr,c,matrixof(result1));
	}
	else if (hd->type==s_cmatrix)
	{	getmatrix(hd,&c,&r,&m);
		if (c!=r || c==0) wrong_arg("non 0-sized square matrix expected");
		result=new_cmatrix(c,c,""); if (error) return;
		result1=new_cmatrix(1,c+1,""); if (error) return;
		mr=matrixof(result);
        memmove(mr,m,(ULONG)c*c*(ULONG)2*sizeof(double));
		ccharpoly(mr,c,matrixof(result1));
	}
	else wrong_arg("matrix expected");
	moveresult(st,result1);
}

void mfind (header *hd)
{	header *st=hd,*hd1,*result;
	double *m,*m1,*mr;
	int i,j,k,c,r,c1,r1;
	hd=getvalue(hd);
	hd1=getvalue(nextof(st));
	if (error) return;
	if ((hd->type!=s_matrix && hd->type!=s_real) || 
	    (hd1->type!=s_matrix && hd1->type!=s_real)) wrong_arg("real value or matrix expected");
	getmatrix(hd,&c,&r,&m);
	getmatrix(hd1,&c1,&r1,&m1);
	if (c!=1 && r!=1) wrong_arg("row or column vector expected");
	if (r!=1) c=r;
	result=new_matrix(c1,r1,""); if (error) return;
	mr=matrixof(result);
	for (i=0; i<r1; i++)
		for (j=0; j<c1; j++)
		{	k=0;
			while (k<c && m[k]<=*m1) k++;
			if (k==c && *m1<=m[c-1]) k=c-1;
			*mr++=k; m1++;
		}
	moveresult(st,result);
}

void mdiag2 (header *hd)
{	header *st=hd,*hd1,*result;
	int c,r,i,n,l;
	double *m,*mh,*mr;
	hd1=next_param(hd);
	hd=getvalue(hd); if (hd1) hd1=getvalue(hd1);
	if (error) return;
	if (hd1->type!=s_real) wrong_arg("2nd arg: real value expected");
	n=(int)*realof(hd1);
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(1,r,""); if (error) return;
		mr=matrixof(result); l=0;
		for (i=0; i<r; i++)
		{	if (i+n>=c) break;
			if (i+n>=0) { l++; *mr++=*mat(m,c,i,i+n); }
		}
		dimsof(result)->c=l;
		result->size=matrixsize(1,c);
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(1,r,""); if (error) return;
		mr=matrixof(result); l=0;
		for (i=0; i<r; i++)
		{	if (i+n>=c) break;
			if (i+n>=0) 
			{	l++;
				mh=cmat(m,c,i,i+n);
				*mr++=*mh++;
				*mr++=*mh;
			}
		}
		dimsof(result)->c=l;
		result->size=cmatrixsize(1,c);
	}
	else wrong_arg("1st arg: real or complex matrix expected");
	moveresult(st,result);
}

void mband (header *hd)
{	header *st=hd,*hd1,*hd2,*result;
	int i,j,c,r,n1,n2;
	double *m,*mr;
	hd1=next_param(hd);
	hd2=next_param(hd1);
	hd=getvalue(hd); hd1=getvalue(hd1); hd2=getvalue(hd2);
	if (error) return;
	if (hd1->type!=s_real || hd2->type!=s_real) wrong_arg("2nd and 3rd args: real value expected");
	n1=(int)*realof(hd1); n2=(int)*realof(hd2);
	if (hd->type==s_matrix || hd->type==s_real)
	{	getmatrix(hd,&r,&c,&m);
		result=new_matrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	if (j-i>=n1 && j-i<=n2) *mr++=*m++;
				else { *mr++=0.0; m++; }
			}
	}
	else if (hd->type==s_cmatrix || hd->type==s_complex)
	{	getmatrix(hd,&r,&c,&m);
		result=new_cmatrix(r,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	if (j-i>=n1 && j-i<=n2) { *mr++=*m++; *mr++=*m++; }
				else { *mr++=0.0; *mr++=0.0; m+=2; }
			}
	}
	else wrong_arg("bad type");
	moveresult(st,result);
}

void mdup (header *hd)
{	header *result,*st=hd,*hd1;
	double x,*m,*m1,*m2;
	int c,i,n,j,r;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (!hd1) wrong_arg("needs a 2nd arg");
	hd1=getvalue(hd1); if (error) return;
	if (hd1->type!=s_real) wrong_arg("2nd arg: real value expected");
	x=*realof(hd1); n=(int)x;
	if (n<1 || x>=INT_MAX) wrong_arg("2nd arg: must be >= 1");
	if (hd->type==s_matrix && dimsof(hd)->r==1)
	{	c=dimsof(hd)->c;
		result=new_matrix(n,c,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<n; i++)
		{	m=mat(m2,c,i,0);
			memmove((char *)m,(char *)m1,c*sizeof(double));
		}
	}
	else if (hd->type==s_matrix && dimsof(hd)->c==1)
	{	r=dimsof(hd)->r;
		result=new_matrix(r,n,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<r; i++)
		{
			for (j=0; j<n; j++)
				*mat(m2,n,i,j)=*mat(m1,1,i,0);
		}
	}
	else if (hd->type==s_real)
	{	result=new_matrix(n,1,""); if (error) return;
		m1=matrixof(result);
		for (i=0; i<n; i++) *m1++=*realof(hd);
	}
	else if (hd->type==s_cmatrix && dimsof(hd)->r==1)
	{	c=dimsof(hd)->c;
		result=new_cmatrix(n,c,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<n; i++)
		{	m=cmat(m2,c,i,0);
            memmove((char *)m,(char *)m1,(ULONG)2*c*sizeof(double));
		}
	}
	else if (hd->type==s_cmatrix && dimsof(hd)->c==1)
	{	r=dimsof(hd)->r;
		result=new_cmatrix(r,n,"");
		if (error) return;
		m1=matrixof(hd); m2=matrixof(result);
		for (i=0; i<r; i++)
		{
			for (j=0; j<n; j++)
				copy_complex(cmat(m2,n,i,j),cmat(m1,1,i,0));
		}
	}
	else if (hd->type==s_complex)
	{	result=new_cmatrix(n,1,""); if (error) return;
		m1=matrixof(result);
		for (i=0; i<n; i++) { *m1++=*realof(hd); *m1++=*imagof(hd); }
	}
	else wrong_arg("bad 1st arg");
	moveresult(st,result);
}

void make_complex (header *hd)
/**** make_complex
	make a function argument complex.
****/
{	header *old=hd,*nextarg;
	ULONG size;
	int r,c,i,j;
	double *m,*m1;
	hd=getvalue(hd);
	if (hd->type==s_real)
	{	size=sizeof(header)+2*sizeof(double);
		nextarg=nextof(old);
		if (newram+(size-old->size)>ramend)
		{	output("Memory overflow!\n"); error=180; return; }
		if (newram>(char *)nextarg)
			memmove((char *)old+size,(char *)nextarg,
				newram-(char *)nextarg);
		newram+=size-old->size;
		*(old->name)=0; old->size=size;
		old->type=s_complex;
		*realof(old)=*realof(hd);
		*imagof(old)=0.0;
	}
	else if (hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		size=cmatrixsize(r,c);
		nextarg=nextof(old);
		if (newram+(size-old->size)>ramend)
		{	output("Memory overflow!\n"); error=180; return; }
		if (newram>(char *)nextarg)
			memmove((char *)old+size,(char *)nextarg,
				newram-(char *)nextarg);
		newram+=size-old->size;
		*(old->name)=0; old->size=size;
		old->type=s_cmatrix;
		dimsof(old)->r=r; dimsof(old)->c=c;
		m1=matrixof(old);
		for (i=r-1; i>=0; i--)
			for (j=c-1; j>=0; j--)
			{	*cmat(m1,c,i,j)=*mat(m,c,i,j);
				*(cmat(m1,c,i,j)+1)=0.0;
			}
	}
}

void mvconcat (header *hd)
{	header *st=hd,*hd1,*result;
	double *m,*m1;
	int r,c,r1,c1;
	ULONG size=0;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (!hd1) wrong_arg("");
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	if (hd1->type==s_real || hd1->type==s_matrix)
		{	getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if (r!=0 && (c!=c1 || (ULONG)r+r1>INT_MAX)) wrong_arg("columns must agree");
			result=new_matrix(r+r1,c1,"");
			if (r!=0)
			{	size=(ULONG)r*c*sizeof(double);
				memmove((char *)matrixof(result),(char *)m,
					size);
			}
			memmove((char *)matrixof(result)+size,
				(char *)m1,(ULONG)r1*c1*sizeof(double));
		}
		else if (hd1->type==s_complex || hd1->type==s_cmatrix)
		{	make_complex(st);
			mvconcat(st);
			return;
		}
		else wrong_arg("bad type");
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	if (hd1->type==s_complex || hd1->type==s_cmatrix)
		{	getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if (r!=0 && (c!=c1 || (ULONG)r+r1>INT_MAX)) wrong_arg("columns must agree");
			result=new_cmatrix(r+r1,c1,"");
			if (r!=0)
			{	size=(ULONG)r*(ULONG)2*c*sizeof(double);
				memmove((char *)matrixof(result),(char *)m,
					size);
			}
			memmove((char *)matrixof(result)+size,
                (char *)m1,(ULONG)r1*(ULONG)2*c1*sizeof(double));
		} 
		else if (hd1->type==s_real || hd1->type==s_matrix)
		{	make_complex(next_param(st));
			mvconcat(st);
			return;
		}
		else wrong_arg("bad type");
	}	
	else wrong_arg("bad type");
	moveresult(st,result);
}

void mhconcat (header *hd)
{	header *st=hd,*hd1,*result;
	double *m,*m1,*mr;
	int r,c,r1,c1,i;
	hd=getvalue(hd);
	hd1=next_param(st);
	if (!hd1) wrong_arg("");
	hd1=getvalue(hd1); if (error) return;
	if (hd->type==s_string && hd1->type==s_string)
	{	result=new_string(stringof(hd),
			strlen(stringof(hd))+strlen(stringof(hd1))+1,"");
		strcat(stringof(result),stringof(hd1));
	}
	else if (hd->type==s_real || hd->type==s_matrix)
	{	if (hd1->type==s_real || hd1->type==s_matrix)
		{	getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if (c!=0 && (r!=r1 || (ULONG)c+c1>INT_MAX)) wrong_arg("rows must agree");
			result=new_matrix(r1,c+c1,"");
			mr=matrixof(result);
			for (i=0; i<r1; i++)
			{	if (c!=0) memmove((char *)mat(mr,c+c1,i,0),
					(char *)mat(m,c,i,0),c*sizeof(double));
				memmove((char *)mat(mr,c+c1,i,c),
					(char *)mat(m1,c1,i,0),c1*sizeof(double));
			}
		}
		else if (hd1->type==s_complex || hd1->type==s_cmatrix)
		{	make_complex(st);
			mhconcat(st);
			return;
		}
		else wrong_arg("bad type");
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	if (hd1->type==s_complex || hd1->type==s_cmatrix)
		{	getmatrix(hd,&r,&c,&m);
			getmatrix(hd1,&r1,&c1,&m1);
			if (c!=0 && (r!=r1 || (ULONG)c+c1>INT_MAX)) wrong_arg("rows must agree");
			result=new_cmatrix(r1,c+c1,"");
			mr=matrixof(result);
			for (i=0; i<r1; i++)
			{	if (c!=0) memmove((char *)cmat(mr,c+c1,i,0),
                    (char *)cmat(m,c,i,0),c*(ULONG)2*sizeof(double));
				memmove((char *)cmat(mr,c+c1,i,c),
                    (char *)cmat(m1,c1,i,0),c1*(ULONG)2*sizeof(double));
			}
		}
		else if (hd1->type==s_real || hd1->type==s_matrix)
		{	make_complex(next_param(st)); if (error) return;
			mhconcat(st);
			return;
		}
		else wrong_arg("bad type");
	}	
	else wrong_arg("bad type");
	moveresult(st,result);
}

void cscalp (double *s, double *si, double *x, double *xi,
	double *y, double *yi);
void ccopy (double *y, double *x, double *xi);

void wmultiply (header *hd)
/***** multiply
	matrix multiplication for weakly nonzero matrices.
*****/
{	header *result,*st=hd,*hd1;
	dims *d,*d1;
	double *m,*m1,*m2,*mm1,*mm2,x,y,null=0.0;
	int i,j,c,r,k;
	hd=getvalue(hd); hd1=getvalue(nextof(st));
	if (error) return;
	if (hd->type==s_matrix && hd1->type==s_matrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_matrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	mm1=mat(m1,d->c,i,0); mm2=m2+j;
				x=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm1!=0.0)&&(*mm2!=0.0)) x+=(*mm1)*(*mm2);
					mm1++; mm2+=d1->c;
				}
				*mat(m,c,i,j)=x;
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_matrix && hd1->type==s_cmatrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
            {   mm1=mat(m1,d->c,i,0); mm2=m2+(ULONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0 || *(mm2+1)!=0.0) &&
							(*mm1!=0.0))
					cscalp(&x,&y,mm1,&null,mm2,mm2+1);
					mm1++; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_matrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	mm1=cmat(m1,d->c,i,0); mm2=m2+j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0) &&
							(*mm1!=0.0 ||  *(mm1+1)!=0.0))
					cscalp(&x,&y,mm1,mm1+1,mm2,&null);
					mm1+=2; mm2+=d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_cmatrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
            {   mm1=cmat(m1,d->c,i,0); mm2=m2+(ULONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0 || *(mm2+1)!=0.0) &&
							(*mm1!=0.0 || *(mm1+1)!=0.0))
					cscalp(&x,&y,mm1,mm1+1,mm2,mm2+1);
					mm1+=2; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	else wrong_arg("bad type");
}

void smultiply (header *hd)
/***** multiply
	matrix multiplication for weakly nonzero symmetric matrices.
*****/
{	header *result,*st=hd,*hd1;
	dims *d,*d1;
	double *m,*m1,*m2,*mm1,*mm2,x,y,null=0.0;
	int i,j,c,r,k;
	hd=getvalue(hd); hd1=getvalue(nextof(st));
	if (error) return;
	if (hd->type==s_matrix && hd1->type==s_matrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_matrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=i; j<c; j++)
			{	mm1=mat(m1,d->c,i,0); mm2=m2+j;
				x=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm1!=0.0)&&(*mm2!=0.0)) x+=(*mm1)*(*mm2);
					mm1++; mm2+=d1->c;
				}
				*mat(m,c,i,j)=x;
				*mat(m,c,j,i)=x;
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_matrix && hd1->type==s_cmatrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=i; j<c; j++)
            {   mm1=mat(m1,d->c,i,0); mm2=m2+(ULONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0 || *(mm2+1)!=0.0) &&
							(*mm1!=0.0))
					cscalp(&x,&y,mm1,&null,mm2,mm2+1);
					mm1++; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y); y=-y;
				ccopy(cmat(m,c,j,i),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_matrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=i; j<c; j++)
			{	mm1=cmat(m1,d->c,i,0); mm2=m2+j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0) &&
							(*mm1!=0.0 ||  *(mm1+1)!=0.0))
					cscalp(&x,&y,mm1,mm1+1,mm2,&null);
					mm1+=2; mm2+=d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y); y=-y;
				ccopy(cmat(m,c,j,i),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	if (hd->type==s_cmatrix && hd1->type==s_cmatrix)
	{	d=dimsof(hd);
		d1=dimsof(hd1);
		if (d->c != d1->r)
		{	error=8; output("Cannot multiply these!\n");
			return;
		}
		r=d->r; c=d1->c;
		result=new_cmatrix(r,c,"");
		if (error) return;
		m=matrixof(result);
		m1=matrixof(hd);
		m2=matrixof(hd1);
		for (i=0; i<r; i++)
			for (j=i; j<c; j++)
            {   mm1=cmat(m1,d->c,i,0); mm2=m2+(ULONG)2*j;
				x=0.0; y=0.0;
				for (k=0; k<d->c; k++)
				{	if ((*mm2!=0.0 || *(mm2+1)!=0.0) &&
							(*mm1!=0.0 || *(mm1+1)!=0.0))
					cscalp(&x,&y,mm1,mm1+1,mm2,mm2+1);
					mm1+=2; mm2+=2*d1->c;
				}
				ccopy(cmat(m,c,i,j),&x,&y); 
				y=-y;
				ccopy(cmat(m,c,j,i),&x,&y);
			}
		moveresult(st,result);
		return;
	}
	else wrong_arg("bad type");
}

/****************************************************************
 *	polynom ops
 ****************************************************************/
#define max(x,y) ((x)>(y)?(x):(y))

static double *polynom;
static int degree,polreal;

static double peval (double x)
{	int i;
	double *p=polynom+degree,res;
	res=*p--;
	for (i=degree-1; i>=0; i--) res=res*x+(*p--);
	return res;
}

static void cpeval (double *x, double *xi, double *z, double *zi)
{	int i;
	double *p,h,hi;
	p=polynom+(polreal?degree:(2l*degree));
	*z=*p; *zi=(polreal)?0.0:*(p+1);
	if (polreal) p--;
	else p-=2;
	for (i=degree-1; i>=0; i--)
	{	complex_multiply(x,xi,z,zi,&h,&hi);
		*z= h + *p;
		if (!polreal) { *zi=hi+*(p+1); p--; }
		else { *zi=hi; }
		p--;
	}
}

void polyval (header *hd)
{	header *st=hd,*hd1;
	int r,c;
	hd1=next_param(hd);
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&polynom);
		if (r!=1) wrong_arg("row vector expected");
		degree=c-1;
		if (degree<0) wrong_arg("non 0-sized vector expected");
		polreal=1;
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	make_complex(hd1);
		getmatrix(hd,&r,&c,&polynom);
		if (r!=1) wrong_arg("row vector expected");
		degree=c-1;
		if (degree<0) wrong_arg("non 0-sized vector expected");
		polreal=0;
	}	
	else wrong_arg("bad type");
	spread1(peval,cpeval,hd1);
	moveresult(st,hd1);
}

static int testparams (header **hd1, header **hd2)
{	header *h1=*hd1,*h2=*hd2;
	*hd1=getvalue(h1); if (error) return 0;
	*hd2=getvalue(h2); if (error) return 0;
	if ((*hd1)->type==s_complex || (*hd1)->type==s_cmatrix
		|| (*hd2)->type==s_complex || (*hd2)->type==s_cmatrix)
	{	if ((*hd1)->type!=s_complex && (*hd1)->type!=s_cmatrix)
		{	make_complex(h1); *hd1=h1; *hd2=nextof(h1);
			*hd2=getvalue(*hd2);
		}
		else if ((*hd2)->type!=s_complex && (*hd2)->type!=s_cmatrix)
		{	make_complex(h2); *hd2=h2;
		}
		return 1;
	}
	else return 0;
}

void polyadd (header *hd)
{	header *st=hd,*hd1,*result;
	int flag,c,c1,c2,i,r;
	double *m1,*m2,*mr;
	complex *mc1,*mc2,*mcr;
	hd1=next_param(st);
	flag=testparams(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1); if (r!=1) wrong_arg("row vector expected");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) wrong_arg("row vector expected");
	c=max(c1,c2);
	if (flag) /* complex values */
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c,""); if (error) return;
		mcr=(complex *)matrixof(result);
		for (i=0; i<c; i++)
		{	if (i>=c1) { c_copy(*mcr,*mc2); mcr++; mc2++; }
			else if (i>=c2) { c_copy(*mcr,*mc1); mcr++; mc1++; }
			else { c_add(*mc1,*mc2,*mcr); mc1++; mc2++; mcr++; }
		}
	}
	else
	{	result=new_matrix(1,c,""); if (error) return;
		mr=matrixof(result);
		for (i=0; i<c; i++)
		{	if (i>=c1) { *mr++ = *m2++; }
			else if (i>=c2) { *mr++ = *m1++; }
			else { *mr++ = *m1++ + *m2++; }
		}	
	}
	moveresult(st,result);
}

void polymult (header *hd)
{	header *st=hd,*hd1,*result;
	int flag,c,c1,c2,i,r,j,k;
	double *m1,*m2,*mr,x;
	complex *mc1,*mc2,*mcr,xc,hc;
	hd1=next_param(st);
	flag=testparams(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1); if (r!=1) wrong_arg("row vector expected");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) wrong_arg("row vector expected");
	if ((ULONG)c1+c2-1>INT_MAX) wrong_arg("can't handle those large vectors");
	c=c1+c2-1;
	if (flag)
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c,""); if (error) return;
		mcr=(complex *)matrixof(result);
		c_copy(xc,*mc1); mc1++;
		for (i=0; i<c2; i++) c_mult(xc,mc2[i],mcr[i]);
		for (j=1; j<c1; j++)
		{	c_copy(xc,*mc1); mc1++;
			for (k=j,i=0; i<c2-1; i++,k++) 
			{	c_mult(xc,mc2[i],hc);
				c_add(hc,mcr[k],mcr[k]);
			}
			c_mult(xc,mc2[i],mcr[k]);
		}
	}
	else
	{	result=new_matrix(1,c,""); if (error) return;
		mr=matrixof(result);
		x=*m1++;
		for (i=0; i<c2; i++) mr[i]=x*m2[i];
		for (j=1; j<c1; j++)
		{	x=*m1++;
			for (k=j,i=0; i<c2-1; i++,k++) mr[k]+=x*m2[i];
			mr[k]=x*m2[i];
		}
	}
	moveresult(st,result);
}

void polydiv (header *hd)
{	header *st=hd,*hd1,*result,*rest;
	int flag,c1,c2,i,r,j;
	double *m1,*m2,*mr,*mh,x,l;
	complex *mc1,*mc2,*mcr,*mch,xc,lc,hc;
	hd1=next_param(st);
	flag=testparams(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1);  if (r!=1) wrong_arg("row vector expected");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) wrong_arg("row vector expected");
	if (c1<c2)
	{	result=new_real(0.0,"");
		rest=(header *)newram;
		moveresult(rest,hd1);
	}
	else if (flag)
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c1-c2+1,""); if (error) return;
		mcr=(complex *)matrixof(result);
		rest=new_cmatrix(1,c2,""); if (error) return;
		mch=(complex *)newram;
		if (newram+c1*sizeof(complex)>ramend)
		{	output("Out of memory!\n"); error=190; return;
		}
		memmove((char *)mch,(char *)mc1,c1*sizeof(complex));
		c_copy(lc,mc2[c2-1]);
		if (lc[0]==0.0 && lc[1]==0.0) wrong_arg("");
		for (i=c1-c2; i>=0; i--)
		{	c_div(mch[c2+i-1],lc,xc); c_copy(mcr[i],xc);
			for(j=0; j<c2; j++) 
			{	c_mult(mc2[j],xc,hc);
				c_sub(mch[i+j],hc,mch[i+j]);
			}
		}
		memmove((char *)matrixof(rest),(char *)mch,c2*sizeof(complex));
	}
	else
	{	result=new_matrix(1,c1-c2+1,""); if (error) return;
		mr=matrixof(result);
		rest=new_matrix(1,c2,""); if (error) return;
		mh=(double *)newram;
		if (newram+c1*sizeof(double)>ramend)
		{	output("Out of memory!\n"); error=190; return;
		}
		memmove((char *)mh,(char *)m1,c1*sizeof(double));
		l=m2[c2-1];
		if (l==0.0) wrong_arg("");
		for (i=c1-c2; i>=0; i--)
		{	x=mh[c2+i-1]/l; mr[i]=x;
			for(j=0; j<c2; j++) mh[i+j]-=m2[j]*x;
		}
		memmove((char *)matrixof(rest),(char *)mh,c2*sizeof(double));
	}
	moveresult(st,result);
	moveresult(nextof(st),rest);
}

void dd (header *hd)
{	header *st=hd,*hd1,*result;
	int flag,c1,c2,i,j,r;
	double *m1,*m2,*mr;
	complex *mc1,*mc2,*mcr,hc1,hc2;
	hd1=next_param(st);
	flag=testparams(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1); if (r!=1) wrong_arg("row vector expected");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) wrong_arg("row vector expected");
	if (c1!=c2) wrong_arg("columns must agree");
	if (flag) /* complex values */
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c1,""); if (error) return;
		mcr=(complex *)matrixof(result);
		memmove((char *)mcr,(char *)mc2,c1*sizeof(complex));
		for (i=1; i<c1; i++)
		{	for (j=c1-1; j>=i; j--)
			{	if (mc1[j][0]==mc1[j-i][0] &&
					mc1[j][1]==mc1[j-i][1]) wrong_arg("");
				c_sub(mcr[j],mcr[j-1],hc1);
				c_sub(mc1[j],mc1[j-i],hc2);
				c_div(hc1,hc2,mcr[j]);
			}
		}	
	}
	else
	{	result=new_matrix(1,c1,""); if (error) return;
		mr=matrixof(result);
		memmove((char *)mr,(char *)m2,c1*sizeof(double));
		for (i=1; i<c1; i++)
		{	for (j=c1-1; j>=i; j--)
			{	if (m1[j]==m1[j-i]) wrong_arg("");
				mr[j]=(mr[j]-mr[j-1])/(m1[j]-m1[j-i]);
			}
		}	
	}
	moveresult(st,result);
}

static double *divx,*divdif;
//static int degree,polreal;

static double rddeval (double x)
{	int i;
	double *p=divdif+degree,res;
	res=*p--;
	for (i=degree-1; i>=0; i--) res=res*(x-divx[i])+(*p--);
	return res;
}

static void cddeval (double *x, double *xi, double *z, double *zi)
{	int i;
	double *p,h,hi,*dd,xh,xhi;
	p=divdif+(polreal?degree:(2l*degree));
	dd=divx+(polreal?(degree-1):(2l*(degree-1)));
	*z=*p; *zi=(polreal)?0.0:*(p+1);
	if (polreal) p--;
	else p-=2;
	for (i=degree-1; i>=0; i--)
	{	xh=*x-*dd;
		if (!polreal) { xhi=*xi-*(dd+1); dd--; }
		else xhi=*xi;
		dd--;
		complex_multiply(&xh,&xhi,z,zi,&h,&hi);
		*z= h + *p;
		if (!polreal) { *zi=hi+*(p+1); p--; }
		else { *zi=hi; }
		p--;
	}
}

void ddval (header *hd)
{	header *st=hd,*hdd,*hd1;
	int r,c,c1;
	start: hdd=next_param(st);
	hd1=next_param(hdd);
	hd=getvalue(st); if (error) return;
	hdd=getvalue(hdd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&divx);
		if (r!=1) wrong_arg("row vector expected");
		degree=c-1;
		if (degree<0) wrong_arg("non 0-sized vector expected");
		polreal=1;
		if (hdd->type!=s_real && hdd->type!=s_matrix)
		{	if (hdd->type==s_complex || hdd->type==s_cmatrix)
			{	make_complex(st); goto start; }
			else wrong_arg("bad type");
		}
		getmatrix(hdd,&r,&c1,&divdif);
		if (r!=1 || c1!=c) wrong_arg("");
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	make_complex(hd1);
		getmatrix(hd,&r,&c,&divx);
		if (r!=1) wrong_arg("row vector expected");
		degree=c-1;
		if (degree<0) wrong_arg("non 0-sized vector expected");
		polreal=0;
		if (hdd->type!=s_complex && hdd->type!=s_cmatrix)
		{	if (hdd->type==s_real || hdd->type==s_matrix)
			{	make_complex(nextof(st)); goto start; }
			else wrong_arg("bad type");
		}
		getmatrix(hdd,&r,&c1,&divdif);
		if (r!=1 || c1!=c) wrong_arg("");
	}
	else wrong_arg("bad type");
	spread1(rddeval,cddeval,hd1);
	moveresult(st,hd1);
}

void polyzeros (header *hd)
{	header *st=hd,*result;
	int i,j,r,c;
	double *m,*mr,x;
	complex *mc,*mcr,xc,hc;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m);
		if (r!=1) wrong_arg("row vector expected");
		result=new_matrix(1,c+1,""); if (error) return;
		mr=matrixof(result);
		mr[0]=-m[0]; mr[1]=1.0;
		for (i=1; i<c; i++)
		{	x=-m[i]; mr[i+1]=1.0;
			for (j=i; j>=1; j--) mr[j]=mr[j-1]+x*mr[j];
			mr[0]*=x;
		}
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m); mc=(complex *)m;
		if (r!=1) wrong_arg("row vector expected");
		result=new_cmatrix(1,c+1,""); if (error) return;
		mcr=(complex *)matrixof(result);
		mcr[0][0]=-mc[0][0]; mcr[0][1]=-mc[0][1];
		mcr[1][0]=1.0; mcr[1][1]=0.0;
		for (i=1; i<c; i++)
		{	xc[0]=-mc[i][0]; xc[1]=-mc[i][1];
			mcr[i+1][0]=1.0; mcr[i+1][1]=0.0;
			for (j=i; j>=1; j--) 
			{	c_mult(xc,mcr[j],hc);
				c_add(hc,mcr[j-1],mcr[j]);
			}
			c_mult(xc,mcr[0],mcr[0]);
		}
	}
	else wrong_arg("bad type");
	moveresult(st,result);
}

void polydd (header *hd)
{	header *st=hd,*hd1,*result;
	int flag,c1,c2,i,j,r;
	double *m1,*m2,*mr,x;
	complex *mc1,*mc2,*mcr,hc,xc;
	hd1=next_param(st);
	flag=testparams(&hd,&hd1); if (error) return;
	getmatrix(hd,&r,&c1,&m1); if (r!=1) wrong_arg("row vector expected");
	getmatrix(hd1,&r,&c2,&m2); if (r!=1) wrong_arg("row vector expected");
	if (c1!=c2) wrong_arg("columns must agree");
	if (flag) /* complex values */
	{	mc1=(complex *)m1; mc2=(complex *)m2;
		result=new_cmatrix(1,c1,""); if (error) return;
		mcr=(complex *)matrixof(result);
		c_copy(mcr[c1-1],mc2[c1-1]);
		for (i=c1-2; i>=0; i--)
		{	c_copy(xc,mc1[i]);
			c_mult(xc,mcr[i+1],hc);
			c_sub(mc2[i],hc,mcr[i]);
			for (j=i+1; j<c1-1; j++) 
			{	c_mult(xc,mcr[j+1],hc);
				c_sub(mcr[j],hc,mcr[j]);
			}
		}
	}
	else
	{	result=new_matrix(1,c1,""); if (error) return;
		mr=matrixof(result);
		mr[c1-1]=m2[c1-1];
		for (i=c1-2; i>=0; i--)
		{	x=m1[i];
			mr[i]=m2[i]-x*mr[i+1];
			for (j=i+1; j<c1-1; j++) mr[j]=mr[j]-x*mr[j+1];
		}
	}
	moveresult(st,result);
}

void polytrunc (header *hd)
{	header *st=hd,*result;
	double *m;
	complex *mc;
	int i;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix && dimsof(hd)->r==1)
	{	m=matrixof(hd);
		for (i=dimsof(hd)->c-1; i>=0; i--)
		{	if (fabs(m[i])>epsilon) break;
		}
		if (i<0) result=new_real(0.0,"");
		else 
		{	result=new_matrix(1,i+1,"");
			memmove((char *)matrixof(result),(char *)matrixof(hd),
				(i+1)*sizeof(double));
		}
	}
	else if (hd->type==s_complex && dimsof(hd)->r==1)
	{	mc=(complex *)matrixof(hd);
		for (i=dimsof(hd)->c-1; i>=0; i--)
		{	if (fabs(mc[i][0])>epsilon && fabs(mc[i][1])>epsilon) 
				break;
		}
		if (i<0) result=new_complex(0.0,0.0,"");
		else 
		{	result=new_cmatrix(1,i+1,"");
			memmove((char *)matrixof(result),(char *)matrixof(hd),
				(i+1)*sizeof(complex));
		}
	}
	else wrong_arg("bad type");
	moveresult(st,result);
}

void mzeros (header *hd)
{	header *st=hd,*result;
	int r,c;
	double *m;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix) 
	{	make_complex(st); if (error) return;
		hd=getvalue(st); if (error) return; 
	}
	if (hd->type!=s_cmatrix || dimsof(hd)->r!=1 || dimsof(hd)->c<2)
	{	output("Need a complex polynomial\n"); error=300; return; }
	getmatrix(hd,&r,&c,&m);
	result=new_cmatrix(1,c-1,""); if (error) return;
	bauhuber(m,c-1,matrixof(result),1,0,0);
	moveresult(st,result);
}

void mzeros1 (header *hd)
{	header *st=hd,*hd1,*result;
	int r,c;
	double *m,xr,xi;
	hd1=nextof(hd);
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_matrix)
	{	make_complex(st); if (error) return;
		hd=getvalue(st); if (error) return;
	}
	hd1=nextof(st);
	hd1=getvalue(hd1); if (error) return;
	if (hd1->type==s_real)
	{	xr=*realof(hd1); xi=0;
	}
	else if (hd1->type==s_complex)
	{	xr=*realof(hd1); xi=*(realof(hd1)+1);
	}
	else
	{	output("Need a starting value!\n"); error=300; return; }
	if (hd->type!=s_cmatrix || dimsof(hd)->r!=1 || dimsof(hd)->c<2)
	{	output("Need a complex polynomial\n"); error=300; return; }
	getmatrix(hd,&r,&c,&m);
	result=new_complex(0,0,""); if (error) return;
	bauhuber(m,c-1,realof(result),0,xr,xi);
	moveresult(st,result);
}


/****************************************************************
 *	builtin core
 ****************************************************************/
int builtin_count;

extern builtintyp builtin_list[];

void print_builtin (void)
{	int i, c, cend, lw=linelength/MAXNAME;
	
	for (i=0; i<builtin_count; i+=lw) {
		cend = i+lw;
		if (cend>=builtin_count) cend=builtin_count;
		for (c=i; c<cend ; c++) {
			output1(OUTFMT,builtin_list[c].name);
			if (test_key()==escape) return;
		}
		output("\n");
	}
	output("\n");
}

int builtin_compare (const builtintyp *p1, const builtintyp *p2)
{	int h;
	h=strcmp(p1->name,p2->name);
	if (h) return h;
	else 
	{	if (p1->nargs==-1 || p2->nargs==-1) return 0;
		else if (p1->nargs<p2->nargs) return -1; 
		else if (p1->nargs>p2->nargs) return 1;
		else return 0;
	}
}

void sort_builtin (void)
{	builtin_count=0;
	while (builtin_list[builtin_count].name) builtin_count++;
	qsort(builtin_list,builtin_count,sizeof(builtintyp),
		(int (*) (const void *, const void *))builtin_compare);
}

int exec_builtin (char *name, int nargs, header *hd)
{	builtintyp *b=builtin_list,h;
	h.name=name; h.nargs=nargs;
	b=bsearch(&h,builtin_list,builtin_count,sizeof(builtintyp),
		(int (*) (const void *, const void *))builtin_compare);
	if (b)
	{	if (nargs==0) hd=0;
		b->f(hd); return 1;
	}
	else return 0;
}

builtintyp *find_builtin (char *name)
{	builtintyp h;
	h.name=name; h.nargs=-1;
	return (builtintyp *)bsearch(&h,builtin_list,builtin_count,sizeof(builtintyp),
		(int (*) (const void *, const void *))builtin_compare);
}

	