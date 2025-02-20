#!sh
#
# Created by: Arni Magnusson [2009-05-27] revision 243
#
# Copyright (c) 2008-2020 ADMB Foundation
#
shopt -s expand_aliases
alias help='echo -e "
Builds AD Model Builder executable or library.

Release Version: 12.3
Location: $BASH_SOURCE

Usage: admb [-c] [-d] [-f] [-g] [-p] [-r] model [src(s)]

Options:
 -c     Build only object file(s) (.obj).
 -d     Build a shared library (.so).
 -f     Build with fast optimized mode library (no bounds checking).
 -g     Build with debug symbols.
 -p     Build portable model executable (statically linked).
 -r     Build with random effects library (ADMB-RE).
 model  TPL file, i.e. 'simple.tpl' or 'simple' with no .tpl extension.
 src(s) C/C++ source file(s) containing classes, methods and variables.
"'

if [[ "$1" == "" ]]; then help; exit; fi
if [[ "$1" == "-h" ]]; then help; exit; fi
if [[ "$1" == "-help" ]]; then help; exit; fi
if [[ "$1" == "--help" ]]; then help; exit; fi

ADMB_HOME=
if [ -z "$ADMB_HOME" ]; then
  FILE=$0
  DIRECTORY=$(cd `dirname "$FILE"` && pwd)
  while [ -h "${FILE}" ]; do
    pushd "$DIRECTORY" &> /dev/null
    FILE=$(readlink "$DIRECTORY"/admb)
    DIRECTORY=$(cd `dirname "$FILE"` && pwd)
    popd &> /dev/null
  done

  if [ -e "${DIRECTORY}/bin/admb.sh" ]
  then
    pushd "$DIRECTORY" &> /dev/null
  else
    pushd "$DIRECTORY/.." &> /dev/null
  fi

  declare -rx ADMB_HOME=$PWD
  popd &> /dev/null
fi
if [ "$OS" == "Windows_NT" ]; then
  PATH=$ADMB_HOME/utilities/mingw/bin:$PATH
fi
PATH=$ADMB_HOME/bin:$PATH

# Pop args until model=$1
compileonly=
dll=
library=safe
debug=
output=
static=
parser=
while getopts "cdfgo:prs" A; do
  case $A in
    c)
       compileonly=yes
       ;;
    d)
       dll=-dll
       SHARED=-shared
       ;;
    f)
       library=opt
       ;;
    g)
       debug=-debug
       ;;
    o)
       output="$OPTARG"
       ;;
    p) static=-static
       ;;
    r)
       parser=tpl2rem
       ;;
    s)
       ;;
    *)
       help
       exit 1
       ;;
  esac
done
shift $((OPTIND-1))

tpls=
srcs=
objs=
nontpls=

for file in $*
do
  if [ "${file: -4}" = ".tpl" ]; then
    tpls="$tpls ${file%.*}"
  elif [ "${file: -4}" = ".cpp" -o "${file: -2}" = ".c" -o "${file: -3}" = ".cc" -o "${file: -4}" = ".cxx" ]; then
    srcs="$srcs $file"
    nontpls="$nontpls $file"
  elif [ "${file: -2}" = ".o" -o "${file: -4}" = ".obj" ]; then
    objs="$objs $file"
    nontpls="$nontpls $file"
  else
    tpls="$tpls $file"
  fi
done
if [ -z "$tpls" ]; then
  if [ -z "$srcs" ]; then
    if [ -z "$objs" ]; then
      if [ -z "$nontpls" ]; then
        echo -e "Error: Nothing to build."
        exit 1
      fi
    fi
  fi
fi
if [ "$OS" == "Windows_NT" ]; then
  UNAME_S="_NT"
else
  UNAME_S="`uname -s`"
fi
if [ "$UNAME_S" == "Linux" ]; then
  if [ "$CXX" == "" ]; then
    CXX=g++
  fi
  if [[ "`$CXX -dumpmachine`" =~ "i686" ]]; then
    OS_NAME=-i686-linux
  else
    if [[ "`$CXX -dumpmachine`" =~ "x86_64" ]]; then
      OS_NAME=-x86_64-linux
    else
      OS_NAME=-arm64-linux
    fi
  fi
