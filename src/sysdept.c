/* Primitive user interface for Retro.
Missing are:
	- graphics
	- single keystrokes (e.g. for tracing)
	- extended command line editing and command history
	- wait does not work, but waits for a keystroke
*/

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>		/* chdir and getcwd */
#include <errno.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <termios.h>

#ifdef RS6000
#include <sys/time.h>
void gettimer (int, struct timestruc_t *);
#endif

#define TERMIO

char outputs[256];

#include "header.h"
#include "sysdep.h"

long size=512*1024l; /* default stack size in Byte */

int linelength=80; /* width of text screen */
int wchar,hchar; /* font metrics of graphic text */
static int editing=0;
static int quiet=0;		/* quiet mode (no input echoed) */


int fillcolor1=3,fillcolor2=11,gscreenlines=40;
int markerfactor;

int cred[16]=  {255,0,100,0  ,0  ,0  ,100,150,100,50,220 ,80  ,80  ,80  ,140 ,190};
int cgreen[16]={255,0,0  ,100,0  ,100,100,150,100,50,80  ,220 ,80  ,140 ,140 ,190};
int cblue[16]= {255,0,0  ,0  ,100,100,  0,150,100,50,80  ,80  ,220 ,140 , 80,190};

double getcolor (int i, int j)
{	switch (j)
	{	case 0 : return cred[i]/256.0;
    	case 1 : return cgreen[i]/256.0;
        case 2 : return cblue[i]/256.0;
    }
    return 0;
}

int usecolors=1;

int memory_init (void)
/***** memory_init
	get memory for stack.
*****/
{	ramstart=(char *)malloc(size);
	if (!ramstart) return 0;
	ramend=ramstart+size;
	return 1;
}


/******************** writing the meta file ************/

void longwrite (double n)
/***** write a double to the metafile as long
*****/
{	long k[1];
	k[0]=(long)(n*1000.0);
	if (metafile) fwrite(k,sizeof(long),1,metafile);
}

void intwrite (int n)
/***** write an int to the metafile
*****/
{	int k[1];
	k[0]=n;
	if (metafile) fwrite(k,sizeof(int),1,metafile);
}

void stringwrite (char *s)
/***** write a string to the metafile
*****/
{	int c;
	if (metafile)
	{	c=0;
		while (*s) { putc(*s++,metafile); c++; }
		putc(0,metafile); c++;
		if (c%2) putc(0,metafile);
	}
}

/******************* Graphics ***************************/

/****
The graphic screen has coordinates from 0.0 to 1024.0 (double).
There should be a function, which computes the correct screen
coordinates from these internal ones.
****/

void clear_graphics (void)
/***** clear the graphics screen 
*****/
{	if (metafile) intwrite(6);
}

void gsubplot(int r, int c, int index)
{

}

void gline (double c, double r, double c1, double r1, int col,
	int st, int width)
/***** gline
	draw a line.
	col is the color, where 0 should be white and 1 black.
	st is a style from linetyp.
	width is the linewidth, where 0 or 1 are equal defaults.
*****/
{	if (fabs(r)>10000.0) return;
	if (fabs(c)>10000.0) return;
	if (fabs(r1)>10000.0) return;
	if (fabs(c1)>10000.0) return;
	if (metafile)
	{	intwrite(1);
		longwrite(c); longwrite(r); longwrite(c1); longwrite(r1);
		intwrite(col); intwrite(st); intwrite(width);
	}
}

void gmarker (double c, double r, int col, int type)
/***** gmarker
	plot a single marker on screen.
	col is the color.
	type is a type from markertyp.
*****/
{	if (metafile)
	{	intwrite(2);
		longwrite(c); longwrite(r);
		intwrite(col); intwrite(type);
	}
}

void gfill (double c[], int st, int n, int connect[])
/***** gfill
	fill an area given by n pairs of points (in c: x,y,x,y,...)
	with the style st from filltyp.
	connect pairs of points indicated in connect by a black line.
*****/
{	int i;
	for (i=0; i<2*n; i++) if (fabs(c[i])>10000.0) return;
	if (metafile)
	{	intwrite(3);
		for (i=0; i<2*n; i+=2)
		{	longwrite(c[i]); longwrite(c[i+1]);
			intwrite(connect[i]);
		}
		intwrite(st);
	}
}

