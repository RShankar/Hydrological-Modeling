
AC_INIT(NEWS)
AC_CONFIG_AUX_DIR(config)

AC_CANONICAL_SYSTEM
AM_CONFIG_HEADER(src/config.h)
AM_INIT_AUTOMAKE(systemc-ams,[1.0Beta2])




REVISION="\$Revision: 1199 $"
AC_REVISION($Revision: 1199 $)
AC_DEFINE_UNQUOTED(REVISION, "$REVISION", [SVN Revision])



define([CDATE], [`sh -c "date"`])
AC_DEFINE_UNQUOTED(CONFIGURE_DATE, "CDATE", [Date of configure])



AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_INSTALL

CXX_COMP=`basename $CXX`
CC_COMP=`basename $CC`

AR_TOOL="ar cru"


case "$depmode" in
  gcc3)
   OPT_CXX_VERSION_FLAGS="-fmessage-length=0"
   ;;
   *)
   OPT_CXX_VERSION_FLAGS=""
   ;;
esac



AC_ARG_WITH( systemc_path,    
             AS_HELP_STRING([--with-systemc=DIR],[Path to OSCI SystemC library]))

case "${with_systemc:-}" in
  "no")  systemc_enab="no";    systemc_path="";;
  "yes") systemc_enab="yes";   systemc_path="";;
  "")    systemc_enab="maybe"; systemc_path="";;
  *)     systemc_enab="yes";   systemc_path="${with_systemc}";;
esac

if test -z "$systemc_path"; then
  AC_MSG_CHECKING(for systemc directory)
  if test -d "${SYSTEMC_PATH}"; then
      builddir=`pwd`;
      systemc_path=`cd ${SYSTEMC_PATH}; pwd`
      AC_MSG_RESULT(found in \$SYSTEMC_PATH)
  elif test -d "${SYSTEMC_HOME}"; then
      builddir=`pwd`;
      systemc_path=`cd ${SYSTEMC_HOME}; pwd`
      AC_MSG_RESULT(found in \$SYSTEMC_HOME)
  elif test -d "`pwd`/systemc-2.2"; then
    builddir=`pwd`;
    systemc_path=`cd $builddir/systemc-2.2; pwd`
  elif test -d "`pwd`/../systemc-2.2"; then
    builddir=`pwd`;
    systemc_path=`cd $builddir/../systemc-2.2; pwd`
  fi
  if test -z "$systemc_path"; then
    AC_MSG_WARN(SystemC directory was not found)
    AC_MSG_RESULT(searched \$SYSTEMC_PATH)
    AC_MSG_RESULT(searched \$SYSTEMC_HOME)
    AC_MSG_RESULT(searched `pwd`/systemc-2.2)
    AC_MSG_RESULT(searched `pwd`/../systemc-2.2)
    AC_MSG_RESULT(unable to find systemc directory. use --with-systemc=<systemc_dir>)
  else
    AC_MSG_RESULT($systemc_path)
  fi 
fi  
   
if test "$systemc_enab" != "no"; then
  if test -r "$systemc_path"; then
    systemc_path=`cd $systemc_path;pwd`;
  else
    AC_MSG_ERROR(SystemC installation root directory does not exist.
       Was systemC installed correctly?)
  fi
fi

if test "$systemc_enab" != "no"; then
  header_found=0
  if test $header_found = 0; then
    SYSC_INC_PREFIX="$systemc_path/include"
    AC_MSG_CHECKING("for $SYSC_INC_PREFIX/systemc.h")
    if test -r "$SYSC_INC_PREFIX/systemc.h"; then
      header_found=1
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  fi
  if test $header_found = 0; then
    SYSC_INC_PREFIX=`cd $systemc_path/../include; pwd`
    AC_MSG_CHECKING("for $SYSC_INC_PREFIX/systemc.h")
    if test -r "$SYSC_INC_PREFIX/systemc.h"; then
      header_found=1
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  fi
  if test $header_found = 0; then
    AC_MSG_ERROR(Could not find all required files for SystemC.
      Was systemC installed correctly?)
  fi
fi