fi
if [ "$UNAME_S" == "Darwin" ]; then
  if [ "$CXX" == "" ]; then
    CXX=c++
  fi
  if [[ "`$CXX -dumpmachine`" =~ "i686" ]]; then
    OS_NAME=-i686-macos
  else
    if [[ "`$CXX -dumpmachine`" =~ "x86_64" ]]; then
      OS_NAME=-x86_64-macos
    else
      OS_NAME=-arm64-macos
    fi
  fi
fi
if [[ "$UNAME_S" =~ "_NT" ]]; then
  if [ "$CXX" == "" ]; then
    CXX=g++
  fi
  if [ "$CXX" == "clang++" ]; then
    if [[ "`$CXX -dumpmachine`" =~ "x86_64" ]]; then
      OS_NAME=-win64
    else
      OS_NAME=-win32
    fi
  fi
  if [ "$CXX" == "g++" ]; then
    if [[ "`$CXX -dumpmachine`" =~ "x86_64" ]]; then
      OS_NAME=-mingw64
    else
      OS_NAME=-mingw32
    fi
  fi
fi
if [ "$CXX" == "g++" ]; then
  if [ "$UNAME_S" == "Darwin" ]; then
    if [[ "`$CXX --version`" =~ "clang-11" ]]; then
      CXXVERSION=$OS_NAME-clang11
    else
      if [[ "`$CXX --version`" =~ "clang-10" ]]; then
        CXXVERSION=$OS_NAME-clang10
      else
        CXXVERSION=$OS_NAME-clang
      fi
    fi
  else
    DUMPVERSION="`g++ -dumpversion | cut -f1 -d. `"
    CXXVERSION="$OS_NAME-$CXX$DUMPVERSION"
  fi
fi
if [ "$CXX" == "clang++" ]; then
  if [ "$UNAME_S" == "Darwin" ]; then
    if [[ "`$CXX --version`" =~ "clang-11" ]]; then
      CXXVERSION=$OS_NAME-clang11
    else
      if [[ "`$CXX --version`" =~ "clang-10" ]]; then
        CXXVERSION=$OS_NAME-clang10
      else
        CXXVERSION=$OS_NAME-clang
      fi
    fi
  else
    DUMPVERSION="`clang++ -dumpversion | cut -f1 -d. `"
    CXXVERSION="$OS_NAME-$CXX$DUMPVERSION"
  fi
fi
if [ "$CXX" == "c++" ]; then
  if [ "$UNAME_S" == "Darwin" ]; then
    if [[ "`$CXX --version`" =~ "clang-11" ]]; then
      CXXVERSION=$OS_NAME-clang11
    else
      if [[ "`$CXX --version`" =~ "clang-10" ]]; then
        CXXVERSION=$OS_NAME-clang10
      else
        CXXVERSION=$OS_NAME-clang
      fi
    fi
  else
    DUMPVERSION="`c++ -dumpversion | cut -f1 -d. `"
    CXXVERSION="$OS_NAME-$CXX$DUMPVERSION"
  fi
fi
if [ "$CXX" == "analyze-c++" ]; then
  DUMPVERSION="`analyze-c++ -dumpversion | cut -f1 -d. `"
  CXXVERSION="$OS_NAME-$CXX$DUMPVERSION"
fi
if [ "$CXX" == "icpc" ]; then
  DUMPVERSION="`icpc -dumpversion | cut -f1 -d. `"
  CXXVERSION="$OS_NAME-$CXX$DUMPVERSION"
fi
if [ -f "$ADMB_HOME/bin/admb-cfg-$CXX_VERSION.sh" ]; then
  source "$ADMB_HOME/bin/admb-cfg-$CXX_VERSION.sh"
  CXXFLAGS="$CXXFLAGS $ADMB_CFG_CXXFLAGS"
  LDFLAGS="$LDFLAGS $ADMB_CFG_LDFLAGS"
else
  CXXFLAGS="$CXXFLAGS"
  LDFLAGS="$LDFLAGS"
