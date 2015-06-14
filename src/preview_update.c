/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   preview_update.c for ObConf, the configuration tool for Openbox
   Copyright (c) 2003-2008   Dana Jansens
   Copyright (c) 2003        Tim Riley

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

#include "preview.h" /* preview_theme */
#include "preview_update.h"
#include "main.h"

/* Disable not used
static gboolean restart_theme_preview_update = TRUE;
*/

static GtkTreeView  *tree_view            = NULL;
static GtkListStore *list_store           = NULL;
static gchar        *title_layout         = NULL;
static RrFont       *active_window_font   = NULL;
static RrFont       *inactive_window_font = NULL;
static RrFont       *menu_title_font      = NULL;
static RrFont       *menu_item_font       = NULL;
static RrFont       *osd_active_font      = NULL;
static RrFont       *osd_inactive_font    = NULL;

void preview_update_all()
{
#if RR_CHECK_VERSION(3, 5, 0)
    if (!list_store) return;

    if (!(title_layout && active_window_font && inactive_window_font &&
          menu_title_font && menu_item_font &&
	  osd_active_font && osd_inactive_font))
        return; /* not set up */

    char* name;
    GtkTreeIter it;
    GtkTreeSelection* sel = gtk_tree_view_get_selection(tree_view);
    if(gtk_tree_selection_get_selected(sel, NULL, &it))
    {
        gtk_tree_model_get( GTK_TREE_MODEL(list_store), &it, 0, &name, -1);
        GdkPixbuf* pix = preview_theme(name, title_layout, active_window_font,
                                         inactive_window_font, menu_title_font,
                                         menu_item_font, osd_active_font,
					 osd_inactive_font);
        GtkWidget* preview = get_widget("preview");
        gtk_image_set_from_pixbuf( GTK_IMAGE(preview), pix);
        g_object_unref(pix);
    }
#endif
}

void preview_update_set_tree_view(GtkTreeView *tr, GtkListStore *ls)
{
    g_assert(!!tr == !!ls);

    if (list_store) g_idle_remove_by_data(list_store);

    tree_view = tr;
    list_store = ls;

    if (list_store) preview_update_all();
}

void preview_update_set_active_font(RrFont *f)
{
    RrFontClose(active_window_font);
    active_window_font = f;
    preview_update_all();
}

void preview_update_set_inactive_font(RrFont *f)
{
    RrFontClose(inactive_window_font);
    inactive_window_font = f;
    preview_update_all();
}

void preview_update_set_menu_header_font(RrFont *f)
{
    RrFontClose(menu_title_font);
    menu_title_font = f;
    preview_update_all();
}

void preview_update_set_menu_item_font(RrFont *f)
{
    RrFontClose(menu_item_font);
    menu_item_font = f;
    preview_update_all();
}

void preview_update_set_osd_active_font(RrFont *f)
{
    RrFontClose(osd_active_font);
    osd_active_font = f;
    preview_update_all();
}

void preview_update_set_osd_inactive_font(RrFont *f)
{
    RrFontClose(osd_inactive_font);
    osd_inactive_font = f;
    preview_update_all();
}

void preview_update_set_title_layout(const gchar *layout)
{
    g_free(title_layout);
    title_layout = g_strdup(layout);
    preview_update_all();
}
