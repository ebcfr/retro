#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h> /* for chdir and getcwd */
#include <dirent.h>

#ifdef RS6000
#include <sys/time.h>
void gettimer (int, struct timestruc_t *);
#endif

#define MAXPLOT 100000.0

#define WINDOW

#include "header.h"
#include "sysdep.h"

/************************ Screen *****************************/
static int wscreen,hscreen;	/*screen width & height [pixel] */

/********************* Text buffer ***************************/
#define TEXTSIZE (128*1024l)

static char textstart[TEXTSIZE]={0};	/* output buffer */
static char *textend=textstart,			/* end of used buffer */
            *textwindow=textstart,		/* start of buffer in the
            							   view */
            *oldtextwindow;

static int textheight,		/* char width [pixel] */
           textwidth,		/* line height [pixel]*/
           textoffset;		/* offset all around the window [pixel] */

int wchar,hchar;			/* char width & height in screen coords [1024x1024] */

int linelength;				/* screen width [char] (for other file use) */
static int cwscreen;		/* screen width in [char] */
static int chscreen;		/* screen height in [line] */

static int cx,cy;			/* current char pos relative to the screen 
							   in char count/linecount */
#if 0
static int cyview=0;		/* nb of the line corresponding to the
							   textview position in the buffer */
#endif
static int cursoron=0,		/* cursor is visible */
           scrolled=0,		/* view scrolled ? */
           in_text=1;		/* text window is the active view */

/*************************************************************/
int userbreak=0;

double nextcheck;

/*************** XWindow things ******************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xft/Xft.h>

#define MAXCOLORS 16

#include "icon.h"

void process_event (XEvent *event);

char sdisp[64]="";
Display *display;
int screen, depth;
Window window;
unsigned long mask;
Atom sel_prop;
GC gc,cleargc,stipplegc;
Pixmap stipple[64];
unsigned long blackpixel, whitepixel;
Colormap colormap;
int usedcolors=MAXCOLORS;
unsigned long color[MAXCOLORS];
char *colorname[MAXCOLORS]=
{
	"White","Black","Green","Lightblue","Blue","SteelBlue",
	"Khaki","Tan","Grey","Yellow","Green","Red","LightBlue",
	"LightSteelBlue","LimeGreen","Navy"
};
Pixmap pixmap;

int userwidth=800,userheight=600,userx=0,usery=0,usersize=0,userpos=0;

XftFont *gfont, *gsfont, *gvfont, *tfont;
XftDraw *draw;
Visual* visual;
XftColor tfgcolor, tbgcolor;

char* tfontname="Mono:size=10";
char* gfontname="Sans:size=10";
char* gsfontname="Sans:size=6";
char* gvfontname="Sans:size=10:matrix=0 -1 1 0";
//char* gfontname="Mono:size=10";

#if 0
/* Purely graphic info */
typedef struct {
	Display *dpy;
	Colormap cmap;
	Window win;
	Drawable buf;
	Atom xembed, wmdeletewin, netwmname, netwmpid;
	XIM xim;
	XIC xic;
	Visual *vis;
	XSetWindowAttributes attrs;
	int scr;
	int isfixed;	/* is fixed geometry? */
	int l, t;		/* left and top offset */
	int gm;			/* geometry mask */
	int tw, th;		/* tty width and height */
	int w, h;		/* window width and height */
	int ch;			/* char height */
	int cw;			/* char width  */
	char state;		/* focus, redraw, visible */
	int cursor;		/* cursor style */
} XWindow;


/* text window buffer */
#define MAX_BUF_LINES	1024

typedef struct _buf_t {
	char*			data;		// the data line
	int				size;		// size of the data
	unsigned int	flags		// type of line: command, output, comment
} buf_t;

/* the terminal object */
typedef struct _term_t {
	buf_t	tbuf[MAX_BUF_LINES];// circular buffer of lines
	int		tbuf_r_id=0;		// current start of the buffer
	int		tbuf_w_id=0;		// current end

	int		view_line_length;	// nb of viewable chars on the display
	int		view_max_lines;		// nb of lines that be displayed in the view
	int		view_top;			// tbuf index at the top of the view
	
} term_t;

term_t term;

int term_push(term_t* term, char* data, int flags);

#endif

void die(const char *errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}

void computechar (void)
{
	XGlyphInfo extents;
	
	XftTextExtents8(display,gfont,(XftChar8*)"0",1,&extents);
	hchar=(gfont->ascent+gfont->descent+2)*1024l/hscreen;
//	wchar=gfont->max_advance_width*1024l/wscreen;
	wchar=extents.width*1024l/wscreen;
}

void quitX (void)
/* Disconnect from server */
{   
	XDestroyWindow(display,window);
	XftFontClose(display,tfont);
	XftFontClose(display,gfont);
	XFreePixmap(display,pixmap);
	XCloseDisplay(display);
}

Window open_window (int x, int y, int width, int height, int flag)
/* create and open a window plus a graphic context and a pixmap */
{	
	XSetWindowAttributes attributes;
	XSizeHints sizehints;
	Window window;
	Pixmap iconpixmap;
	XWMHints iconhints;
	/* XClassHint classhints; */
	unsigned int cursorshape;
	Cursor cursor;
	int i,j,k,st;
	unsigned char byte,bits[512];
	XColor rgb,hardware;
	XGCValues gcvalues;
	XRenderColor black = {.red=0, .green=0, .blue=0, .alpha=0xFFFF};
	XRenderColor white = {.red=0xFFFF, .green=0xFFFF, .blue=0xFFFF, .alpha=0xFFFF};
	
	/* window attributes */
	attributes.border_pixel=BlackPixel(display,screen);
	attributes.background_pixel=WhitePixel(display,screen);

	mask= CWBackPixel | CWBorderPixel;

	/* create the window */
	window= XCreateWindow(display,RootWindow(display,screen),
	    x,y,width,height,
	    2, /* Border width */
	    depth,
	    InputOutput,
	    CopyFromParent,
	    mask,
	    &attributes);

	/* Icon setting */
	iconpixmap= XCreateBitmapFromData(display,window,
	    icon_bits,icon_width,icon_height);
	iconhints.icon_pixmap=iconpixmap;
	iconhints.initial_state=NormalState;
	iconhints.flags=IconPixmapHint|StateHint;
	XSetWMHints(display,window,&iconhints);

	/* Cursor */
	cursorshape=XC_dotbox;
	cursor=XCreateFontCursor(display,cursorshape);
	XDefineCursor(display,window,cursor);

	/* class hints */
	/* classhints.res_name="Retro";
	classhints.res_class="Retro";
	XSetClassHint(display,window,classhints); */

	/* Window and Icon name */
	XStoreName(display,window,"retro");
	XSetIconName(display,window,"retro");

	/* size hints */
	sizehints.flags=PMinSize;
	if (userpos) sizehints.flags|= USPosition | USSize;
	else sizehints.flags|= PPosition | USSize;
	sizehints.x=x;
	sizehints.y=y;
	sizehints.width=width;
	sizehints.height=height;
	sizehints.min_width=200;
	sizehints.min_height=200;
	XSetNormalHints(display,window,&sizehints);

	/* create graphical contexts */
	colormap=DefaultColormap(display,screen);
	color[0]=whitepixel;
	color[1]=blackpixel;
	for (i=0; i<usedcolors; i++)
	{
		st=XLookupColor(display,colormap,colorname[i],&rgb,&hardware);
		if (st)
		{	st=XAllocColor(display,colormap,&hardware);
			if (st) color[i]=hardware.pixel;
			else color[i]=blackpixel;
		}
		else color[i]=whitepixel;
	}

	gc=XCreateGC(display,window,0,&gcvalues);
	XSetForeground(display,gc,blackpixel);
	XSetBackground(display,gc,whitepixel);
				
	cleargc=XCreateGC(display,window,0,&gcvalues);
	XSetForeground(display,cleargc,whitepixel);
	XSetBackground(display,cleargc,blackpixel);

	stipplegc=XCreateGC(display,window,0,&gcvalues);
	XSetForeground(display,stipplegc,blackpixel);
	XSetBackground(display,stipplegc,whitepixel);
	XSetFillStyle(display,stipplegc,FillOpaqueStippled);
	
	for (i=0; i<64; i++)
	{	for (j=0; j<512; j++)
		{	byte=0;
			if (i==0) byte=255;
			else if (i==63) byte=0;
			else for (k=0; k<8; k++) byte=(byte<<1)|((rand()&63)>i);
			bits[j]=byte;
		}
		stipple[i]=XCreateBitmapFromData(display,window,(char*)bits,64,64);
	}

	draw = XftDrawCreate(display, window, visual, colormap);
	XftColorAllocValue(display, visual, colormap, &black, &tfgcolor);
	XftColorAllocValue(display, visual, colormap, &white, &tbgcolor);
					
	/* events, we like to receive */
	mask=KeyPressMask|ExposureMask|ButtonPressMask|ButtonReleaseMask|Button1MotionMask|StructureNotifyMask;
	XSelectInput(display,window,mask);

	/* show the window */
	XMapWindow(display,window);

	/* create a pixmap of same size */
	pixmap=XCreatePixmap(display,window,width,height,depth);
	XFillRectangle(display,pixmap,cleargc,0,0,width,height);

	return window;
}