fi
if [ ! -z "$dll" ]; then
  if [ "$OS" != "Windows_NT" ]; then
    CXXFLAGS="-fPIC $CXXFLAGS"
  fi
  LDFLAGS="-shared $LDFLAGS"
fi
if [ ! -z "$debug" ]; then
  if [ "$CXX" == "openCC" ]; then
    CXXFLAGS="-g $CXXFLAGS"
    LDFLAGS="-g $LDFLAGS"
  else
    CXXFLAGS="-g -DDEBUG $CXXFLAGS"
    LDFLAGS="-g -DDEBUG $LDFLAGS"
  fi
else
  if [ "$CXX" == "openCC" ]; then
    CXXFLAGS="-O2 $CXXFLAGS"
    LDFLAGS="-O2 $LDFLAGS"
  elif [ "$CXX" == "icpc" ]; then
    CXXFLAGS=" $CXXFLAGS"
    LDFLAGS=" $LDFLAGS"
  else
    CXXFLAGS="-O3 $CXXFLAGS"
    LDFLAGS="-O3 $LDFLAGS"
  fi
fi
if [ "`uname`" == "Darwin" ]; then
  CXXFLAGS="-std=c++14 $CXXFLAGS"
  LDFLAGS="-std=c++14 $LDFLAGS"
elif [ "$CXX" == "clang++" ]; then
  CXXFLAGS="-std=c++11 $CXXFLAGS"
  LDFLAGS="-std=c++11 $LDFLAGS"
elif [ "$CXX" == "icpc" ]; then
  ICPCMAJVER="`icpc -dumpversion | cut -f1 -d.`"
  if [ "$ICPCMAJVER" == "17" ]; then
    CXXFLAGS="-std=c++14 $CXXFLAGS"
    LDFLAGS="-std=c++14 $LDFLAGS"
  else
    CXXFLAGS="-std=c++11 $CXXFLAGS"
    LDFLAGS="-std=c++11 $LDFLAGS"
  fi
elif [ "$CXX" == "CC" ]; then
  CXXFLAGS="-std=c++11 $CXXFLAGS"
  LDFLAGS="-std=c++11 $LDFLAGS"
elif [ "$CXX" == "g++" ]; then
  GCCMAJVER="`gcc -dumpversion | cut -f1 -d.`"
  if [ "$GCCMAJVER" == "4" ]; then
    GCCMINVER="`gcc -dumpversion | cut -f2 -d.`"
    if [ "$GCCMINVER" == "9" ]; then
      CXXFLAGS="-std=c++11 $CXXFLAGS"
      LDFLAGS="-std=c++11 $LDFLAGS"
    else
      if [ "$GCCMINVER" == "8" ]; then
        CXXFLAGS="-std=c++11 $CXXFLAGS"
        LDFLAGS="-std=c++11 $LDFLAGS"
      else
        CXXFLAGS="-std=c++0x $CXXFLAGS"
        LDFLAGS="-std=c++0x $LDFLAGS"
      fi
    fi
  elif [ "$GCCMAJVER" == "5" ]; then
    CXXFLAGS="-std=c++11 $CXXFLAGS"
    LDFLAGS="-std=c++11 $LDFLAGS"
  else
    CXXFLAGS="-std=c++14 $CXXFLAGS"
    LDFLAGS="-std=c++14 $LDFLAGS"
  fi
fi
CXXFLAGS="$CXXFLAGS -D_USE_MATH_DEFINES"
if [ "$library" == "opt" ]; then
  CXXFLAGS="$CXXFLAGS -DOPT_LIB"
fi
if [[ -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED.a" || -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED-debug.a" ]]; then
  CXXFLAGS="$CXXFLAGS -DUSE_ADMB_CONTRIBS -I. -I\"$ADMB_HOME/include\" -I\"$ADMB_HOME/include/contrib\""
else
  CXXFLAGS="$CXXFLAGS -I. -I\"$ADMB_HOME/include\""
fi
if [[ "$static" == "-static" || "$CXX" == "adcomp-x86_64-w64-mingw32" ]]; then
  LDFLAGS="--static $LDFLAGS"
