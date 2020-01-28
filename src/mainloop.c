#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <float.h>
#include <stdarg.h>
#include <limits.h>
	
#include "sysdep.h"
#include "header.h"
#include "core.h"
#include "funcs.h"
#include "graphics.h"

char *next,*udfline;

FILE *metafile=0;

double epsilon;

int error,quit,surpressed,udf=0,errorout,outputing=1,stringon=0,
	trace=0;
char line[MAXLINE];

long loopindex=0;

int fieldw=15,linew=5;
double maxexpo=1.0e5,minexpo=1.0e-7;
char expoformat[16]=" %14.5E";
char fixedformat[16]=" %14.7F";

int nosubmref=0;

FILE *infile=NULL,*outfile=NULL;

header commandheader;
int commandtype;

/* dumping to file */

void output (char *s)
{	text_mode();
	if (outputing || error) gprint(s);
	if (outfile)
	{	fprintf(outfile,"%s",s);
		if (ferror(outfile))
		{	
			error=200;
			fclose(outfile); outfile=NULL;
			output("Error on dump file (disk full?).\n");
		}
	}
}

void output1 (char *fmt, ...)
{	char text [1024];
	va_list v;
	text_mode();
	va_start(v,fmt);
	vsnprintf(text,1024,fmt,v);
	va_end(v);
	if (outputing || error) gprint(text);
	if (outfile)
	{	va_start(v,fmt);
		vfprintf(outfile,fmt,v);
		va_end(v);
		if (ferror(outfile))
		{	
			error=200;
			fclose(outfile); outfile=NULL;
			output("Error on dump file (disk full?).\n");
		}
	}
}

/* help */

static commandtyp command_list[];

int extend(char* start, char extend[16][MAXNAME])
/* extend
Extend a start string in up to 16 ways to a command or function.
This function is called from the line editor, whenever the HELP
key is pressed.
*/
{	int count=0,ln,l;
	header *hd=(header *)ramstart;
	builtintyp *b=builtin_list;
	commandtyp *c=command_list;
	ln=strlen(start);
	while (b->name)
	{	if (!strncmp(start,b->name,ln))
		{	l=(int)strlen(b->name)-ln;
			if (l>0 && l<16)
			{	strcpy(extend[count],b->name+ln);
				count++;
			}
			if (count>=16) return count;
		}
		b++;
	}
	while (hd<(header *)udfend)
	{	if (!strncmp(start,hd->name,ln))
		{	l=(int)strlen(hd->name)-ln;
			if (l>0 && l<16)
			{	strcpy(extend[count],hd->name+ln);
				count++;
			}
			if (count>=16) return count;
		}
		hd=nextof(hd);
	}
	while (c->name)
	{	if (!strncmp(start,c->name,ln))
		{	l=(int)strlen(c->name)-ln;
			if (l>0 && l<16)
			{	strcpy(extend[count],c->name+ln);
				count++;
			}
			if (count>=16) return count;
		}
		c++;
	}
	return count;
}

/* functions that manipulate the stack */

/* Organization of the stack                      when global
                                                     scope
   ----------------------------- <-- ramstart          |
   udf (user defined functions)                        V
   ----------------------------- <-- udfend      [startlocal]
   global variables
   ----------------------------- <-- startlocal  [endlocal]
   running function local scope
   ----------------------------- <-- newram = endlocal (top of stack)
   
   
               free
   
   
   ----------------------------- <-- ramend
   
   the running function local is transient and exists only while
   the function is executed.
   
   startlocal and endlocal relates either to the current running
   local scope or to the global scope when an expression is
   evaluated at the global level of the interpreter.
   
 */

/***************** support functions ************************/

void print_error (char *p)
{	int i;
	char *q,outline[1024];
	double x;
	commandtyp *com;
	if (errorout) return;
	if (line<=p && line+1024>p)
	{	output1("error in:\n%s\n",line);
		if ((int)(p-line)<linelength-2)
			for (i=0; i<(int)(p-line); i++) output(" ");
		output("^\n");
	}
	else if (udfon)
	{	q=outline; p=udfline;
		while (*p)
		{	if (*p==2)
			{	p++; memmove((char *)(&x),p,sizeof(double));
				p+=sizeof(double);
				sprintf(q,"%g",x);
				q+=strlen(q);
			}
			else if (*p==3)
			{	p++;
				memmove((char *)(&com),p,sizeof(commandtyp *));
				p+=sizeof(commandtyp *);
				sprintf(q,"%s",com->name);
				q+=strlen(q);
			}
			else *q++=*p++;
			if (q>outline+1022)
			{	q=outline+1023;
				break;
			}
		}
		*q++=0;
		output1("Error in :\n%s\n",outline); output("\n");
	}
	errorout=1;
}

static char *type_udfline (char *start)
{	char outline[1024],*p=start,*q;
	double x;
	commandtyp *com;
	q=outline;
	while (*p)
	{	if (*p==2)
		{	p++; memmove((char *)(&x),p,sizeof(double));
			p+=sizeof(double);
			sprintf(q,"%g",x);
			q+=strlen(q);
		}
		else if (*p==3)
		{	p++;
			memmove((char *)(&com),p,sizeof(commandtyp *));
			p+=sizeof(commandtyp *);
			sprintf(q,"%s",com->name);
			q+=strlen(q);
		}
		else *q++=*p++;
		if (q>outline+1022)
		{	q=outline+1023;
			break;
		}
	}
	*q++=0;
	output(outline); output("\n");
	return p+1;
}

void trace_udfline (char *next)
{	int oldtrace;
	scantyp scan;
	extern header *running;
	header *hd,*res;
	output1("%s: ",running->name); type_udfline(next);
	again: wait_key(&scan);
	switch (scan)
	{	case fk1 :
		case cursor_down :
			break;
		case fk2 :
		case cursor_up :
			trace=2; break;
		case fk3 :
		case cursor_right :
			trace=0; break;
		case fk4 :
		case help :
			hd=(header *)newram;
			oldtrace=trace; trace=0;
			new_string("Expression",12,""); if (error) goto cont;
			minput(hd); if (error) goto cont;
			res=getvalue(hd); if (error) goto cont;
			give_out(res);
			cont : newram=(char *)hd;
			trace=oldtrace;
			goto again;
		case fk9 :
		case escape :
			output("Trace interrupted\n"); error=11010; break;
		case fk10 :
			trace=-1; break;
		default :
			output(
				"\nKeys:\n"
				"F1 (cursor_down)  Single step\n"
				"F2 (cursor_up)    Step over subroutines\n"
				"F3 (cursor_right) Go until return\n"
				"F4 (help)         Evaluate expression\n"
				"F9 (escape)       Abort execution\n"
				"F10               End trace\n\n");
			goto again;
	}
}

static void read_line (char *line)
{	int count=0,input;
	char *p=line;
	while(1)
	{	input=getc(infile);
		if (input==EOF)
		{	fclose(infile); *p++=1; infile=0;
			break; 
		}
		if (input=='\n') break;
		if (count>=1023) 
		{	output("Line too long!\n"); error=50; *line=0; return;
		}
		if ((char)input>=' ' || (signed char)input<0 || (char)input==TAB)
		{	*p++=(char)input; count++;
		}
	}
	*p=0;
}

static void next_line (void)
/**** next_line
	read a line from keyboard or file.
****/
{	if (udfon)
	{	while (*next) next++;
		next++;
		if (*next==1) udfon=0; else udfline=next;
		if (trace>0) trace_udfline(next);
		return;
	}
	else
	{	if (trace==-1) trace=1;
		if (stringon)
		{	error=2300; output("Input ended in string!\n");
			return;
		}
		if (!infile) {
			scantyp r=edit(line);
			if (r==eot) strcpy(line,"quit");
		}
		else read_line(line);
		next=line;
	}
}



/********************* interpreter **************************/

void double_out (double x)
/***** double_out
	print a double number.
*****/
{	if ((fabs(x)>maxexpo || fabs(x)<minexpo) && x!=0.0) 
		output1(expoformat,x);
	else if (x==0.0) output1(fixedformat,0.0); /* take care of -0 */
	else output1(fixedformat,x);
}