void computetext (void)
{
	chscreen=(hscreen-2*textoffset)/textheight;
	linelength=cwscreen=(wscreen-2*textoffset)/textwidth;
}

void initX (void)
/* switch to a graphic screen */
{
	display=XOpenDisplay(sdisp);
	if (!display)
	{       
		fprintf(stderr,"ERROR: Connetion to X server %s failed.\n",
		    sdisp);
		exit(1);
	}
	screen=DefaultScreen(display);			/* screen # */
	visual=DefaultVisual(display,screen);	/* visual */
	depth=DefaultDepth(display,screen);		/* color depth */
	switch(depth)
	{
		case 1 : usedcolors=2; break;
		case 2 : usedcolors=4; break;
		case 3 : usedcolors=8; break;
		default : usedcolors=MAXCOLORS; break;
	}
	blackpixel=BlackPixel(display,screen);
	whitepixel=WhitePixel(display,screen);

	/* load fonts */
	tfont = XftFontOpenName(display, screen, tfontname);
	if (!tfont) die("Cannot find %s font\n",tfontname);

	textheight=tfont->ascent+tfont->descent+2;
	textwidth=tfont->max_advance_width;
	textoffset=textwidth/2;

	gfont = XftFontOpenName(display, screen, gfontname);
	if (!gfont) die("Cannot find %s font\n",gfontname);
	
	gsfont = XftFontOpenName(display, screen, gsfontname);
	if (!gfont) die("Cannot find %s font\n",gfontname);
	
	gvfont = XftFontOpenName(display, screen, gvfontname);
	if (!gvfont) die("Cannot find %s font\n",gvfontname);
	
	if (usersize)
	{	
		userwidth=abs(userwidth);
		if (userwidth<256) userwidth=256;
		if (userwidth>DisplayWidth(display,screen))
			userwidth=DisplayWidth(display,screen);
		userheight=abs(userheight);
		if (userheight<256) userheight=256;
		if (userheight>DisplayHeight(display,screen))
			userheight=DisplayHeight(display,screen);
	}
	else
	{
		userwidth=DisplayWidth(display,screen)/3*2;
		userheight=DisplayHeight(display,screen)/3*2;
	}
	if (userpos)
	{
		if (userx<0) userx=DisplayWidth(display,screen)
			+userx-userwidth;
		if (usery<0) usery=DisplayHeight(display,screen)
			+usery-userheight;
		if (userx<0) userx=0;
		if (usery<0) usery=0;
	}
	else
	{
		userx=usery=0;
	}
	
	window=open_window(userx,usery,userwidth,userheight,0);
	
	wscreen=userwidth;
	hscreen=userheight;
	computetext();
	computechar();
	
	sel_prop=XInternAtom(display,"SEL_PROP",False);
	
	atexit(quitX);
	
}

void setcolor (int c)
{       
	static int oldcolor=-1;
	if (c>=usedcolors) c=usedcolors-1;
	if (c<0) c=0;
	if (c!=oldcolor)
	{
		XSetForeground(display,gc,color[c]);
		oldcolor=c;
	}
}

#ifdef WINDOW
#define draw(x1,x2,y1,y2) XDrawLine(display,pixmap,gc,x1,x2,y1,y2),\
	XDrawLine(display,window,gc,x1,x2,y1,y2)
#define plot(x1,x2) XDrawPoint(display,pixmap,gc,x1,x2),\
	XDrawPoint(display,window,gc,x1,x2)
#else
#define draw(x1,x2,y1,y2) XDrawLine(display,pixmap,gc,x1,x2,y1,y2)
#define plot(x1,x2) XDrawPoint(display,pixmap,gc,x1,x2)
#endif

/******************** some graphic support functions ************/

int column (double c)
{	
	long res;
	res=(long)((c*wscreen)/1024);
	return (int)res;
}

int row (double c)
{	
	long res;
	res=(long)((c*hscreen)/1024);
	return (int)res;
}

void longwrite (double n)
{	
	int k[1];
	k[0]=(long)(n*1000.0);
	if (metafile) fwrite(k,sizeof(long),1,metafile);
}

void intwrite (int n)
{	
	int k[1];
	k[0]=n;
	if (metafile) fwrite(k,sizeof(int),1,metafile);
}

void stringwrite (char *s)
{	
	int c;
	if (metafile)
	{	
		c=0;
		while (*s) { 
			putc(*s++,metafile); 
			c++; 
		}
		putc(0,metafile); 
		c++;
		if (c%2) putc(0,metafile);
	}
}

void refresh_window (void);

void graphic_mode (void)
/***** graphics
	switch to graphics. text should not be deleted.
*****/
{   
	if (in_text)
	{
		in_text=0;
		refresh_window();
	}
}

void text_mode (void)
/***** text_mode
	switch to text. graphics should not be deleted.
*****/
{	
	if (!in_text)
	{
		in_text=1;
		refresh_window();
	}
}

void gclear (void)
{	
	XFillRectangle(display,pixmap,cleargc,0,0,wscreen,hscreen);
#ifdef WINDOW
	XFillRectangle(display,window,cleargc,0,0,wscreen,hscreen);
#endif
	XFlush(display);
}

void gflush (void)
{
#ifndef WINDOW
	XCopyArea(display,pixmap,window,gc,0,0,wscreen,hscreen,0,0);
#endif
	XFlush(display);
}

void gclip(plot_t *p)
{
	short xf = (p->upperc*wscreen)>>10;
	short yf = (p->upperr*hscreen)>>10;
	short wf = ((p->lowerc-p->upperc)*wscreen)>>10;
	short hf = ((p->lowerr-p->upperr)*hscreen)>>10;
	XRectangle r;
	r.x=xf; r.y=yf; r.width=wf; r.height=hf;
	XSetClipRectangles(display, gc, 0, 0, &r, 1, Unsorted);
}

void gunclip(plot_t *p)
{
	XSetClipMask(display,gc,None);
}

void gsubplot(int r, int c, int index)
{

}

int oldst=-1,oldcol=-1,oldwidth=-1;

void setline (int w, int st)
{	if (w==1) w=0;
	if (oldwidth==w && oldst==st) return;
	oldst=st;
	switch (st)
	{	case line_dotted :
		case line_dashed : st=LineOnOffDash; break;
		default : st=LineSolid; break;
	}
	XSetLineAttributes(display,gc,w,st,CapRound,JoinRound);
	oldwidth=w;	
}

void gframe(plot_t* p)
{
	short xf = (p->upperc*wscreen)>>10;
	short yf = (p->upperr*hscreen)>>10;
	short wf = ((p->lowerc-p->upperc)*wscreen)>>10;
	short hf = ((p->lowerr-p->upperr)*hscreen)>>10;
	setcolor(1);
	XSetLineAttributes(display,gc,1,LineSolid,CapRound,JoinRound);
	XDrawRectangle(display,window,gc,xf,yf,wf,hf);
	XDrawRectangle(display,pixmap,gc,xf,yf,wf,hf);
}

