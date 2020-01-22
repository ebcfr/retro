#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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

char outputs[256];

#include "header.h"
#include "sysdep.h"

#define RESERVE 16*1024l
long reserve=RESERVE;

char *graph_screen=0,*text_screen=0;
int graphscr=1,colors,wscreen,hscreen;
int linelength,wchar,hchar,wchart,hchart,in_text=1;
int userbreak=0;
int planes=0x01;
unsigned long mask;

#define TEXTSIZE (128*1024l)

char textstart[TEXTSIZE]={0};

int maxlines,cx,cy;
int textheight,textwidth,textoffset;
int cursoron=1,thchar,twchar,editor=0,scrolled=0;
char *textend=textstart,*textwindow=textstart,*oldtextwindow;

long memsize=1024*1024l;

double nextcheck;

/*************** XWindow things ******************************/

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

#define maxcolors 16

Display *display;
char sdisp[64]="";
int screen, depth;
int winit=0;
Window window;
Atom sel_prop;
GC gc,cleargc,stipplegc,textgc,invtextgc;
Pixmap stipple[64];
unsigned long blackpixel, whitepixel;
Colormap colormap;
int usedcolors=maxcolors;
unsigned long color[maxcolors];
char *colorname[maxcolors]=
{
	"White","Black","Green","Lightblue","Blue","SteelBlue",
	"Khaki","Tan","Grey","Yellow","Green","Red","LightBlue",
	"LightSteelBlue","LimeGreen","Navy"
};
XFontStruct *font,*textfont;
Pixmap pixmap;

char fontname[32]=GFONT;
char textfontname[32]=FONT;
int userwidth=700,userheight=700,userx=0,usery=0,usersize=0,userpos=0;

#include "icon.h"

void process_event (XEvent *event);

void computechar (void)
{
	hchar=(font->ascent+font->descent+2)*1024l/hscreen;
	wchar=(XTextWidth(font,"m",1))*1024l/wscreen;
}

void initX (void)
/* Sets up the connection to the X server */
{	
	display=XOpenDisplay(sdisp);
	if (!display)
	{       
		fprintf(stderr,"ERROR: Connetion to X server %s failed.\n",
		    sdisp);
		exit(1);
	}
	screen=DefaultScreen(display); /* screen # */
	depth=DefaultDepth(display,screen); /* color depth */
	switch(depth)
	{
		case 1 : usedcolors=2; break;
		case 2 : usedcolors=4; break;
		case 3 : usedcolors=8; break;
		default : usedcolors=maxcolors; break;
	}
	blackpixel=BlackPixel(display,screen);
	whitepixel=WhitePixel(display,screen);

	/* load a font */
	textfont=XLoadQueryFont(display,textfontname);
	if (!textfont)
	{
		fprintf(stderr,"Cannot find %s font\n",textfontname);
		exit(1);
	}
	textheight=textfont->ascent+textfont->descent+2;
	textwidth=XTextWidth(textfont,"m",1);
	if (textwidth!=XTextWidth(textfont,"i",1))
	{
		fprintf(stderr,
			"You cannot use the proportional font %s!\n",textfontname);
		exit(1);
	}
	textoffset=textwidth/2;

	font=XLoadQueryFont(display,fontname);
	if (!font)
	{
		fprintf(stderr,"Cannot find %s font\n",fontname);
		exit(1);
	}

	sel_prop=XInternAtom(display,"SEL_PROP",False);
	
	XFlush(display);

}

void quitX (void)
/* Disconnect from server */
{   
	static int quitted=0;
	if (quitted) return;
	XDestroyWindow(display,window);
	XFreeFont(display,font);
	XFreePixmap(display,pixmap);
	XCloseDisplay(display);
}

int createGC (Window window, GC *gc)
/* Create a new graphic context */
{	
	XGCValues gcvalues;
	*gc=XCreateGC(display,window,(unsigned long)0,&gcvalues);
	if (*gc==0) exit(1);
	return 1;
}

