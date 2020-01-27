Extend
======

INTRODUCTION
------------

This document describes how to extend Retro with own functions. Of
course, you need a compiler capable of translating the source code.
And you might probably need some good programming experience in C.
Furthermore, it is necessary that you have some practice in using
Retro.


BASICS
------

Retro uses a stack for keeping the variables (local and global) and
the parameters of functions. This stack starts with the user defined
functions (udf). Next we have the global variables, and on top of them
the parameters and variables of running functions. The stack grows
from lower to higher memory. This description makes clear, why one
cannot change the size of a global variable from within a function.

The elements of the stack start with a header of data type

	typedef struct { LONG size; char name[16]; int xor; stacktyp type; }
		header;

consisting of the total size of the stack element, its name and an
index code of the name (to compare it easily), and the type of the
data, which an enum type

	typedef enum { s_real,s_complex,s_matrix,s_cmatrix,
		s_reference,s_command,s_submatrix,s_csubmatrix,s_string,s_udf }
		stacktyp;

The different data types will be explaned in detail below. Every stack
element has a header, followed by a further description and the data.

`s_real` : A double value.

`s_complex` : Two double values (real and imaginary part).

`s_matrix` : A matrix, consisting of the dimension of type
		typedef struct { int c,r; } dims;
	followed by r rows of c double values.

`s_cmatrix` : A complex matrix, which differs from s_matrix by having r
	rows of c times two double values.

`s_reference` : A reference to a variable, which consists of a pointer
	to the header of this variable.

`s_string` : A string, which consists of the string followed by a 0.

All other types are of no interest here.

You can avoid to no many of the details, if you use the service
macros, which we will describe now. "hd" is always a pointer to a
header here.

*	`#define realof(hd) ((double *)((hd)+1))`
	
	computes the address of the value of a header of type s\_real or the
	real part of a header of type s\_complex. (note: it returns a
	pointer to the double value!)


*	`#define matrixof(hd) ((double *)((char *)((hd)+1)+sizeof(dims)))`
	
	computes address the (0,0)-element of a matrix (real or complex).

*	`#define dimsof(hd) ((dims *)((hd)+1))`
	
	computes the addres of the dimension field of a matrix. I.e.,
	dimsof(hd)->r is the number of rows.

*	`#define referenceof(hd) (*((header **)((hd)+1)))`
	
	computes a pointer to a pointer to a header for references.

*	`#define imagof(hd) ((double *)((hd)+1)+1)`
	
	gives a pointer to the imaginary part of a s_complex header.

*	`#define stringof(hd) ((char *)((hd)+1))`
	
	computes a pointer to the string for s_string headers.

*	`#define nextof(hd) ((header *)((char *)(hd)+(hd)->size))`
	
	computes the a pointer to the header of the next stack element.

With these functions, you might get pointers to the details of stack
elements. There are a few more functions and makros.

*	`void getmatrix (header *hd, int *r, int *c, double **m);`
	
	computes the size (rows and columns) and the start address of a
	matrix (real or complex). This works also for real and complex
	values.
	
*	`#define mat(m,c,i,j) (m+(LONG)(c)*(i)+(j))`
	
	computes the address of the (i,j)-th element of a matrix starting
	at m. c is the number of columns of the matrix (i.e. the size of a row).

*	`#define cmat(m,c,i,j) (m+2*((LONG)(c)*(i)+(j)))`
	
	The same for a complex matrix.

There are two important variables

>	`char *newram,*ramend;`

The pointer newram indicates the start of the unused area of the
stack. You may use the portion of memory between newram and ramend for
own purposes, as long as you do not allocate new stack elements.


DECLARING FUNCTIONS
-------------------

Each function must be declared in "extend.c". First of all, you need
to add a prototype to the long list of prototypes in that file. It
simply has the form

>	`void myfunction (header *hd);`

Next, you need to append the function to the **builtin_list** array. You
must add a line of the form

>	`{"myname",2,myfunction},`

Here, "myname" is the name the function can be called in Retro. 2 (or
any other number) is the number of parameters the function accepts.
You may have two functions with the same name and a different number
of arguments. "myfunction" is the name of the function in the source
code. If the argument number is equal to 0, myname must be unique, and
the function is called without regard for the number of arguments.

Finally, you will have to program the function. You could simply add
the function to the file "extend.c".


FUNCTIONS OF ONE REAL OR COMPLEX VALUE
--------------------------------------

This is easy. You define a function for the real and a function for
the complex case.

