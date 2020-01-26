/*
 *	Euler - a numerical lab
 *
 *	platform : all
 *
 *	file : spread.h -- spread function elementwise
 */

#ifndef _SPREAD_H_
#define _SPREAD_H_

#include "header.h"

header *map1 (void f(double *, double *),
	void fc(double *, double *, double *, double *),
	header *hd);
header *map1r (void f(double *, double *),
	void fc(double *, double *, double *),
	header *hd);

void spread1 (double f (double),
	void fc (double *, double *, double *, double *),
	header *hd);
void spread1r (double f (double),
	void fc (double *, double *, double *),
	header *hd);

header * map2 (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *, double *),
	header *hd1, header *hd2);
header * map2r (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *),
	header *hd1, header *hd2);

void spread2 (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *, double *),
	header *hd);
void spread2r (void f (double *, double *, double *),
	void fc (double *, double *, double *, double *, double *),
	header *hd);

void mmap1 (header *hd);

#endif