void gpath(plot_t* p, double *x, double *y, int n)
{
	XPoint* curve=(XPoint*)malloc(n*sizeof(XPoint));
	short xf = (p->upperc*wscreen)>>10;
	short yf = (p->upperr*hscreen)>>10;
	short wf = ((p->lowerc-p->upperc)*wscreen)>>10;
	short hf = ((p->lowerr-p->upperr)*hscreen)>>10;
	double kx, ky;
	
	if (!p->xlog) {
		kx = (double)wf/(p->x_max-p->x_min);
		for (int i=0 ; i<n ; i++) {
			curve[i].x = xf + kx*(x[i]-p->x_min);
			
		}
	} else {
		kx = (double)wf/log10(p->x_max/p->x_min);
		for (int i=0 ; i<n ; i++) {
			curve[i].x = xf + kx*log10(x[i]/p->x_min);
		}
	}
	if (!p->ylog) {
		ky = (double)hf/(p->y_max-p->y_min);
		for (int i=0 ; i<n ; i++) {
			if (y[i]>p->y_max)			/* avoid int16 overflow */
				curve[i].y = yf - 5;
			else if (y[i]<p->y_min)		/* avoid int16 overflow */
				curve[i].y = yf + hf + 5;
			else						/* calculate standard case */
				curve[i].y = yf + hf - ky*(y[i]-p->y_min);
		}
	} else {
		ky = (double)hf/log10(p->y_max/p->y_min);
		for (int i=0 ; i<n ; i++) {
			curve[i].y = yf + hf - ky*log10(y[i]/p->y_min);
		}
	}
	
	setcolor(linecolor);
	setline(linewidth,linetype);
	
	switch (linetype) {
	case line_solid:
	case line_dotted:
	case line_dashed:
		XDrawLines(display,window,gc,curve,n,CoordModeOrigin);
		XDrawLines(display,pixmap,gc,curve,n,CoordModeOrigin);
		break;
	case line_comb:
		for (int i=0;i<n;i++) {
			/* draw a line from bottom or 0 axis to the point */
			XSegment s;
			s.x1 = curve[i].x;
			s.y1 = p->y_min<0.0 ? yf + hf + ky*p->y_min : yf+hf;
			s.x2 = curve[i].x;
			s.y2 = curve[i].y;
			XDrawSegments(display,window,gc,&s,1);
			XDrawSegments(display,pixmap,gc,&s,1);
		}
		break;
	case line_none:
	default:
		break;
	}

	if (markertype!=marker_none) {
		XSetLineAttributes(display,gc,1,LineSolid,CapProjecting,JoinRound);
		if (markertype==marker_dot) {
			XDrawPoints(display,window,gc,curve,n,CoordModeOrigin);
			XDrawPoints(display,pixmap,gc,curve,n,CoordModeOrigin);
		} else {
			for (int i=0;i<n;i++) {
				switch (markertype) {
				case marker_cross: {
					XSegment xs[2];
					xs[0].x1=curve[i].x-markersize/2;
					xs[0].y1=curve[i].y-markersize/2;
					xs[0].x2=curve[i].x+markersize/2+1;
					xs[0].y2=curve[i].y+markersize/2+1;
					xs[1].x1=curve[i].x-markersize/2;
					xs[1].y1=curve[i].y+markersize/2;
					xs[1].x2=curve[i].x+markersize/2+1;
					xs[1].y2=curve[i].y-markersize/2-1;
					XDrawSegments(display,window,gc,xs,2);
					XDrawSegments(display,pixmap,gc,xs,2);
					break;
				}
				case marker_plus: {
					XSegment xs[2];
					xs[0].x1=curve[i].x-markersize/2;
					xs[0].y1=curve[i].y;
					xs[0].x2=curve[i].x+markersize/2;
					xs[0].y2=curve[i].y;
					xs[1].x1=curve[i].x;
					xs[1].y1=curve[i].y+markersize/2;
					xs[1].x2=curve[i].x;
					xs[1].y2=curve[i].y-markersize/2;
					XDrawSegments(display,window,gc,xs,2);
					XDrawSegments(display,pixmap,gc,xs,2);
					break;
				}
				case marker_star: {
					short d=markersize/2*239/338;			// markersize/2*0.707
					short d1=(markersize/2+1)*239/338;		// markersize/2*0.707
					XSegment xs[4];
					xs[0].x1=curve[i].x-markersize/2;
					xs[0].y1=curve[i].y;
					xs[0].x2=curve[i].x+markersize/2;
					xs[0].y2=curve[i].y;
					xs[1].x1=curve[i].x;
					xs[1].y1=curve[i].y+markersize/2;
					xs[1].x2=curve[i].x;
					xs[1].y2=curve[i].y-markersize/2;
					xs[2].x1=curve[i].x-d;
					xs[2].y1=curve[i].y-d;
					xs[2].x2=curve[i].x+d1;
					xs[2].y2=curve[i].y+d1;
					xs[3].x1=curve[i].x-d;
					xs[3].y1=curve[i].y+d;
					xs[3].x2=curve[i].x+d1;
					xs[3].y2=curve[i].y-d1;
					XDrawSegments(display,window,gc,xs,4);
					XDrawSegments(display,pixmap,gc,xs,4);
					break;
				}
				case marker_square:
					XDrawRectangle(display,window,gc,curve[i].x-markersize/2,curve[i].y-markersize/2,markersize,markersize);
					XDrawRectangle(display,pixmap,gc,curve[i].x-markersize/2,curve[i].y-markersize/2,markersize,markersize);
					break;
				case marker_circle:
					XDrawArc(display,window,gc,curve[i].x-markersize/2,curve[i].y-markersize/2,markersize,markersize,0,360<<6);
					XDrawArc(display,pixmap,gc,curve[i].x-markersize/2,curve[i].y-markersize/2,markersize,markersize,0,360<<6);
					break;
				case marker_diamond: {
					XPoint p[5];
					p[0].x=curve[i].x-markersize/2;
					p[0].y=curve[i].y;
					p[1].x=curve[i].x;
					p[1].y=curve[i].y-markersize/2;
					p[2].x=curve[i].x+markersize/2;
					p[2].y=curve[i].y;
					p[3].x=curve[i].x;
					p[3].y=curve[i].y+markersize/2;
					p[4].x=curve[i].x-markersize/2;
					p[4].y=curve[i].y;
					XDrawLines(display,window,gc,p,5,CoordModeOrigin);
					XDrawLines(display,pixmap,gc,p,5,CoordModeOrigin);
					break;
				}
				case marker_fsquare:
					XFillRectangle(display,window,gc,curve[i].x-markersize/2,curve[i].y-markersize/2,markersize,markersize);
					XFillRectangle(display,pixmap,gc,curve[i].x-markersize/2,curve[i].y-markersize/2,markersize,markersize);
					break;
				case marker_fcircle:
					XFillArc(display,window,gc,curve[i].x-markersize/2,curve[i].y-markersize/2,markersize,markersize,0,360<<6);
					XFillArc(display,pixmap,gc,curve[i].x-markersize/2,curve[i].y-markersize/2,markersize,markersize,0,360<<6);
					break;
				case marker_fdiamond: {
					XPoint p[4];
					p[0].x=curve[i].x-markersize/2;
					p[0].y=curve[i].y;
					p[1].x=curve[i].x;
					p[1].y=curve[i].y-markersize/2;
					p[2].x=curve[i].x+markersize/2;
					p[2].y=curve[i].y;
					p[3].x=curve[i].x;
					p[3].y=curve[i].y+markersize/2;
					XFillPolygon(display,window,gc,p,4,Nonconvex,CoordModeOrigin);
					XFillPolygon(display,pixmap,gc,p,4,Nonconvex,CoordModeOrigin);
					break;
				}
				case marker_arrow: {
					XPoint p[3];
					p[0].x=curve[i].x;
					p[0].y=curve[i].y;
					p[1].x=curve[i].x+markersize*13/38;
					p[1].y=curve[i].y-markersize;
					p[2].x=curve[i].x-markersize*13/38;
					p[2].y=curve[i].y-markersize;
					XFillPolygon(display,window,gc,p,3,Nonconvex,CoordModeOrigin);
					XFillPolygon(display,pixmap,gc,p,3,Nonconvex,CoordModeOrigin);
					break;
				}
				case marker_dot:
					break;
				default:
					break;
				}
			}
		}
	}
	free(curve);
}

#define TICKSIZE		5
#define SUBTICKSIZE		3

