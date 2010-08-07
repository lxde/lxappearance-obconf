AC_PREREQ([2.54])
AC_INIT([lxappearance-obconf], [0.0.1], [http://lxde.org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AC_PREFIX_DEFAULT([/usr/local])
test "$prefix" = "NONE" && prefix=$ac_default_prefix

# Pick compiler specific/build target flags, and set $CVS
AM_PROG_CC_C_O
AC_C_CONST
AC_C_INLINE

AC_PROG_INSTALL

AC_PROG_LIBTOOL

# CHECK_EXPORT_DYNAMIC
# LDFLAGS="$LDFLAGS $EXPORT_DYNAMIC_FLAGS"

# intltool
IT_PROG_INTLTOOL([0.40.0])

ALL_LINGUAS=""
AM_GNU_GETTEXT_VERSION(0.15)
AM_GNU_GETTEXT([external])

PKG_CHECK_MODULES(OPENBOX, [obrender-3.0 >= 3.4.2 obparser-3.0 >= 3.4.2])
AC_SUBST(OPENBOX_CFLAGS)
AC_SUBST(OPENBOX_LIBS)

PKG_CHECK_MODULES(GTK, [gtk+-2.0])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

PKG_CHECK_MODULES(GDK_PIXBUF, [gdk-pixbuf-2.0])
AC_SUBST(GDK_PIXBUF_CFLAGS)
AC_SUBST(GDK_PIXBUF_LIBS)

PKG_CHECK_MODULES(GMODULE, [gmodule-export-2.0])
AC_SUBST(GMODULE_CFLAGS)
AC_SUBST(GMODULE_LIBS)

PKG_CHECK_MODULES(LXAPPEARANCE, [lxappearance])
AC_SUBST(LXAPPEARANCE_CFLAGS)
AC_SUBST(LXAPPEARANCE_LIBS)

# Make it an lxappearance plugin.
prefix=$(pkg-config --variable=prefix lxappearance)
LXAPPEARANCE_LIBDIR=$(pkg-config --variable=libdir lxappearance)/lxappearance
LXAPPEARANCE_DATADIR=$(pkg-config --variable=datadir lxappearance)/lxappearance
AC_SUBST(LXAPPEARANCE_LIBDIR)
AC_SUBST(LXAPPEARANCE_DATADIR)

LXAPPEARANCE_MODULE="-avoid-version \
    -no-undefined "'-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LXAPPEARANCE_MODULE)

AC_CONFIG_FILES([
  Makefile
  po/Makefile.in
])
AC_OUTPUT

AC_MSG_RESULT
AC_MSG_RESULT([configure complete, now type \"make\"])