void out_matrix (header *hd)
/***** out_matrix
   print a matrix.
*****/
{	int c,r,i,j,c0,cend;
	double *m,*x;
	getmatrix(hd,&r,&c,&m);
	for (c0=0; c0<c; c0+=linew)
	{	cend=c0+linew-1; 
		if (cend>=c) cend=c-1;
		if (c>linew) output1("Column %d to %d:\n",c0+1,cend+1);
		for (i=0; i<r; i++)
		{	x=mat(m,c,i,c0);
			for (j=c0; j<=cend; j++) double_out(*x++);
			output("\n");
			if (test_key()==escape) return;
		}
	}
}

void complex_out (double x, double y)
/***** double_out
	print a complex number.
*****/
{	if ((fabs(x)>maxexpo || fabs(x)<minexpo) && x!=0.0) 
		output1(expoformat,x);
	else output1(fixedformat,x);
	output("+");
	if ((fabs(y)>maxexpo || fabs(y)<minexpo) && y!=0.0) 
		output1(expoformat,y);
	else output1(fixedformat,y);
	output("i ");
}

void out_cmatrix (header *hd)
/***** out_matrix
   print a complex matrix.
*****/
{	int c,r,i,j,c0,cend;
	double *m,*x;
	getmatrix(hd,&r,&c,&m);
	for (c0=0; c0<c; c0+=linew/2)
	{	cend=c0+linew/2-1; 
		if (cend>=c) cend=c-1;
		if (c>linew/2) output1("Column %d to %d:\n",c0+1,cend+1);
		for (i=0; i<r; i++)
		{	x=cmat(m,c,i,c0);
			for (j=c0; j<=cend; j++) { complex_out(*x,*(x+1)); 
				x+=2; }
			output("\n");
			if (test_key()==escape) return;
		}
	}
}

void give_out (header *hd)
/***** give_out
	print a value.
*****/
{	switch(hd->type)
	{	case s_real : double_out(*realof(hd)); output("\n"); break;
		case s_complex : complex_out(*realof(hd),*(realof(hd)+1));
			output("\n"); break;
		case s_matrix : out_matrix(hd); break;
		case s_cmatrix : out_cmatrix(hd); break;
		case s_string : output(stringof(hd)); output("\n"); break;
		default : output("?\n");
	}
}

/***************** some builtin commands *****************/
static void scan_filename (char *name, int lmax);
static void scan_name (char *name);
static void scan_end (void);
static void scan_endif (void);
static void scan_else (void);


char * path[MAX_PATH];
int npath=0;

static void load_file (void)
/***** load_file
	inerpret a file.
*****/
{	char filename[256];
	char oldline[1024],*oldnext;
	FILE *oldinfile;

	if (udfon) {
		output("Cannot load a file in a function!\n");
		error=221; return;
	}
	
	/* get a file name or path*/
	scan_space();
	if (*next=='(') {
		header* hd=scan_value();
		if (error) return;
		if (hd->type!=s_string) {
			output("String value expected!\n");
			error=1; return;
		}
		strncpy(filename,stringof(hd),255);filename[255]=0;
	} else {
		scan_filename(filename,256);
	}
	if (error) return;
	
	/* try to open it */
	oldinfile=infile;
	if (filename[0]==PATH_DELIM_CHAR) {	/* an absolute path, use it */
		infile=fopen(filename,"r");
	} else {							/* use standard path */
		for (int k=0; k<npath; k++) {
			char fn[strlen(path[k])+strlen(stringof(filename))+strlen(EXTENSION)+strlen(PATH_DELIM_STR)+1];
			
			strcpy(fn,path[k]);strcat(fn,PATH_DELIM_STR);strcat(fn,filename);
			infile=fopen(fn,"r");
			if (infile) break;
			else {
				strcat(fn,EXTENSION);
				infile=fopen(fn,"r");
				if (infile) break;
			}
		}
	}
	
	/* interpret the file if it exists */
	if (infile) {
		strcpy(oldline,line); oldnext=next;
		*line=0; next=line;
		while (!error && infile && !quit) {
			/* reset global context for commands evaluated in the 
	    	   lower level context */
			startglobal=startlocal;
			endglobal=endlocal;
			command();
		}
		if (infile) fclose(infile);
		infile=oldinfile;
		strcpy(line,oldline); next=oldnext;
	} else {
		output1("Could not open %s!\n",stringof(filename));
		error=53; infile=oldinfile; return;
	}
}

static commandtyp *preview_command (size_t *l);

void get_udf (void)
/***** get_udf
	define a user defined function.
*****/
{	char name[MAXNAME],argu[MAXNAME],*p,*firstchar,*startp;
	int *ph,*phh,count=0,n;
	size_t l;
	header *var,*result,*hd;
	FILE *actfile=infile;
	commandtyp *com;
	double x;
	if (udfon==1)
	{	output("Cannot define a function in a function!\n");
		error=60; return;
	}
	scan_space(); scan_name(name); if (error) return;
	kill_udf(name);
	var=new_reference(0,name); if (error) return;
	result=new_udf(""); if (error) return;
	p=udfof(result); udf=1; /* udf is for the prompt! */
	scan_space(); 
	ph=(int *)p; p+=sizeof(int);
	if (*next=='(')
	{	while(1)
		{	next++;
			scan_space();
			if (*next==')') break;
			phh=(int *)p; *phh=0; p+=sizeof(int);
			scan_name(argu); if (error) goto aborted;
			count++;
			strcpy(p,argu); p+=16; 
			*((int *)p)=xor(argu); p+=sizeof(int);
			test: scan_space();
			if (*next==')') break;
			else if (*next=='=')
			{	next++;
				*phh=1;
				newram=p;
				hd=(header *)p;
				scan_value(); if (error) goto aborted;
				strcpy(hd->name,argu);
				hd->xor=xor(argu);
				p=newram;
				goto test;
			}
			else if (*next==',') continue;
			else 
			{	output("Error in parameter list!\n"); error=701;
				goto aborted;
			}
		}
		next++;
	}
	*ph=count;
	if (*next==0) { next_line(); }
	while (1) /* help section of the udf */
	{	if (*next=='#' && *(next+1)=='#')
		{	while (*next)
			{	*p++=*next++;
				if (p>=ramend)
				{	output("Memory overflow!\n"); error=210; goto stop;
				}
			}
			*p++=0; next_line();
		}
		else break;
		if (actfile!=infile)
		{	output("End of file reached in function definition!\n");
			error=2200; goto stop;
		}
	}
	*udfstartof(result)=(p-(char *)result);
	startp=p;
	firstchar=next;
	while (1)
	{	if (error) goto stop;
		if (!strncmp(next,"endfunction",strlen("endfunction")))
		{	if (p==startp || *(p-1)) *p++=0;
			*p++=1; next+=strlen("endfunction"); break;
		}
		if (actfile!=infile)
		{	output("End of file reached in function definition!\n");
			error=2200; goto stop;
		}
		if (*next=='#' && *(next+1)=='#')
		{	*p++=0; next_line(); firstchar=next;
		}
		else 
		if (*next) 
		{	if (*next=='"')
			{	*p++=*next++;
				while (*next!='"' && *next) *p++=*next++;
				if (*next=='"') *p++=*next++;
				else { output("\" missing.\n"); error=2200; goto stop; }
			}
			else if (isdigit(*next) || 
				     	(*next=='.' && isdigit(*(next+1))) )
			{	if (next!=firstchar && isalpha(*(next-1)))
				{	*p++=*next++;
					while (isdigit(*next)) *p++=*next++;
				}
				else
				{
					if ((p-(char *)result)%2==0) *p++=' ';
					*p++=2;
		   			sscanf(next,"%lg%n",&x,&n);
		   			next+=n;
		   			memmove(p,(char *)(&x),sizeof(double));
		   			p+=sizeof(double);
			   	}
			}
			else if (isalpha(*next) &&
				(next==firstchar || !isalpha(*(next-1))) &&
				(com=preview_command(&l))!=0)
			/* Try to find a builtin command */
			{	
				if ((p-(char *)result)%2==0) *p++=' ';
				*p++=3;
				memmove(p,(char *)(&com),sizeof(commandtyp *));
				p+=sizeof(commandtyp *);
				next+=l;
			}
			else if (*next=='.' && *(next+1)=='.')
			{	*p++=' '; next_line(); firstchar=next;
			}
			else *p++=*next++;
		}
		else { *p++=0; next_line(); firstchar=next; }
		if (p>=ramend-80)
		{	output("Memory overflow!\n"); error=210; goto stop;
		}
	}
	stop:
	udf=0; if (error) return;
	result->size=((p-(char *)result)/2+1)*2;
#ifdef SPECIAL_ALIGNMENT
	result->size=((result->size-1)/8+1)*8;
#endif
	newram=(char *)result+result->size;
	assign(var,result);
	aborted:
	udf=0;
}

