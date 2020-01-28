#ifndef _FUNCS_H_
#define _FUNCS_H_

#include "core.h"


typedef struct {
	char *name;
	int nargs;
	void (*f) (header *);
} builtintyp;

extern builtintyp builtin_list[];

/* udf */
extern int udfon;
extern header *running;

void interpret_udf (header *var, header *args, int nargs);

/* basic ops */
void add (header *hd, header *hd1);
void subtract (header *hd, header *hd1);
void dotmultiply (header *hd, header *hd1);
void dotdivide (header *hd, header *hd1);
void invert (header *hd);

void multiply (header *hd, header *hd1);
void divide (header *hd, header *hd1);

void make_complex (header *hd);
void ccopy (double *y, double *x, double *xi);
void complex_multiply (double *x, double *xi, double *y, double *yi,
	double *z, double *zi);
void complex_divide (double *x, double *xi, double *y, double *yi,
	double *z, double *zi);
void cscalp (double *s, double *si, double *x, double *xi,
	double *y, double *yi);

void mgreater (header *hd);
void mless (header *hd);
void mgreatereq (header *hd);
void mlesseq (header *hd);
void mequal (header *hd);
void munequal (header *hd);
void maboutequal (header *hd);

void mnot (header *hd);
void mand (header *hd);
void mor (header *hd);
void mvconcat (header *hd);
void mhconcat (header *hd);

void msin (header *hd); 
void mcos (header *hd);
void mtan (header *hd);
void matan (header *hd);
void mexp (header *hd);
void mlog (header *hd);
void msqrt (header *hd);
void mre (header *hd);
void mim (header *hd);
void mpi (header *hd);

void mdup (header *hd);
void msum (header *hd);
void mprod (header *hd);
void mcomplex (header *hd);

void mpower (header *hd);

void msolve (header *hd);

void mindex (header *hd);

void minmax (double *x, size_t n, double *min, double *max,
	int *imin, int *imax);
void transpose (header *hd);
void vectorize (header *init, header *step, header *end);
void mfft (header *hd);
void mifft (header *hd);
void mtridiag (header *hd);
void mcharpoly (header *hd);
void mfind (header *hd);
void mdiag2 (header *hd);
void mband (header *hd);
void mdup (header *hd);
void make_complex (header *hd);
void mvconcat (header *hd);
void mhconcat (header *hd);
void wmultiply (header *hd);
void smultiply (header *hd);

/* polynom funcs */
void polyval (header *hd);
void polyadd (header *hd);
void polymult (header *hd);
void polydiv (header *hd);
void dd (header *hd);
void ddval (header *hd);
void polydd (header *hd);
void polyzeros (header *hd);
void polytrunc (header *hd);
void mzeros (header *hd);
void mzeros1 (header *hd);

int exec_builtin (char *name, int nargs, header *hd);
builtintyp *find_builtin (char *name);
void print_builtin (void);
void sort_builtin (void);

#endif