void gxgrid(plot_t* p, double factor, double* ticks, int n)
{
	XSegment* grid=(XSegment*)malloc(n*sizeof(XSegment));
	short xf = (p->upperc*wscreen)>>10;
	short yf = (p->upperr*hscreen)>>10;
	short wf = ((p->lowerc-p->upperc)*wscreen)>>10;
	short hf = ((p->lowerr-p->upperr)*hscreen)>>10;
	double kx;
	
	if (!p->xlog) {
		kx = (double)wf/(p->x_max-p->x_min);
		for (int i=0 ; i<n ; i++) {
			grid[i].x1 = xf + kx*(ticks[i]-p->x_min);
			grid[i].y1 = yf;
			grid[i].x2 = grid[i].x1;
			grid[i].y2 = yf + hf;
		}
		
		setcolor(gridcolor);
		setline(1,line_dashed);
		if (p->xgrid) {
			XDrawSegments(display,window,gc,grid,n);
			XDrawSegments(display,pixmap,gc,grid,n);
		}
				
		setcolor(1);
		setline(1,line_solid);
		
		for (int i=0 ; i<n ; i++) {
			grid[i].y2 = yf+TICKSIZE;
		}
		XDrawSegments(display,window,gc,grid,n);
		XDrawSegments(display,pixmap,gc,grid,n);
		
		for (int i=0 ; i<n ; i++) {
			XGlyphInfo extents;
			char s[32];
			int len = snprintf(s,32,"%g",fabs(ticks[i]/factor) < 1e-6 ? 0.0 : ticks[i]/factor);
			grid[i].y2 = yf+hf;
			grid[i].y1 = grid[i].y2-TICKSIZE;
			XftTextExtents8(display, gfont, (XftChar8*)s, len, &extents);
			XftDrawString8(draw, &tfgcolor, gfont, grid[i].x2-extents.width/2, grid[i].y2+gfont->ascent+hchar*hscreen/4096, (XftChar8*)s, len);
			XftDrawChange(draw, pixmap);
			XftDrawString8(draw, &tfgcolor, gfont, grid[i].x2-extents.width/2, grid[i].y2+gfont->ascent+hchar*hscreen/4096, (XftChar8*)s, len);
			XftDrawChange(draw, window);
		}
		XDrawSegments(display,window,gc,grid,n);
		XDrawSegments(display,pixmap,gc,grid,n);
		
		if (factor!=1.0) {
			XGlyphInfo extents;
			char s[32];
			int len = snprintf(s,32,"x%g", factor);
			XftTextExtents8(display, gfont, (XftChar8*)s, len, &extents);
			XftDrawString8(draw, &tfgcolor, gfont, xf+wf-extents.width, yf+hf+(gfont->ascent+gfont->descent+2)*5/2, (XftChar8*)s, len);
			XftDrawChange(draw, pixmap);
			XftDrawString8(draw, &tfgcolor, gfont, xf+wf-extents.width, yf+hf+(gfont->ascent+gfont->descent+2)*5/2, (XftChar8*)s, len);
			XftDrawChange(draw, window);
		}
	} else {
	
		kx = (double)wf/log10(p->x_max/p->x_min);
		for (int i=0 ; i<n ; i++) {
			grid[i].x1 = xf + kx*log10(ticks[i]/p->x_min);
			grid[i].y1 = yf;
			grid[i].x2 = grid[i].x1;
			grid[i].y2 = yf + hf;
		}
		
		setcolor(gridcolor);
		setline(1,line_dashed);
		if (p->xgrid) {
			XDrawSegments(display,window,gc,grid,n);
			XDrawSegments(display,pixmap,gc,grid,n);
		}
		
		setcolor(1);
		setline(1,line_solid);
		
		for (int i=0 ; i<n ; i++) {
			double d = ticks[i]/pow(10.0,floor(log10(ticks[i])));
			if (d!=1.0) {
				grid[i].y2 = yf+SUBTICKSIZE;
			} else {
				grid[i].y2 = yf+TICKSIZE;
			}
		}
		XDrawSegments(display,window,gc,grid,n);
		XDrawSegments(display,pixmap,gc,grid,n);
		
		for (int i=0 ; i<n ; i++) {
			XGlyphInfo extents, extents1;
			char s[32];
			grid[i].y2 = yf+hf;
			
			double e = floor(log10(ticks[i]));
			double d = ticks[i]/pow(10.0,e);
			if (d!=1.0) {
				int len = snprintf(s,32,"%g", d);
				grid[i].y1 = grid[i].y2-SUBTICKSIZE;
				XftTextExtents8(display, gsfont, (XftChar8*)s, len, &extents);
				XftDrawString8(draw, &tfgcolor, gsfont, grid[i].x2-extents.width/2, grid[i].y2+gsfont->ascent+gsfont->descent+2, (XftChar8*)s, len);
				XftDrawChange(draw, pixmap);
				XftDrawString8(draw, &tfgcolor, gsfont, grid[i].x2-extents.width/2, grid[i].y2+gsfont->ascent+gsfont->descent+2, (XftChar8*)s, len);
				XftDrawChange(draw, window);
			} else {
				char s1[3]="10";
				grid[i].y1 = grid[i].y2-TICKSIZE;
				int len = snprintf(s,32,"%g", e);
				XftTextExtents8(display, gfont, (XftChar8*)s1, 2, &extents1);
				XftTextExtents8(display, gsfont, (XftChar8*)s, len, &extents);
				XftDrawString8(draw, &tfgcolor, gfont, grid[i].x2-extents1.width/2, grid[i].y2+(gfont->ascent+gfont->descent+2)*11/8, (XftChar8*)s1, 2);
				XftDrawString8(draw, &tfgcolor, gsfont, grid[i].x2+extents1.width/2, grid[i].y2+(gfont->ascent+gfont->descent+2)*7/8, (XftChar8*)s, len);
				XftDrawChange(draw, pixmap);
				XftDrawString8(draw, &tfgcolor, gfont, grid[i].x2-extents1.width/2, grid[i].y2+(gfont->ascent+gfont->descent+2)*11/8, (XftChar8*)s1, 2);
				XftDrawString8(draw, &tfgcolor, gsfont, grid[i].x2+extents1.width/2, grid[i].y2+(gfont->ascent+gfont->descent+2)*7/8, (XftChar8*)s, len);
				XftDrawChange(draw, window);				
			}
		}
		
		XDrawSegments(display,window,gc,grid,n);
		XDrawSegments(display,pixmap,gc,grid,n);
	}
	
	free(grid);
}

void gygrid(plot_t* p, double factor, double* ticks, int n)
{
	XSegment* grid=(XSegment*)malloc(n*sizeof(XSegment));
	short xf = (p->upperc*wscreen)>>10;
	short yf = (p->upperr*hscreen)>>10;
	short wf = ((p->lowerc-p->upperc)*wscreen)>>10;
	short hf = ((p->lowerr-p->upperr)*hscreen)>>10;
	double ky;
	
	if (!p->ylog) {
		ky = (double)hf/(p->y_max-p->y_min);
		for (int i=0 ; i<n ; i++) {
			grid[i].x1 = xf;
			grid[i].y1 = yf + hf - ky*(ticks[i]-p->y_min);
			grid[i].x2 = xf + wf;
			grid[i].y2 = grid[i].y1;
		}
		
		setcolor(gridcolor);
		setline(1,line_dashed);
		if (p->ygrid) {
			XDrawSegments(display,window,gc,grid,n);
			XDrawSegments(display,pixmap,gc,grid,n);
		}
		
		setcolor(1);
		setline(1,line_solid);
		
		for (int i=0 ; i<n ; i++) {
			XGlyphInfo extents;
			char s[32];
			int len = snprintf(s,32,"%g",fabs(ticks[i]/factor) < 1e-6 ? 0.0 : ticks[i]/factor);
			grid[i].x2 = xf+TICKSIZE;
			XftTextExtents8(display, gfont, (XftChar8*)s, len, &extents);
			XftDrawString8(draw, &tfgcolor, gfont, grid[i].x1-wchar*wscreen/1024-extents.width, grid[i].y1+gfont->ascent-hchar*hscreen/2048, (XftChar8*)s, len);
			XftDrawChange(draw, pixmap);
			XftDrawString8(draw, &tfgcolor, gfont, grid[i].x1-wchar*wscreen/1024-extents.width, grid[i].y1+gfont->ascent-hchar*hscreen/2048, (XftChar8*)s, len);
			XftDrawChange(draw, window);
		}
		XDrawSegments(display,window,gc,grid,n);
		XDrawSegments(display,pixmap,gc,grid,n);
		
		for (int i=0 ; i<n ; i++) {
			grid[i].x2 = xf + wf;
			grid[i].x1 = grid[i].x2-TICKSIZE;
		}
		
		XDrawSegments(display,window,gc,grid,n);
		XDrawSegments(display,pixmap,gc,grid,n);
		
		if (factor!=1.0) {
			XGlyphInfo extents;
			char s[32];
			int len = snprintf(s,32,"x%g", factor);
			XftTextExtents8(display, gfont, (XftChar8*)s, len, &extents);
			XftDrawString8(draw, &tfgcolor, gfont, xf-wchar*wscreen*7/1024, yf-(gfont->ascent+gfont->descent+2)/2-gfont->descent, (XftChar8*)s, len);
			XftDrawChange(draw, pixmap);
			XftDrawString8(draw, &tfgcolor, gfont, xf-wchar*wscreen*7/1024, yf-(gfont->ascent+gfont->descent+2)/2-gfont->descent, (XftChar8*)s, len);
			XftDrawChange(draw, window);
		}
		
	} else {
	
		ky = (double)hf/log10(p->y_max/p->y_min);
		for (int i=0 ; i<n ; i++) {
			grid[i].x1 = xf;
			grid[i].y1 = yf + hf - ky*log10(ticks[i]/p->y_min);
			grid[i].x2 = xf + wf;
			grid[i].y2 = grid[i].y1;
		}
		
		setcolor(gridcolor);
		setline(1,line_dashed);
		if (p->ygrid) {
			XDrawSegments(display,window,gc,grid,n);
			XDrawSegments(display,pixmap,gc,grid,n);
		}
		
		setcolor(1);
		setline(1,line_solid);
		
		for (int i=0 ; i<n ; i++) {
			XGlyphInfo extents, extents1;
			char s[32];
			double e = floor(log10(ticks[i]));
			double d = ticks[i]/pow(10.0,e);
			if (d!=1.0) {
				int len = snprintf(s,32,"%g",d);
				grid[i].x2 = xf+SUBTICKSIZE;
				XftTextExtents8(display, gsfont, (XftChar8*)s, len, &extents);
				XftDrawString8(draw, &tfgcolor, gsfont, grid[i].x1-wchar*wscreen/1024-extents.width, grid[i].y1+(gsfont->ascent+gsfont->descent)/2, (XftChar8*)s, len);
				XftDrawChange(draw, pixmap);
				XftDrawString8(draw, &tfgcolor, gsfont, grid[i].x1-wchar*wscreen/1024-extents.width, grid[i].y1+(gsfont->ascent+gsfont->descent)/2, (XftChar8*)s, len);
				XftDrawChange(draw, window);
			} else {
				char s1[3]="10";
				grid[i].x2 = xf+TICKSIZE;
				int len = snprintf(s,32,"%g", e);
				XftTextExtents8(display, gfont, (XftChar8*)s1, 2, &extents1);
				XftTextExtents8(display, gsfont, (XftChar8*)s, len, &extents);
				XftDrawString8(draw, &tfgcolor, gfont, grid[i].x1-3*wchar*wscreen/1024-extents1.width, grid[i].y1+(gfont->ascent+gfont->descent)*1/4, (XftChar8*)s1, 2);
				XftDrawString8(draw, &tfgcolor, gsfont, grid[i].x1-3*wchar*wscreen/1024, grid[i].y1+(gfont->ascent+gfont->descent)*(-2)/4, (XftChar8*)s, len);
				XftDrawChange(draw, pixmap);
				XftDrawString8(draw, &tfgcolor, gfont, grid[i].x1-3*wchar*wscreen/1024-extents1.width, grid[i].y1+(gfont->ascent+gfont->descent)*1/4, (XftChar8*)s1, 2);
				XftDrawString8(draw, &tfgcolor, gsfont, grid[i].x1-3*wchar*wscreen/1024, grid[i].y1+(gfont->ascent+gfont->descent)*(-2)/4, (XftChar8*)s, len);
				XftDrawChange(draw, window);

			}
		}
		XDrawSegments(display,window,gc,grid,n);
		XDrawSegments(display,pixmap,gc,grid,n);
		
		for (int i=0 ; i<n ; i++) {
			grid[i].x2 = xf + wf;
			double d = ticks[i]/pow(10.0,floor(log10(ticks[i])));
			if (d!=1.0) {
				grid[i].x1 = grid[i].x2-SUBTICKSIZE;
			} else {
				grid[i].x1 = grid[i].x2-TICKSIZE;
			}
		}
		
		XDrawSegments(display,window,gc,grid,n);
		XDrawSegments(display,pixmap,gc,grid,n);
	}
		
	free(grid);

}