static void do_global (void)
{	char name[MAXNAME];
	int r;
	header *hd;
	if (!udfon)	{
		output("It makes no sense to use \"global\" outside of functions.\n");
		error=500; return;
	}
	while (1)
	{	scan_space();
		/* parse 'global *' */
		if (*next=='*' && (*(next+1)==';' || *(next+1)==0)) {
			next++;
			searchglobal=1;
			return;
		}
		/* parse list of identifiers */
		scan_name(name); r=xor(name);
		hd=(header *)udfend;
		if (hd==(header *)startlocal) break;
		while ((char *)hd<startlocal)
		{	if (r==hd->xor && !strcmp(hd->name,name)) break;
			hd=nextof(hd);
		}
		if ((char *)hd>=startlocal)
		{	output1("Variable %s not found!\n",name);
			error=160; return;
		}
		newram=endlocal;
		hd=new_reference(hd,name);
		newram=endlocal=(char *)nextof(hd);
		scan_space();
		if (*next!=',') break;
		else next++;
	}
}

static void do_return (void)
{	if (!udfon)
	{	output("No user defined function active!\n");
		error=56; return;
	}
	else udfon=2;
}

static void do_break (void)
{	if (!udfon)
	{	output("End only allowed in functions!\n"); error=57;
	}
}

static void do_for (void)
/***** do_for
	do a for command in a UDF.
	for i=value to value step value; .... ; end
*****/
{	int h,signum;
	char name[16],*jump;
	header *hd,*init,*end,*step;
	double vend,vstep;
	struct { header hd; double value; } rv;
	if (!udfon)
	{	output("For only allowed in functions!\n"); error=57; return;
	}
	rv.hd.type=s_real; *rv.hd.name=0;
	rv.hd.size=sizeof(header)+sizeof(double); rv.value=0.0;
	scan_space(); scan_name(name); if (error) return;
	kill_local(name);
	newram=endlocal;
	hd=new_reference(&rv.hd,name); if (error) return;
	endlocal=newram=(char *)hd+hd->size;
	scan_space(); if (*next!='=')
	{	output("Syntax error in for.\n"); error=71; goto end;
	}
	next++; init=scan(); if (error) goto end;
	init=getvalue(init); if (error) goto end;
	if (init->type!=s_real)
	{	output("Startvalue must be real!\n"); error=72; goto end;
	}
	rv.value=*realof(init);
	scan_space(); if (strncmp(next,"to",2))
	{	output("Endvalue missing in for!\n"); error=73; goto end;
	}
	next+=2;
	end=scan(); if (error) goto end;
	end=getvalue(end); if (error) goto end;
	if (end->type!=s_real)
	{	output("Endvalue must be real!\n"); error=73; goto end;
	}
	vend=*realof(end);
	scan_space(); 
	if (!strncmp(next,"step",4))
	{	next+=4;
		step=scan(); if (error) goto end;
		step=getvalue(step); if (error) goto end;
		if (step->type!=s_real)
		{	output("Stepvalue must be real!\n"); error=73; goto end;
		}
		vstep=*realof(step);
	}
	else vstep=1.0;
	signum=(vstep>0);
	if (signum && rv.value>vend) { scan_end(); goto end; }
	else if (!signum && rv.value<vend) { scan_end(); goto end; }
	newram=endlocal;
	scan_space(); if (*next==';' || *next==',') next++;
	jump=next;
	vend=vend+epsilon*vstep;
	while (!error)
	{	if (*next==1)
		{	output("End missing!\n");
			error=401; goto end;
		}
		h=command();
		if (h==c_return) break;
		if (h==c_break) { scan_end(); break; }
		if (h==c_end)
		{	rv.value+=vstep;
			if (signum==1 && rv.value>vend) break;
			else if (!signum && rv.value<vend) break;
			else next=jump;
			if (test_key()==escape) { error=1; break; }
		}
	}
	end : kill_local(name);
}

static void do_loop (void)
/***** do_loop
	do a loop command in a UDF.
	loop value to value; .... ; end
*****/
{	int h;
	char *jump;
	header *init,*end;
	long vend,oldindex;
	if (!udfon)
	{	output("Loop only allowed in functions!\n"); error=57; return;
	}
	init=scan(); if (error) return;
	init=getvalue(init); if (error) return;
	if (init->type!=s_real)
	{	output("Startvalue must be real!\n"); error=72; return;
	}
	oldindex=loopindex;
	loopindex=(long)*realof(init);
	scan_space(); if (strncmp(next,"to",2))
	{	output("Endvalue missing in for!\n"); error=73; goto end;
	}
	next+=2;
	end=scan(); if (error) goto end;
	end=getvalue(end); if (error) goto end;
	if (end->type!=s_real)
	{	output("Endvalue must be real!\n"); error=73; goto end;
	}
	vend=(long)*realof(end);
	if (loopindex>vend) { scan_end(); goto end; }
	newram=endlocal;
	scan_space(); if (*next==';' || *next==',') next++;
	jump=next;
	while (!error)
	{	if (*next==1)
		{	output("End missing!\n");
			error=401; goto end;
		}
		h=command();
		if (h==c_return) break;
		if (h==c_break) { scan_end(); break; }
		if (h==c_end)
		{	loopindex++;
			if (loopindex>vend) break;
			else next=jump;
			if (test_key()==escape) { error=1; break; }
		}
	}
	end : loopindex=oldindex;
}

static void do_repeat (void)
/***** do_loop
	do a loop command in a UDF.
	for value to value; .... ; endfor
*****/
{	int h;
	char *jump;
	if (!udfon)
	{	output("Loop only allowed in functions!\n"); error=57; return;
	}
	newram=endlocal;
	scan_space(); if (*next==';' || *next==',') next++;
	jump=next;
	while (!error)
	{	if (*next==1)
		{	output("End missing!\n");
			error=401; break;
		}
		h=command();
		if (h==c_return) break;
		if (h==c_break) { scan_end(); break; }
		if (h==c_end)
		{	next=jump;
			if (test_key()==escape) { error=1; break; }
		}
	}
}

static void do_end (void)
{	if (!udfon)
	{	output("End only allowed in functions!\n"); error=57;
	}
}

static void do_else (void)
{	if (!udfon)
	{	output("Else only allowed in functions!\n"); error=57; return;
	}
	scan_endif();
}

static void do_elseif (void)
{	if (!udfon)
	{	output("Elseif only allowed in functions!\n"); error=57; return;
	}
	scan_endif();
}

static void do_endif (void)
{	if (!udfon)
	{	output("Endif only allowed in functions!\n"); error=57;
	}
}

static int ctest (header *hd)
/**** ctest
	test, if a matrix contains nonzero elements.
****/
{	double *m;
	LONG n,i;
	hd=getvalue(hd); if (error) return 0;
	if (hd->type==s_string) return (*stringof(hd)!=0);
	if (hd->type==s_real) return (*realof(hd)!=0.0);
	if (hd->type==s_complex) return (*realof(hd)!=0.0 &&
		*imagof(hd)!=0.0);
	if (hd->type==s_matrix)
	{	n=(LONG)(dimsof(hd)->r)*dimsof(hd)->c;
		m=matrixof(hd);
		for (i=0; i<n; i++) if (*m++==0.0) return 0;
		return 1;
	}
	if (hd->type==s_cmatrix)
	{	n=(LONG)(dimsof(hd)->r)*dimsof(hd)->c;
		m=matrixof(hd);
		for (i=0; i<n; i++) 
		{	if (*m==0.0 && *m==0.0) return 0; m+=2; }
		return 1;
	}
	return 0;
}

