/*
 * $Id$
 *
 * Author: David Fournier
 * Copyright (c) 2008-2012 Regents of the University of California
 */
/**
  \file sgradclc.cpp
  Functions to compute gradient from the global \ref gradient_structure.
 */
#if !defined(DOS386)
  #define DOS386
#endif

#include "fvar.hpp"

#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#ifdef __TURBOC__
  #pragma hdrstop
  #include <iostream.h>
#endif

#ifdef __ZTC__
  #include <iostream.hpp>
#endif

#if defined (__WAT32__)
#  include <io.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
  #ifdef _M_X64
  typedef __int64 ssize_t;
  #else
  typedef int ssize_t;
  #endif
  #define  read _read
  #define write _write
#else
  #include <iostream>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
#endif

#ifdef __SUN__
  #include <iostream.h>
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <unistd.h>
#endif

#if defined(__NDPX__ )
  extern "C" {
    int LSEEK(int, int, int);
    int read(int, char*, int);
  };
#endif

#include <math.h>

#if defined(__ZTC__)
  void _far * _cdecl _farptr_norm(void _far *);
  void _far * _cdecl _farptr_fromlong(unsigned long);
  long _cdecl _farptr_tolong(void _far *);
#endif

#if !defined(OPT_LIB) || defined(_MSC_VER)
  #include <cassert>
#endif

/**
Compute the gradient from the data stored in the global \ref gradient_structure.

\param nvar Number of variables in the gradient.
\param _g Vector from 1 to nvar. On return contains the gradient.
*/
void gradcalc(int nvar, const dvector& _g)
{
  if (nvar!=0)
  {
    if (nvar != gradient_structure::NVAR)
    {
      cerr << "nvar != gradient_structure::NVAR in gradcalc" << endl;
      cerr << "  nvar = " << nvar << endl;
      cerr << "  gradient_structure::NVAR = " << gradient_structure::NVAR
           << endl;
      cerr << "  in " __FILE__ << endl;
      ad_exit(1);
    }
  }
  dvector& g= (dvector&) _g;
  gradient_structure::TOTAL_BYTES = 0;
  gradient_structure::PREVIOUS_TOTAL_BYTES=0;
  if(!gradient_structure::instances)
  {
    g.initialize();
    return;
  }

  if (g.size() < static_cast<unsigned int>(nvar))
  {
    cerr  << "gradient vector size is less than the number of variables.\n";
    ad_exit(1);
  }

   gradient_structure::GRAD_STACK1->_GRADFILE_PTR =
     gradient_structure::GRAD_STACK1->gradfile_handle();

  int& _GRADFILE_PTR=gradient_structure::GRAD_STACK1->_GRADFILE_PTR;

  LSEEK(_GRADFILE_PTR,0L,SEEK_CUR);

  if (gradient_structure::GRAD_STACK1->ptr <=
        gradient_structure::GRAD_STACK1->ptr_first)
  {
#ifdef DIAG
    cerr << "warning -- calling gradcalc when no calculations generating"
         << endl << "derivative information have occurred" << endl;
#endif
    g.initialize();
    return;
  }    // current is one past the end so -- it

  if (gradient_structure::save_var_flag)
  {
    gradient_structure::save_arrays();
    gradient_structure::save_variables();
  }

  gradient_structure::GRAD_STACK1->ptr--;

  gradient_structure::GRAD_LIST->initialize();

  memset(gradient_structure::ARRAY_MEMBLOCK_BASE, 0,
    gradient_structure::ARR_LIST1->get_max_last_offset());

  *gradient_structure::GRAD_STACK1->ptr->dep_addr = 1;

  //int icount=0;
  int break_flag=1;
  do
  {
    gradient_structure::GRAD_STACK1->ptr++;
#ifdef FAST_ASSEMBLER
    gradloop();
#else
    grad_stack_entry* grad_ptr_first =
      gradient_structure::GRAD_STACK1->ptr_first;
    while (gradient_structure::GRAD_STACK1->ptr-- >
             grad_ptr_first)
    {
      (*(gradient_structure::GRAD_STACK1->ptr->func))();
/*
      icount++;
      if (icount%1000==0)
      {
        //cout << "icount = " << icount << endl;
      }
*/
    }
#endif

   // back up the file one buffer size and read forward
   OFF_T offset = (OFF_T)(sizeof(grad_stack_entry)
                  * gradient_structure::GRAD_STACK1->length);
   OFF_T lpos = LSEEK(gradient_structure::GRAD_STACK1->_GRADFILE_PTR,
      -offset,SEEK_CUR);

    break_flag=gradient_structure::GRAD_STACK1->read_grad_stack_buffer(lpos);
  } while (break_flag);

  {
#ifdef GRAD_DIAG
    long int ttmp =
#endif
    LSEEK(gradient_structure::GRAD_STACK1->_GRADFILE_PTR, 0,SEEK_CUR);
#ifdef GRAD_DIAG
    cout << "Offset in file at end of gradcalc is " << ttmp
         << " bytes from the beginning\n";
#endif
  }

  for (int i=0, j=g.indexmin(); i < nvar; ++i, ++j)
  {
    g[j] = *gradient_structure::INDVAR_LIST->get_address(i);
  }

  gradient_structure::GRAD_STACK1->ptr =
    gradient_structure::GRAD_STACK1->ptr_first;

  if (gradient_structure::save_var_flag)
  {
    gradient_structure::restore_arrays();
    gradient_structure::restore_variables();
  }
}
/**
Compute the gradient from the data stored in the global \ref gradient_structure.

\param nvar Number of variables in the gradient.
\param _g Vector from 1 to nvar. On return contains the gradient.
\param f objective function
\returns likelihood value
double gradcalc(int nvar, const dvector& _g, dvariable& f)
{
  double v = value(f);
  gradcalc(nvar, _g);
  return v;
}
*/
/**
 */
