#ifndef _HEADER_H_
#define _HEADER_H_

#include <stdio.h>

#include "sysdep.h"
#include "core.h"

#define MAXLINE		1024		/* Maximum input line length */
#define	MAXHIST		32			/* Maximum entries in editing history */

/* Format string for */
#define XSTR(x) #x
#define STR(x) XSTR(x)
#define OUTFMT "%-" STR(MAXNAME) "s"

extern int disp_mode;
extern int disp_digits;
extern int disp_fieldw;
extern int disp_eng_sym;
extern int hchar,wchar;
extern double maxexpo,minexpo;
extern char expoformat[],fixedformat[];

extern FILE *infile,*outfile;
void output(char *s);
void output1(char *fmt, ...);


extern double epsilon;

/* commands */
typedef enum {
	c_none, c_allv, c_quit, c_hold, 
	c_shg, c_load, c_udf, c_return, c_for, c_end, c_break,
	c_loop, c_if, c_repeat, c_endif, c_else, c_elseif,
	c_clear, c_clg, c_cls, c_exec, c_forget, c_global
} comtyp;

typedef struct {
	char*	name;
	comtyp	nr;
	void 	(*f)(void);
} commandtyp;

/* edit.c */
scantyp edit (char *s);
void prompt (void);

/* mainloop.c */
extern int nosubmref,trace;

extern char *argname[];
extern char xors[];

void print_error (char *p);
#define wrong_arg(x) { error=26; output1("Wrong argument: %s\n", (x)); return; }

void main_loop (int argc, char *argv[]);

void scan_space (void);
void trace_udfline(char *);

void give_out (header *hd);

int command (void);

/* express.c */

header *scan(void);
header *scan_value(void);
void copy_complex (double *, double *);

/* several */

void make_xors (void);
void clear_fktext (void);

extern FILE *metafile;

#endif
