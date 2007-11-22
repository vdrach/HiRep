#ifndef INVERTERS_H
#define INVERTERS_H

#include "suN_types.h"
#include "complex.h"


typedef void (*spinor_operator)(spinor_field *out, spinor_field *in);
typedef void (*spinor_operator_flt)(spinor_field_flt *out, spinor_field_flt *in);

typedef struct _mshift_par {
   int n; /* number of shifts */
   double *shift;
   double err2; /* relative error of the solutions */
   int max_iter; /* maximum number of iterations: 0 => infinity */
	 void *add_par; /* additional parameters for specific inverters */
} mshift_par;

/*
 * performs the multi-shifted CG inversion:
 * out[i] = (M-(par->shift[i]))^-1 in
 * returns the number of cg iterations done.
 */
int cg_mshift(mshift_par *par, spinor_operator M, spinor_field *in, spinor_field *out);

int BiCGstab_mshift(mshift_par *par, spinor_operator M, spinor_field *in, spinor_field *out);
int HBiCGstab_mshift(mshift_par *par, spinor_operator M, spinor_field *in, spinor_field *out);

int g5QMR_mshift(mshift_par *par, spinor_operator M, spinor_field *in, spinor_field *out);
/*int g5QMR_mshift_flt(mshift_par *par, spinor_operator_flt M, suNf_spinor_flt *in, suNf_spinor_flt **out); */

int MINRES_mshift(mshift_par *par, spinor_operator M, spinor_field *in, spinor_field *out);

typedef struct _MINRES_par {
  double err2; /* maximum error on the solutions */
  int max_iter; /* maximum number of iterations: 0 => infinity */
} MINRES_par;
int MINRES(MINRES_par *par, spinor_operator M, spinor_field *in, spinor_field *out, spinor_field *trial);

int eva(int len, int nev,int nevt,int init,int kmax,
               int imax,double ubnd,double omega1,double omega2,
               spinor_operator Op,
               spinor_field *ws,spinor_field *ev,double d[],int *status);

void jacobi1(int n,double a[],double d[],double v[]);
void jacobi2(int n,complex a[],double d[],complex v[]);

void dirac_eva_onemass(int nev,int nevt,int kmax,
        int imax,double omega1,double omega2,double mass,
        spinor_field *ev,double d[],int *status);
void dirac_eva(int nev,int nevt,int kmax,
        int imax,double omega1,double omega2,int n_masses,double *mass,
        spinor_field *ev,double d[],int *status);


#endif