void gradient_structure::save_arrays()
{
  void * temp_ptr;
  unsigned long bytes_needed =
    min(gradient_structure::ARR_LIST1->get_last_offset() + 1,
        ARRAY_MEMBLOCK_SIZE);
  gradient_structure::save_var_file_flag=0;
#ifdef __ZTC__
   if ( (temp_ptr = farmalloc(bytes_needed) ) == 0)
#else
   //if ( (temp_ptr = malloc(bytes_needed) ) == 0)
   if ((temp_ptr = (void *)malloc(bytes_needed)) == 0)
  #define __USE_IOSTREAM__
#endif
   {
     gradient_structure::save_var_file_flag=1;
     cerr << "insufficient memory to allocate space for ARRAY_MEMBLOCK"
          << " save buffer " << endl;
   }
   if (gradient_structure::save_var_file_flag==0)
   {
     ARRAY_MEMBLOCK_SAVE = temp_ptr;
#if defined(DOS386)
  #ifndef USE_ASSEMBLER
         memcpy((char*)ARRAY_MEMBLOCK_SAVE,(char*)ARRAY_MEMBLOCK_BASE,
           bytes_needed);
  #else
         dw_block_move((double*)ARRAY_MEMBLOCK_SAVE,
           (double*)ARRAY_MEMBLOCK_BASE,bytes_needed/8);
  #endif
#else
     unsigned long int max_move=50000;
     unsigned long int left_to_move=bytes_needed;
     humungous_pointer dest = ARRAY_MEMBLOCK_SAVE;
     humungous_pointer src = ARRAY_MEMBLOCK_BASE;
     while(left_to_move > max_move)
     {
       memcpy((char*)dest,(char*)src,max_move);
       left_to_move-=max_move;
       src+=max_move;
       dest+=max_move;
     }
     memcpy((char*)dest,(char*)src,left_to_move);
#endif
  }
  else
  {
     humungous_pointer src = ARRAY_MEMBLOCK_BASE;
     LSEEK(gradient_structure::GRAD_STACK1->_VARSSAV_PTR,0L,SEEK_SET);
#if defined(DOS386)
  #ifdef OPT_LIB
     write(gradient_structure::GRAD_STACK1->_VARSSAV_PTR,
       (char*)src, bytes_needed);
  #else
     ssize_t ret = write(gradient_structure::GRAD_STACK1->_VARSSAV_PTR,
       (char*)src, bytes_needed);
     assert(ret != -1);
  #endif
#else
     unsigned long int max_move=500;
     unsigned long int left_to_move=bytes_needed;
     while(left_to_move > max_move)
     {
       write(_VARSSAV_PTR,(char*)src,max_move);
       left_to_move-=max_move;
       src+=max_move;
     }
     write(gradient_structure::GRAD_STACK1->_VARSSAV_PTR,(char*)src,
       left_to_move);
#endif
  }
}

/**
 */