void gline (double c, double r, double c1, double r1, int col, int st, int width)
/***** gline
	draw a line.
*****/
{   
	if (fabs(r)>MAXPLOT) return;
	if (fabs(c)>MAXPLOT) return;
	if (fabs(r1)>MAXPLOT) return;
	if (fabs(c1)>MAXPLOT) return;
	if (metafile)
	{	
		intwrite(1);
		longwrite(c); 
		longwrite(r); 
		longwrite(c1); 
		longwrite(r1);
		intwrite(col); 
		intwrite(st);
		intwrite(width);
	}
	if (st==line_none) setcolor(0);
	else setcolor(col);
	setline(width,st);
	draw(column(c),row(r),column(c1),row(r1));
}

void gmarker (double c, double r, int col, int type)
/***** gmarker
	plot a single marker on screen.
*****/
{	
	if (fabs(r)>MAXPLOT) return;
	if (fabs(c)>MAXPLOT) return;
	if (metafile)
	{	
		intwrite(0x0204);
		intwrite(c); 
		intwrite(r);
		intwrite(col); 
		intwrite(type);
	}
	setcolor(col);
	switch(type)
	{
	case marker_dot : plot(column(c),row(r)); break;
	case marker_plus :	
		draw(column(c+10),row(r),column(c-10),row(r));
		draw(column(c),row(r-10),column(c),row(r+10));
		break;
	case marker_square :
	case marker_circle :
		draw(column(c+10),row(r-10),column(c+10),row(r+10));
		draw(column(c+10),row(r+10),column(c-10),row(r+10));
		draw(column(c-10),row(r+10),column(c-10),row(r-10));
		draw(column(c-10),row(r-10),column(c+10),row(r-10));
		break;
	case marker_diamond :
		draw(column(c),row(r-10),column(c+10),row(r));
		draw(column(c+10),row(r),column(c),row(r+10));
		draw(column(c),row(r+10),column(c-10),row(r));
		draw(column(c-10),row(r),column(c),row(r-10));
		break;
	case marker_star :
		draw(column(c+10),row(r),column(c-10),row(r));
		draw(column(c),row(r-10),column(c),row(r+10));		
	default :
		draw(column(c+10),row(r-10),column(c-10),row(r+10));
		draw(column(c-10),row(r-10),column(c+10),row(r+10));
	}
}

void gfill (double c[], int st, int n, int connect[])
/***** gfill
	fill an area given by n pairs of points (in c: x,y,x,y,...)
	with the style.
*****/
{	
	int i,cc[64],ci[64],j,k,count;
	XPoint points[64];
	if (n>32) return;
	for (i=0; i<2*n; i++) if (fabs(c[i])>MAXPLOT) return;
	if (metafile)
	{	
		intwrite(3);
		intwrite(n);
		for (i=0; i<2*n; i+=2)
		{	
			longwrite(c[i]); 
			longwrite(c[i+1]);
			intwrite(connect[i]);
		}
		intwrite(st);
	}
	for (i=0; i<2*n; i+=2) ci[i]=(int)column(c[i]);
	for (i=1; i<2*n; i+=2) ci[i]=(int)row(c[i]);
	for (i=0; i<n; i++) { 
		points[i].x=ci[2*i]; 
		points[i].y=ci[2*i+1]; 
	}
	setcolor(st==fill_filled?0:2);
#ifdef WINDOW
	XFillPolygon(display,window,gc,points,n,Complex,CoordModeOrigin);
#endif
	XFillPolygon(display,pixmap,gc,points,n,Complex,CoordModeOrigin);
	i=0;
	setline(0,line_solid);
	setcolor(1);
	while (i<n)
	{	
		j=0; 
		count=0;
		while (i<n && connect[i])
		{	
			cc[j++]=ci[2*i]; 
			cc[j++]=ci[2*i+1];
			i++; 
			count++;
		}
		if (i==n)
		{	
			cc[j++]=ci[0]; 
			cc[j++]=ci[1]; 
			count++;
		}
		else
		{	
			cc[j++]=ci[2*i]; 
			cc[j++]=ci[2*i+1]; 
			count++;
		}
		for (k=0; k<count-1; k++)
			draw(cc[2*k],cc[2*k+1],cc[2*k+2],cc[2*k+3]);
		while (i<n && !connect[i]) i++;
	}
}

int oldstipple=-1,oldscolor=-1;

void sethue (double hue, int col)
{	int k,nostipple=0;
	if (col>=usedcolors) col=usedcolors-1;
	else if (col==0)
	{	hue-=floor(hue); col=floor(hue*(usedcolors-2)*0.9999)+2;
		nostipple=1;
	}
	else if (col<0)
	{	hue-=floor(hue);
		hue*=(usedcolors-2)*0.9999;
		col=floor(hue)+2;
	}
	if (oldscolor!=col) XSetForeground(display,stipplegc,color[oldscolor=col]);
	if (!nostipple)
	{	hue-=floor(hue); hue*=0.9999; k=(int)(hue*64);
	}
	else k=0;
	if (k!=oldstipple) XSetStipple(display,stipplegc,stipple[oldstipple=k]);
}

void gfillh (double c[], double hue, int color, int connect)
/***** gfillh
	fill an area given by 4 pairs of points (in c: x,y,x,y,...)
	with the hue and color. connect determines, if an outline is
	to be drawn.
*****/
{	
	int i,ci[8];
	XPoint points[5];
	for (i=0; i<8; i++) if (fabs(c[i])>MAXPLOT) return;
	if (metafile)
	{	
		intwrite(7);
		for (i=0; i<8; i+=2)
		{	
			longwrite(c[i]); 
			longwrite(c[i+1]);
		}
		longwrite(hue);
		intwrite(color);
		intwrite(connect);
	}
	for (i=0; i<8; i+=2) ci[i]=(int)column(c[i]);
	for (i=1; i<8; i+=2) ci[i]=(int)row(c[i]);
	for (i=0; i<4; i++) { 
		points[i].x=ci[2*i]; 
		points[i].y=ci[2*i+1]; 
	}
	points[4].x=points[0].x;
	points[4].y=points[0].y;
	sethue(hue,color);
#ifdef WINDOW
	XFillPolygon(display,window,stipplegc,points,4,Complex,CoordModeOrigin);
#endif
	XFillPolygon(display,pixmap,stipplegc,points,4,Complex,CoordModeOrigin);
	if (!connect) return;
	setline(0,line_solid);
	setcolor(1);
	XDrawLines(display,pixmap,gc,points,5,CoordModeOrigin);
#ifdef WINDOW
	XDrawLines(display,window,gc,points,5,CoordModeOrigin);
#endif
}

