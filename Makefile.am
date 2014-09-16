SUBDIRS = po

localedir       = $(datadir)/locale
gladedir        = $(LXAPPEARANCE_DATADIR)/obconf
pixmapdir       = $(datadir)/pixmaps
themedir        = $(datadir)/openbox/themes
desktopdir      = $(datadir)/applications
mimedir         = $(datadir)/mime
mimepkgdir      = $(datadir)/mime/packages
kdemimedir      = $(datadir)/mimelnk/application

AUTOMAKE_OPTIONS = subdir-objects foreign
ACLOCAL_AMFLAGS = -I m4

module_LTLIBRARIES = src/obconf.la
moduledir = $(libdir)/lxappearance/plugins

src_obconf_la_CPPFLAGS = \
	$(OPENBOX_CFLAGS) \
	$(GTK_CFLAGS) \
	$(LXAPPEARANCE_CFLAGS) \
	$(GMODULE_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS) \
	$(XML_CFLAGS) \
	$(ADDITIONAL_FLAGS) \
	-DLOCALEDIR=\"$(localedir)\" \
	-DPIXMAPDIR=\"$(pixmapdir)\" \
	-DGLADEDIR=\"$(gladedir)\" \
	-DTHEMEDIR=\"$(themedir)\" \
	-DG_LOG_DOMAIN=\"Obconf\"

src_obconf_la_LIBADD = \
	$(OPENBOX_LIBS) \
	$(GTK_LIBS) \
	$(LXAPPEARANCE_LIBS) \
	$(GMODULE_LIBS) \
	$(GDK_PIXBUF_LIBS) \
	$(LIBINTL)

src_obconf_la_LDFLAGS = \
	-module \
	@LXAPPEARANCE_MODULE@

src_obconf_la_SOURCES = \
	src/archive.c \
	src/archive.h \
	src/main.c \
	src/main.h \
	src/appearance.c \
	src/appearance.h \
	src/preview.c \
	src/preview.h \
	src/preview_update.c \
	src/preview_update.h \
	src/theme.c \
	src/theme.h \
	src/tree.c \
	src/tree.h

dist_glade_DATA = \
	src/obconf.glade

EXTRA_DIST = \
	autogen.sh \
	README \
	CHANGELOG \
	COPYING \
	AUTHORS

# Packages need to run these...

install-data-hook:
#	update-mime-database $(mimedir)
#	update-desktop-database $(desktopdir)

uninstall-hook:
#	update-mime-database $(mimedir)
#	update-desktop-database $(desktopdir)