void gradient_structure::restore_arrays()
{
  unsigned long bytes_needed =
    min(gradient_structure::ARR_LIST1->get_last_offset() + 1,
        ARRAY_MEMBLOCK_SIZE);
  if (gradient_structure::save_var_file_flag==0)
  {
#if defined(DOS386)
  #ifndef USE_ASSEMBLER
        memcpy((char*)ARRAY_MEMBLOCK_BASE,(char*)ARRAY_MEMBLOCK_SAVE,
          bytes_needed);
  #else
         dw_block_move((double*)ARRAY_MEMBLOCK_BASE,
           (double*)ARRAY_MEMBLOCK_SAVE,bytes_needed/8);
  #endif
#else
     unsigned long max_move=50000;

     long int left_to_move=bytes_needed;
     humungous_pointer src = ARRAY_MEMBLOCK_SAVE;
     humungous_pointer dest = ARRAY_MEMBLOCK_BASE;
     while(left_to_move > max_move)
     {
       memcpy((char*)dest,(char*)src,max_move);
       left_to_move-=max_move;
       src+=max_move;
       dest+=max_move;
     }
     memcpy((char*)dest,(char*)src,left_to_move);
#endif
    ARRAY_MEMBLOCK_SAVE.free();
  }
  else
  {
    humungous_pointer dest = ARRAY_MEMBLOCK_BASE;
    LSEEK(gradient_structure::GRAD_STACK1->_VARSSAV_PTR,0L,SEEK_SET);
#if defined(DOS386)
  #if defined(OPT_LIB) && !defined(_MSC_VER)
    read(gradient_structure::GRAD_STACK1->_VARSSAV_PTR,
      (char*)dest,bytes_needed);
  #else
    ssize_t ret = read(gradient_structure::GRAD_STACK1->_VARSSAV_PTR,
      (char*)dest,bytes_needed);
    assert(ret != -1);
  #endif
#else
     unsigned long int max_move=50000;

     long int left_to_move=bytes_needed;
     while(left_to_move > max_move)
     {
       read(gradient_structure::GRAD_STACK1->_VARSSAV_PTR,
         (char*)dest,max_move);
       left_to_move-=max_move;
       dest+=max_move;
     }
     read(gradient_structure::GRAD_STACK1->_VARSSAV_PTR,
       (char*)dest,left_to_move);
#endif
  }
}
/**
Save variables to a buffer.
*/
void gradient_structure::save_variables()
{
  GRAD_LIST->save_variables();
}
/**
Restore variables from buffer.
*/
void gradient_structure::restore_variables()
{
  GRAD_LIST->restore_variables();
}
/**
Rewind buffer.
*/
void reset_gradient_stack(void)
{
  gradient_structure::GRAD_STACK1->ptr =
    gradient_structure::GRAD_STACK1->ptr_first;

  int& _GRADFILE_PTR=gradient_structure::GRAD_STACK1->_GRADFILE_PTR;

  LSEEK(_GRADFILE_PTR,0L,SEEK_SET);
}
/**
  Sets the gradient stack entry for a function or operator with a single
  independent variable.
  \param func Pointer to function to compute the derivative of the dependent
   variable with respect to the independent variable.
   Function prototype: void func(void);
  \param dep_addr Address of dependent variable; pointer to double.
  \param ind_addr1 Address of independent variable; pointer to double
 */
void grad_stack::set_gradient_stack1(void (* func)(void),
  double* dep_addr, double* ind_addr1)
{
#ifdef NO_DERIVS
  if (!gradient_structure::no_derivatives)
  {
#endif
    if (ptr > ptr_last)
    {
       // current buffer is full -- write it to disk and reset pointer
       // and counter
       this->write_grad_stack_buffer();
    }
    //test_the_pointer();
    ptr->func = func;
    ptr->dep_addr = dep_addr;
    ptr->ind_addr1 = ind_addr1;
    ptr++;
#ifdef NO_DERIVS
  }
#endif
}
#ifdef DIAG
void test_the_pointer(void)
{
/*
  static int inner_count = 0;
  static grad_stack_entry * pgse = (grad_stack_entry*) (0x1498fffc);
  inner_count++;
  if (inner_count == 404849)
  {
    cout << ptr << endl;
    cout << ptr->func << endl;
    cout << ptr->dep_addr << endl;
    cout << (int)(ptr->dep_addr)%8 << endl;
  }
  pgse->func = (void (*)())(100);
  pgse->dep_addr = (double*) 100;
  pgse->ind_addr1 = (double*) 100;
*/
}
#endif