void gtext (double c, double r, char *text, int color, int alignment)
/***** gtext
	output a graphic text on screen.
	alignment is left=0, centered=1, right=2.
*****/
{	if (metafile)
	{	intwrite(4); longwrite(c);
		longwrite(r); intwrite(color); intwrite(alignment);
		stringwrite(text);
	}
}

void scale (double s)
/***** scale
	scale the screen according s = true width / true height.
	This is not necessary on a window based system.
*****/
{	if (metafile)
	{	intwrite(5);
		longwrite((int)(s*1000));
	}
}

void gbar (double c, double r, double c1, double r1, double hue,
	int color, int connect)
/***** Draw a rectangle.
	hue is a hue intensity from 0 to 1.
	connect determines, if a black boundary should be drawn.
******/
{	if (metafile)
	{	intwrite(7);
		longwrite(c); longwrite(r);
		longwrite(c1); longwrite(r1);
		longwrite((hue-floor(hue))*1000.0); intwrite(color);
	}
}

void gfillh (double c[8], double hue, int color, int connect)
/***** Draw a filled polygon.
	Works like gfill, but uses hue.
*****/
{	int i;
	for (i=0; i<8; i++) if (fabs(c[i])>10000.0) return;
	if (metafile)
	{	intwrite(8);
		for (i=0; i<8; i+=2)
		{	longwrite(c[i]); longwrite(c[i+1]);
		}
		longwrite((hue-floor(hue))*1000.0);
		intwrite(color); intwrite(connect);
	}
}

/*********************** input routines **********************/

void mouse (int* x, int* y)
/****** mouse
	wait, until the user marked a screen point with the mouse.
	Return screen coordinates.
******/
{	*x=0; *y=0;
}

#ifndef TERMIO
int wait_key (scantyp *scan)
/***** 
	wait for a keystroke. return the scancode and the ascii code.
	scancode should be a code from scantyp. Do at least generate
	'enter'.
*****/
{	int c;
	c=getchar();
	if (c==EOF) exit(0);
	switch (c)
	{	case '\n' : *scan=enter; break;
		default : *scan=0;
	}
	return c;
}
#else
// Return a char read from the terminal
static scantyp translate(char* s, int *n)
{
	char c;
  	
  	if (*n) {(*n)--;c=*s++;} else return key_none;
  	
	if(isprint(c)) {
		return key_none;
	} else if (c == 0x04) {
		return eot;	// ctrl D
	} else if( c == 0x1B ) {		// escape sequence
		if (*n) {(*n)--;c=*s++;} else return escape;
		if(c=='[') {
			if (*n) {(*n)--;c=*s++;} else return escape;
			switch( c ) {
			case 0x31:
				if (*n) {(*n)--;c=*s++;} else return escape;
				switch (c) {
				case 0x35:
					if (*n) {(*n)--;c=*s++;} else return escape;
					if (c=='~') return fk5;
				case 0x37:
					if (*n) {(*n)--;c=*s++;} else return escape;
					if (c=='~') return fk6;
				case 0x38:
					if (*n) {(*n)--;c=*s++;} else return escape;
					if (c=='~') return fk7;
				case 0x39:
					if (*n) {(*n)--;c=*s++;} else return escape;
					if (c=='~') return fk8;
				default:
					return key_none;
				}
			case 0x32:
				if (*n) {(*n)--;c=*s++;} else return escape;
				switch (c) {
				case 0x30:
					if (*n) {(*n)--;c=*s++;} else return escape;
					if (c=='~') return fk9;
				case 0x31:
					if (*n) {(*n)--;c=*s++;} else return escape;
					if (c=='~') return fk10;
				case 0x33:
					if (*n) {(*n)--;c=*s++;} else return escape;
					if (c=='~') return fk11;
				case 0x34:
					if (*n) {(*n)--;c=*s++;} else return escape;
					if (c=='~') return fk12;
				case '~':
					return help;
				default:
					return key_none;
				}
			case 0x33:
				if (*n) {(*n)--;c=*s++;} else return escape;
				if (c=='~')	return delete;
				else return key_none;
			case 0x36:
				if (*n) {(*n)--;c=*s++;} else return escape;
				if (c=='~') return page_down;
				else return key_none;
			case 0x35:
				if (*n) {(*n)--;c=*s++;} else return escape;
				if (c=='~') return page_up;
				else return key_none;
			case 0x41:
				return cursor_up;
			case 0x42:
				return cursor_down;
			case 0x43:
				return cursor_right;
			case 0x44:
				return cursor_left;               
			case 0x05:
			case 0x46:
				return line_end;
			case 0x02:
			case 0x48:
				return line_start;
			case 'Z':
				return switch_screen;
			}
		} else if (c==0x4f) {
			if (*n) {(*n)--;c=*s++;} else return escape;
			switch(c) {
			case 0x05:
			case 0x46:
				return line_end;
			case 0x02:
			case 0x48:
				return line_start;
			case 0x50:
				return fk1;
			case 0x51:
				return fk2;
			case 0x52:
				return fk3;
			case 0x53:
				return fk4;
			default:
				return key_none;
			}
		} else return escape;
	} else {
		switch(c) {
		case 0x0D:
		case 0x0A:		// CR/LF sequence, read the second char (LF) if applicable
			return enter;
		case 0x09:
			*(s-1)=' ';
			return key_none;
		case 0x7F:
		case 0x08:
			return backspace;
		default:
			return key_none;
		}
	}
	return key_none;
}

