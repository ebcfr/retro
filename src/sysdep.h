#ifndef _SYSDEP_H_
#define _SYSDEP_H_

#ifndef M_PI
#define M_PI            3.14159265358979323846
#endif

#ifndef ALIGNMENT
#define ALIGNMENT 8
#endif

/* Filesystem interaction */
#ifdef _WIN32
#define PATH_DELIM_CHAR '\\'
#define PATH_DELIM_STR "\\"
#else
#define PATH_DELIM_CHAR '/'
#define PATH_DELIM_STR "/"
#endif

#define MAX_PATH	10

extern char* path[MAX_PATH];
extern int npath;

char *cd (char *dir);
int scan_dir(char *dir_name, char *pat, char ** files[], int *files_count);
char *dir (char *pattern);
int execute (char *, char *);

#define EXTENSION ".e"

extern char fktext[12][64];

typedef struct _plot_t {
	int 			upperc, upperr, lowerc, lowerr;		// frame in a 1024x1024 canvas
	double			x_min, x_max, y_min, y_max;			// data ranges
	char			style[32];							// style attached to plots
	unsigned int	newframe:1;							// the graph needs a new frame
	unsigned int	holding:1;							// don't erase
	unsigned int	scaling:1;							// finding out scale ranges is needed
	unsigned int	xlog:1;								// use log scale for x axis
	unsigned int	ylog:1;								// use log scale for y axis
	unsigned int	xgrid:1;							// draw xgrid
	unsigned int	xticks:1;							// draw xticks
	unsigned int	ygrid:1;							// draw ygrid
	unsigned int	yticks:1;							// draw yticks
	unsigned int	autocolor:1;						// auto-increment color when several curves are to be plot
} plot_t;

typedef enum {
	line_none, line_solid, line_dotted, line_dashed, line_comb
} line_t;

typedef enum {
	marker_none, marker_cross, marker_plus, marker_dot, marker_star,
	marker_circle, marker_fcircle, marker_square, marker_fsquare,
	marker_diamond, marker_fdiamond, marker_arrow
	
} marker_t;

typedef enum {
	fill_blank, fill_filled
} fill_t;

extern int framecolor, linecolor, gridcolor, textcolor, wirecolor, densitycolor;
extern int linewidth, markersize;
extern marker_t markertype;
extern line_t   linetype;

typedef enum {
	key_none, cursor_up, cursor_down, cursor_left, cursor_right,
	escape, delete, backspace, clear_home, switch_screen, enter,
	space, line_end, line_start, fk1, fk2, fk3, fk4, fk5, fk6, fk7,
	fk8, fk9, fk10, fk11, fk12, word_left, word_right, help, sel_insert,
	page_up, page_down, eot
} scantyp;

void text_mode (void);
void graphic_mode (void);
void gclip(plot_t *p);
void gunclip(plot_t *p);
void gclear (void);
void gsubplot(int r, int c, int index);
void gpath(plot_t* p, double *x, double *y, int n);
void gframe(plot_t* p);
void gxgrid(plot_t* p, double factor, double* ticks, int n);
void gygrid(plot_t* p, double factor, double* ticks, int n);

void gline (double c, double r, double c1, double r1, int color,
	int st, int width);
void gtext (double c, double r, char *text, int color, int centered);
void gmarker (double c, double r, int color, int st);
void gfill (double c[], int st, int n, int connect[]);
void scale (double s);
void mouse (int *, int *);
void gbar (double c, double r, double c1, double r1, double hue,
	int color, int connect);
void gfillh (double c[], double hue, int color, int connect);
void getpixel (double *x, double *y);


int wait_key (scantyp *scan);
int test_key (void);

void clear_screen (void);
int shrink (size_t size);

double myclock (void);

void gprint (char *s); /* print an output text (no newline) */
void gflush (void); /* flush out graphics */

void edit_on_cb (void);
void edit_off_cb (void);
void cursor_off_cb (void); 
void cursor_on_cb (void);
void move_cr_cb (void);
void move_cl_cb (void);
void page_up_cb(void);
void page_down_cb(void);

void clear_eol (void);


void sys_wait (double delay, int *scan);

#endif