void gbar (double c, double r, double c1, double r1, double hue,
	int color, int connect)
{	
	int x,y,w,h;
	if (fabs(c)>MAXPLOT) return;
	if (fabs(r)>MAXPLOT) return;
	if (fabs(c1)>MAXPLOT) return;
	if (fabs(r1)>MAXPLOT) return;
	x=(int)column(c); y=(int)row(r);
	w=(int)column(c1)-x; h=(int)row(r1)-y;
	if (w<=0) w=1;
	if (h<=0) h=1;
	if (metafile)
	{	
		intwrite(8);
		longwrite(c); longwrite(r);
		longwrite(c1); longwrite(r1);
		longwrite(hue);
		intwrite(color);
		intwrite(connect);
	}
	sethue(hue,color);
#ifdef WINDOW
	XFillRectangle(display,window,stipplegc,x,y,w,h);
#endif
	XFillRectangle(display,pixmap,stipplegc,x,y,w,h);
	if (!connect) return;
	setline(0,line_solid);
	setcolor(1);
	XDrawRectangle(display,pixmap,gc,x,y,w,h);
#ifdef WINDOW
	XDrawRectangle(display,window,gc,x,y,w,h);
#endif
}

void gtext (double c, double r, char *text, int color, int align)
/***** gtext
	output a graphic text on screen.
*****/
{	
	int len = strlen(text);
	XGlyphInfo extents;
	
	if (metafile)
	{	
		intwrite(4); 
		longwrite(c);
		longwrite(r); 
		intwrite(color); 
		intwrite(align);
		stringwrite(text);
	}
	setcolor(color);

	switch (align) {
	case 0:
		XftTextExtents8(display, gfont, (XftChar8*)text, len, &extents);
		XftDrawString8(draw, &tfgcolor, gfont, column(c), row(r)+gfont->ascent, (XftChar8*)text, len);
		XftDrawChange(draw, pixmap);
		XftDrawString8(draw, &tfgcolor, gfont, column(c), row(r)+gfont->ascent, (XftChar8*)text, len);
		break;
	case 1:
		XftTextExtents8(display, gfont, (XftChar8*)text, len, &extents);
		XftDrawString8(draw, &tfgcolor, gfont, column(c)-extents.width/2, row(r)+gfont->ascent, (XftChar8*)text, len);
		XftDrawChange(draw, pixmap);
		XftDrawString8(draw, &tfgcolor, gfont, column(c)-extents.width/2, row(r)+gfont->ascent, (XftChar8*)text, len);
		break;
	case 2:
		XftTextExtents8(display, gfont, (XftChar8*)text, len, &extents);
		XftDrawString8(draw, &tfgcolor, gfont, column(c)-extents.width, row(r)+gfont->ascent, (XftChar8*)text, len);
		XftDrawChange(draw, pixmap);
		XftDrawString8(draw, &tfgcolor, gfont, column(c)-extents.width, row(r)+gfont->ascent, (XftChar8*)text, len);
		break;
	case 3:
		break;
	case 4:
		XftTextExtents8(display, gvfont, (XftChar8*)text, len, &extents);
		XftDrawString8(draw, &tfgcolor, gvfont, column(c)+extents.width, row(r)+extents.height/2, (XftChar8*)text, len);
		XftDrawChange(draw, pixmap);
		XftDrawString8(draw, &tfgcolor, gvfont, column(c)+extents.width, row(r)+extents.height/2, (XftChar8*)text, len);
		break;
	case 5:
		break;
	default:
		break;
	}
	
	XftDrawChange(draw, window);
}

void scale (double s)
/***** scale
	scale the screen according s = true width / true height.
*****/
{	
	if (metafile)
	{	
		intwrite(5);
		intwrite((int)(s*1000));
	}
}

void translate (XKeyEvent *event, int *key, scantyp *scan);
void mouse (int *x, int *y)
/****** mouse
	wait, until the user marked a screen point with the mouse.
******/
{	
	XEvent event;
	int taste;
	scantyp scan;
	while (1)
	{
		XWindowEvent(display,window,mask,&event);
		if (event.type==ButtonPress)
		{	
			*x=(int)((event.xkey.x)*1024.0/wscreen);
			*y=(int)((event.xkey.y)*1024.0/hscreen);
			return;
		}
		else if (event.type==KeyPress)
		{
			translate(&(event.xkey),&taste,&scan);
			if (scan==escape)
			{
				error=1; return;
			}
		}
	}
}

void getpixel (double *x, double *y)
{	*x=1024.0/wscreen;
	*y=1024.0/hscreen;
}


/********************* text screen **************************/
/*
             +---------------------------------+
  textstart->|
             |
             |
             |
             |
             |
             |
             +---------------------------------+
textwindow-> |     |
             |-----+ cy
             |     cx       SCREEN
             |
             |     textend->\0X
             +---------------------------------+
 
 
 */
 
void edit_on_cb (void)
{
}

void edit_off_cb (void)
{
}

void move_cl_cb (void)
{
	if (cx>0) cx--;
}

void move_cr_cb (void)
{
	cx++;
}

void cursor_on_cb (void)
{
	int h=tfont->ascent+tfont->descent;
	char cstring[]=" ";

	if (scrolled || cx>cwscreen) return;
	
	cstring[0]=textend[cx];
	if (cstring[0]==0) cstring[0]=' ';
	XftDrawRect(draw, &tfgcolor,
		textoffset+cx*textwidth,
		cy*textheight+textoffset,
		textwidth, h
	);
	XftDrawString8(draw,&tbgcolor,tfont,
		textoffset+cx*textwidth,
		cy*textheight+tfont->ascent+textoffset,
		(XftChar8*)cstring,1);

	cursoron=1;
}

void cursor_off_cb (void)
{
	int h=tfont->ascent+tfont->descent;
	char cstring[]=" ";

	if (scrolled || cx>cwscreen) return;
	
	cstring[0]=textend[cx];
	if (cstring[0]==0) cstring[0]=' ';
	XftDrawRect(draw, &tbgcolor,
		textoffset+cx*textwidth,
		cy*textheight+textoffset,
		textwidth, h
	);
	XftDrawString8(draw,&tfgcolor,tfont,
		textoffset+cx*textwidth,
		cy*textheight+tfont->ascent+textoffset,
		(XftChar8*)cstring,1);

	cursoron=0;
}

void page_up_cb(void)
{
	int i;
	
	if (!in_text || textwindow==textstart) return;
	if (!scrolled)
	{	oldtextwindow=textwindow;
		scrolled=1;
	}
	for (i=0; i<chscreen-1; i++)
	{	textwindow--;
		while (textwindow>textstart)
		{	if (*(textwindow-1)==0) break;
			textwindow--;
		}
		if (textwindow==textstart) break;
	}
	refresh_window();
}

void page_down_cb(void)
{
	int i;
	
	if (!scrolled || !in_text) return;
	for (i=0; i<chscreen-1; i++)
	{	textwindow+=strlen(textwindow)+1;
		if (textwindow>=oldtextwindow) break;
	}
	if (textwindow>=oldtextwindow)
	{	textwindow=oldtextwindow;
		scrolled=0;
	}
	refresh_window();
}

/* textline
 *   draw a line of text starting at char pos in the string p+x
 *   at pos (x,y) [char,line]
 */
static void textline (char *p, int x, int y)
{
	int len=strlen(p+x);
	XftDrawRect(draw,&tbgcolor,
		textoffset+x*textwidth,
		y*textheight+textoffset,
		textwidth*len,
		tfont->ascent+tfont->descent
	);
	XftDrawString8(draw,&tfgcolor,tfont,textoffset+x*textwidth,
		y*textheight+tfont->ascent+textoffset,(XftChar8*)p+x,
		strlen(p+x));
}

/* clearline
 *   clear a line of text starting at char pos in the string p+x
 *   at pos (x,y) [char,line]
 */
static void clearline (char *p, int cx, int cy)
{
	XftDrawRect(draw,&tbgcolor,
		textoffset+cx*textwidth,textoffset+cy*textheight,
		wscreen-(textoffset+cx*textwidth),textheight);
	memset(p+cx,0,textstart+TEXTSIZE-(p+cx));
}

/* clear_eol [CALLBACK] */
void clear_eol (void)
{
	/* SEEM TO BUG! when XftDrawRect commented and inserting in
	   the middle of a string
	 */
	clearline(textend,cx,cy);
}

/* clear_screen [CALLBACK]
 *   clear the whole text screen, reset the buffer
 */
void clear_screen (void)
{	XftDrawRect(draw,&tbgcolor,0,0,wscreen,hscreen);
	cx=0; cy=0; scrolled=0;
	textwindow=textend=textstart;
	memset(textstart,0,TEXTSIZE);
}

/* textupdate
 *   update the text screen (starting from textwindow pos in the buffer)
 */