int wait_key (scantyp* scan)
/***** 
	wait for a keystroke. return the scancode and the ascii code.
	scancode should be a code from scantyp. Do at least generate
	'enter'.
*****/
{
	fd_set fds;
	int ret;
	static int i=0, n=0;
	static char s[20];
	
	*scan=key_none;
	if (n) {
		int n0=n;
//		fprintf(stderr,"++%d : %c++\n",n,s[i]);
		*scan=translate(s+i,&n);
		n0=n0-n;i+=n0;
		return s[i-n0];
	}
	
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO,&fds);
	
	while (1) {
		ret=select(1,&fds,NULL,NULL,NULL);
		if (ret==-1) {			// select interrupted or error
			if (errno!=EINTR && errno!=EAGAIN) {	// not interrupted, so error
				*scan=escape;
				perror("select");
				break;
			}
		} else if (FD_ISSET(STDIN_FILENO,&fds)){
			n=read(STDIN_FILENO,s,20);
			if (n>0) {			// some chars to read
/*				fprintf(stderr,"%3d : ", n);
				for (int j;j<n;j++)
					fprintf(stderr,"%02X ",s[j]);
				fprintf(stderr,"\n");*/
				int n0=n;
				i=0;
				*scan=translate(s,&n);
				n0=n0-n;i+=n0;
//				fprintf(stderr,"--%d--\n",n);
/*				switch(s[i])
				{	case '\n': *scan=enter; break;
					case 0x04: exit(0); break;
					case 0x08:
					case 0x7F: *scan=backspace; break;
					default : *scan=0;
				}*/
				return s[0];
			} else if (n<=0) {	// EOF
				exit(0);
			}
		}
	}
	return 0;
}
#endif

int test_key (void)
/***** test_key
	see, if user pressed the keyboard.
	return the scancode, if he did.
*****/
{	return 0;
}

int test_code (void)
/***** test_code
	see, if user pressed the keyboard.
	return the scancode, if he did.
*****/
{	return 0;
}

/**************** directory *******************/

#include <unistd.h>

char curdir[256];

char *cd (char *dir)
/***** sets the path if dir!=0 and returns the path
*****/
{	
	chdir(dir);
	if (getcwd(curdir,256)) return curdir;
	return dir;
}

#include "dirent.h"

int match (char *pat, char *s)
{	if (*pat==0) return *s==0;
	if (*pat=='*')
	{	pat++;
		if (!*pat) return 1;
		while (*s)
		{	if (match(pat,s)) return 1;
			s++;
		}
		return 0;
	}
	if (*s==0) return 0;
	if (*pat=='?') return match(pat+1,s+1);
	if (*pat!=*s) return 0;
	return match(pat+1,s+1);
}

char *dir (char *pattern)
/* if pattern==0, find next file, else find pattern.
   return 0, if there is no file.
*/
{	static DIR *dir=0;
	static struct dirent *entry;
	static char pat[256];
	if (pattern)
	{	strcpy(pat,pattern);
		if (!pat[0]) strcpy(pat,"*");
		if (dir) { closedir(dir); dir=0; }
		dir=opendir(".");
	}
	if (dir)
	{	again: entry=readdir(dir);
		if (!entry)
		{	closedir(dir);
			dir=0;
		}
		else
		{	if (match(pat,entry->d_name)) return entry->d_name;
			else goto again;
		}
	}
	return 0;
}

