#include "preview_update.h"
#include "main.h"

static gboolean restart_theme_preview_update = TRUE;

static GtkTreeView  *tree_view            = NULL;
static GtkListStore *list_store           = NULL;
static gchar        *title_layout         = NULL;
static RrFont       *active_window_font   = NULL;
static RrFont       *inactive_window_font = NULL;
static RrFont       *menu_title_font      = NULL;
static RrFont       *menu_item_font       = NULL;
static RrFont       *osd_font             = NULL;


void preview_update_all()
{
    if (!list_store) return;

    if (!(title_layout && active_window_font && inactive_window_font &&
          menu_title_font && menu_item_font && osd_font))
        return; /* not set up */

    char* name;
    GtkTreeIter it;
    GtkTreeSelection* sel = gtk_tree_view_get_selection(tree_view);
    if(gtk_tree_selection_get_selected(sel, NULL, &it))
    {
        gtk_tree_model_get(list_store, &it, 0, &name, -1);
        GdkPixbuf* pix = preview_theme(name, title_layout, active_window_font,
                                         inactive_window_font, menu_title_font,
                                         menu_item_font, osd_font);
        GtkWidget* preview = get_widget("preview");
        gtk_image_set_from_pixbuf(preview, pix);
        g_object_unref(pix);
    }
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

void preview_update_set_osd_font(RrFont *f)
{
    RrFontClose(osd_font);
    osd_font = f;
    preview_update_all();
}

void preview_update_set_title_layout(const gchar *layout)
{
    g_free(title_layout);
    title_layout = g_strdup(layout);
    preview_update_all();
}
