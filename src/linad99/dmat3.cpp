/**
 * $Id: dmat3.cpp 789 2010-10-05 01:01:09Z johnoel $
 *
 * Author: David Fournier
 * Copyright (c) 2009-2012 ADMB Foundation
 */
#include "fvar.hpp"
#include <math.h>
#ifndef OPT_LIB
  #include <cassert>
  #include <climits>
#endif

#ifdef ISZERO
  #undef ISZERO
#endif
#define ISZERO(d) ((d)==0.0)

/**
\def TINY
A small number. Used to avoid divide by zero in the LU decomposition. Locally defined,
undefined, redefined and undefined in this file.
*/
#define TINY 1.0e-20;

void lubksb(dmatrix a, const ivector&  indx,dvector b);
void ludcmp(const dmatrix& a, const ivector& indx, const double& d);

/** Inverse of a constant matrix by LU decomposition.
    \ingroup matop
    \param m1 A dmatrix, \f$M\f$, for which the inverse is to be computed.
    \return A dmatrix containing \f$M^{-1}\f$.
    \n\n The implementation of this algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 2
*/
dmatrix inv(const dmatrix& m1)
{
  double d;
  if (m1.rowmin()!=m1.colmin() || m1.rowmax() != m1.colmax())
  {
    cerr << " Error in dmatrix inv(const dmatrix&) -- matrix not square \n";
  }

  dmatrix a(m1.rowmin(),m1.rowmax(),m1.rowmin(),m1.rowmax());

  int i;
  for (i=m1.rowmin(); i<=m1.rowmax(); i++)
  {
    for (int j=m1.rowmin(); j<=m1.rowmax(); j++)
    {
      a[i][j]=m1[i][j];
    }
  }
  ivector indx(m1.rowmin(),m1.rowmax());
  //int indx[30];

  ludcmp(a,indx,d);

  dmatrix y(m1.rowmin(),m1.rowmax(),m1.rowmin(),m1.rowmax());
  dvector col(m1.rowmin(),m1.rowmax());

  for (int j=m1.rowmin(); j<=m1.rowmax(); j++)
  {
    for (i=m1.rowmin(); i<=m1.rowmax(); i++)
    {
      col[i]=0;
    }
    col[j]=1;

    lubksb(a,indx,col);

    for (i=m1.rowmin(); i<=m1.rowmax(); i++)
    {
      y[i][j]=col[i];
    }
  }
  return(y);
}

/** Inverse of a constant matrix by LU decomposition.
    \param m1 A dmatrix, \f$M\f$, for which the inverse is to be computed.
    \param _ln_det On return contains \f$|\log M|\f$
    \param _sign
    \return A dmatrix containing \f$M^{-1}\f$.
    \n\n The implementation of this algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 2
*/
dmatrix inv(const dmatrix& m1,const double& _ln_det, const int& _sgn)
{
  double d = 0.0;
  double& ln_det=(double&)(_ln_det);
  ln_det=0.0;
  int& sgn=(int&)(_sgn);

  if (m1.rowmin()!=m1.colmin() || m1.rowmax() != m1.colmax())
  {
    cerr << " Error in dmatrix inv(const dmatrix&) -- matrix not square \n";
  }

  dmatrix a(m1.rowmin(),m1.rowmax(),m1.rowmin(),m1.rowmax());

  int i;
  for (i=m1.rowmin(); i<=m1.rowmax(); i++)
  {
    for (int j=m1.rowmin(); j<=m1.rowmax(); j++)
    {
      a[i][j]=m1[i][j];
    }
  }
  ivector indx(m1.rowmin(),m1.rowmax());
  //int indx[30];

  ludcmp(a,indx,d);
  if (d>.1)
  {
    sgn=1;
  }
  else if (d<-0.1)
  {
    sgn=-1;
  }
  else
  {
    sgn=0;
  }
  int j;
  for (j=m1.rowmin();j<=m1.rowmax();j++)
  {
    if (a(j,j)>0)
    {
      ln_det+=log(a[j][j]);
    }
    else if (a(j,j)<0)
    {
      sgn=-sgn;
      ln_det+=log(-a[j][j]);
    }
    else
    {
      sgn=0;
    }
  }

  dmatrix y(m1.rowmin(),m1.rowmax(),m1.rowmin(),m1.rowmax());
  dvector col(m1.rowmin(),m1.rowmax());

  for (j=m1.rowmin(); j<=m1.rowmax(); j++)
  {
    for (i=m1.rowmin(); i<=m1.rowmax(); i++)
    {
      col[i]=0;
    }
    col[j]=1;

    lubksb(a,indx,col);

    for (i=m1.rowmin(); i<=m1.rowmax(); i++)
    {
      y[i][j]=col[i];
    }
  }
  return(y);
}

