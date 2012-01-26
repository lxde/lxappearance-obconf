/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   main.c for ObConf, the configuration tool for Openbox
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "main.h"
#include "archive.h"
#include "theme.h"
#include "appearance.h"
#include "preview_update.h"
#include "tree.h"
#include <glib/gi18n-lib.h>

#include "lxappearance/lxappearance.h"

#include <gdk/gdkx.h>

GtkWidget *mainwin = NULL;

GtkBuilder* builder;
xmlDocPtr doc;
xmlNodePtr root;
RrInstance *rrinst;
gchar *obc_config_file = NULL;
ObtPaths *paths;
ObtXmlInst *xml_i;

/* Disable, not used
static gchar *obc_theme_install = NULL;
static gchar *obc_theme_archive = NULL;
*/

/* Forwarded */
extern gboolean plugin_load(LXAppearance* app, GtkBuilder* lxappearance_builder);
extern void plugin_unload(LXAppearance* app) ;
/* End Forwarded */

void obconf_error(gchar *msg, gboolean modal)
{
    GtkWidget *d;

    d = gtk_message_dialog_new(mainwin ? GTK_WINDOW(mainwin) : NULL,
                               GTK_DIALOG_DESTROY_WITH_PARENT,
                               GTK_MESSAGE_ERROR,
                               GTK_BUTTONS_CLOSE,
                               "%s", msg);
    gtk_window_set_title(GTK_WINDOW(d), "ObConf Error");
    if (modal)
        gtk_dialog_run(GTK_DIALOG(d));
    else {
        g_signal_connect_swapped(GTK_WIDGET(d), "response",
                                 G_CALLBACK(gtk_widget_destroy),
                                 GTK_WIDGET(d));
        gtk_widget_show(d);
    }
}

static gboolean get_all(Window win, Atom prop, Atom type, gint size,
                        guchar **data, guint *num)
{
    gboolean ret = FALSE;
    gint res;
    guchar *xdata = NULL;
    Atom ret_type;
    gint ret_size;
    gulong ret_items, bytes_left;

    res = XGetWindowProperty(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), win, prop, 0l, G_MAXLONG,
                             FALSE, type, &ret_type, &ret_size,
                             &ret_items, &bytes_left, &xdata);
    if (res == Success) {
        if (ret_size == size && ret_items > 0) {
            guint i;

            *data = g_malloc(ret_items * (size / 8));
            for (i = 0; i < ret_items; ++i)
                switch (size) {
                case 8:
                    (*data)[i] = xdata[i];
                    break;
                case 16:
                    ((guint16*)*data)[i] = ((gushort*)xdata)[i];
                    break;
                case 32:
                    ((guint32*)*data)[i] = ((gulong*)xdata)[i];
                    break;
                default:
                    g_assert_not_reached(); /* unhandled size */
                }
            *num = ret_items;
            ret = TRUE;
        }
        XFree(xdata);
    }
    return ret;
}

static gboolean prop_get_string_utf8(Window win, Atom prop, gchar **ret)
{
    gchar *raw;
    gchar *str;
    guint num;

    if (get_all(win, prop,
                gdk_x11_get_xatom_by_name("UTF8_STRING"),
                8,(guchar**)&raw, &num))
    {
        str = g_strndup(raw, num); /* grab the first string from the list */
        g_free(raw);
        if (g_utf8_validate(str, -1, NULL)) {
            *ret = str;
            return TRUE;
        }
        g_free(str);
    }
    return FALSE;
}

static void on_response(GtkDialog* dlg, int res, LXAppearance* app)
{
    if(res == GTK_RESPONSE_APPLY)
    {
        tree_apply();
    }
}

/* int main(int argc, char **argv) */
extern gboolean plugin_load(LXAppearance* app, GtkBuilder* lxappearance_builder)
{
    gboolean exit_with_error = FALSE;

    /* ABI compatibility check. */
    if(app->abi_version > LXAPPEARANCE_ABI_VERSION)
        return FALSE;

    /* detect openbox */
    const char* wm_name = gdk_x11_screen_get_window_manager_name(gtk_widget_get_screen(app->dlg));
    if(g_strcmp0(wm_name, "Openbox"))
        return FALSE; /* don't load the plugin if openbox is not in use. */

#ifdef ENABLE_NLS
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
#endif

    mainwin = app->dlg;

    builder = gtk_builder_new();
    gtk_builder_set_translation_domain(builder, GETTEXT_PACKAGE);
    g_debug(GLADEDIR"/obconf.glade");
    if(!gtk_builder_add_from_file(builder, GLADEDIR"/obconf.glade", NULL))
    {
        obconf_error(_("Failed to load the obconf.glade interface file. ObConf is probably not installed correctly."), TRUE);
        exit_with_error = TRUE;
    }
    gtk_builder_connect_signals(builder, NULL);
    gtk_box_pack_start( GTK_BOX(app->wm_page), get_widget("obconf_vbox"), TRUE, TRUE, 0);
    gtk_widget_show_all(app->wm_page);

    g_signal_connect(app->dlg, "response", G_CALLBACK(on_response), app);

    paths = obt_paths_new();
    xml_i = obt_xml_instance_new();

    if (!obc_config_file) {
        gchar *p;

        if (prop_get_string_utf8(GDK_ROOT_WINDOW(),
                                 gdk_x11_get_xatom_by_name("_OB_CONFIG_FILE"),
                                 &p))
        {
            obc_config_file = g_filename_from_utf8(p, -1, NULL, NULL, NULL);
            g_free(p);
        }
    }

    xmlIndentTreeOutput = 1;
    if (!((obc_config_file &&
       obt_xml_load_file(xml_i, obc_config_file, "openbox_config")) ||
       obt_xml_load_config_file(xml_i, "openbox", "rc.xml", "openbox_config")))
    {
        obconf_error(_("Failed to load an rc.xml. Openbox is probably not installed correctly."), TRUE);
        exit_with_error = TRUE;
    }
    else {
        doc = obt_xml_doc(xml_i);
        root = obt_xml_root(xml_i);
    }

    /* look for parsing errors */
    {
        xmlErrorPtr e = xmlGetLastError();
        if (e) {
            char *a = g_strdup_printf
                (_("Error while parsing the Openbox configuration file. Your configuration file is not valid XML.\n\nMessage: %s"),
                 e->message);
            obconf_error(a, TRUE);
            g_free(a);
            exit_with_error = TRUE;
        }
    }

    rrinst = RrInstanceNew(GDK_DISPLAY_XDISPLAY(gdk_display_get_default()), gdk_x11_get_default_screen());
    if (!exit_with_error) {
        theme_setup_tab();
        appearance_setup_tab();
        theme_load_all();
    }
    return !exit_with_error;
}

extern void plugin_unload(LXAppearance* app)
{
    preview_update_set_active_font(NULL);
    preview_update_set_inactive_font(NULL);
    preview_update_set_menu_header_font(NULL);
    preview_update_set_menu_item_font(NULL);
    preview_update_set_osd_active_font(NULL);
    preview_update_set_osd_inactive_font(NULL);
    preview_update_set_title_layout(NULL);

    RrInstanceFree(rrinst);
    obt_xml_instance_unref(xml_i);
    obt_paths_unref(paths);

    xmlFreeDoc(doc);
}