static void do_if (void)
{	header *cond;
	int flag;
	if (!udfon)
	{	output("If only allowed in functions!\n"); error=111; return;
	}
	cond=scan(); if (error) return;
	flag=ctest(cond); if (error) return;
	if (!flag) scan_else();
}

static void do_trace(void)
/**** do_trace
	toggles tracing or sets the trace bit of a udf.
****/
{	header *f;
	char name[64];
	scan_space();
	if (!strncmp(next,"off",3))
	{	trace=0; next+=3;
	}
	else if (!strncmp(next,"alloff",6))
	{	next+=6;
		f=(header *)ramstart;
		while ((char *)f<udfend && f->type==s_udf)
		{	f->flags&=~1;
			f=nextof(f);
		}
		trace=0;
	}	
	else if (!strncmp(next,"on",2))
	{	trace=1; next+=2;
	}
	else if (*next==';' || *next==',' || *next==0) trace=!trace;
	else
	{	if (*next=='"') next++;
		scan_name(name); if (error) return;
		if (*next=='"') next++;
		f=searchudf(name);
		if (!f || f->type!=s_udf)
		{	output("Function not found!\n");
			error=11021; return;
		}
		f->flags^=1;
		if (f->flags&1) output1("Tracing %s\n",name);
		else output1("No longer tracing %s\n",name);
		scan_space();
	}
	if (*next==';' || *next==',') next++;
}

static void do_clg (void)
{	graphic_mode(); clear_graphics(); gflush();
}

void do_cls (void)
{	text_mode(); clear_screen();
}

static void do_clear (void)
/***** clear
	clears from the stack and remove all variables and functions.
*****/
{
	char name[MAXNAME];
	if (udfon)
	{	output("Cannot use \"clear\" in a function!\n");
		error=120; return;
	}
	scan_space();
	if (*next==';' || *next==',' || *next==0)
	{	endlocal=startlocal;
	}
	else
	while(1)
	{	scan_name(name); if (error) return;
		kill_local(name);
		scan_space();
		if (*next==',') { next++; continue; }
		else break;
	}
}

static void do_forget (void)
{	char name[MAXNAME];
	header *hd;
	int r;
	if (udfon)
	{	output("Cannot forget functions in a function!\n");
		error=720; return;
	}
	while (1)
	{	scan_space();
		scan_name(name);
		r=xor(name);
		hd=(header *)ramstart;
		while ((char *)hd<udfend)
		{	if (r==hd->xor && !strcmp(hd->name,name)) break;
			hd=nextof(hd);
		}
		if ((char *)hd>=udfend)
		{	output1("Function %s not found!\n",name);
			error=160; return;
		}
		kill_udf(name);
		scan_space();
		if (*next!=',') break;
		else next++;
	}
}

static void print_commands (void);

static void do_list (void)
{	header *hd;
	int i=0, lw=linelength/MAXNAME;
	output("  *** Builtin functions:\n");
	print_builtin();
	output("  *** Commands:\n");
	print_commands();
	output("  *** Your functions:\n");
	hd=(header *)ramstart;
	while ((char*)hd<udfend)
	{	if (hd->type!=s_udf) break;
		output1("%-16s",hd->name);
		if (test_key()==escape) return;
		hd=nextof(hd);
		i++;
		if (i==lw) {
			i=0;
			output("\n");
		}
	}
	output("\n");
}

static void do_listvar (void)
{	header *hd=(header *)startlocal;
	while (hd<(header*)endlocal)
	{	switch (hd->type) {
		case s_real:
			output1("%-20sType: %s\n",hd->name,"Real");
			break;
		case s_complex:
			output1("%-20sType: %s\n",hd->name,"Complex");
			break;
		case s_string:
			output1("%-20sType: %s\n",hd->name,"String");
			break;
		case s_matrix:
			output1("%-20sType: %s (%dx%d)\n",hd->name,
				"Real Matrix",dimsof(hd)->r,dimsof(hd)->c);
			break;
		case s_cmatrix:
			output1("%-20sType: %s (%dx%d)\n",hd->name,
				"Complex Matrix",dimsof(hd)->r,dimsof(hd)->c);
			break;
		case s_reference:
			output1("%-20sType: %s\n",hd->name,"Reference");
			break;
		case s_submatrix:
			output1("%-20sType: %s (%dx%0d)",hd->name,
				"Real Submatrix",
				submdimsof(hd)->r,submdimsof(hd)->c);
			break;
		case s_csubmatrix :
			output1("%-20sType: %s (%dx%0d)",hd->name,
				"Complex Submatrix",
				submdimsof(hd)->r,submdimsof(hd)->c);
			break;
		default:
			output1("%-20sType: %s\n",hd->name,"Unknown Type");
			break;
		}
		hd=nextof(hd);
		if (test_key()==escape) break;
	}
}

static void do_type (void)
{	char name[16];
	header *hd;
	char *p,*pnote;
	int i,count,defaults;
	scan_space();
	scan_name(name); hd=searchudf(name);
	if (hd && hd->type==s_udf)
	{	output1("function %s (",name);
		p=helpof(hd);
		count=*((int *)p);
		p+=sizeof(int);
		pnote=p;
		for (i=0; i<count; i++)
		{	defaults=*(int *)p; p+=sizeof(int);
			output1("%s",p);
			p+=16+sizeof(int);
			if (defaults)
			{	output("=...");
				p=(char *)(nextof((header *)p));
			}
			if (i!=count-1) output(",");
		}
		output(")\n");
		p=pnote;
		for (i=0; i<count; i++)
		{	defaults=*(int *)p; p+=sizeof(int);
			if (defaults) output1("## Default for %s :\n",p);
			p+=16+sizeof(int);
			if (defaults)
			{	give_out((header *)p);
				p=(char *)nextof((header *)p);
			}
		}		
		p=udfof(hd);
		while (*p!=1 && p<(char *)nextof(hd))
			p=type_udfline(p);
		output("endfunction\n");
	}
	else
	{	output("No such function!\n"); error=173;
	}
}

static void do_help (void)
{	char name[16];
	header *hd;
	int count,i,defaults;
	char *p,*end,*pnote;
	scan_space();
	scan_name(name); hd=searchudf(name);
	if (hd && hd->type==s_udf)
	{	output1("function %s (",name);
		end=udfof(hd);
		p=helpof(hd);
		count=*((int *)p);
		p+=sizeof(int);
		pnote=p;
		for (i=0; i<count; i++)
		{	defaults=*(int *)p; p+=sizeof(int);
			output1("%s",p);
			p+=16+sizeof(int);
			if (defaults)
			{	output("=...");
				p=(char *)nextof((header *)p);
			}
			if (i!=count-1) output(",");
		}
		output(")\n");
		p=pnote;
		for (i=0; i<count; i++)
		{	defaults=*(int *)p; p+=sizeof(int);
			if (defaults) output1("## Default for %s :\n",p);
			p+=16+sizeof(int);
			if (defaults)
			{	give_out((header *)p);
				p=(char *)nextof((header *)p);
			}
		}		
		while (*p!=1 && p<end)
		{	output(p); output("\n");
			p+=strlen(p); p++;
		}
	}
	else
	{	output("\n\n Help needs a function name ,e.g.:"
			"\n >help shortformat\n"
			" You can get a list of all functions with\n >list\n\n"
			" If you need online help for builtin functions enter:\n"
			" >load \"help\""
			"\n\n To run a demo use:\n >load \"demo\"\n >demo()\n"
			"\n >quit\n quits this program.\n\n");
	}
}