/** Lu decomposition of a constant matrix.
    \param _a  A dmatrix; replaced by the by its resulting LU decomposition
    \param _indx An ivector containing the row permutations generated by partial pivoting
    \param _d A double containing -1 or +1 depending whether the number of row interchanges was even or odd, repectively.
    \n\n The implementation of this algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 2
*/
void ludcmp(const dmatrix& _a, const ivector& _indx, const double& _d)
{
  int i=0;
  int j=0;
  int k=0;
  double& d=(double&)_d;
  dmatrix& a=(dmatrix&)_a;
  ivector& indx=(ivector&)_indx;

#if !defined(OPT_LIB) && (__cplusplus >= 201103L)
  int n = [](unsigned int colsize) -> int
  {
    assert(colsize <= INT_MAX);
    return static_cast<int>(colsize);
  } (a.colsize());
#else
  int n = static_cast<int>(a.colsize());
#endif
  int lb=a.colmin();
  int ub=a.colmax();

  double big,dum,sum,temp;

  dvector vv(lb,ub);

  d=1.0;

  for (i=lb;i<=ub;i++)
  {
    big=0.0;
    for (j=lb;j<=ub;j++)
    {
      temp=fabs(a[i][j]);
      if (temp > big)
      {
        big=temp;
      }
    }
    if (big == 0.0)
    {
      // this is caught in other locations so suppress for compact output
     //  if(function_minimizer::output_flag==2)
    // 	cerr << "Error in matrix inverse -- matrix singular in inv(dmatrix)\n";
    }
    vv[i]=1.0/big;
  }



  for (j=lb;j<=ub;j++)
  {
    for (i=lb;i<j;i++)
    {
      sum=a[i][j];
      for (k=lb;k<i;k++)
      {
        sum = sum - a[i][k]*a[k][j];
      }
      a[i][j]=sum;
    }
    int imax=j;
    big=0.0;
    for (i=j;i<=ub;i++)
    {
      sum=a[i][j];
      for (k=lb;k<j;k++)
      {
        sum = sum - a[i][k]*a[k][j];
      }
      a[i][j]=sum;
      dum=vv[i]*fabs(sum);
      if ( dum >= big)
      {
        big=dum;
        imax=i;
      }
    }
    if (j != imax)
    {
      for (k=lb;k<=ub;k++)
      {
        dum=a[imax][k];
        a[imax][k]=a[j][k];
        a[j][k]=dum;
      }
      d = -d;
      vv[imax]=vv[j];
    }
    indx[j]=imax;

    if (a[j][j] == 0.0)
    {
      a[j][j]=TINY;
    }

    if (j != n)
    {
      dum=1.0/(a[j][j]);
      for (i=j+1;i<=ub;i++)
      {
        a[i][j] = a[i][j] * dum;
      }
    }
  }
}
#undef TINY

#define TINY 1.0e-50;

