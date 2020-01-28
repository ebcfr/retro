#ifndef _CORE_H_
#define _CORE_H_

extern char *ramstart,
            *ramend,
            *startglobal,
            *endglobal,
            *startlocal,
            *endlocal,
            *newram,
            *udfend;
	
#define MAXNAME		16			/* Maximum length of an identifier */

extern int error,surpressed,udf,udfon,linelength,stringon;
extern long loopindex;
extern char *next,*udfline;
extern int searchglobal;

typedef enum {
	s_real,
	s_complex,
	s_matrix,
	s_cmatrix,
	s_reference,
	s_command,
	s_submatrix,
	s_csubmatrix,
	s_string,
	s_udf
} stacktyp;

typedef struct {
	size_t size;
	char name[MAXNAME];
	int xor;
	stacktyp type;
	int flags;
} header;

typedef struct {
	int c,r;
} dims;

typedef struct { header hd; double val; } realtyp;

#define realof(hd) ((double *)((hd)+1))
#define matrixof(hd) ((double *)((char *)((hd)+1)+sizeof(dims)))
#define dimsof(hd) ((dims *)((hd)+1))
#define commandof(hd) ((int *)((hd)+1))
#define referenceof(hd) (*((header **)((hd)+1)))
#define imagof(hd) ((double *)((hd)+1)+1)
#define rowsof(hd) ((int *)((dims *)((header **)((hd)+1)+1)+1))
#define colsof(hd) ((int *)((dims *)((header **)((hd)+1)+1)+1)+submdimsof((hd))->r)
#define submrefof(hd) (*((header **)((hd)+1)))
#define submdimsof(hd) ((dims *)((header **)((hd)+1)+1))
#define stringof(hd) ((char *)((hd)+1))
#define udfof(hd) ((char *)(hd)+(*((size_t *)((hd)+1))))
#define udfstartof(hd) ((size_t *)((hd)+1))
#define helpof(hd) ((char *)(hd)+sizeof(header)+sizeof(size_t))
#define nextof(hd) ((header *)((char *)(hd)+(hd)->size))

#define mat(m,c,i,j) (m+(((size_t)(c))*(i)+(j)))
#define cmat(m,c,i,j) (m+(2*(((size_t)(c))*(i)+(j))))

#define matrixsize(c,r) (sizeof(header)+sizeof(dims)+(c)*(size_t)(r)*sizeof(double))
#define cmatrixsize(c,r) (sizeof(header)+sizeof(dims)+2l*(c)*(size_t)(r)*sizeof(double))

int xor (char *n);

header *new_matrix (int c, int r, char *name);
header *new_cmatrix (int c, int r, char *name);
header *new_real (double x, char *name);
header *new_complex (double x, double y, char *name);
header *new_reference (header *hd, char *name);
header *new_submatrix (header *hd, header *cols, header *rows,
	char *name);
header *new_csubmatrix (header *hd, header *cols, header *rows,
	char *name);
header *new_command (int no);
header *new_string (char *s, size_t size, char *name);
header *new_udf (char *name);
header *new_subm (header *var, size_t l, char *name);
header *new_csubm (header *var, size_t l, char *name);

void getmatrix (header *hd, int *r, int *c, double **m);
void get_element (int nargs, header *var, header *hd);
void get_element1 (char *name, header *hd);

header *getvalue (header *);
header *getvariable (header *);
header *next_param (header *hd);

void moveresult (header *stack, header *result);
void moveresult1 (header *stack, header *result);

void interpret_udf (header *var, header *args, int nargs);

header *searchvar (char *name);
header *searchudf (char *name);

void kill_local (char *name);
void kill_udf (char *name);

int sametype (header *hd1, header *hd2);

header *assign (header *var, header *value);

#endif
