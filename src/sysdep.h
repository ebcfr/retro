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

typedef enum { line_none, line_solid, line_dotted, line_dashed }
	linetyp;
typedef enum { marker_cross, marker_circle, marker_diamond, marker_dot,
	marker_plus, marker_square, marker_star }
	markertyp;
typedef enum { fill_blank, fill_filled } filltyp;
typedef enum { key_none, cursor_up, cursor_down, cursor_left, cursor_right,
	escape, delete, backspace, clear_home, switch_screen, enter,
	space, line_end, line_start, fk1, fk2, fk3, fk4, fk5, fk6, fk7,
	fk8, fk9, fk10, fk11, fk12, word_left, word_right, help, sel_insert,
	page_up, page_down, eot } 
	scantyp;

void text_mode (void);
void graphic_mode (void);
void gsubplot(int r, int c, int index);
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

void clear_graphics (void);
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