static void textupdate (void)
{	char *tp;
	int i;
	XftDrawRect(draw,&tbgcolor,0,0,wscreen,hscreen);
	tp=textwindow; i=0;
	while (tp<=textend && i<chscreen) {
		textline(tp,0,i);
		i++; tp+=strlen(tp)+1;
	}
	XFlush(display);
}

/* newline
 *   update textend, scroll the buffer when it is near to be full,
 *   scroll the screen display when we are at the bottom line
 */
static void new_line (void)
{	int length;
	char *tp;
	cy++; cx=0;
	textend+=strlen(textend)+1;
	/* if buffer near to be full, cut 1/8th of the buffer */
	if (textend>textstart+TEXTSIZE-256) {
		tp=textstart+(TEXTSIZE-256)/8;
		tp+=strlen(tp)+1;
		memmove(textstart,tp,textend-tp);
		length=tp-textstart;
		textend-=length; textwindow-=length;
	}
	/* if at the bottom of the screen, scroll it of 1 line up */
	if (cy>=chscreen) {
		cy--; textwindow+=strlen(textwindow)+1;
		XCopyArea(display,window,window,gc,
			0,textoffset+textheight,
			wscreen,
			(chscreen-1)*textheight,
			0,textoffset);
		clearline(textend,0,cy);
	}
}

/* gprint [CALLBACK]
 *   print a line onto the screen from pos (cx,cy) starting at
 *   buffer pos textend+cx, parse tabs and \n
 */
void gprint (char *s)
{
	int cx0=cx,cx1,i;
	if (scrolled) {
		textwindow=oldtextwindow;
		scrolled=0;
		refresh_window();
	}
	while (*s) {
		switch(*s) {
		case 10 : s++; textline(textend,cx0,cy); cx0=0;
			new_line(); break;
		case 9 :
			cx1=(cx/4+1)*4;
			for (i=cx; i<cx1; i++) textend[i]=' ';
			cx=cx1; s++;
			break;
		default :
			textend[cx]=*s; cx++;
			s++;
			break;
		}
		if (textend+cx>textstart+TEXTSIZE) {
			cx0=0; new_line();
		}
	}
	textline(textend,cx0,cy);
	XFlush(display);
}

void ev_scroll_up()
{
//	fprintf(stderr,"ev scroll up\n");
}

void ev_scroll_down()
{
//	fprintf(stderr,"ev scroll down\n");
}

/**************** refresh routine **********************/

void refresh_window (void)
/* refresh the screen */
{
	if (in_text) textupdate();
	else
	{
		XCopyArea(display,pixmap,window,gc,0,0,wscreen,hscreen,0,0);
		XFlush(display);
	}
}

/******************** keyboard and other events **********************/

void translate (XKeyEvent *event, int *key, scantyp *scan)
/* Translate events into key codes */
{
	int length;
	char buffer[65];
	KeySym keysym;
	XComposeStatus status;

	*scan=0; *key=0;
	length=XLookupString(event,buffer,64,&keysym,&status);

	if ((event->state & ControlMask) && keysym==0x64) {
		*scan = eot;
		return;
	}
	if (((keysym>=' ') && (keysym<='~')) && length>0)
	{
		*key=buffer[0];
		return;
	}
	switch (keysym)
	{
		case XK_Return : *scan=enter; break;
		case XK_Escape : *scan=escape; break;
		case XK_BackSpace : *scan=backspace; break;
		case XK_Delete : *scan=delete; break;
		case XK_Prior: *scan=page_up; break;
		case XK_Next: *scan=page_down;break;
		case XK_Up :
			if (event->state & ControlMask) *scan=clear_home;
			else *scan=cursor_up ;
			break;
		case XK_Down :
			if (event->state & ControlMask) *scan=clear_home;
			else *scan=cursor_down; 
			break;
		case XK_Right :
			if (event->state & ShiftMask) *scan=line_end;
			else if (event->state & ControlMask) *scan=word_right;
			else *scan=cursor_right; 
			break;
		case XK_Left :
			if (event->state & ShiftMask) *scan=line_start;
			else if (event->state & ControlMask) *scan=word_left;                         else *scan=cursor_left;
			break;
		case XK_Insert : *scan=help; break;
		case XK_F1 : *scan=fk1; break;
		case XK_F2 : *scan=fk2; break;
		case XK_F3 : *scan=fk3; break;
		case XK_F4 : *scan=fk4; break;
		case XK_F5 : *scan=fk5; break;
		case XK_F6 : *scan=fk6; break;
		case XK_F7 : *scan=fk7; break;
		case XK_F8 : *scan=fk8; break;
		case XK_F9 : *scan=fk9; break;
		case XK_F10 : *scan=fk10; break;
		case XK_F11 : *scan=fk11; break;
		case XK_F12 : *scan=fk12; break;
		case XK_KP_Enter : *scan=enter; break;
		case XK_Tab : *scan=switch_screen; break;
		case XK_End :
			if (event->state & ShiftMask) *scan=clear_home;
			else if (event->state & ControlMask) *scan=clear_home;
			else *scan=line_end;
			break;
		case XK_Begin :
		case XK_Home :
			if (event->state & ShiftMask) *scan=clear_home;
			else if (event->state & ControlMask) *scan=clear_home;
			else *scan=line_start;
			break;
	}
}

void process_event (XEvent *event)
{
	XEvent dummyevent;
	switch (event->type)
	{
		case ButtonPress:
			if (event->xbutton.button==4) {
				if (in_text) ev_scroll_up();
			} else if (event->xbutton.button==5) {
				if (in_text) ev_scroll_down();
			}
			break;
		case ButtonRelease:
			if (event->xbutton.button==1) {
				fprintf(stderr,"button1 release x=%d, y=%d\n",event->xbutton.x,event->xbutton.y);
			}
			break;
		case MotionNotify:
			fprintf(stderr,"button1 release x=%d, y=%d\n",event->xmotion.x,event->xmotion.y);
			break;
		case GraphicsExpose:
			if (event->xgraphicsexpose.count>0) break;
//			fprintf(stderr,"Graphics Expose\n");
			refresh_window(); break;
		case Expose:
			if (event->xexpose.count>0) break;
//			fprintf(stderr,"Expose\n");
			refresh_window(); break;
		case ConfigureNotify :
			while (XCheckWindowEvent(display,window,mask,&dummyevent));
			if (event->xconfigure.width==wscreen &&
			    event->xconfigure.height==hscreen) break;
//			fprintf(stderr,"Configure Notify\n");
			wscreen=event->xconfigure.width;
			hscreen=event->xconfigure.height;
			computechar();
			computetext();
			XFreePixmap(display,pixmap);
			pixmap=XCreatePixmap(display,window,wscreen,hscreen,depth);
			gclear();
			textwindow=textend-1;
			while (textwindow>=textstart && *textwindow) textwindow--;
			textwindow++; oldtextwindow=textwindow; cy=0;
			scrolled=0;
			break;
		case DestroyNotify :
			exit(1); break;
	}
}

int wait_key (scantyp* scan)
/***** 
	wait for a keystroke. return the scancode.
*****/
{   
	int ch;
	XEvent event;
	
	Window sel_owner;
	static unsigned char* selection=NULL;
	static unsigned char* sel_ptr=NULL;
	
	*scan=key_none; ch=0;
	while (1) {
		if (in_text && selection) {
			while (*sel_ptr=='\r') sel_ptr++;
			ch=*sel_ptr++;
			if (ch=='\n') *scan=enter;
			else if (ch=='\t') ch=' ';
			if (*sel_ptr==0) {
				XFree(selection);
				selection = NULL;
			}
			break;
		}
		XNextEvent(display, &event);
		if (event.type==KeyPress) {
			translate(&(event.xkey),&ch,scan);
			if (*scan || ch) break;
		} else if (event.type==ButtonPress && 
			event.xbutton.button==Button2) {
			sel_owner = XGetSelectionOwner(display, XA_PRIMARY);
		    if (sel_owner != None) {
		    	/* - ask the server for the selection */
				XConvertSelection(display, XA_PRIMARY, XA_STRING,
					sel_prop, window, CurrentTime);
			}
		} else if (event.type==SelectionNotify){
			if (event.xselection.property!=None) {
			    Atom da, type;
			    int di;
			    unsigned long size, dul;
			
			    /* Dummy call to get type and size. */
			    XGetWindowProperty(display, window, event.xselection.property, 0, 0, False, AnyPropertyType,
			                       &type, &di, &dul, &size, &selection);
			    XFree(selection);
				selection=NULL;
				
/*			    Atom incr = XInternAtom(dpy, "INCR", False);
			    if (type == incr)
			    {
			        fprintf(stderr,"Data too large and INCR mechanism not implemented\n");
			        return;
			    }*/
			
			    /* Read the data in one go. */
//			    fprintf(stderr,"Property size: %lu\n", size);
			
			    XGetWindowProperty(display, window, event.xselection.property, 0, size, False, AnyPropertyType,
			                       &da, &di, &dul, &dul, &selection);
//			    fprintf(stderr,"%s", selection);
			    sel_ptr = selection;
			
			    /* Signal the selection owner that we have 
			       successfully read the data. */
			    XDeleteProperty(display, window, event.xselection.property);
			}
		} else process_event(&event);
	}
	return ch;
}