static void do_dump (void)
{	header *file;
	if (outfile)
	{	if (fclose(outfile))
		{	output("Error while closing dumpfile.\n");
		}
		outfile=0;
	}
	scan_space();
	if (*next==';' || *next==',' || *next==0)
	{	if (*next) next++; return; }
	file=scan_value();
	if (error || file->type!=s_string)
	{	output("Dump needs a filename!\n");
		error=201; return;
	}
	outfile=fopen(stringof(file),"a");
	if (!outfile)
	{	output1("Could not open %s.\n",stringof(file));
	}
}

static void do_meta (void)
{	header *file;
	if (metafile)
	{	if (fclose(metafile))
		{	output("Error while closing metafile.\n");
		}
		metafile=0;
	}
	scan_space();
	if (*next==';' || *next==',' || *next==0)
	{	if (*next) next++; return; }
	file=scan_value();
	if (error || file->type!=s_string)
	{	output("Meta needs a filename!\n");
		error=201; return;
	}
	metafile=fopen(stringof(file),"ab");
	if (!metafile)
	{	output1("Could not open %s.\n",stringof(file));
	}
}

static void do_remove (void)
{	header *file;
	file=scan_value();
	if (error || file->type!=s_string)
	{	output("Remove needs a string!\n");
		error=202; return;
	}
	remove(stringof(file));
}

static void do_do (void)
{	int udfold;
	char name[16];
	char *oldnext=next,*udflineold;
	header *var;
	scan_space(); scan_name(name); if (error) return;
	var=searchudf(name);
	if (!var || var->type!=s_udf)
	{	output("Need a udf!\n"); error=220; return;
	}
	udflineold=udfline; udfline=next=udfof(var); udfold=udfon; udfon=1;
	while (!error && udfon==1)
	{	command();
		if (udfon==2) break;
		if (test_key()==escape) 
		{	output("User interrupted!\n"); error=58; break; 
		}
	}
	if (error) output1("Error in function %s\n",var->name);
	if (udfon==0)
	{	output1("Return missing in %s!\n",var->name); error=55; }
	udfon=udfold; udfline=udflineold;
	if (udfon) next=oldnext;
	else { next=line; *next=0; }
}

static void do_mdump (void)
{	header *hd;
	output1("ramstart : 0\nstartlocal : %ld\n",startlocal-ramstart);
	output1("endlocal : %ld\n",endlocal-ramstart);
	output1("newram   : %ld\n",newram-ramstart);
	output1("ramend   : %ld\n",ramend-ramstart);
	hd=(header *)ramstart;
	while ((char *)hd<newram)
	{
		output1("%6ld : %16s, ",(char *)hd-ramstart,hd->name);
		output1("size %6ld ",(long)hd->size);
		output1("type %d\n",hd->type);
		hd=nextof(hd);
	}
}

static void hex_out1 (int n)
{	if (n<10) output1("%c",n+'0');
	else output1("%c",n-10+'A');
}

static void hex_out (unsigned int n)
{	hex_out1(n/16);
	hex_out1(n%16);
	output(" ");
}

static void string_out (unsigned char *p)
{	int i;
	unsigned char a;
	for (i=0; i<16; i++) 
	{	a=*p++;
		output1("%c",(a<' ')?'_':a);
	}
}

static void do_hexdump (void)
{	char name[16];
	unsigned char *p,*end;
	int i=0,j;
	size_t count=0;
	header *hd;
	scan_space(); scan_name(name); if (error) return;
	hd=searchvar(name);
	if (!hd) hd=searchudf(name);
	if (error || hd==0) return;
	p=(unsigned char *)hd; end=p+hd->size;
	output1("\n%5lx: ",count);
	while (p<end)
	{	hex_out(*p++); i++; count++;
		if (i>=16) 
		{	i=0; string_out(p-16);
			output1("\n%5lx: ",count);
			if (test_key()==escape) break;
		}
	}
	for (j=i; j<16; j++) output("   ");
	string_out(p-i);
	output("\n");
}

static void do_output (void)
/**** do_output
	toggles output.
****/
{	scan_space();
	if (!strncmp(next,"off",3))
	{	outputing=0; next+=3;
	}
	else if (!strncmp(next,"on",2))
	{	outputing=1; output("\n"); next+=2;
	}
	else outputing=!outputing;
}

static void do_comment (void)
{	FILE *fp=infile;
	if (!fp || udfon)
	{	output("comment illegal at this place");
		error=1001; return;
	}
	while (strncmp(next,"endcomment",10)!=0)
	{	next_line();
		if (infile!=fp)
		{	output("endcomment missing!\n"); error=1002;
			return;
		}
	}
	next_line();
}

static void do_quit (void)
{	quit=1;
}

static void do_exec (void)
{	header *name;
	char *s;
	name=scan_value(); if (error) return;
	if (name->type!=s_string)
	{	output("Cannot execute a number or matrix!\n");
		error=130; return;
	}
	s=stringof(name);
	while (*s && !isspace(*s)) s++;
	if (*s) *s++=0;
	if (execute(stringof(name),s))
	{	output("Execution failed or program returned a failure!\n");
		error=131;
	}
}

static int command_count;

static commandtyp command_list[] = {
	{"quit",c_quit,do_quit},
	{"hold",c_hold,ghold},
	{"shg",c_shg,show_graphics},
	{"load",c_load,load_file},
	{"function",c_udf,get_udf},
	{"return",c_return,do_return},
	{"for",c_for,do_for},
	{"endif",c_endif,do_endif},
	{"end",c_end,do_end},
	{"break",c_break,do_break},
	{"loop",c_loop,do_loop},
	{"else",c_else,do_else},
	{"elseif",c_elseif,do_elseif},
	{"if",c_if,do_if},
	{"repeat",c_repeat,do_repeat},
	{"clear",c_clear,do_clear},
	{"clg",c_clg,do_clg},
	{"cls",c_cls,do_cls},
	{"exec",c_exec,do_exec},
	{"forget",c_forget,do_forget},
	{"global",c_global,do_global},
	{"list",c_global,do_list},
	{"listvar",c_global,do_listvar},
	{"type",c_global,do_type},
	{"dump",c_global,do_dump},
	{"remove",c_global,do_remove},
	{"help",c_global,do_help},
	{"do",c_global,do_do},
	{"memorydump",c_global,do_mdump},
	{"hexdump",c_global,do_hexdump},
	{"output",c_global,do_output},
	{"meta",c_global,do_meta},
	{"comment",c_global,do_comment},
	{"trace",c_global,do_trace},
	{0,0,0}
};

static void print_commands (void)
{	int i, c, cend, lw=linelength/MAXNAME;
	
	for (i=0; i<command_count; i+=lw) {
		cend = i+lw;
		if (cend>=command_count) cend=command_count;
		for (c=i; c<cend ; c++) {
			output1("%-16s",command_list[c].name);
			if (test_key()==escape) return;
		}
		output("\n");
	}
	output("\n");
}

static int command_compare (const commandtyp *p1, const commandtyp *p2)
{	return strcmp(p1->name,p2->name);
}

static void sort_command (void)
{	command_count=0;
	while (command_list[command_count].name) command_count++;
	qsort(command_list,command_count,sizeof(commandtyp),
		(int (*)(const void *, const void *))command_compare);
}

static commandtyp *preview_command (size_t *l)
{	commandtyp h;
	char name[MAXNAME],*a,*n;
	*l=0;
	a=next; n=name;
	while (*l<15 && isalpha(*a)) { *n++=*a++; *l+=1; }
	*n++=0; if (isalpha(*a)) return 0;
	h.name=name;
	return bsearch(&h,command_list,command_count,sizeof(commandtyp),
		(int (*)(const void *, const void *))command_compare);
}

static int builtin (void)
/***** builtin
	interpret a builtin command, number no.
*****/
{	size_t l;
	commandtyp *p;
	if (*next==3)
	{	next++;
#ifdef SPECIAL_ALIGNMENT
		memmove((char *)(&p),next,sizeof(commandtyp *));
#else
		p=*((commandtyp **)next);
#endif
		l=sizeof(commandtyp *);
	}
	else if (udfon) return 0;
	else p=preview_command(&l);
	if (p)
	{	next+=l;
		p->f();
		if (*next==';' || *next==',') next++;
		commandtype=p->nr;
		return 1;
	}
	return 0;
}