/***************** clock and wait ********************/

#ifndef SY_CLK_TCK
#define SY_CLK_TCK 50
#endif

double myclock (void)
/***** define a timer in seconds. 
******/
{
#ifdef RS6000
	struct timestruc_t t;
	gettimer(TIMEOFDAY,&t);
	return (t.tv_sec+t.tv_nsec/1000000000.0);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)(tv.tv_sec+tv.tv_usec*1e-6);
#endif
}

#ifndef TERMIO
void sys_wait (double time, int *scan)
/***** Wait for time seconds or until a key press.
Return the scan code or 0 (time exceeded).
******/
{	int c=getchar();
	printf(" *** Press Return\n");
	switch(c)
	{	case 27 : *scan=escape; break;
		default : *scan=enter;
	}
}
#else
/***** Wait for time seconds or until a key press.
Return the scan code or 0 (time exceeded).
******/
void sys_wait (double time, int *scan)
{
	fd_set fds;
	int ret;
	struct timeval tv, *tvp=NULL;
	
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO,&fds);
	
	if ((long)time!=0) {
		tv.tv_sec = (long)time;
		tv.tv_usec = (long)((time-tv.tv_sec)*1e6);
		tvp=&tv;
	}
	
	while (1) {
		ret=select(1,&fds,NULL,NULL,tvp);
		if (ret==-1) {
			*scan=escape;
			perror("select");
			return;
		} else if (!ret) {	// timeout
			break;
		}
		
		if (FD_ISSET(STDIN_FILENO,&fds)){
			int c=getchar();
//			printf(" *** Press Return\n");
			switch(c)
			{	case 27 : *scan=escape; break;
				default : *scan=enter;
			}
			return;
		}
	}
	*scan=0;
}
#endif
/***************** div. ***********************************/

void getpixel (double *x, double *y)
/***** Compute the size of pixel in screen coordinates.
******/
{	*x=1;
	*y=1;
}

void gflush (void)
/***** Flush out remaining graphic commands (for multitasking).
This serves to synchronize the graphics on multitasking systems.
******/
{
}

/**************** Text screen ***************************/

void clear_screen (void)
/***** Clear the text screen 
******/
{
	gprint("\x1b[2J\x1b[H");
}

void graphic_mode ()
{
}

void text_mode ()
{
}

/********************** Text output ***************************/

/****
The following text screen commands should be emulated on a graphic
work station. This can be done by a standard emulator (e.g. VT100)
or within a window displaying the text. Additional features may be
added, such as viewing old text. But the input line should be
visible as soon as a key is pressed by the user.
****/

void move_cl (void)
/* move the text cursor left */
{
	gprint("\x1b[1D");
}

void move_cr (void)
/* move the text cursor right */
{
	gprint("\x1b[1C");
}

void cursor_on (void)
/* switch cursor on */
{
	gprint("\x1b[?25h");
}

void cursor_off (void)
/* switch cursor off */
{
	gprint("\x1b[?25l");
}

void clear_eol (void)
/* clear the text line from cursor position */
{
	gprint("\x1b[K");
}

void gprint (char *s)
/*****
Print a line onto the text screen, parse tabs and '\n'.
Printing should be done at the cursor position. There is no need
to clear the line at a '\n'.
The cursor should move forward with the print.
Think of the function as a simple emulator.
If you have a line buffered input with echo then do not print,
when the command line is on.
*****/
{	if (!(quiet && editing)) {
		printf("%s",s);
		fflush(stdout);
	}
}

void edit_off (void)
/* the command line is no longer in use (graphics or computing) */
{	editing=0;
}

void edit_on (void)
/* the command line is active */
{	editing=1;
}

/********** execute programs *************************/

int execute (char *name, char *args)
/**** execute
	Call an external program, return 0, if there was no error.
	No need to support this on multitasking systems.
****/
{	return 0;
}

/*************** main ********************************/
static struct termios old_tio, new_tio;

/*
 * signal handler.
 */
static void sigHandler(int sig)
{
	struct winsize ws;

	switch (sig) {
	case SIGINT:	/* Ctrl-C */
		exit(0);
	case SIGWINCH:
		ws.ws_col = 0;
		ws.ws_row = 0;
		ioctl(1, TIOCGWINSZ, &ws);
		if (ws.ws_col) {
			linelength = ws.ws_col;
//			fprintf(stderr,"linelength : %d\n", linelength);
		}
		break;
	}
}