/** LU decomposition. Used to set up determinant computation.
    \param _a  A dmatrix; replaced by the by its resulting LU decomposition
    \param _indx An ivector containing the row permutations generated by partial pivoting
    \param _d A double containing -1 or +1 depending whether the number of row interchanges was even or odd, repectively.
    \n\n The implementation of this algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 2
*/
void ludcmp_det(const dmatrix& _a, const ivector& _indx, const double& _d)
{
  int i,j,k;
  double& d=(double&)_d;
  dmatrix& a=(dmatrix&)_a;
  ivector& indx=(ivector&)_indx;

#if !defined(OPT_LIB) && (__cplusplus >= 201103L)
  int n = [](unsigned int colsize) -> int
  {
    assert(colsize <= INT_MAX);
    return static_cast<int>(colsize);
  } (a.colsize());
#else
  int n = static_cast<int>(a.colsize());
#endif
  int lb=a.colmin();
  int ub=a.colmax();

  double big,dum,sum,temp;

  dvector vv(lb,ub);


  d=1.0;

  for (i=lb;i<=ub;i++)
  {
    big=0.0;
    for (j=lb;j<=ub;j++)
    {
      temp=fabs(a[i][j]);
      if (temp > big)
      {
        big=temp;
      }
    }
    if (big == 0.0)
    {
      d=0.;
    }
    vv[i]=1.0/big;
  }



  for (j=lb;j<=ub;j++)
  {
    for (i=lb;i<j;i++)
    {
      sum=a[i][j];
      for (k=lb;k<i;k++)
      {
        sum = sum - a[i][k]*a[k][j];
      }
      a[i][j]=sum;
    }
    int imax = j;
    big=0.0;
    for (i=j;i<=ub;i++)
    {
      sum=a[i][j];
      for (k=lb;k<j;k++)
      {
        sum = sum - a[i][k]*a[k][j];
      }
      a[i][j]=sum;
      dum=vv[i]*fabs(sum);
      if ( dum >= big)
      {
        big=dum;
        imax=i;
      }
    }
    if (j != imax)
    {
      for (k=lb;k<=ub;k++)
      {
        dum=a[imax][k];
        a[imax][k]=a[j][k];
        a[j][k]=dum;
      }
      d = -d;
      vv[imax]=vv[j];
    }
    indx[j]=imax;

    if (a[j][j] == 0.0)
    {
      a[j][j]=TINY;
    }

    if (j != n)
    {
      dum=1.0/(a[j][j]);
      for (i=j+1;i<=ub;i++)
      {
        a[i][j] = a[i][j] * dum;
      }
    }
  }
}


/** LU decomposition back susbstitution alogrithm for constant object.
    \param a A dmatrix containing LU decomposition of input matrix. \f$a\f$.
    \param indx Permutation vector from ludcmp.
    \param b A dvector containing the RHS, \f$b\f$ of the linear equation
    \f$A\cdot X = B\f$, to be solved, and containing on return the solution vector \f$X\f$.

    \n\n The implementation of this algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 2
*/
void lubksb(dmatrix a, const ivector& indx, dvector b)
{
  int i,ii=0,ip,j,iiflag=0;
  double sum;
  int lb=a.colmin();
  int ub=a.colmax();
  for (i=lb;i<=ub;i++)
  {
    ip=indx[i];
    sum=b[ip];
    b[ip]=b[i];
    if (iiflag)
    {
      for (j=ii;j<=i-1;j++)
      {
        sum -= a[i][j]*b[j];
      }
    }
    else if (!ISZERO(sum))
    {
      ii=i;
      iiflag=1;
    }
    b[i]=sum;
  }

  for (i=ub;i>=lb;i--)
  {
    sum=b[i];
    for (j=i+1;j<=ub;j++)
    {                        // !!! remove to show bug
      sum -= a[i][j]*b[j];
    }                        // !!! remove to show bug
    b[i]=sum/a[i][i];
  }
}

/** Compute determinant of a constant matrix.
    \ingroup matop
    \param m1 A dmatrix, \f$M\f$, for which the determinant is computed.
    \return A double containing \f$|M|\f$.
    \n\n The implementation of this algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 2
*/
double det(const dmatrix& m1)
{
  double d = 0.0;
  dmatrix a(m1.rowmin(),m1.rowmax(),m1.rowmin(),m1.rowmax());

  if (m1.rowmin()!=m1.colmin()||m1.rowmax()!=m1.colmax())
  {
    cerr << "Matrix not square in routine det()" << endl;
    ad_exit(1);
  }

  for (int i=m1.rowmin(); i<=m1.rowmax(); i++)
  {
    for (int j=m1.rowmin(); j<=m1.rowmax(); j++)
    {
      a[i][j]=m1[i][j];
    }
  }

  ivector indx(m1.rowmin(),m1.rowmax());
  ludcmp_det(a,indx,d);
  for (int j=m1.rowmin();j<=m1.rowmax();j++)
  {
      d*=a[j][j];
  }

  return(d);
}