/***************** scanning ***************************/
void scan_space (void)
{	start: while (*next==' ' || *next==TAB) next++;
	if (!udfon && *next=='.' && *(next+1)=='.')
		{	next_line(); if (error) return; goto start; }
}

static void do_end (void);
static void do_loop (void);
static void do_repeat (void);
static void do_for (void);

static void scan_end (void)
/***** scan_end
	scan for "end".
*****/
{	commandtyp *com;
	char *oldline=udfline;
	while (1)
	{	switch (*next)
		{	case 1 : 
				output("End missing!\n");
				error=110; udfline=oldline; return;
			case 0 : udfline=next+1; next++; break;
			case 2 : next+=1+sizeof(double); break;
			case 3 : next++; 
				memmove((char *)(&com),next,sizeof(commandtyp *));
				next+=sizeof(commandtyp *);
				if (com->f==do_end)
				{	if (trace>0) trace_udfline(udfline);
					return;
				}
				else if (com->f==do_repeat || com->f==do_loop ||
					com->f==do_for)
				{	scan_end(); if (error) return; }
				break;
			default : next++;
		}
	}
}

static void scan_endif (void)
/***** scan_endif
	scan for "endif".
*****/
{	commandtyp *com;
	char *oldline=udfline;
	while (1)
	{	switch (*next)
		{	case 1 : 
				output("Endif missing, searching for endif!\n");
				error=110; udfline=oldline; return;
			case 0 : udfline=next+1; next++; break;
			case 2 : next+=1+sizeof(double); break;
			case 3 : next++; 
				memmove((char *)(&com),next,sizeof(commandtyp *));
				next+=sizeof(commandtyp *);
				if (com->f==do_endif)
				{	if (trace>0) trace_udfline(udfline);
					return;
				}
				else if (com->f==do_if)
				{	scan_endif(); if (error) return; }
				break;
			default : next++;
		}
	}
}

static void scan_else (void)
/***** scan_else
	scan for "else".
*****/
{	commandtyp *com;
	char *oldline=udfline;
	while (1)
	{	switch (*next)
		{	case 1 : 
				output("Endif missing, searching for else!\n");
				error=110; udfline=oldline; return;
			case 0 : udfline=next+1; next++; break;
			case 2 : next+=1+sizeof(double); break;
			case 3 : next++; 
				memmove((char *)(&com),next,sizeof(commandtyp *));
				next+=sizeof(commandtyp *);
				if (com->f==do_endif || com->f==do_else)
				{	if (trace>0) trace_udfline(udfline);
					return;
				}
				else if (com->f==do_if)
				{	scan_endif(); if (error) return; }
				break;
			default : next++;
		}
	}
}

static void scan_filename (char *name, int lmax)
{	int count=0;
	if (*next=='\"')
	{	next++;
		while (*next!='\"' && *next)
		{	*name++=*next++; count++;
			if (count>=lmax-1)
			{	output("Name too long!\n");
				error=11; break;
			}
		}
		if (*next=='\"') next++;
		else
		{
			output("\" missing\n");
			error=11;
		}
	}
	else if (!isalpha(*next) && *next!='_' && *next!=PATH_DELIM_CHAR)
	{   output1("Valid file name or path expected at:\n%s\n",next);
		error=11; *name=0; return;
	}
	else
	{	if (*next=='_') { *name++=*next++; count++; }
		while (isalpha(*next) || isdigit(*next) || *next==PATH_DELIM_CHAR || *next=='-' || *next=='.')
		{	*name++=*next++; count++;
			if (count>=lmax-1)
			{	output("Name too long!\n");
				error=11; break;
			}
		}
	}
	*name=0;
}

static void scan_name (char *name)
{	int count=0;
	if (!isalpha(*next))
	{	error=11; *name=0; return;
	}
	while (isalpha(*next) || isdigit(*next))
	{	*name++=*next++; count++;
		if (count>=15)
		{	output("Name too long!\n");
			error=11; break;
		}
	}
	*name=0;
}

void copy_complex (double *x, double *y)
{	*x++=*y++;
	*x=*y;
}

static int scan_arguments (void)
/* look ahead for arguments */
{	int count=0,olds=nosubmref,nocount=0;
	header *hd,*hdold;
	nosubmref=1;
	while (1)
	{	scan_space();
		if (*next==')' || *next==']') break;
		if (*next==',')
			hd=new_reference(0,"");
		else
		{	hd=scan(); scan_space();
		}
		if (*next=='=')
		{	next++;
			hdold=(header *)newram;
			scan_value();
			if (!error)
			{	strcpy(hdold->name,hd->name);
				hdold->xor=hd->xor;
				moveresult(hd,hdold);
				nocount=1;
			}
		}
		else if (nocount)
		{	output("Illegal parameter after named parameter!\n");
			error=2700;
		}
		if (error)
		{	nosubmref=olds;
			return 0;
		}
		while (hd<(header *)newram)
		{	if (!nocount) count++;
			hd=nextof(hd);
		}
		if (count>=10)
		{	output("Too many arguments!\n"); error=56; return 0; }
		if (*next!=',') break;
		next++;
	}
	if (*next!=')' && *next!=']')
	{	output("Error in arguments!\n"); error=19; return 0; }
	next++;
	nosubmref=olds;
	return count;
}

static void scan_matrix (void)
/***** scan_matrix
	scan a matrix from input.
	form: [x y z ... ; v w u ...],
	where x,y,z,u,v,w are sums.
*****/
{	header *hd,*result;
	dims *d;
	int c,r,count,i,j,complex=0;
	size_t ic;
	size_t allcount;
	double *m,*ms,cnull[2]={0,0};
	hd=new_matrix(0,0,""); /* don't know how big it will be! */
	if (error) return;
	count=0;
	getmatrix(hd,&r,&c,&m); ms=m;
	r=0; c=0;
	scan_space();
	while (1)
	{	scan_space();
		if (*next==0) { next_line(); scan_space(); }
			/* matrix is allowed to pass line ends */
		if (*next==';' || *next==']') 
			/* new column starts */
		{	if (*next==';') next++;
			if ((char *)(ms+count*(r+1))>=ramend)
			{	output("Memory overflow!\n"); error=18; return;
			}
			if (count>c) /* this column is too long */
			{	if (r>0) /* expand matrix */
				{	for (j=count-1; j>=0; j--)
					{	if (complex) copy_complex(cmat(ms,count,r,j),
							cmat(ms,c,r,j));
						else *mat(ms,count,r,j)=*mat(ms,c,r,j);
					}
					for (i=r-1; i>=0; i--)
					{	if (i>0)
						for (j=c-1; j>=0; j--)
						{	if (complex) 
								copy_complex(cmat(ms,count,i,j),
									cmat(ms,c,i,j));
							else *mat(ms,count,i,j)=*mat(ms,c,i,j);
						}
						for (j=c; j<count; j++)
						if (complex) copy_complex(cmat(ms,count,i,j),
							cnull);
						else *mat(ms,count,i,j)=0.0;
					}
				}
				c=count;
			}
			else if (count<c)
			{	for (j=count; j<c; j++)
					if (complex) copy_complex(cmat(ms,c,r,j),
							cnull);
					else *mat(ms,c,r,j)=0.0;
			}
            r++; newram=(char *)(ms+(complex?2l:1l)*(size_t)c*r);
			m=(double *)newram;
			count=0;
		}
		if (*next==']') break;
		if (*next==',') next++;
		if (*next==0) next_line();
		result=scan_value(); if (error) return;
		newram=(char *)result;
		if (!complex && result->type==s_complex)
		{	complex=1;
			/* make matrix complex up to now (oh boy!) */
			allcount=((char *)m-(char *)ms)/sizeof(double);
			if (newram+allcount*sizeof(double)+result->size>ramend)
			{	output("Memory overflow!\n"); error=16; return;
			}
			if (allcount)
			{	memmove(newram+allcount*sizeof(double),newram,result->size);
				result=(header *)((char *)result+allcount*sizeof(double));
				for (ic=allcount-1; ic>0; ic--)
                {   *(ms+(size_t)2*ic)=*(ms+ic); 
                	*(ms+(size_t)2*ic+1)=0.0;
				}
				*(ms+1)=0.0;
            newram=(char *)(ms+(size_t)2*allcount); m=(double *)newram;
			}
			hd->type=s_cmatrix;
		}
		else if (result->type==s_real);
		else if (result->type==s_complex && complex);
		else
		{	error=-1; output("Illegal vector!\n"); return;
		}
		*m++=*realof(result); count++;
		if (complex)
		{	if (result->type==s_complex) *m++=*imagof(result);
			else *m++=0.0;
		}
		if (count>=INT_MAX)
		{	output1("Matrix has more than %d columns!\n",INT_MAX);
			error=17; return;
		}
		newram=(char *)m;
		if (newram>=ramend) 
		{	output("Memory overflow!\n"); error=16; return; 
		}
	}
	next++;
	d=(dims *)(hd+1);
	if (c==0) r=0;
	d->c=c; d->r=r;
	if (r>=INT_MAX)
	{	output1("Matrix has more than %d rows!\n",INT_MAX);
		error=18; return;
	}
	hd->size=complex?cmatrixsize(c,r):matrixsize(c,r);
	newram=(char *)hd+hd->size;
}