int test_key (void)
/***** test_key
	see, if user pressed the keyboard.
	return the scan_code, if he did.
*****/
{   
	XEvent event;
	int key;
	scantyp scan;
	
	if (userbreak)
	{
		userbreak=0; return escape;
	}
	else if (XCheckWindowEvent(display,window,mask,&event))
	{
		if (event.type==KeyPress)
		{	translate(&(event.xkey),&key,&scan);
			return scan;
		}
		else process_event(&event);
	}
	return 0;
}

/**************** directory *******************/

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

/* search path list:
 *   path[0]       --> current directory
 *   path[npath-1] --> INSTALL_DIR/share/retro/progs/ 
 */
char * path[MAX_PATH];
int npath=0;

char *cd (char *dir)
/* sets the path if dir!=0 and returns the path */
{
	if (dir && dir[0]) {
		char* p = path_expand(dir);
		if (p) {
			free(path[0]);
			path[0] = p;
			chdir(p);
		}
	}
	return path[0];
}

static int match (char *pat, char *s)
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

/*
 *	scan a directory and get :
 *		files : an array of entries matching the pattern
 *		files_count : number of files entries
 *	the function returns the max length of a file entry
 */
static int entry_cmp(const char**e1, char**e2)
{
	return strcmp(*e1,*e2);
}

int scan_dir(char *dir_name, char *pat, char ** files[], int *files_count)
{
	DIR *dir;
	struct dirent *entry;
	int entry_count=0, len=0;
	char **buf = NULL;

	dir = opendir(dir_name);

	if (dir) {
		while((entry = readdir(dir)) != NULL) {
			if (match(pat,entry->d_name)) {
				if (strcmp(entry->d_name,".")==0 || strcmp(entry->d_name,"..")==0) 
					continue;
				int isdir=path_is_dir(entry->d_name);
				int l=strlen(entry->d_name);
				len = len>l ? len : l;
				buf = (char**)realloc(buf,(entry_count+1)*sizeof(char *));
				if (buf) {
					buf[entry_count]=(char*)malloc(isdir ? l+2 : l+1);
					if (buf[entry_count]) {
						strcpy(buf[entry_count],entry->d_name);
						if (isdir) {
							strcat(buf[entry_count],"/");
						}
						entry_count++;
					} else break;
				} else break;
			}
		}

		closedir(dir);
		
		if (buf)
			qsort(buf,entry_count,sizeof(char*),(int (*)(const void*, const void*))entry_cmp);
	}

	*files = buf;
	*files_count = entry_count;

	return len;
}

#if 0
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
		dir=opendir(path[0]);
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
#endif
/***************** clear screens ********************/

int execute (char *name, char *args)
/**** execute
	call an external program, return 0, if there was no error.
****/
{	return 0;
}

#ifndef SY_CLK_TCK
#define SY_CLK_TCK 50
#endif

double myclock (void)
/* Return the time in seconds */
{
#ifdef RS6000
	struct timestruc_t t;
	gettimer(TIMEOFDAY,&t);
	return (t.tv_sec+t.tv_nsec/1000000000.0);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (double)(tv.tv_sec+tv.tv_usec*1e-6);
/*	struct tms b;
	times(&b);
	return (double)(b.tms_utime)/(SY_CLK_TCK);*/
#endif
}

void sys_wait (double time, int *scan)
{
	fd_set fds;
	int ret, xfd;
	struct timeval tv, *tvp=NULL;
	
	FD_ZERO(&fds);
	XSync(display,0);
	xfd=XConnectionNumber(display);
	FD_SET(xfd,&fds);
	
	if (time>0.0) {
		tv.tv_sec = (long)time;
		tv.tv_usec = (long)((time-floor(time))*1e6);
		tvp=&tv;
	}
	
	while (1) {
		ret=select(xfd+1,&fds,NULL,NULL,tvp);
		if (ret==-1) {
			*scan=escape;
			perror("select");
			return;
		} else if (!ret) {	// timeout
			break;
		}
		
		if (FD_ISSET(xfd,&fds)){
			*scan=test_key();
			if (*scan) {
				if (*scan==switch_screen)
				{	if (in_text)
						graphic_mode();
					else
						text_mode();
					*scan=0;
				} else if (*scan==escape || *scan==enter) return;
//				else break;
			}
		}
	}
	*scan=0;
}

/************ main *******************/

char titel[]="This is Retro, Version 1.00 compiled %s.\n\n"
	"Type help(Return) for help.\n"
	"Enter command: (%ld Bytes free.)\n\n";

void usage (void)
{
	fprintf(stderr,"Use: xretro  [-d DISPLAY] [-geom GEOMETRY] [-f FONT] [-g FONT]\n"
		"[-0..15 COLOR] [-s KBYTES] files\n");
}

void get_geometry (char *s)
{
	long d;
	char *end;
	d=strtol(s,&end,10);
	if (!end || end==s) return; else { userwidth=d; s=end; }
	if (*s++!='x') { usage(); exit(1); }
	d=strtol(s,&end,10);
	if (!end || end==s) return; else { userheight=d; s=end; }
	usersize=1;
	if (*s==0) return;
	d=strtol(s,&end,10);
	if (!end || end==s) return; 
	else 
	{ 
		userx=d;
		if (*s=='-' && d==0) userx=-1;
		s=end; 
	}
	userpos=1;
	if (*s==0) return;
	d=strtol(s,&end,10);
	if (!end || end==s) return;
	else 
	{ 
		usery=d;
		if (*s=='-' && d==0) usery=-1;
		s=end; 
	}
}

void setint (int code)
{       
	userbreak=1;
	signal(SIGTERM,setint);
}

#ifdef FPE

void setfpe (int code)
{       
	error=1;
	signal(SIGFPE,setfpe);
}

#endif

int ioerrorhandler (Display *display)
{	exit(1)	;
	return 0;
}

int main (int argc, char *argv[])
{
	XEvent event;
	int nn;

	unsigned long stacksize=1024*1024l;
	
	signal(SIGINT,setint);
#ifdef FPE
	signal(SIGFPE,setfpe);
#endif
	XSetIOErrorHandler(ioerrorhandler);

	/* parse args */
	while (argc>1)
	{
		if (argv[1][0]=='=')
		{	get_geometry(argv[1]+1);
			argc--; argv++;
		}
		else if (argv[1][0]=='-')
		{
			switch(argv[1][1])
			{
				case 'f' :
					if (argv[1][2])
					{	strcpy(tfontname,argv[1]+2); break;
					}
					else if (argv[2])
					{	strcpy(tfontname,argv[2]); argc--; argv++;
						break;
					}
					break;
				case 'g' :
					if (strncmp(argv[1],"-geom",5)==0)
					{	if (argv[2])
						{	get_geometry(argv[2]); argc--; argv++;
						}
					}
					else if (argv[1][2])
					{	strcpy(gfontname,argv[1]+2); break;
					}
					else if (argv[2])
					{	strcpy(gfontname,argv[2]); argc--; argv++;
						break;
					}
					break;
				case 's' : 
					if (argv[1][2])
					{	stacksize=atoi(argv[1]+2)*1024l; break;
					}
					else if (argv[2])
					{	stacksize=atoi(argv[2])*1024l; argc--; argv++;
						break;
					}
					break;
				case 'd' :
					strcpy(sdisp,argv[2]); argc--; argv++; break;
				default :
					if (sscanf(argv[1]+1,"%d",&nn)==1 && nn>=0 && nn<MAXCOLORS)
					{	colorname[nn]=argv[2]; argc--; argv++;
						break;
					}
					usage(); exit(1);
			}
			argc--; argv++;
		}
		else break;
	}
	
	/* Allocate the stack, initialize stack limit pointers */
	if (stacksize<64*1024l) stacksize=64*1024l;
	ramstart=(char *)malloc(stacksize);
	
	if (!ramstart) die("can't allocate the stack");
	
	ramend=ramstart+stacksize;
	
	
	/* set up default pathes and directory */
	char* s=getenv("RETRO");
	if (!s) s="~/.retro/progs:";
	
	// get a buffer of the required size
	char str[strlen(s)+1];
	s=strcpy(str,s);
	
	path[0]=path_expand(".");
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

	nextcheck=myclock();
	
	initX();
	XWindowEvent(display,window,ExposureMask,&event);
	process_event(&event);
	
	output1(titel,__DATE__,(unsigned long)(ramend-ramstart));

	main_loop(argc,argv);
	return 0;
}