/** Compute log determinant of a constant matrix.
    \param m1 A dmatrix, \f$M\f$, for which the determinant is computed.
    \param _sgn
    \return A double containing \f$|\log(M)|\f$.
    \n\n The implementation of this algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 2
*/
double ln_det(const dmatrix& m1, int& sgn)
{
  double d = 0.0;
  dmatrix a(m1.rowmin(),m1.rowmax(),m1.rowmin(),m1.rowmax());

  if (m1.rowmin()!=m1.colmin()||m1.rowmax()!=m1.colmax())
  {
    cerr << "Matrix not square in routine det()" << endl;
    ad_exit(1);
  }

  for (int i=m1.rowmin(); i<=m1.rowmax(); i++)
  {
    for (int j=m1.rowmin(); j<=m1.rowmax(); j++)
    {
      a[i][j]=m1[i][j];
    }
  }

  ivector indx(m1.rowmin(),m1.rowmax());
  ludcmp_det(a,indx,d);
  double ln_det=0.0;

  if (d>.1)
  {
    sgn=1;
  }
  else if (d<-0.1)
  {
    sgn=-1;
  }
  else
  {
    sgn=0;
  }
  for (int j=m1.rowmin();j<=m1.rowmax();j++)
  {
    if (a(j,j)>0)
    {
      ln_det+=log(a[j][j]);
    }
    else if (a(j,j)<0)
    {
      sgn=-sgn;
      ln_det+=log(-a[j][j]);
    }
    else
    {
      sgn=0;
    }
  }
  return(ln_det);
}

/** LU decomposition.
    \deprecated This function may be completely unused?

    \n\n The implementation of this algorithm was inspired by
    "Numerical Recipes in C", 2nd edition,
    Press, Teukolsky, Vetterling, Flannery, chapter 2
*/
void ludcmp_index(const dmatrix& _a, const ivector& _indx, const double& _d)
{
  int i=0;
  int j=0;
  int k=0;
  double& d=(double&)_d;
  dmatrix& a=(dmatrix&)_a;
  ivector& indx=(ivector&)_indx;
#if !defined(OPT_LIB) && (__cplusplus >= 201103L)
  int n = [](unsigned int colsize) -> int
  {
    assert(colsize <= INT_MAX);
    return static_cast<int>(colsize);
  } (a.colsize());
#else
  int n = static_cast<int>(a.colsize());
#endif
  int lb=a.colmin();
  int ub=a.colmax();
  indx.fill_seqadd(lb,1);

  double big,dum,sum,temp;

  dvector vv(lb,ub);


  d=1.0;

  for (i=lb;i<=ub;i++)
  {
    big=0.0;
    for (j=lb;j<=ub;j++)
    {
      temp=fabs(a[i][j]);
      if (temp > big)
      {
        big=temp;
      }
    }
    if (big == 0.0)
    {
      cerr << "Error in matrix inverse -- matrix singular in inv(dmatrix)\n";
    }
    vv[i]=1.0/big;
  }



  for (j=lb;j<=ub;j++)
  {
    for (i=lb;i<j;i++)
    {
      sum=a[i][j];
      for (k=lb;k<i;k++)
      {
        sum = sum - a[i][k]*a[k][j];
      }
      a[i][j]=sum;
    }
    int imax=j;
    big=0.0;
    for (i=j;i<=ub;i++)
    {
      sum=a[i][j];
      for (k=lb;k<j;k++)
      {
        sum = sum - a[i][k]*a[k][j];
      }
      a[i][j]=sum;
      dum=vv[i]*fabs(sum);
      if ( dum >= big)
      {
        big=dum;
        imax=i;
      }
    }
    if (j != imax)
    {
      for (k=lb;k<=ub;k++)
      {
        dum=a[imax][k];
        a[imax][k]=a[j][k];
        a[j][k]=dum;
      }
      d = -d;
      vv[imax]=vv[j];
      int itemp=indx.elem(imax);
      indx.elem(imax)=indx.elem(j);
      indx.elem(j)=itemp;
    }

    if (a[j][j] == 0.0)
    {
      a[j][j]=TINY;
    }

    if (j != n)
    {
      dum=1.0/(a[j][j]);
      for (i=j+1;i<=ub;i++)
      {
        a[i][j] = a[i][j] * dum;
      }
    }
  }
}
#undef TINY