void create_contexts (Window window)
{
	int i,j,k,st;
	unsigned char byte,bits[512];
	XColor rgb,hardware;
	
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

	createGC(window,&gc);
	XSetForeground(display,gc,blackpixel);
	XSetBackground(display,gc,whitepixel);
	XSetFont(display,gc,font->fid);
				
	createGC(window,&cleargc);
	XSetForeground(display,cleargc,whitepixel);
	XSetBackground(display,cleargc,blackpixel);

	createGC(window,&stipplegc);
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

	createGC(window,&textgc);
	XSetForeground(display,textgc,blackpixel);
	XSetBackground(display,textgc,whitepixel);
	XSetFont(display,textgc,textfont->fid);

	createGC(window,&invtextgc);
	XSetForeground(display,invtextgc,whitepixel);
	XSetBackground(display,invtextgc,blackpixel);
	XSetFont(display,invtextgc,textfont->fid);
}	

Window open_window (int x, int y, int width, int height, int flag)
/* create and open a window plus a graphic context and a pximap */
{	
	XSetWindowAttributes attributes;
	XSizeHints sizehints;
	Window window;
	Pixmap iconpixmap;
	XWMHints iconhints;
	/* XClassHint classhints; */
	unsigned int cursorshape;
	Cursor cursor;

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
	/* classhints.res_name="Euler";
	classhints.res_class="Euler";
	XSetClassHint(display,window,classhints); */

	/* Window and Icon name */
	XStoreName(display,window,"Euler");
	XSetIconName(display,window,"Euler");

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

	create_contexts(window);

	/* events, we like to receive */
	mask=KeyPressMask|ExposureMask|ButtonPressMask|StructureNotifyMask;
	XSelectInput(display,window,mask);

	/* show the window */
	XMapWindow(display,window);

	/* create a pixmap of same size */
	pixmap=XCreatePixmap(display,window,width,height,depth);
	XFillRectangle(display,pixmap,cleargc,0,0,width,height);
	XFlush(display);

	return window;
}

void computetext (void)
{
	maxlines=(hscreen-2*textoffset)/textheight;
	linelength=(wscreen-2*textoffset)/textwidth;
	linew=linelength/fieldw;
}