fi
tplsrcs=
for model in $tpls
do
  if [ ! -f $model.tpl ]; then
    echo -e "Error: $model.tpl not found\\n"
    exit 1
  fi

  rm -f classdef.tmp xxdata.tmp xxhtop.tmp xxhtopm.tmp xxglobal.tmp xxtopm.tmp &> /dev/null
  rm -f xxalloc.tmp xxalloc1.tmp xxalloc2.tmp xxalloc3.tmp xxalloc4.tmp xxalloc5.tmp xxalloc6.tmp header.tmp &> /dev/null
  rm -f tfile1 tfile2 tfile3 tfile4 tfile5 &> /dev/null
  rm -f $model.cpp $model.htp $model.obj $model.o $model &> /dev/null
  rm -f admb.log &> /dev/null

  if [ "$parser" == "tpl2rem" ]; then
    CMD="$parser $debug $dll $model"
    echo -e \*\*\* Parse: $model.tpl\\n$CMD\\n
    eval $CMD
  else
    echo -e \*\*\* Parse: $model.tpl
    CMD="tpl2cpp $debug $dll $model || tpl2rem $debug $dll $model"
    echo -e $CMD\\n
    eval $CMD &> admb.log
  fi

  if [ ! -f $model.cpp -o ! -f $model.htp ]; then
    echo -e "Error: could not parse \"$model.tpl\".\\n"
    cat admb.log
    exit 1
  else
    rm -f admb.log &> /dev/null
  fi
  tplsrcs="$tplsrcs ${model%.*}.cpp"
done

tplobjs=
for file in $tplsrcs
do
  if [ ! -f $file ]; then
    echo -e "Error: $file not found\\n"
    exit 1
  fi
  if [ ! -z "$compileonly" ]; then
    if [ ! -z  "$output" ]; then
      if [ ${output: -4} == ".obj" ]; then
        fileobj="$output"
      else
        fileobj="$output".obj
      fi
    else
      fileobj=${file%.*}.obj
    fi
  else
    fileobj=${file%.*}.obj
  fi
  rm -f $fileobj
  CMD="$CXX -c $CXXFLAGS -o$fileobj $file"
  echo -e \*\*\* Compile: $file\\n$CMD\\n
  eval $CMD
  if [[ ! -f $fileobj ]]; then
    echo -e "Error: Could not compile $file\\n"
    exit 1
  fi
  if [ ! -z "$tplobjs" ]; then
    tplobjs="$tplobjs $fileobj"
  else
    tplobjs="$fileobj"
  fi
done

for file in $srcs
do
  if [ ! -f $file ]; then
    echo -e "Error: $file not found\\n"
    exit 1
  fi
  if [ ! -z "$compileonly" ]; then
    if [ ! -z  "$output" ]; then
      if [ ${output: -4} == ".obj" ]; then
        fileobj="$output"
      else
        fileobj="$output".obj
      fi
    else
      fileobj=${file%.*}.obj
    fi
  else
    fileobj=${file%.*}.obj
  fi
  rm -f $fileobj
  CMD="$CXX -c $CXXFLAGS -o$fileobj $file"
  echo -e \*\*\* Compile: $file\\n$CMD\\n
  eval $CMD
  if [[ ! -f $fileobj ]]; then
    echo -e "Error: Could not compile $file\\n"
    exit 1
  fi
  if [ ! -z "$objs" ]; then
    objs="$fileobj $objs"
  else
    objs="$fileobj"
  fi
done

if [ ! -z "$compileonly" ]; then
  if [ ! -z "$objs" ]; then
    objects=$objs
  else
    objects=$tplobjs
  fi
  exit 0
else
  if [ ! -z "$objs" ]; then
    objects=$objs
  else
    objects=$tplobjs
  fi
fi