```C
double real_case (double x)
{	...
	return ...;
}

void complex_case (double *x, double *xi, double *z, double *zi)
{	*z=...;
	*zi=...;
}

void myfunction (header *hd)
{	spread1(real_case,complex_case,hd);
}
```

If you pass a 0 pointer, instead of complex_case, the complex case is
undefined and a complex value issues an error message. Of course, the
function "myfunction" will work for a matrix element by element.

If the function has a real result for the complex case, you may use
the function "spread1r" instead. Of course the complex case is then to
be defined as

``` C
void complex_case (double *x, double *xi, double *r)
{ ...
}
```

FUNCTIONS OF TWO REAL OR COMPLEX VALUES
---------------------------------------

This is also easy. Again, you define a function for the real and the
complex case.

``` C
void complex_case (double *x, double *xi, double *y, double *yi,
	double *z, double *zi)
{	...
	*z=...;
	*zi=...;
}

void real_case (double *x, double *y, double *z)
{	...
	*z=...;
}

void myfunction (header *hd)
{	spread2(real_case,complex_case,hd);
}
```

There is also a function "spread2r".


THE GENERAL CASE
----------------

This is more difficult. First of all you need to collect all the
parameters. Since some of them may be references, you need to compute
the values of these references. This is done by the following code

``` C
void myfunction (header *hd)
{	header *result,*st=hd,*hd2,*hd3,...;
	hd1=next_param(hd); if (!hd1) return;
	hd2=next_param(hd1); if (!hd2) return;hd3=nextof(hd2); ...
	hd1=getvalue(hd1); hd2=getvalue(hd2); ...
	if (error) return;
	... /* computations */
	moveresult(hd,result);
}
```

Of course, you may not use more arguments than specified in
builtin_list. If you specified 0 arguments, you need to test, if hd1,
hd2, are bigger than newram, like in

>	`if ((char *)hd1 >= newram) ...`

Since an error may occur in a reference to a non-existing variable,
you must check the variable error.

Having done this, you should check the types of the arguments.

>	`if (hd1->type!=s_matrix) ...`

If you do not support the specific type, you should set error to 10000
(a value, reserved for this purpose) and return. Finally, it is a good
idea to use getmatrix to get the size and address of any matrix
involved.

Now you have the parameters. You will need a place to put the return
value (or values, if your function returns multiple values). This can
be done with the "new_..." functions. All these functions have a
parameter name, which you can set to "" simply. If there is no
sufficient space, error is set to 1.

*	`header *new_matrix (int c, int r, char *name);`
	
	creates a matrix with r rows and c columns.

*	`header *new_cmatrix (int c, int r, char *name);`
	
	The same for a complex matrix.

*	`header *new_real (double x, char *name);`
	
	a real number with the value x.

*	`header *new_complex (double x, double y, char *name);`
	
	a complex number with the value x+iy.

*	`header *new_string (char *s, size_t size, char *name);`
	
	a string of size "size", to which s is copied.

For example

	header *result;
	result=new_matrix(10,10,"");

After you computed your result and copied it into the result variable,
you need to move the result to the stack using "moveresult" (see above).
In case of multiple results, use a code like

	header *result1,*result2;
	...
	moveresult(hd,result1); hd=nextof(hd);
	moveresult(hd,result2);

Lets give a simple example. We define a function, which flips a matrix
along the second diagonal (from lower left to upper right). The
changes in builtin_list are then

>	`{"tflip",1,tflip},`

```C
void tflip (header *hd)
{	header *st=hd,*result;
	int i,j,c,r;
	double *m1,*m2;
	hd=getvalue(hd); if (error) return;
	if (hd->type==s_real || hd->type==s_matrix)
	{	getmatrix(hd,&r,&c,&m1);
		result=new_matrix(c,r,""); if (error) return;
		m2=matrixof(result);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
				*mat(m2,r,j,i)=*mat(m1,c,i,j);
	}
	else if (hd->type==s_complex || hd->type==s_cmatrix)
	{	getmatrix(hd,&r,&c,&m1);
		result=new_cmatrix(c,r,""); if (error) return;
		m2=matrixof(result);
		for (i=0; i<r; i++)
			for (j=0; j<c; j++)
			{	*cmat(m2,r,j,i)=*cmat(m1,c,i,j);
				*(cmat(m2,r,j,i)+1)=*(cmat(m1,c,i,j)+1);
			}
	}
	else
	{	output("Error in tflip. Illegal argument.\n");
		error=10000; return;
	}
	moveresult(st,result);
}
```

output(char *) is a means to produce an error message related to the
error.


PLEASE
------

If you have an interesting extension to Retro, please send me a copy.
I might encorporate it to the benefit of other users.