void grafik (void)
/* switch to a graphic screen */
{
	if (!winit)
	{	
		initX();
		atexit(quitX);
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
		winit=1;
	}
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

/*************** graphics and GEM initialization *************/

int memory_init (void)
/***** memory_init
	get colors, wscreen, hscreen.
	get the graphics screen.
	get all memory (minus reservefor editor) for stack.
*****/
{	
	long size;
	size=memsize;
	if (size<64*1024l) size=64*1024l;
	if (size<reserve) return 0;
	ramstart=(char *)malloc(size-reserve);
	if (!ramstart) return 0;
	ramend=ramstart+size;
	linelength=70;
	hchar=8;
	wchar=8;
	return 1;
}

int shrink (size_t size)
{	
	if (size==0) return 0;
	return 1;
}

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

void gtext (double c, double r, char *text, int color, int centered)
/***** gtext
	output a graphic text on screen.
*****/
{	
	int width;
	if (metafile)
	{	
		intwrite(4); 
		longwrite(c);
		longwrite(r); 
		intwrite(color); 
		intwrite(centered);
		stringwrite(text);
	}
	setcolor(color);
	if (!centered)
	{	
#ifdef WINDOW
		XDrawString(display,window,gc,
		    column(c),row(r)+font->ascent,text,strlen(text));
#endif
		XDrawString(display,pixmap,gc,
		    column(c),row(r)+font->ascent,text,strlen(text));
	}
	else if (centered==1)
	{	
		width=XTextWidth(font,text,strlen(text));
#ifdef WINDOW
		XDrawString(display,window,gc,
		    column(c)-width/2,row(r)+font->ascent,
		    text,strlen(text));
#endif
		XDrawString(display,pixmap,gc,
		    column(c)-width/2,row(r)+font->ascent,
		    text,strlen(text));
	}
	else if (centered==2)
	{	
		width=XTextWidth(font,text,strlen(text));
#ifdef WINDOW
		XDrawString(display,window,gc,
		    column(c)-width,row(r)+font->ascent,
		    text,strlen(text));
#endif
		XDrawString(display,pixmap,gc,
		    column(c)-width,row(r)+font->ascent,
		    text,strlen(text));
	}
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

/********************* text cursor ******************/

/* defined as macros */

/************** text screen **************************/

void edit_on (void)
{
}

void edit_off (void)
{
}

void show_cursor (void)
{
	char cstring[]=" ";
	if (!cursoron || cx>linelength) return;
	cstring[0]=textend[cx];
	if (cstring[0]==0) cstring[0]=' ';
	XDrawImageString(display,window,invtextgc,
		textoffset+cx*textwidth,
		cy*textheight+textfont->ascent+textoffset,
		cstring,1);
}

void hide_cursor (void)
{
	char cstring[]=" ";
	if (!cursoron || cx>linelength) return;
	cstring[0]=textend[cx];
	if (cstring[0]==0) cstring[0]=' ';
	XDrawImageString(display,window,textgc,
		textoffset+cx*textwidth,
		cy*textheight+textfont->ascent+textoffset,
		cstring,1);
}

void move_cl (void)
{
	if (cx>0)
	{
		hide_cursor(); cx--; show_cursor();
	}
}

void move_cr (void)
{
		hide_cursor(); cx++; show_cursor();
}

void cursor_on (void)
{
	if (scrolled) return;
	cursoron=1;
	show_cursor();
}

void cursor_off (void)
{
	if (scrolled) return;
	cursoron=0;
	hide_cursor();
}

void textline (char *p, int x, int y)
{
	XDrawImageString(display,window,textgc,textoffset+x*textwidth,
		y*textheight+textfont->ascent+textoffset,p+x,
		strlen(p+x));
	if (y==cy && !scrolled) show_cursor();
}

void clearline (char *p, int cx, int cy)
{
	XFillRectangle(display,window,invtextgc,
		textoffset+cx*textwidth,textoffset+cy*textheight,
		wscreen-(textoffset+cx*textwidth),textheight);
	memset(p+cx,0,textstart+TEXTSIZE-(p+cx));
	if (!scrolled) show_cursor();
}

void clear_eol (void)
{	clearline(textend,cx,cy);
}

void clear_screen (void)
{	XFillRectangle(display,window,invtextgc,0,0,wscreen,hscreen);
	cx=0; cy=0; scrolled=0;
	textwindow=textend=textstart;
	memset(textstart,0,TEXTSIZE);
}

void textupdate (void)
{	char *tp;
	int i;
	XFillRectangle(display,window,invtextgc,0,0,wscreen,hscreen);
	tp=textwindow; i=0;
	while (tp<=textend && i<maxlines)
	{	textline(tp,0,i);
		i++; tp+=strlen(tp)+1;
	}
	XFlush(display);
}

void new_line (void)
{	int length;
	char *tp;
	hide_cursor();
	cy++; cx=0;
	textend+=strlen(textend)+1;
	if (textend>textstart+TEXTSIZE-256)
	{	tp=textstart+(TEXTSIZE-256)/8;
		tp+=strlen(tp)+1;
		memmove(textstart,tp,textend-tp);
		length=tp-textstart;
		textend-=length; textwindow-=length;
	}
	if (cy>=maxlines)
	{	cy--; textwindow+=strlen(textwindow)+1;
		XCopyArea(display,window,window,gc,
			0,textoffset+textheight,
			wscreen,
			(maxlines-1)*textheight,
			0,textoffset);
		clearline(textend,0,cy);
	}
}

void gprint (char *s)
/* print a line onto the screen, parse tabs and \nl */
{
	int cx0=cx,cx1,i;
	if (scrolled)
	{	textwindow=oldtextwindow;
		show_cursor();
		scrolled=0;
		refresh_window();
	}
	while (*s)
	{
		switch(*s)
		{
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
		if (textend+cx>textstart+TEXTSIZE)
		{	cx0=0; new_line(); }
	}
	textline(textend,cx0,cy);
	XFlush(display);
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
	int length,i;
	char buffer[65];
	KeySym keysym;
	XComposeStatus status;

	*scan=0; *key=0;
	length=XLookupString(event,buffer,64,&keysym,&status);

	if ((event->state & ControlMask) && keysym==0x64) {
		*scan = eot;
		return;
	}
	switch (keysym)
	{	case XK_Prior :
			if (!in_text || textwindow==textstart) break;
			if (!scrolled)
			{	oldtextwindow=textwindow;
				scrolled=1;
			}
			for (i=0; i<maxlines-1; i++)
			{	textwindow--;
				while (textwindow>textstart)
				{	if (*(textwindow-1)==0) break;
					textwindow--;
				}
				if (textwindow==textstart) break;
			}
			hide_cursor();
			refresh_window();
			return;
		case XK_Next :
			if (!scrolled || !in_text) break;
			for (i=0; i<maxlines-1; i++)
			{	textwindow+=strlen(textwindow)+1;
				if (textwindow>=oldtextwindow) break;
			}
			if (textwindow>=oldtextwindow)
			{	textwindow=oldtextwindow;
				scrolled=0;
				show_cursor();
			}
			refresh_window();
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
		case GraphicsExpose:
			if (event->xgraphicsexpose.count>0) break;
			refresh_window(); break;
		case Expose:
			if (event->xexpose.count>0) break;
			refresh_window(); break;
		case ConfigureNotify :
			while (XCheckWindowEvent(display,window,mask,&dummyevent));
			if (event->xconfigure.width==wscreen &&
			    event->xconfigure.height==hscreen) break;
			wscreen=event->xconfigure.width;
			hscreen=event->xconfigure.height;
			computechar();
			computetext();
			XFreePixmap(display,pixmap);
			pixmap=XCreatePixmap(display,window,
				wscreen,hscreen,depth);
			clear_graphics();
			textwindow=textend-1;
			while (textwindow>=textstart && *textwindow) textwindow--;
			textwindow++; oldtextwindow=textwindow; cy=0;
			scrolled=0; cursoron=1;
			refresh_window();
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

char curpath[256];

char *cd (char *dir)
/* sets the path if dir!=0 and returns the path */
{	
	chdir(dir);
	if (getcwd(curpath,256)) return curpath;
	return dir;
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

/***************** clear screens ********************/

void clear_graphics (void)
{	
	XFillRectangle(display,pixmap,cleargc,0,0,wscreen,hscreen);
#ifdef WINDOW
	XFillRectangle(display,window,cleargc,0,0,wscreen,hscreen);
#endif
	XFlush(display);
}

int execute (char *name, char *args)
/**** execute
	call an external program, return 0, if there was no error.
****/
{	return 0;
}

void gflush (void)
{
#ifndef WINDOW
	XCopyArea(display,pixmap,window,gc,0,0,wscreen,hscreen,0,0);
#endif
	XFlush(display);
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
	
	if ((long)time!=0) {
		tv.tv_sec = (long)time;
		tv.tv_usec = 0;
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

/************ path utils *******************/

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

/************ main *******************/

char titel[]="This is EULER, Version 3.04 compiled %s.\n\n"
	"Type help(Return) for help.\n"
	"Enter command: (%ld Bytes free.)\n\n";

void usage (void)
{
	fprintf(stderr,"euler [-f FONT] [-g FONT] [-s KBYTES]\n"
		" [-geom GEOMETRY] [-d DISPLAY] [-0..15 COLOR] files\n");
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
	/* init GEM */
	signal(SIGINT,setint);
#ifdef FPE
	signal(SIGFPE,setfpe);
#endif
	XSetIOErrorHandler(ioerrorhandler);
	nextcheck=myclock();
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
					{	strcpy(textfontname,argv[1]+2); break;
					}
					else if (argv[2])
					{	strcpy(textfontname,argv[2]); argc--; argv++;
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
					{	strcpy(fontname,argv[1]+2); break;
					}
					else if (argv[2])
					{	strcpy(fontname,argv[2]); argc--; argv++;
						break;
					}
					break;
				case 's' : 
					if (argv[1][2])
					{	memsize=atoi(argv[1]+2)*1024l; break;
					}
					else if (argv[2])
					{	memsize=atoi(argv[2])*1024l; argc--; argv++;
						break;
					}
					break;
				case 'd' :
					strcpy(sdisp,argv[2]); argc--; argv++; break;
				default :
					if (sscanf(argv[1]+1,"%d",&nn)==1 && nn>=0 && nn<maxcolors)
					{	colorname[nn]=argv[2]; argc--; argv++;
						break;
					}
					usage(); exit(1);
			}
			argc--; argv++;
		}
		else break;
	}
	if (!memory_init()) exit(1);
	
	/* set up default pathes and directory */
	char* s=getenv("RETRO");
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
	path[npath++]=INSTALL"share/retro/progs\n";
	
#ifdef DEBUG	
	fprintf(stderr,"npath %d\n",npath);
	for (int i=0; i<npath; i++) {
		fprintf(stderr,"%s\n",path[i]);
	}
#endif

	grafik();
	XWindowEvent(display,window,ExposureMask,&event);
	process_event(&event);
	
	output1(titel,__DATE__,(unsigned long)(ramend-ramstart));

	main_loop(argc,argv);
	return 0;
}