#ifdef TERMIO
static void term_restore(void)
{
	/* restore the former settings */
	tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);
}
#endif

static char titel[]="This is Retro, Version 3.04 compiled %s.\n\n"
	"Type help(Return) for help.\n"
	"Enter command: (%ld Bytes free.)\n\n";

/* path_expand
 *   expands ~ and resolve relative path
 *   returns the expanded path. 
 *     result has to be freed if not NULL
 */
static char* path_expand(char* path)
{
	char rpath[PATH_MAX];
	
	if (path[0]=='~') {
		char* home=getenv("HOME");
		if (home) {
			strncat(rpath, home, PATH_MAX);
			strncat(rpath, path+1, PATH_MAX);
			return realpath(rpath, NULL);
		}
		return NULL;
	}
	return realpath(path, NULL);
}

/* path_is_dir
 *   does the path exist and identifies a directory ?
 */
static int path_is_dir(char* path)
{
	struct stat si;
	if (stat(path, &si)==0 && S_ISDIR(si.st_mode)) {
		return 1;
	}
	return 0;
}
#if 0
/* path_is_dir
 *   does the path exist and identifies a directory ?
 */
static int path_is_file(char* path)
{
	struct stat si;
	if (stat(path, &si)==0 && S_ISREG(si.st_mode)) {
		return 1;
	}
	return 0;
}
#endif

int main (int argc, char *argv[])
/******
Initialize memory and call main_loop
******/
{
	char *s;
	
    while (argc>1 && argv[1][0]=='-')
	{	switch (argv[1][1])
		{	case 's' : /* set stacksize */
				if (argc<3) goto error;
				size=atoi(argv[2])*1024l;
				if (size<512*1024l) size=512*1024l;
				argc-=2; argv+=2;
				break;
			case 'q' : /* set quiet mode */
				quiet=1;
				argc-=1; argv+=1;
				break;
			default :
			error :
				fprintf(stderr, "Use: retro [-s KB] [-q] files\n");
				return 1;
		}
	}
	
	if (!memory_init()) return 1;
	
	/* set up default pathes and directory */
	s=getenv("RETRO");
	if (!s) s="~/.retro/progs:";
	
	// get a buffer of the required size
	char str[strlen(s)+1];
	s=strcpy(str,s);
	
	path[0]=".";
	npath=1;
	for (s=strtok(s,":"); s!=NULL; s=strtok(NULL,":")) {
		char* p=path_expand(s);
		if (p && path_is_dir(p)) {
	 		path[npath++]=p;
	 	} else {
	 		fprintf(stderr, "path %s rejected (not a valid path!)\n", s);
	 		if (p) free(p);
	 	}
	 	if (npath==MAX_PATH) {
	 		fprintf(stderr, "Too many pathes\n");
	 		npath=MAX_PATH-1;
	 		break;
	 	}
	}
	path[npath++]=INSTALL"/share/retro/progs\n";
	
#ifdef DEBUG	
	fprintf(stderr,"npath %d\n",npath);
	for (int i=0; i<npath; i++) {
		fprintf(stderr,"%s\n",path[i]);
	}
#endif
	
	/* get width of the terminal */
	struct winsize ws;

	ws.ws_col = 0;
	ws.ws_row = 0;
	ioctl(1, TIOCGWINSZ, &ws);
	if (ws.ws_col) {
		linelength = ws.ws_col;
	}

	/*  */
    struct sigaction sa;

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = sigHandler;
    if (sigaction(SIGWINCH, &sa, NULL) == -1) {
    	fprintf(stderr,"sigaction %d\n", errno);
    	exit(1);
    }

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_RESETHAND;
    sa.sa_handler = sigHandler;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
    	fprintf(stderr,"sigaction %d\n", errno);
    	exit(1);
    }
	
#ifdef TERMIO
	/* get the terminal settings for stdin */
	tcgetattr(STDIN_FILENO,&old_tio);
	/* we want to keep the old setting to restore them a the end */
	new_tio=old_tio;
	/* disable canonical mode (buffered i/o) and local echo */
	new_tio.c_lflag &=(~ICANON & ~ECHO);
	/* set the new settings immediately */
	tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);
	
	atexit(term_restore);
#endif
	
	if (!quiet) output1(titel,__DATE__,(unsigned long)(ramend-ramstart));
	
	main_loop(argc,argv);
	return 0;
}

