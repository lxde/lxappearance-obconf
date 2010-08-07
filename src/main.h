/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   main.h for ObConf, the configuration tool for Openbox
   Copyright (c) 2003        Dana Jansens

   Copyright (C) 2010        Hong Jen Yee (PCMan) <pcman.tw@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

/* This file is part of ObConf. It's taken by Hong Jen Yee on
 * 2010-08-07 and some modifications were done to make it a loadable
 * module of LXAppearance. */

#ifndef obconf__main_h
#define obconf__main_h

#include <openbox/render.h>
#include <openbox/instance.h>
#include <openbox/parse.h>

#include <gtk/gtk.h>

extern GtkBuilder *builder;
extern xmlDocPtr doc;
extern xmlNodePtr root;
extern RrInstance *rrinst;
extern GtkWidget *mainwin;
extern gchar *obc_config_file;

#define get_widget(s) GTK_WIDGET(gtk_builder_get_object(builder, s))

void obconf_error(gchar *msg, gboolean model);

#endif
