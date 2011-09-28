/*
 * $Id$
 */

#include <fvar.hpp>
#include <adstring.hpp>
#if defined(__BORLANDC__)
  #include <strstrea.h>
  #include <iomanip.h>
#else
  #include <strstream>
  using namespace std;
#endif


//extern adstring copyright; ///< Text string containing copyright information; defined in the .tpl file

/*! \brief Generates a boast.
     Provides information on SVN revision number, compiler and operating system.
     Includes the file
     "banner-info.cpp" containg information generated by the makefile.
    \param program_name a adstring containing a version, e.g. "4.0x"
    \return an adstring containg the boast
 */
void banner(const adstring& program_name)
{
  char s[500];
  std::ostrstream ss(s,500);

  ss << "AD Model Builder(ADMB): Copyright (c) 2008-2011 Regents of the University of California\n\n";

  ss << program_name << ": ADMB";

#if defined(ADMB_VERSION)
  ss << " " << ADMB_VERSION;
#endif

#if defined(ADMB_REVISION)
  #define STR(x) #x
  #define STR2(x) STR(x)
  ss << "(Revision: " << STR2(ADMB_REVISION) << ")";
#endif

  if (which_library() == 'o')
     ss << " optimized libraries";
  else
     ss << " safe libraries";

  ss << " compiled with ";

#if defined(__GNUC__)
  ss <<  "Gnu C++ " << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
  #if defined(__x86_64)
  ss <<  "(64bit)";
  #else
  ss <<  "(32bit)";
  #endif
#elif defined(_MSC_VER)
  ss << "Microsoft Visual C++";
  #if (_MSC_VER==1300) 
  ss << "7.0";
  #elif (_MSC_VER==1310) 
  ss << "2003 7.1";
  #elif (_MSC_VER==1400) 
  ss << "2005 8.0";
  #elif (_MSC_VER==1500) 
  ss << "2008 9.0";
  #elif (_MSC_VER==1600) 
  ss << "2010 10.0";
  #endif
  #if defined(_M_X64)
  ss <<  "(64bit)";
  #else
  ss <<  "(32bit)";
  #endif
#elif defined(__BORLANDC__)
  ss << "Borland C++";
  #if (__BORLANDC__==0x550)
  ss << " 5.5";
  #elif (__BORLANDC__==0x551)
  ss << " 5.51";
  #endif
  ss <<  "(32bit)";
#elif defined(__SUNPRO_CC)
  ss << "Sun Studio";
  #if (__SUNPRO_CC==0x420)
  ss << "4.2";
  #elif (__SUNPRO_CC==0x500)
  ss << "5.0";
  #endif
  #if defined(__x86_64)
  ss <<  "(64bit)";
  #else
  ss <<  "(32bit)";
  #endif
#elif defined(__INTEL_COMPILER)
  ss << "Intel C/C++";
  #if (__INTEL_COMPILER==500)
  ss << " 5.0"
  #elif (__INTEL_COMPILER==600)
  ss << " 6.0"
  #elif (__INTEL_COMPILER==800)
  ss << " 8.0"
  #elif (__INTEL_COMPILER==900)
  ss << " 9.0"
  #endif
  #if defined(__x86_64)
  ss <<  "(64bit)";
  #else
  ss <<  "(32bit)";
  #endif
#else
  ss << "unknown compiler";
#endif
  ss << '\n';
  
  ss << '\0';

  cout << s << endl;
}