case "$target" in
    sparc-sun-solaris*)
        case "$CXX_COMP" in
            CC)
                EXTRA_CXXFLAGS=""
                DEBUG_CXXFLAGS="-g -DDEBUG $GPROF_FLAGS"
                OPT_CXXFLAGS="-fast -xO4 -xnolibmil -mc"
                TARGET_ARCH="sparcOS5"
		AR_TOOL="$CXX -xar -o"
        	CC=gcc
        	CFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $GPROF_FLAGS"
                ;;
            c++ | g++)
                TARGET_ARCH="gccsparcOS5"

                EXTRA_CXXFLAGS="-I$SYSC_INC_PREFIX"
                OPT_CXXFLAGS="-O3 -g -Wall"
                CXXFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $OPT_CXX_VERSION_FLAGS $GPROF_FLAGS"
                DEBUG_CXXFLAGS="-g -O0 -DDEBUG $OPT_CXX_VERSION_FLAGS"

        	    CC=gcc
        	    CFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $GPROF_FLAGS"
                ;;
            *)
                AC_MSG_ERROR("sorry... compiler not supported.")
		;;
        esac
        ;;
    x86_64*linux*)
        case "$CXX_COMP" in
            c++ | g++)
                TARGET_ARCH="linux64"

                EXTRA_CXXFLAGS="-I$SYSC_INC_PREFIX -fPIC"
                OPT_CXXFLAGS="-O3 -g -Wall -pedantic -Wno-long-long"
                CXXFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $OPT_CXX_VERSION_FLAGS $GPROF_FLAGS"
                DEBUG_CXXFLAGS="-O0 -g $OPT_CXX_VERSION_FLAGS"

        	    CC=gcc
        	    CFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $GPROF_FLAGS"
                ;;
            *)
                AC_MSG_ERROR("sorry... compiler not supported.")
		;;
        esac
        ;;
    *linux*)
        case "$CXX_COMP" in
            c++ | g++)
                TARGET_ARCH="linux"

                EXTRA_CXXFLAGS="-I$SYSC_INC_PREFIX -fPIC"
                OPT_CXXFLAGS="-O3 -g -Wall -pedantic -Wno-long-long"
                CXXFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $OPT_CXX_VERSION_FLAGS $GPROF_FLAGS"
                DEBUG_CXXFLAGS="-O0 -g $OPT_CXX_VERSION_FLAGS"

        	    CC=gcc
        	    CFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $GPROF_FLAGS"
                ;;
            *)
                AC_MSG_ERROR("sorry... compiler not supported.")
		;;
        esac
        ;;
    *cygwin*)
        case "$CXX_COMP" in
            c++ | g++)
                TARGET_ARCH="cygwin"

                EXTRA_CXXFLAGS="-I$SYSC_INC_PREFIX"
                OPT_CXXFLAGS="-O3 -g -Wall -pedantic -Wno-long-long"
                CXXFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $OPT_CXX_VERSION_FLAGS $GPROF_FLAGS"
                DEBUG_CXXFLAGS="-O0 -g $OPT_CXX_VERSION_FLAGS"

        	    CC=gcc
        	    CFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $GPROF_FLAGS"
                ;;
            *)
                AC_MSG_ERROR("sorry... compiler not supported.")
		;;
        esac
        ;;
    *mingw32*)
        case "$CXX_COMP" in
            c++ | g++)
                TARGET_ARCH="mingw32"

                EXTRA_CXXFLAGS="-I$SYSC_INC_PREFIX"
                OPT_CXXFLAGS="-O3 -Wall -g -pedantic -Wno-long-long"
                CXXFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $OPT_CXX_VERSION_FLAGS $GPROF_FLAGS"
                DEBUG_CXXFLAGS="-O0 -g $OPT_CXX_VERSION_FLAGS"

        	    CC=gcc
        	    CFLAGS="$EXTRA_CXXFLAGS $OPT_CXXFLAGS $GPROF_FLAGS"
                ;;
            *)
                AC_MSG_ERROR("sorry... compiler not supported.")
		;;
        esac
        ;;
    *)
        AC_MSG_ERROR("sorry... architecture not supported.")
        ;;
esac


test "x$prefix" = xNONE && prefix=$srcdir
current_dir=`pwd`; cd $prefix; prefix=`pwd`; cd $current_dir



AC_SUBST(EXTRA_CXXFLAGS)
AC_SUBST(DEBUG_CXXFLAGS)
AC_SUBST(OPT_CXXFLAGS)
AC_SUBST(TARGET_ARCH)
AC_SUBST(AR_TOOL)



AC_OUTPUT(Makefile src/Makefile \
          src/scams/Makefile \
          src/scams/analysis/Makefile \
          src/scams/analysis/ac/Makefile \
          src/scams/core/Makefile \
          src/scams/impl/Makefile \
          src/scams/impl/core/Makefile \
          src/scams/impl/predefined_moc/Makefile \
          src/scams/impl/predefined_moc/conservative/Makefile \
          src/scams/impl/predefined_moc/eln/Makefile \
          src/scams/impl/predefined_moc/lsf/Makefile \
          src/scams/impl/predefined_moc/tdf/Makefile \
          src/scams/impl/solver/Makefile \
          src/scams/impl/solver/linear/Makefile \
          src/scams/impl/solver/tdf/Makefile \
          src/scams/impl/solver/utilities/Makefile \
          src/scams/impl/solver/utilities/sparse_library/Makefile \
          src/scams/impl/synchronization/Makefile \
          src/scams/impl/util/Makefile \
          src/scams/impl/util/data_types/Makefile \
          src/scams/impl/util/tracing/Makefile \
          src/scams/impl/util/reporting/Makefile \
          src/scams/impl/analysis/Makefile \
          src/scams/impl/analysis/ac/Makefile \
          src/scams/predefined_moc/Makefile \
          src/scams/predefined_moc/eln/Makefile \
          src/scams/predefined_moc/lsf/Makefile \
          src/scams/predefined_moc/tdf/Makefile \
          src/scams/utility/Makefile \
          src/scams/utility/data_types/Makefile \
          src/scams/utility/reporting/Makefile \
          src/scams/utility/tracing/Makefile \
          documents/Makefile )


dnl Taf!