for file in $tplobjs
do
  if [ ! -z  "$output" ]; then
    model="$output"
  else
    model=${file%.*}
  fi
  if [ ! -z  "$dll" ]; then
    CMD="$CXX $LDFLAGS -o$model.so"
  else
    CMD="$CXX $LDFLAGS -o$model"
  fi
  CMD="$CMD $file $objs"
  if [ ! -z  "$debug" ]; then
    if [ -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED-debug.a" ]; then
      if [[ "$library" == "opt" ]]; then
        CMD="$CMD \"$ADMB_HOME/lib/libadmb-contribo$CXXVERSION$SHARED-debug.a\""
      else
        CMD="$CMD \"$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED-debug.a\""
      fi
    else
      if [ -f "$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED-debug.a" ]; then
        if [[ "$library" == "opt" ]]; then
          CMD="$CMD \"$ADMB_HOME/lib/libadmbo$CXXVERSION$SHARED-debug.a\""
        else
          CMD="$CMD \"$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED-debug.a\""
        fi
      else
        if [ -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED.a" ]; then
          if [[ "$library" == "opt" ]]; then
            CMD="$CMD \"$ADMB_HOME/lib/libadmb-contribo$CXXVERSION$SHARED.a\""
          else
            CMD="$CMD \"$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED.a\""
          fi
        else
          if [ -f "$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED.a" ]; then
            if [[ "$library" == "opt" ]]; then
              CMD="$CMD \"$ADMB_HOME/lib/libadmbo$CXXVERSION$SHARED.a\""
            else
              CMD="$CMD \"$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED.a\""
            fi
          else
            echo -e "Error: Unable to find libadmb-contrib$CXXVERSION$SHARED-debug.a\\n"
            exit 0
          fi
        fi
      fi
    fi
  else
    if [ -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED.a" ]; then
      if [[ "$library" == "opt" ]]; then
        CMD="$CMD \"$ADMB_HOME/lib/libadmb-contribo$CXXVERSION$SHARED.a\""
      else
        CMD="$CMD \"$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED.a\""
      fi
    else
      if [ -f "$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED.a" ]; then
        if [[ "$library" == "opt" ]]; then
          CMD="$CMD \"$ADMB_HOME/lib/libadmbo$CXXVERSION$SHARED.a\""
        else
          CMD="$CMD \"$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED.a\""
        fi
      else
        if [ -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED-debug.a" ]; then
          if [[ "$library" == "opt" ]]; then
            CMD="$CMD \"$ADMB_HOME/lib/libadmb-contribo$CXXVERSION$SHARED-debug.a\""
          else
            CMD="$CMD \"$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED-debug.a\""
          fi
        else
          if [ -f "$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED-debug.a" ]; then
            if [[ "$library" == "opt" ]]; then
              CMD="$CMD \"$ADMB_HOME/lib/libadmbo$CXXVERSION$SHARED-debug.a\""
            else
              CMD="$CMD \"$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED-debug.a\""
            fi
          else
            echo -e "Error: Unable to find libadmb-contrib$CXXVERSION$SHARED.a\\n"
            exit 0
          fi
        fi
      fi
    fi
  fi
  echo -e \*\*\* Linking: $file $objs\\n$CMD\\n
  eval $CMD
  if [[ -z $dll ]]; then
    if [[ ! -f $model ]]; then
        echo -e "Error: Could not build $model\\n"
        exit 1
    fi
  else
    if [[ ! -f $model.so ]]; then
        echo -e "Error: Could not build $model.so\\n"
        exit 1
    fi
  fi
done

if [[ "$tplobjs" == "" ]]; then
  outputfilebasename=
  if [ ! -z  "$output" ]; then
    outputfilebasename="$output"
  fi
  listobjs=
  for file in $nontpls
  do
    if [ "${file: -2}" = ".o" -o "${file: -4}" = ".obj" ]; then
      listobjs="$listobjs $file"
    else
      listobjs="$listobjs ${file%.*}.obj"
    fi
    if [ -z  "$outputfilebasename" ]; then
      outputfilebasename="`basename $file`"
      outputfilebasename=${file%.*}
    fi
  done
  if [ ! -z  "$dll" ]; then
    CMD="$CXX $LDFLAGS -o$outputfilebasename.so"
  else
    CMD="$CXX $LDFLAGS -o$outputfilebasename"
  fi
  CMD="$CMD $listobjs"
  if [ ! -z  "$debug" ]; then
    if [ -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED-debug.a" ]; then
      if [[ "$library" == "opt" ]]; then
        CMD="$CMD \"$ADMB_HOME/lib/libadmb-contribo$CXXVERSION$SHARED-debug.a\""
      else
        CMD="$CMD \"$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED-debug.a\""
      fi
    else
      if [ -f "$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED-debug.a" ]; then
        if [[ "$library" == "opt" ]]; then
          CMD="$CMD \"$ADMB_HOME/lib/libadmbo$CXXVERSION$SHARED-debug.a\""
        else
          CMD="$CMD \"$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED-debug.a\""
        fi
      else
        if [ -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED.a" ]; then
          if [[ "$library" == "opt" ]]; then
            CMD="$CMD \"$ADMB_HOME/lib/libadmb-contribo$CXXVERSION$SHARED.a\""
          else
            CMD="$CMD \"$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED.a\""
          fi
        else
          if [ -f "$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED.a" ]; then
            if [[ "$library" == "opt" ]]; then
              CMD="$CMD \"$ADMB_HOME/lib/libadmbo$CXXVERSION$SHARED.a\""
            else
              CMD="$CMD \"$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED.a\""
            fi
          else
            echo -e "Error: Unable to find libadmb-contrib$CXXVERSION$SHARED-debug.a\\n"
            exit 0
          fi
        fi
      fi
    fi
  else
    if [ -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED.a" ]; then
      if [[ "$library" == "opt" ]]; then
        CMD="$CMD \"$ADMB_HOME/lib/libadmb-contribo$CXXVERSION$SHARED.a\""
      else
        CMD="$CMD \"$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED.a\""
      fi
    else
      if [ -f "$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED.a" ]; then
        if [[ "$library" == "opt" ]]; then
          CMD="$CMD \"$ADMB_HOME/lib/libadmbo$CXXVERSION$SHARED.a\""
        else
          CMD="$CMD \"$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED.a\""
        fi
      else
        if [ -f "$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED-debug.a" ]; then
          if [[ "$library" == "opt" ]]; then
            CMD="$CMD \"$ADMB_HOME/lib/libadmb-contribo$CXXVERSION$SHARED-debug.a\""
          else
            CMD="$CMD \"$ADMB_HOME/lib/libadmb-contrib$CXXVERSION$SHARED-debug.a\""
          fi
        else
          if [ -f "$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED-debug.a" ]; then
            if [[ "$library" == "opt" ]]; then
              CMD="$CMD \"$ADMB_HOME/lib/libadmbo$CXXVERSION$SHARED-debug.a\""
            else
              CMD="$CMD \"$ADMB_HOME/lib/libadmb$CXXVERSION$SHARED-debug.a\""
            fi
          else
            echo -e "Error: Unable to find libadmb-contrib$CXXVERSION$SHARED.a\\n"
            exit 0
          fi
        fi
      fi
    fi
  fi
  echo -e \*\*\* Linking: $listobjs\\n$CMD\\n
  eval $CMD
  if [ ! -z  "$output" ]; then
    if [[ -z $dll ]]; then
      if [[ ! -f $output ]]; then
        echo -e "Error: Could not build $output\\n"
        exit 1
      fi
    else
      if [[ ! -f $output.so ]]; then
        echo -e "Error: Could not build $output.so\\n"
        exit 1
      fi
    fi
  else
    declare -a amodel=( $listobjs )
    m=${amodel[0]}
    m2=${m%.*}
    if [[ -z $dll ]]; then
      if [[ ! -f $m2 ]]; then
        echo -e "Error: Could not build $m2\\n"
        exit 1
      fi
    else
      if [[ ! -f $m2.so ]]; then
        echo -e "Error: Could not build $m2.so\\n"
        exit 1
      fi
    fi
  fi
fi

if [ ! -z "$dll" ]; then
  echo -e "Successfully built.\\n"
else
  echo -e "Successfully built executable.\\n"
fi
exit 0
