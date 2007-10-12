#include "inverters.h"
#include "linear_algebra.h"
#include "complex.h"
#include "malloc.h"
#include "memory.h"
#include "update.h"
#include <stdio.h>
#include <math.h>

#define _print_par(c) \
  printf("[%d] " #c " = %e\n",cgiter,c)

/*
 * performs the multi-shifted BiCGstab inversion for the hermitean matrix M:
 * out[i] = (M-(par->shift[i]))^-1 in
 * returns the number of cg iterations done.
 */
int HBiCGstab_mshift(mshift_par *par, spinor_operator M, suNf_spinor *in, suNf_spinor **out){

  suNf_spinor **s;
  suNf_spinor *r, *r1, *o, *Ms, *Mo, *o0;
  suNf_spinor *sptmp;

  double delta, phi; 
  double *z1, *z2, *z3, *alpha, *beta, *chi, *rho;
  double rtmp1,rtmp2,rtmp3, oldalpha,oldchi;

  int i;
  int cgiter;
  char *sflags;
  unsigned short notconverged;
	unsigned int spinorlen;
   
  /* fare qualche check sugli input */
  /* par->n deve essere almeno 2! */
  /*
    printf("numero vettori n=%d\n",par->n);
    for (i=0; i<(par->n); ++i) {
    printf("shift[%d]=%f\n",i,par->shift[i]);
    printf("out[%d]=%p\n",i,out[i]);      
    }
  */
   
  /* allocate spinors fields and aux real variables */
  /* implementation note: to minimize the number of malloc calls
   * objects of the same type are allocated together
   */
	get_spinor_len(&spinorlen);
  s = (suNf_spinor **)malloc(sizeof(suNf_spinor*)*(par->n));
  s[0] = alloc_spinor_field_f((par->n)+6);
  for (i=1; i<(par->n); ++i) {
    s[i] = s[i-i]+(spinorlen);
  }
  r = s[par->n-1]+(spinorlen);
  r1 = r+(spinorlen);
  o = r1+(spinorlen);
  Ms = o+(spinorlen);
  Mo = Ms+(spinorlen);
  o0 = Mo+(spinorlen);

  z1 = (double *)malloc(sizeof(double)*7*(par->n));
  z2 = z1+(par->n);
  z3 = z2+(par->n);
  alpha = z3+(par->n);
  beta = alpha+(par->n);
  chi = beta+(par->n);
  rho = chi+(par->n);

  sflags = (char *)malloc(sizeof(char)*(par->n));
   
  /* init recursion */
  cgiter = 0;
  notconverged = 1;
  beta[0]=1.;
  spinor_field_copy_f(r, in);
  for (i=0; i<(par->n); ++i) {
    rho[i]=z1[i]=z2[i]=beta[0];
    alpha[i]=0.;
    spinor_field_copy_f(s[i], in);
    spinor_field_zero_f(out[i]);
    sflags[i]=1;
  }
  chi[0]=0;
  /* choose omega so that delta and phi are not zero */
  /* spinor_field_copy_f(o, in);  omega = in ; this may be changed */
  /* gaussian_spinor_field(o0); */
  spinor_field_copy_f(o0, in); 
  spinor_field_copy_f(o, o0);
  delta = spinor_field_prod_re_f(o, r);

  M(Ms,s[0]);
  phi=spinor_field_prod_re_f(o0, Ms)/delta; /* o = in (see above) */

  _print_par(delta);
  _print_par(phi);
  _print_par(alpha[0]);
  _print_par(beta[0]);


  /* cg recursion */
  do {
    ++cgiter;

    /* compute beta[0] and store in ctmp1 the old value (needed in the shifted update */
    rtmp1=beta[0];
    beta[0]=-1./phi; /* b=1/phi */

    /* compute omega and chi[0] */
    spinor_field_mul_f(o,beta[0],Ms);
    spinor_field_add_assign_f(o,r);
    
    M(Mo, o);

    oldchi=chi[0];
    chi[0]=spinor_field_prod_re_f(Mo,o)/spinor_field_sqnorm_f(Mo);
    
    /* compute r1 */
    spinor_field_mul_f(r1,chi[0],Mo);
    spinor_field_sub_f(r1,o,r1); 

    /* update delta and alpha[0] */
    oldalpha=alpha[0];
    alpha[0]=-beta[0]/(chi[0]*delta);
    delta = spinor_field_prod_re_f(o0,r1); /* in = omega al passo zero */
    alpha[0]*=delta;

    /* compute new out[0] */
    rtmp2=-beta[0];
    spinor_field_lc_add_assign_f(out[0],rtmp2,s[0],chi[0],o);

    /* compute new s[0] */
    rtmp3=-alpha[0]*chi[0];
    spinor_field_lc_f(Mo,alpha[0],s[0],rtmp3,Ms); /* use Mo as temporary storage */
    spinor_field_add_f(s[0],r1,Mo);

    /* assign r<-r1 */
    sptmp=r;
    r=r1;
    r1=sptmp; /*exchange pointers */

    /* update phi */
    M(Ms,s[0]);
    phi = spinor_field_prod_re_f(o0, Ms)/delta;

    _print_par(delta);
    _print_par(phi);
    _print_par(alpha[0]);
    _print_par(beta[0]);
    _print_par(chi[0]);

    for (i=1; i<(par->n); ++i) { /* update shifted quantities i=1..n-1 */
      if(sflags[i]) {
	/* compute z3[i]/z2[i] */
	z3[i]=z1[i]*rtmp1/(beta[0]*oldalpha*(z1[i]-z2[i])+z1[i]*rtmp1*(1.+par->shift[i-1]*beta[0]));
	_print_par(z3[i]);
	/*compute beta[i] */
	beta[i]=beta[0]*z3[i];
	_print_par(beta[i]);
	/*compute chi[i] */
	chi[i]=chi[0]/(1.-chi[0]*par->shift[i-1]);
	_print_par(chi[i]);
	/* compute alpha[i] */
	alpha[i]=alpha[0]*z3[i]*z3[i];
	_print_par(alpha[i]);
	/* compute z3[i] */
	z3[i]*=z2[i];
	_print_par(z3[i]);
	/* update solution */
	rtmp2=-beta[i];
	rtmp3=chi[i]*rho[i]*z3[i];
	spinor_field_lc_add_assign_f(out[i],rtmp2,s[i],rtmp3,o);
	/* update s[i] */
	rtmp2=chi[i]/beta[i]*rho[i];
	rtmp3=rtmp2*z2[i];
	rtmp2*=-z3[i];	  
	spinor_field_lc_add_assign_f(s[i],rtmp2,o,rtmp3,r1); /* not done yet */
	
	rho[i]/=(1.-rho[0]*par->shift[i-1]); /* update rho */
	_print_par(rho[i]);

	rtmp2=z3[i]*rho[i];
	spinor_field_lc_f(Mo,rtmp2,r,alpha[i],s[i]); /* use Mo as temporary storage */
	spinor_field_copy_f(s[i],Mo); 

	/* change pointers instead */
	/*
	  sptmp=s[i];
	  s[i]=Mo;
	  Mo=sptmp;
	*/

	 /* shift z2<-z3; z1<-z2 */
	 z2[i]=z3[i];
	 z1[i]=z2[i];

      }
    }

    rtmp1=spinor_field_sqnorm_f(r);

    if(rtmp1<par->err2)
      notconverged=0;
	
    printf("[ %d ] residuo=%e\n",cgiter,rtmp1);
    
    /* Uncomment this to print cg recursion parameters
       printf("[ %d ] alpha=%e\n",cgiter,alpha);
       printf("[ %d ] omega=%e\n",cgiter,omega);
       printf("[ %d ] still runnning=%d\n",cgiter,notconverged);
       for (i=0;i<par->n;++i) printf("z3[%d]=%e; ",i,z3[i]);
       printf("\n[ %d ] gamma=%e\n",cgiter,gamma);
       printf("[ %d ] delta=%e\n",cgiter,delta);
    */
      
  } while ((par->max_iter==0 || cgiter<par->max_iter) && notconverged);

  /* test results */
#ifndef NDEBUG
  for(i=0;i<par->n;++i){
    double norm;
    M(Ms,out[i]);
    if(i!=0) {
      spinor_field_mul_add_assign_f(Ms,-par->shift[i-1],out[i]);
    }
    spinor_field_mul_add_assign_f(Ms,-1.0,in);
    norm=spinor_field_sqnorm_f(Ms);
    if (fabs(norm)>5.*par->err2)
      printf("BiCGstab Failed: err2[%d] = %e\n",i,norm);
  }
#endif
   
  /* free memory */
  free_field(s[0]);
  free(s);
  free(z1);
  free(sflags);

  /* return number of cg iter */
  return cgiter;
}