static void scan_elementary (void)
/***** scan_elemtary
	scan an elementary expression, like a value or variable.
	scans also (...).
*****/
{	double x;
	int n,nargs=0,hadargs=0;
	header *hd=(header *)newram,*var;
	char name[MAXNAME],*s;
	scan_space();
	if ((*next=='.' && isdigit(*(next+1))) || isdigit(*next))
	{	sscanf(next,"%lf%n",&x,&n);
		next+=n;
		if (*next=='i') /* complex number! */
		{	next++;
			new_complex(0,x,"");
		}
		else new_real(x,"");
	}
	else if (*next==2) /* a double stored in binary form */
	{	next++;
#ifdef SPECIAL_ALIGNMENT
		memmove((char *)(&x),next,sizeof(double));
#else
		x=*((double *)next);
#endif
		next+=sizeof(double);
		if (*next=='i') /* complex number! */
		{	next++;
			new_complex(0,x,"");
		}
		else new_real(x,"");
	}
	else if (*next==3)
	{	output("Command name used as variable!\n");
		error=4000; return;
	}
	else if (isalpha(*next))
	{	scan_name(name); if (error) return;
		scan_space(); nargs=0;
		if (*next=='{')
		{	next++; scan(); if (error) return; scan_space();
			if (*next!='}')
			{	output("} missing!\n"); error=1010; return;
			}
			next++;
			get_element1(name,hd);
			goto after;
		}
		if (*next=='(' || *next=='[') /* arguments or indices */
		{	hadargs=(*next=='[')?2:1;
			next++; nargs=scan_arguments();
			if (error) return;
		}
		if (hadargs==1 && exec_builtin(name,nargs,hd));
		else
		{	if (hadargs==2) var=searchvar(name);
			else if (hadargs==1)
			{	var=searchudf(name);
				if (!var) var=searchvar(name);
			}
			else var=searchvar(name);
			if (var && var->type==s_udf && hadargs==1)
			{	interpret_udf(var,hd,nargs); if (error) return;
			}
			else if (!var && hadargs)
			{	error=24;
				if (hadargs==2)
				output1("%s no variable!\n",name);
				else
				output1(
			"%s no function or variable, or wrong argument number!\n",
			name);
				return;
			}
			else if (var && hadargs)
			{	get_element(nargs,var,hd);
			}
			else hd=new_reference(var,name);
		}
	}
	else if (*next=='#' && *(next+1)!='#')
	{	next++; mindex(hd);
	}
	else if (*next=='+')
	{	next++; scan_elementary();
	}
	else if (*next=='-')
	{	next++; scan_elementary();
		if (!error) invert(hd);
	}
	else if (*next=='(')
	{	next++;
		scan(); if (error) return;
		scan_space();
		if (*next!=')')
		{	output("Closing bracket ) missing!\n");
			error=5; return;
		}
		newram=(char *)nextof(hd);
		next++;
	}
	else if (*next=='[')
	{	next++;
		scan_matrix();
	}
	else if (*next=='\"')
	{	next++; s=next; 
		while (*next!='\"' && *next!=0) next++;
		hd=new_string(s,next-s,"");
		if (*next!='\"') { output("\" missing\n"); error=1; return; }
		next++;
	}
	else error=1;
	after: if (error) return;
	/* for things, that come after an elementary expression */
	scan_space();
	if (*next=='\'') { next++; transpose(hd); }
	else if (*next=='^' || (*next=='*' && *(next+1)=='*'))
	{	if (*next=='^') next++; 
		else next+=2;
		newram=(char *)nextof(hd);
		scan_elementary();
		if (error) return;
		mpower(hd);
	}
}

static void scan_factor (void)
{	scan_elementary();
}

static void scan_summand (void)
{	header *hd=(header *)newram,*hd1;
	scan_space();
	scan_factor();
	if (error) return;
	while (1)
	{	hd1=(header *)newram;
		scan_space();
		if ((*next=='*' && *(next+1)!='*') 
				|| (*next=='.' && *(next+1)=='*'))
		{	if (*next=='*') next++;
			else next+=2;
			scan_factor();
			if (!error) dotmultiply(hd,hd1);
		}
		else if (*next=='/' || (*next=='.' && *(next+1)=='/'))
		{	if (*next=='/') next++;
			else next+=2;
			scan_factor();
			if (!error) dotdivide(hd,hd1);
		}
		else if (*next=='.') 
		{	next++;
			scan_factor();
			if (!error) multiply(hd,hd1);
		}
		else if (*next=='\\')
		{	next++;
			newram=(char *)nextof(hd);
			scan_factor();
			if (!error) msolve(hd);
		}
		else break;
		if (error) break;
	}
}

static void scan_summe (void)
{	header *hd=(header *)newram,*hd1;
	scan_space();
	scan_summand();
	if (error) return;
	while (1)
	{	hd1=(header *)newram;
		scan_space();
		if (*next=='+')
		{	next++;
			scan_summand();
			if (!error) add(hd,hd1);
		}
		else if (*next=='-')
		{	next++;
			scan_summand();
			if (!error) subtract(hd,hd1);
		}
		else break;
		if (error) break;
	}
}

static void scan_dp (void)
{	header *hd=(header *)newram,*hd1,*hd2;
	scan_space();
	if (*next==':')
	{	next++;
		new_command(c_allv);
		return;
	}
	scan_summe();
	if (*next==':') /* a vector a:b:c or a:c */
	{	next++;
		hd1=(header *)newram; scan_summe();
		if (error) return;
		scan_space();
		if (*next==':')
		{	next++; hd2=(header *)newram; 
			scan_summe(); if (error) return;
		}
		else
		{	hd2=hd1; hd1=new_real(1.0,"");
		}
		if (error) return;
		vectorize(hd,hd1,hd2);
	}
}

static void scan_compare (void)
{	header *hd=(header *)newram;
	scan_space();
	if (*next=='!')
	{	next++; scan_compare(); mnot(hd); return;
	}
	scan_dp(); if (error) return;
	scan_space();
	if (*next=='<')
	{	next++;
		newram=(char *)nextof(hd);
		if (*next=='=')
		{	next++; scan_dp(); if (error) return; mlesseq(hd); return;
		}
		else if (*next=='>')
		{	next++; scan_dp(); if (error) return; munequal(hd); return;
		}
		scan_dp(); if (error) return;
		mless(hd);
	}
	else if (*next=='>')
	{	next++; 
		newram=(char *)nextof(hd);
		if (*next=='=')
		{	next++; scan_dp(); if (error) return; mgreatereq(hd); return;
		}
		scan_dp(); if (error) return;
		mgreater(hd);
	}
	else if (*next=='=' && *(next+1)=='=')
	{	next+=2;
		newram=(char *)nextof(hd);
		scan_dp(); if (error) return;
		mequal(hd);
	}
	else if (*next=='~' && *(next+1)=='=')
	{	next+=2; 
		newram=(char *)nextof(hd);
		scan_dp(); if (error) return;
		maboutequal(hd);
	}
	else if (*next=='!' && *(next+1)=='=')
	{	next+=2; 
		newram=(char *)nextof(hd);
		scan_dp(); if (error) return;
		munequal(hd);
	}
	else if (*next=='_')
	{	next++;
		newram=(char *)nextof(hd);
		scan_compare(); if (error) return;
		mvconcat(hd);
	}
	else if (*next=='|' && *(next+1)!='|')
	{	next++;
		newram=(char *)nextof(hd); 
		scan_compare(); if (error) return;
		mhconcat(hd);
	}
}

