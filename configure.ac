AC_PREREQ([2.54])
AC_INIT([lxappearance-obconf], [0.2.3], [http://lxde.org])
AM_INIT_AUTOMAKE([1.11 -Wall -Werror foreign no-dist-gzip dist-xz])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AC_PREFIX_DEFAULT([/usr/local])
test "$prefix" = "NONE" && prefix=$ac_default_prefix

# Support silent build rules. Disable by either passing --disable-silent-rules
# to configure or passing V=1 to make
AM_SILENT_RULES([yes])

AM_MAINTAINER_MODE([enable])

# Pick compiler specific/build target flags, and set $CVS
AM_PROG_CC_C_O
AC_C_CONST
AC_C_INLINE

AC_PROG_INSTALL

#Initialize libtool
LT_PREREQ([2.2])
LT_INIT

# CHECK_EXPORT_DYNAMIC
# LDFLAGS="$LDFLAGS $EXPORT_DYNAMIC_FLAGS"

# intltool
IT_PROG_INTLTOOL([0.40.0])

GETTEXT_PACKAGE=lxappearance-obconf
AC_SUBST(GETTEXT_PACKAGE)

# Generate po/LINGUAS on the fly rather than relying on translators
# to maintain it manually. This also overcome the problem that Transifex
# cannot add a language to po/LINGUAS if a new po file is submitted.
rm -f $srcdir/po/LINGUAS
for po_file in `ls $srcdir/po/*.po | sort`;
do
    lang=`echo "$po_file" | sed "s|.*/po/\(.*\)\.po|\1|g"`
    echo $lang >> $srcdir/po/LINGUAS
done

AM_GLIB_GNU_GETTEXT
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [Gettext package.])

AC_ARG_ENABLE(debug,
    [AC_HELP_STRING([--enable-debug],
        [build lxappearance-obconf with debug support @<:@default=no@:>@])],
    [enable_debug="${enableval}"],
    [enable_debug=no]
)

if test "$enable_debug" = "yes"; then
    # turn on debug and disable optimization
    CPPFLAGS="$CPPFLAGS -DG_ENABLE_DEBUG -O0 -g"
    case "$CC" in
    gcc*)
        CPPFLAGS="$CPPFLAGS -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers"
        ;;
    *)
        ;;
    esac
    dnl Be more strict on portability
    CPPFLAGS="$CPPFLAGS -D_POSIX_C_SOURCE=200112L -D_XOPEN_SOURCE=700"
else
    # turn off glib debug checks
    CPPFLAGS="$CPPFLAGS -DG_DISABLE_ASSERT -DG_DISABLE_CHECKS -DG_DISABLE_CAST_CHECKS"
fi

AC_ARG_ENABLE(more_warnings,
       [AC_HELP_STRING([--enable-more-warnings],
               [Add more warnings @<:@default=no@:>@])],
       [enable_more_warnings="${enableval}"],
       [enable_more_warnings=no]
)

AC_ARG_ENABLE(gtk3,
AC_HELP_STRING([--enable-gtk3],[enable to use gtk-3.0 instead of gtk-2.0]),
[case "${enableval}" in
  yes)  enable_gtk3=yes ;;
  no)   enable_gtk3=no ;;
  *) AC_MSG_ERROR([bad value "${enableval}" for --enable-gtk3, use "yes" (default) or "no".]) ;;
esac],[])

PKG_CHECK_MODULES(OPENBOX, [obrender-3.5 >= 3.5 obt-3.5 >= 3.5])
AC_SUBST(OPENBOX_CFLAGS)
AC_SUBST(OPENBOX_LIBS)

if test "x$enable_gtk3" = "xyes" ; then
  CFLAGS="$CFLAGS -DENABLE_GTK3"
  gtk_modules="gtk+-3.0 >= 3.0.0"
else
  gtk_modules="gtk+-2.0 >= 2.12.0 gthread-2.0"
fi

PKG_CHECK_MODULES(GTK, [$gtk_modules])
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

if test "x$enable_gtk3" = "xyes" ; then
  gdk_modules="gdk-3.0 gdk-pixbuf-2.0"
else
  gdk_modules="gdk-pixbuf-2.0"
fi

PKG_CHECK_MODULES(GDK_PIXBUF, [$gdk_modules])
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

if test x"$enable_more_warnings" = x"yes"; then
  ADDITIONAL_FLAGS="-Wall -Werror=all -Werror=format -Werror=implicit-function-declaration -Werror=implicit-int -Werror=missing-braces -Werror=parentheses -Werror=return-type -Werror=strict-aliasing -Werror=switch -Wuninitialized -Werror=unused-label -Werror=unused-value -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Werror=missing-declarations -Wredundant-decls -Wmissing-noreturn -Wpointer-arith -Wcast-align -Wwrite-strings -Werror=inline -Werror=format-nonliteral -Wformat-nonliteral -Werror=format-security -Wformat-security -Winit-self -Werror=missing-include-dirs -Werror=undef -Werror=aggregate-return -Wmissing-format-attribute -Werror=nested-externs -fno-strict-aliasing -fmessage-length=0 -Wp,-D_FORTIFY_SOURCE=2 -DG_DISABLE_DEPRECATED -DG_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_DEPRECATED -DGDK_PIXBUF_DISABLE_SINGLE_INCLUDES -DGTK_DISABLE_DEPRECATED -DGTK_DISABLE_SINGLE_INCLUDES"
fi
AC_SUBST(ADDITIONAL_FLAGS)

AC_CONFIG_FILES([
  Makefile
  po/Makefile.in
])
AC_OUTPUT

AC_MSG_RESULT
AC_MSG_RESULT([configure complete, now type \"make\"])
