SUBDIRS = po

localedir       = $(datadir)/locale
gladedir        = $(datadir)/lxappearance/obconf
pixmapdir       = $(datadir)/pixmaps
themedir        = $(datadir)/openbox/themes
desktopdir      = $(datadir)/applications
mimedir         = $(datadir)/mime
mimepkgdir      = $(datadir)/mime/packages
kdemimedir      = $(datadir)/mimelnk/application

AUTOMAKE_OPTIONS = subdir-objects foreign
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = \
	src/lxappearance-obconf

src_lxappearance_obconf_CPPFLAGS = \
	$(OPENBOX_CFLAGS) \
	$(GTK_CFLAGS) \
	$(GLADE_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS) \
	$(XML_CFLAGS) \
	$(LIBSTARTUPNOTIFICATION_CFLAGS) \
	-DLOCALEDIR=\"$(localedir)\" \
	-DPIXMAPDIR=\"$(pixmapdir)\" \
	-DGLADEDIR=\"$(gladedir)\" \
	-DTHEMEDIR=\"$(themedir)\" \
	-DG_LOG_DOMAIN=\"Obconf\"
src_lxappearance_obconf_LDADD = \
	$(OPENBOX_LIBS) \
	$(GTK_LIBS) \
	$(GLADE_LIBS) \
	$(GDK_PIXBUF_LIBS) \
	$(LIBINTL) \
	$(LIBSTARTUPNOTIFICATION_LIBS)
src_lxappearance_obconf_SOURCES = \
	src/gettext.h \
	src/about.c \
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
	src/obconf.glade \
	pixmaps/video-display.png

dist_noinst_DATA = \
	src/obconf.gladep

dist_desktop_DATA = \
	obconf.desktop

dist_mimepkg_DATA = \
	obconf.xml

dist_kdemime_DATA = \
	x-openbox-theme.desktop

dist_pixmap_DATA = \
	data/obconf.png

EXTRA_DIST = \
	config.rpath \
	README \
	CHANGELOG \
	COPYING \
	AUTHORS \
	data/video-display.svg

# Packages need to run these...

install-data-hook:
#	update-mime-database $(mimedir)
#	update-desktop-database $(desktopdir)

uninstall-hook:
#	update-mime-database $(mimedir)
#	update-desktop-database $(desktopdir)