static void scan_logical (void)
{	header *hd=(header *)newram;
	scan_compare(); if (error) return;
	scan_space();
	if (*next=='|' && *(next+1)=='|')
	{	next+=2; 
		newram=(char *)nextof(hd);
		scan_compare(); if (error) return;
		mor(hd);
	}
	else if (*next=='&' && *(next+1)=='&')
	{	next+=2; 
		newram=(char *)nextof(hd);
		scan_compare(); if (error) return;
		mand(hd);
	}
}

header *scan (void)
{	header *result=(header *)newram;
	scan_space();
	if (*next=='{')
	{	next++; scan_logical(); if (error) return result;
		while (1)
		{	scan_space();
			if (*next=='}') { next++; return result; }
			if (*next!=',')
			{	output("Error in {}!\n"); error=104; return result;
			}
			next++; scan_logical();
			if (error) return result;
		}
	}
	else
	{	scan_logical();
	}
	return result;
}

header *scan_value (void)
{	header *result=(header *)newram,*hd,*hd1,*marker,*nextresult,
		*endresults;
	int oldnosubmref;
	size_t size;
	scan_space();
	if (*next=='{')
	{	next++; 
		oldnosubmref=nosubmref; nosubmref=1; 
		scan_logical(); nosubmref=oldnosubmref;
		hd1=getvalue(result);
		if (error) return result;
		moveresult(result,hd1);
		while (1)
		{	scan_space();
			if (*next=='}') { next++; return result; }
			if (*next!=',')
			{	output("Error in {}!\n"); error=104; return result;
			}
			next++; hd=(header *)newram; scan_value();
			if (error) return result;
			hd1=getvalue(hd); if (error) return result;
			moveresult(hd,hd1);
		}
	}
	else
	{	scan_logical();
		marker=result;
		endresults=(header *)newram;
		while (marker<endresults)
		{	hd1=getvalue(marker);
			if (error) return result;
			nextresult=nextof(marker);
			if (hd1!=marker)
			{	if (nextresult==endresults)
				{	memmove((char *)marker,(char *)hd1,hd1->size);
					newram=(char *)nextof(marker);
					break;
				}
				size=hd1->size-marker->size;
				memmove((char *)nextresult+size,(char *)nextresult,
					newram-(char *)nextresult);
				if (hd1>nextresult) 
					hd1=(header *)((char *)hd1+size);
				nextresult=(header *)((char *)nextresult+size);
				endresults=(header *)((char *)endresults+size);
				memmove((char *)marker,(char *)hd1,hd1->size);
				newram=(char *)endresults;
			}
			marker=nextresult;
		}
	}
	return result;
}

static header *scan_expression (void)
/***** scan_expression
	scans a variable, a value or a builtin command.
*****/
{	if (builtin()) return &commandheader;
	return scan();
}

#define addsize(hd,size) ((header *)((char *)(hd)+size))

static void do_assignment (header *var)
/***** do_assignment
	assign a value to a variable.
*****/
{	header *variable[8],*rightside[8],*rs,*v,*mark;
	int rscount,varcount,i,j;
	size_t offset,oldoffset,dif;
	char *oldendlocal;
	scan_space();
	if (*next=='=')
	{	next++;
		nosubmref=1; rs=scan_value(); nosubmref=0;
		if (error) return;
		varcount=0;
		/* count the variables, that get assigned something */
		while (var<rs)
		{	if (var->type!=s_reference && var->type!=s_submatrix
				&& var->type!=s_csubmatrix)
			{	output("Illegal assignment!\n");
				error=210;
			}
			variable[varcount]=var; var=nextof(var); varcount++;
			if (varcount>=8)
			{	output("To many commas!\n"); error=100; return;
			}
		}
		/* count and note the values, that are assigned to the
			variables */
		rscount=0;
		while (rs<(header *)newram)
		{	rightside[rscount]=rs;
			rs=nextof(rs); rscount++;
			if (rscount>=8)
			{	output("To many commas!\n"); error=101; return;
			}
		}
		/* cannot assign 2 values to 3 variables , e.g. */
		if (rscount>1 && rscount<varcount)
		{	output("Illegal assignment!\n"); error=102; return;
		}
		oldendlocal=endlocal;
		offset=0;
		/* do all the assignments */
		if (varcount==1) var=assign(variable[0],rightside[0]);
		else
		for (i=0; i<varcount; i++)
		{	oldoffset=offset;
			/* assign a variable */
			var=assign(addsize(variable[i],offset),
				addsize(rightside[(rscount>1)?i:0],offset));
			offset=endlocal-oldendlocal;
			if (oldoffset!=offset) /* size of var. changed */
			{	v=addsize(variable[i],offset);
				if (v->type==s_reference) mark=referenceof(v);
				else mark=submrefof(v);
				/* now shift all references of the var.s */
				if (mark) /* not a new variable */
					for (j=i+1; j<varcount; j++)
					{	v=addsize(variable[j],offset);
						dif=offset-oldoffset;
						if (v->type==s_reference && referenceof(v)>mark)
							referenceof(v)=addsize(referenceof(v),dif);
						else if (submrefof(v)>mark)
							submrefof(v)=addsize(submrefof(v),dif);
					}
			}
		}
	}
	else /* just an expression which is a variable */
	{	var=getvalue(var);
	}
	if (error) return;
	if (*next!=';') give_out(var);
	if (*next==',' || *next==' ' || *next==';') next++;
}

int command (void)
/***** command
	scan a command and interpret it.
	return, if the user wants to quit.
*****/
{	header *expr;
	int ret=c_none;
	quit=0; error=0; errorout=0;
	while(1)
	{	scan_space();
		if (*next) break;
		else next_line();
	}
	if (*next==1) return ret;
	expr=scan_expression();
	if (!expr) { newram=endlocal; return ret; }
	if (error) 
	{	newram=endlocal; 
		print_error(next); 
		next=line; line[0]=0; 
		return ret; 
	}
	if (expr==&commandheader)
	{	newram=endlocal;
		return commandtype;
	}
	switch (expr->type)
	{	case s_real :
		case s_complex :
		case s_matrix :
		case s_cmatrix :
		case s_string :
			if (*next!=';') give_out(expr);
			if (*next==',' || *next==' ' || *next==';') next++;
			break;
		case s_reference :
		case s_submatrix :
		case s_csubmatrix :
			do_assignment(expr);
			break;
		default : break;
	}
	if (error) print_error(next);
	newram=endlocal;
	if (error) { next=line; line[0]=0; }
	return ret;
}

/******************* main functions ************************/

void main_loop (int argc, char *argv[])
{	int i;
	newram=startlocal=endlocal=ramstart;
	udfend=ramstart;
	epsilon=10000*DBL_EPSILON;
	sort_builtin(); sort_command(); make_xors(); clear_fktext();
	next=line;		/* clear input line */
	strcpy(line,"load \"retro.cfg\";");
	for (i=1; i<argc; i++)
	{	strcat(line," load \"");
		strcat(line,argv[i]);
		strcat(line,"\";");
	}
	while (!quit)
	{	/* reset global context for commands evaluated in the 
	       lower level context */
		startglobal=startlocal;
		endglobal=endlocal;
		command();	/* interpret until "quit" */
		if (trace<0) trace=0;
	}
}

