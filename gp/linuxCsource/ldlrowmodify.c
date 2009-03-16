/* LDLROWMODIFY    Modify the LDL factorization of C, when kth row and column change
 *
 * function L = rowmodify(L, w, w2, k)
 * Function to modify the cholesky factorization L*D*L' = C (which 
 * is stored in L), when a row and column k of C have changed from c to 
 * c2. The change in C is assumed to be such that the sparsity structure 
 * of C is remained the same. c is the old kth column of C and c2 is 
 * the new column.
 *   
 * See gpep_e for usage
 *
 * See Davis and Hager 2005 (Row Modification of a sparse Cholesky 
 * factorization) section 4 for details of the algorithm.
 *
 *   Note! The function assumes that the sparsity structure of the ldl
 *         factorization does not change and that the sparsity structure
 *         of the removed and added column/row is same!
 *
 */

/* -----------------------------------------------------------------------------
 * Copyright (C) 2005-2006 Timothy A. Davis
 * Copyright (c) 2009      Jarno Vanhatalo
 *
 * This software is distributed under the GNU General Public
 * License (version 2 or later); please refer to the file
 * License.txt, included with the software, for details.
 /* -----------------------------------------------------------------------------

 /* -----------------------------------------------------------------------------
 * The function uses CHOLMOD/MATLAB Module by Timothy A. Davis. Parts of the
 * code are copied from ldlchol.c function.
 *
 * The CHOLMOD/MATLAB Module is licensed under Version 2.0 of the GNU
 * General Public License.
 * -------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mex.h"
#define max(a,b) (((a) > (b)) ? (a) : (b))
#define min(a,b) (((a) < (b)) ? (a) : (b))

void mexFunction
(
    int	nargout,
    mxArray *pargout [ ],
    int	nargin,
    const mxArray *pargin [ ]
    )
{
  double *Lx, *Lx2, *cx, *cx2, d, db, alpha, alpha2, beta, beta2, gamma, gamma2, *w, *wu, *wd, *deltal12;
  mwSize nnz, n;
  mwIndex p, i, j, k, cind, *Li, *Lp, *Li2, *Lp2, *ci, *ci2, *cp, *krowind, *krowcol, lrowk;
 
  /* ---------------------------------------------------------------------- */
  /* check inputs */
  /* ---------------------------------------------------------------------- */

  if (nargout > 1 || nargin < 3 || nargin > 4)
    {
      mexErrMsgTxt ("Usage: L = ldlrowmodify (L, c, c2, k)") ; 
    }
  
  n = mxGetN (pargin [0]) ;

  if (!mxIsSparse (pargin [0]) || !mxIsSparse (pargin [1])
      || !mxIsSparse (pargin [2]) || n != mxGetM (pargin [0])
      || n != mxGetM (pargin [1]) || n != mxGetM (pargin [2])
      || mxIsComplex (pargin [0]) || mxIsComplex (pargin [1])
      || mxIsComplex (pargin [2]))
    {
      
      mexErrMsgTxt ("ldlrowmodify: R and/or L not sparse, complex, or wrong"
		    " dimensions") ;
    }

  /* ---------------------------------------------------------------------- */
  /* get ki: column integer of update */
  /* ---------------------------------------------------------------------- */
  k = (mwIndex) *mxGetPr(pargin[3]);
  k = k-1;
  
  /* ---------------------------------------------------------------------- */
  /* Get L: the sparse LDL factorization */
  /* ---------------------------------------------------------------------- */
  Lp2 = mxGetJc(pargin[0]);
  Li2 = mxGetIr(pargin[0]);
  Lx2 = mxGetPr(pargin[0]);

  
  /* ---------------------------------------------------------------------- */
  /* get c and c2: sparse matrices of incoming/outgoing columns */
  /* ---------------------------------------------------------------------- */
  cp = mxGetJc(pargin[1]);
  ci = mxGetIr(pargin[1]);
  cx = mxGetPr(pargin[1]);
  ci2 = mxGetIr(pargin[2]);
  cx2 = mxGetPr(pargin[2]);

  /* ---------------------------------------------------------------------- */
  /* Create the output: the modified sparse LDL factorization */
  /* ---------------------------------------------------------------------- */
  nnz = (mwSize) Lp2[n];
  pargout[0] = mxCreateSparse(n,n,nnz,mxREAL);   /* The sparse matrix */
  Lp = mxGetJc(pargout[0]);
  Li = mxGetIr(pargout[0]);  
  Lx = mxGetPr(pargout[0]);  

  /* ---------------------------------------------------------------------- */
  /* Copy the input into output and find the indices of k'th row of L */
  /* ---------------------------------------------------------------------- */
  if (k>0){
    krowind = mxCalloc((mwSize)k, sizeof(mwIndex));
    krowcol = mxCalloc((mwSize)k, sizeof(mwIndex));
    lrowk=0;
  }
  
  for (j = 0 ;  j < n ; j++) {
    Lp[j] = Lp2[j];
    
    for ( i = Lp2[j] ; i < Lp2[j+1] ; i++) {
      Li[i] = Li2[i];
      Lx[i] = Lx2[i];
      if (Li[i] == k && j < k) {
	krowind[lrowk] = i;
	krowcol[lrowk] = j;
	lrowk++;
      }
    }
  }
  Lp[n] = Lp2[n];
      
  /* ---------------------------------------------------------------------- */
  /* Solve the l_12 vector and the D22 element */
  /* ---------------------------------------------------------------------- */
  if ( k>0 ) {
    deltal12 = mxCalloc((mwSize)k, sizeof(double));
    for (p = cp[0] ; p < cp[1] ; p++){
      if (ci[p] >=k ) break;
      deltal12[ci[p]] = cx2[p] - cx[p];   /* deltal12 is a full vector */
    }

    /* Solve L11*D11*deltal12 = deltac12 */
    for (j = 0 ; j < k ; j++ ){
      for (p = Lp[j]+1 ; p < Lp[j+1] ; p++) {
	if (Li[p] >= k) break;
	deltal12[Li[p]] -= Lx[p] * deltal12[j];
      }
      deltal12[j] /= Lx[ Lp[j] ];
    }

    /* Set the new l12' elements into L */
    for (j = 0 ; j < lrowk ; j++) {
      Lx[krowind[j]] = Lx[krowind[j]] + deltal12[krowcol[j]];
    }

    /* Evaluate the D22 element*/
    d = Lx[Lp[k]];
    for ( i=0; i < cp[1] ; i++){
      if (ci[i] == k) {
	cind = i;
	break;
      }
    }
    if (i >= cp[1]) mexErrMsgTxt ("The vector c must contain k'th element!");
    db = d + cx2[cind] - cx[cind];

    for (j = 0; j < lrowk ; j++)  {
      deltal12[krowcol[j]] =  deltal12[krowcol[j]] * Lx2[Lp2[krowcol[j]]];
      db = db - deltal12[krowcol[j]] * (Lx2[krowind[j]] + Lx[krowind[j]]);
    }
    Lx[Lp[k]] = db;
  }
  else {
    /* Since k==0 we have to evaluate only the D22 element*/
    d = Lx[Lp[k]];
    cind = 0;
    db = d + cx2[cind] - cx[cind];
    Lx[Lp[k]] = db;
  }

     
  /* ---------------------------------------------------------------------- */
  /* Solve the l_32 vector and LDL_33*/
  /* ---------------------------------------------------------------------- */

  if (k < n-1){
    /* Update the l_32 part */
    if (k>0) {
      /* Evaluate w = L31*D11*deltal12 */
      /* Note that deltal12 is actually D11*deltal12 (see above) */
      w = mxCalloc(n, sizeof(double));   /* full vector of length n */
      for (j = 0 ; j < k ; j++){
	for (p = Lp[j] ; p < Lp[j+1] ; p++) {
	  if (Li[p] > k)  w[Li[p]] += Lx2[p] * deltal12[j];
	}
      }
      /* evaluate l32 = (deltac_32 + l_22*d_22 - w) / db_22 */
      cind += 1;
      for ( p = Lp[k]+1 ; p < Lp[k+1] ; p++) {	
	if (Li[p] == ci[cind]) {
	  Lx[p] = (cx2[cind] - cx[cind] + Lx[p] * d - w[Li[p]] ) / db;
	  cind++;
	} else {
	  Lx[p] = (Lx[p] * d - w[Li[p]] ) / db;
	}
      }
      /* Free memory */
      mxFree(w);
    } else {
      cind = cp[k]+1;
      for ( p = Lp[k]+1 ; p < Lp[k+1] ; p++) {	
	if (Li[p] == ci[cind]) {
	  Lx[p] = (cx2[cind] - cx[cind] + Lx[p] * d ) / db;
	  cind++;
	} else {
	  Lx[p] = Lx[p] * d / db;
	}
      }
    }

    /* Update L_33 part */
    alpha = 1;
    alpha2 = 1;
    wu = mxCalloc(n, sizeof(double));   /* full vector of length n */
    wd = mxCalloc(n, sizeof(double));   /* full vector of length n */
    for (p = Lp[k]+1 ; p < Lp[k+1] ; p++){
      wu[Li[p]] = Lx2[p] * sqrt(d);
      wd[Li[p]] = Lx[p] * sqrt(db);
    }
    
    for (i = k+1 ; i < n ; i++ ) {

      if (wu[i] != 0) {
	beta = alpha + wu[i]*wu[i] / Lx[Lp[i]]; 
	gamma = wu[i] / (beta*Lx[Lp[i]]);
	Lx[Lp[i]] = (beta/alpha)*Lx[Lp[i]];
	alpha = beta;
	
	beta2 = alpha2 - wd[i]*wd[i] / Lx[Lp[i]]; 
	gamma2 = wd[i] / (beta2*Lx[Lp[i]]);
	Lx[Lp[i]] = (beta2/alpha2)*Lx[Lp[i]];
	alpha2 = beta2;

	for (p = Lp[i]+1 ; p < Lp[i+1] ; p++) {
	  wu[Li[p]] -= wu[i] * Lx[p];
	  Lx[p] += gamma * wu[Li[p]];
	  
	  wd[Li[p]] -= wd[i] * Lx[p];
	  Lx[p] -= gamma2 * wd[Li[p]];	  
	}	
      }	
    }    
  }

  /* Free the memory */
  if (k>0) {  
    mxFree(krowcol);
    mxFree(krowind);
    mxFree(deltal12);
  }
  mxFree(wu);
  mxFree(wd); 

}
