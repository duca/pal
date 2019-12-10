dnl 1. Ensure target device toolchain is present
dnl 2. configure device subdir
dnl 3. AC_DEFINE(ENABLE_DEV_${device}
dnl
dnl Requires AX_CONFIG_DIR
dnl
dnl Arg 1 target name
dnl Arg 2 target canonical name

AC_DEFUN([PAL_CONFIG_DEVICE], [

dnl Ensure toolchain is present.
m4_define([target], translit($1, [a-z], [A-Z]))dnl
m4_define([target_ar],      m4_expand([target])_AR)
m4_define([target_cc],      m4_expand([target])_CC)
m4_define([target_cpp],     m4_expand([target])_CPP)
m4_define([target_cxx],     m4_expand([target])_CXX)
m4_define([target_cxxcpp],  m4_expand([target])_CXXCPP)
m4_define([target_ld],      m4_expand([target])_LD)
m4_define([target_nm],      m4_expand([target])_NM)
m4_define([target_objdump], m4_expand([target])_OBJDUMP)
m4_define([target_ranlib],  m4_expand([target])_RANLIB)
m4_define([target_strip],   m4_expand([target])_STRIP)

AC_CHECK_PROG([target_ar],      $2-ar,       [$2-ar])
AC_CHECK_PROG([target_cc],      $2-gcc,      [$2-gcc])
AC_CHECK_PROG([target_cpp],     $2-gcc,      [$2-gcc -E])
AC_CHECK_PROG([target_cxx],     $2-g++,      [$2-g++])
AC_CHECK_PROG([target_cxxcpp],  $2-g++,      [$2-g++ -E])
AC_CHECK_PROG([target_ld],      $2-ld,       [$2-ld])
AC_CHECK_PROG([target_nm],      $2-nm,       [$2-nm -B])
AC_CHECK_PROG([target_objdump], $2-objdump,  [$2-objdump])
AC_CHECK_PROG([target_ranlib],  $2-ranlib,   [$2-ranlib])
AC_CHECK_PROG([target_strip],   $2-strip,    [$2-strip])

AS_IF([test "x$m4_expand(target_ar)"      = "x" -o \
            "x$m4_expand(target_cc)"      = "x" -o \
            "x$m4_expand(target_cpp)"     = "x" -o \
            "x$m4_expand(target_cxx)"     = "x" -o \
            "x$m4_expand(target_cxxcpp)"  = "x" -o \
            "x$m4_expand(target_ld)"      = "x" -o \
            "x$m4_expand(target_nm)"      = "x" -o \
            "x$m4_expand(target_objdump)" = "x" -o \
            "x$m4_expand(target_ranlib)"  = "x" -o \
            "x$m4_expand(target_strip)"   = "x"],
      [AC_MSG_ERROR([$2 toolchain not found])])

AC_SUBST(m4_expand(target_ar))
AC_SUBST(m4_expand(target_cc))
AC_SUBST(m4_expand(target_cpp))
AC_SUBST(m4_expand(target_cxx))
AC_SUBST(m4_expand(target_cxxcpp))
AC_SUBST(m4_expand(target_ld))
AC_SUBST(m4_expand(target_nm))
AC_SUBST(m4_expand(target_objdump))
AC_SUBST(m4_expand(target_ranlib))
AC_SUBST(m4_expand(target_strip))

AC_MSG_NOTICE([$2 toolchain found])

dnl Save flags
pal_save_prefix=$prefix
pal_save_exec_prefix=$exec_prefix
pal_save_ac_configure_args=$ac_configure_args
pal_save_CFLAGS=$CFLAGS
pal_save_CPPFLAGS=$CPPFLAGS
pal_save_CXXFLAGS=$CXXFLAGS
pal_save_LDFLAGS=$LDFLAGS

dnl Save tools
pal_save_AR=$AR
pal_save_CC=$CC
pal_save_CPP=$CPP
pal_save_CXX=$CXX
pal_save_CXXCPP=$CXXCPP
pal_save_NM=$NM
pal_save_OBJDUMP=$OBJDUMP
pal_save_RANLIB=$RANLIB
pal_save_STRIP=$STRIP

host_ac_configure_args=$ac_configure_args
ac_configure_args=

dnl Use target flags
m4_expand([CFLAGS=${])m4_expand([target])_CFLAGS:-"-g -O2"}
m4_expand([CXXFLAGS=${])m4_expand([target])_CXXFLAGS:-"-g -O2"}
m4_expand([CPPFLAGS=$])m4_expand([target])_CPPFLAGS
m4_expand([LDFLAGS=$])m4_expand([target])_LDFLAGS
export CFLAGS
export CXXFLAGS
export CPPFLAGS
export LDFLAGS

dnl Use target tools
m4_expand([AR=$])m4_expand([target])_AR
m4_expand([CC=$])m4_expand([target])_CC
m4_expand([CPP=$])m4_expand([target])_CPP
m4_expand([CXX=$])m4_expand([target])_CXX
m4_expand([CXXCPP=$])m4_expand([target])_CXXCPP
m4_expand([NM=$])m4_expand([target])_NM
m4_expand([OBJDUMP=$])m4_expand([target])_OBJDUMP
m4_expand([RANLIB=$])m4_expand([target])_RANLIB
m4_expand([STRIP=$])m4_expand([target])_STRIP
export AR
export CC
export CPP
export CXX
export CXXCPP
export NM
export OBJDUMP
export RANLIB
export STRIP

# Whitelist arguments
for host_arg in $host_ac_configure_args
do
AS_CASE([$host_arg],
        [*--enable-assert*], [ac_configure_args="$ac_configure_args $host_arg"],
        [*--disable-assert*],[ac_configure_args="$ac_configure_args $host_arg"],
        [])
done

# Disable everything but lib
ac_configure_args="--enable-tests --disable-benchmark --disable-examples --disable-doc $ac_configure_args"

# Insert target as host
ac_configure_args="--host=$2 $ac_configure_args"

# Expand default prefix NOW.
test "x$prefix" = xNONE && host_prefix=$ac_default_prefix || host_prefix=$prefix
test "x$target_prefix" = x && target_prefix="${host_prefix}/$2"
prefix=$target_prefix
exec_prefix=$prefix

# Insert target prefix
ac_configure_args="$ac_configure_args --prefix=${target_prefix}"

# Custom command that configures subdir NOW, so we can restore variables after.
AX_CONFIG_DIR(devices/$1, [../..])

# Restore flags
prefix=$pal_save_prefix
exec_prefix=$pal_save_exec_prefix
ac_configure_args=$pal_save_ac_configure_args
CFLAGS=$pal_save_CFLAGS
CPPFLAGS=$pal_save_CPPFLAGS
CXXFLAGS=$pal_save_CXXFLAGS
LDFLAGS=$pal_save_LDFLAGS
export CFLAGS
export CXXFLAGS
export CPPFLAGS
export LDFLAGS

dnl Restore tools
AR=$pal_save_AR
CC=$pal_save_CC
CPP=$pal_save_CPP
CXX=$pal_save_CXX
CXXCPP=$pal_save_CXXCPP
NM=$pal_save_NM
OBJDUMP=$pal_save_OBJDUMP
RANLIB=$pal_save_RANLIB
STRIP=$pal_save_STRIP
export AR
export CC
export CPP
export CXX
export CXXCPP
export NM
export OBJDUMP
export RANLIB
export STRIP

dnl Define device in config.h
AC_DEFINE([ENABLE_DEV_]m4_expand([target]), [1], [$1 device support])

])
