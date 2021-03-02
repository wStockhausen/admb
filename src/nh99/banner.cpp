/**
@file
@author David Fournier
@copyright Copyright (c) 2008-2020 ADMB Foundation

@brief Function banner
*/

#include <fvar.hpp>
#include <adstring.hpp>
#if defined(__BORLANDC__)
  #include <strstrea.h>
  #include <iomanip.h>
#else
  #include <sstream>
  using namespace std;
#endif

#include <sstream>
using std::ostringstream;

#define STR(x) #x
#define STR2(x) STR(x)

/*! \brief Generates a boast.
     Provides information on SVN revision number, compiler and operating system.
     Includes the file
     "banner-info.cpp" containg information generated by the makefile.
    \param program_name a adstring containing a version, e.g. "4.0x"
    \return an adstring containg the boast
 */
void banner(const adstring& program_name)
{
  ostringstream ss;

  ss << "ADMB Program: " << program_name << "\n\n";

  ss << "ADMB";

#if defined(ADMB_VERSION)
  ss << "-" << STR2(ADMB_VERSION);
#endif

  if (which_library() == 'o')
     ss << " optimized libraries";
  else
     ss << " safe libraries";

  ss << " compiled with ";

#if defined(__GNUC__)
  ss <<  "GNU C++ " << __GNUC__ << '.' << __GNUC_MINOR__ << '.'
    << __GNUC_PATCHLEVEL__;
  #if defined(__x86_64)
  ss <<  " (64bit)";
  #else
  ss <<  " (32bit)";
  #endif
#elif defined(_MSC_VER)
  ss << "Microsoft Visual C++ ";
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
  #elif (_MSC_VER==1700)
  ss << "2012 11.0";
  #elif (_MSC_VER==1800)
  ss << "2013 12.0";
  #elif (_MSC_VER==1900)
  ss << "2015 14.0";
  #elif (_MSC_VER<1920)
  ss << "2017 Version 15";
  #elif (_MSC_VER<1930)
  ss << "2019 Version 16";
  #else
    #if DEBUG
      #error "Unknown MSVC version."
    #endif
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
  ss << "Solaris Studio";
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
  ss << "\n";
  ss << "Build date: " << __DATE__ << "\n";
#if defined(ADMB_REVISION)
  ss << "Revision: " << STR2(ADMB_REVISION) << "\n";
#endif
  ss << "Copyright (c) 2008-2021 ADMB Foundation"
     << " and Regents of the University of California\n";

  cout << ss.str() << endl;
}
