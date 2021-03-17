/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   preview.c for ObConf, the configuration tool for Openbox
   Copyright (c) 2007        Javeed Shaikh
   Copyright (c) 2007        Dana Jansens

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

/*
 * This file was taken from ObConf; and some modifications were done
 * to make it a loadable module of LXAppearance.
 * Later, it was modified to support GTK3, in addition to GTK2.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "theme.h"
#include "main.h"
#include "tree.h"

#include <string.h>

#include <obrender/theme.h>

#if GTK_CHECK_VERSION(3, 0, 0)
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <gdk/gdkx.h>
#endif

#define PADDING 2 /* openbox does it :/ */

/* Forwarded */

GdkPixbuf *preview_theme(const gchar *name, const gchar *titlelayout,
                         RrFont *active_window_font,
                         RrFont *inactive_window_font,
                         RrFont *menu_title_font,
                         RrFont *menu_item_font,
                         RrFont *osd_active_font,
                         RrFont *osd_inactive_font);

typedef struct _CairoComposeParams {
    Display *display;             /* xLib Display */
    cairo_t *context;             /* cairo context for compositing surface */
    Visual *visual;               /* xLib display Visual */
    cairo_surface_t *surface_src; /* RrAppearance.Pixmap conversion surface */

} CairoComposeParams;

/* End forwarded */

static void theme_pixmap_paint(RrAppearance *a, gint w, gint h)
{
    Pixmap out = RrPaintPixmap(a, w, h);
    if (out) XFreePixmap(RrDisplay(a->inst), out);
}

static guint32 rr_color_pixel(const RrColor *c)
{
    return (guint32)((RrColorRed(c) << 24) + (RrColorGreen(c) << 16) +
                     (RrColorBlue(c) << 8) + 0xff);
}

/*! Initialize a Cairo context and surface
    to be used for compositing various Openbox themed window decoration
    for the "Window Border" preview pane.
    These will presumably be destroyed in finalize_composition()
    after any number of rendering passes through compose_surface()
    @param params  pointer to a CairoComposeParams struct
                   which is to describe/hold the destination Cairo context
    @param w       the desired width of the composite image
    @param h       the desired height of the composite image
    @return        returns a new Cairo sufrace */
static cairo_surface_t* initialize_composite(CairoComposeParams *params,
                                             gint w, gint h)
{
  cairo_surface_t* surface;

  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  params->display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());
  params->context = cairo_create(surface);
  params->visual = DefaultVisual(params->display, 0);

  return surface;
}

/*! Compose a pre-rendered preview of an Openbox themed window decoration
    as a layer onto a composite Cairo surface,
    as created in initialize_composite().
    @param params     pointer to a CairoComposeParams struct
                      describing/holding the destination Cairo context
    @param appearance pointer to an RrAppearance struct, belonging to an RrTheme
                      to be used as the layer source
    @param x          the destination x coordinate, for source pixel 0,0
    @param y          the destination y coordinate, for source pixel 0,0
    @param w          the width of the source RrAppearance Pixmap
    @param h          the height of the source RrAppearance Pixmap
    @return           returns void */
static void compose_surface(CairoComposeParams *params, RrAppearance *appearance,
                            gint x, gint y, gint w, gint h)
{
    params->surface_src = cairo_xlib_surface_create(params->display,
                                                    appearance->pixmap,
                                                    params->visual,
                                                    w, h);

    cairo_set_source_surface(params->context, params->surface_src, x, y);
    cairo_paint(params->context);

    cairo_surface_destroy(params->surface_src);
}

/*! Copy composed Cairo surface onto a new GdkPixbuf;
    then destroy the source Cairo context and underlying composite surface.
    These were presumably created in initialize_composite().
    @param params  pointer to a CairoComposeParams struct
                   describeing/holding the Cairo context to be destroyed
    @param surface pointer to a cairo_surface_t
                   to be used as the source for the GdkPixbuf,
                   then destroyed
    @param w       the width of the composite image
    @param h       the height of the composite image
    @return        returns a new GdkPixbuf */
static GdkPixbuf* finalize_composition(CairoComposeParams *params,
                                       cairo_surface_t *surface,
                                       gint w, gint h)
{
  GdkPixbuf *pixbuf;

  pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, w, h);

  cairo_surface_destroy(surface);
  cairo_destroy(params->context);

  return pixbuf;
}

/* XXX: Make this more general */
static GdkPixbuf* preview_menu(RrTheme *theme)
{
    RrAppearance *title;
    RrAppearance *title_text;

    RrAppearance *menu;
    RrAppearance *background;

    RrAppearance *normal;
    RrAppearance *disabled;
    RrAppearance *selected;
    RrAppearance *bullet; /* for submenu */
#if GTK_CHECK_VERSION(3, 0, 0)
    cairo_surface_t *surface; /* compositing destination, source for pixbuf */
    CairoComposeParams compose_params; /* composite surface context */
#else
    GdkPixmap *pixmap;
#endif
    GdkPixbuf *pixbuf;

    /* width and height of the whole menu */
    gint width, height;
    gint x, y;
    gint title_h;
    gint tw, th;
    gint bw, bh;
    gint unused;

    /* set up appearances */
    title = theme->a_menu_title;

    title_text = theme->a_menu_text_title;
    title_text->surface.parent = title;
    title_text->texture[0].data.text.string = "Menu";

    normal = theme->a_menu_text_normal;
    normal->texture[0].data.text.string = "Normal";

    disabled = theme->a_menu_text_disabled;
    disabled->texture[0].data.text.string = "Disabled";

    selected = theme->a_menu_text_selected;
    selected->texture[0].data.text.string = "Selected";

    bullet = theme->a_menu_bullet_normal;

    /* determine window size */
    RrMinSize(normal, &width, &th);
    width += th + PADDING; /* make space for the bullet */
    //height = th;

    width += 2*theme->mbwidth + 2*PADDING;

    /* get minimum title size */
    RrMinSize(title, &tw, &title_h);

    /* size of background behind each text line */
    bw = width - 2*theme->mbwidth;
    //title_h += 2*PADDING;
    title_h = theme->menu_title_height;

    RrMinSize(normal, &unused, &th);
    bh = th + 2*PADDING;

    height = title_h + 3*bh + 3*theme->mbwidth;

    //height += 3*th + 3*theme->mbwidth + 5*PADDING;

#if GTK_CHECK_VERSION(3, 0, 0)
    /* initialize cairo context and composite destination surface */
    surface = initialize_composite(&compose_params, width, height);
#endif

    /* set border */
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
    gdk_pixbuf_fill(pixbuf, rr_color_pixel(theme->menu_border_color));

    /* draw title */
    x = y = theme->mbwidth;
    theme_pixmap_paint(title, bw, title_h);

    /* draw title text */
    title_text->surface.parentx = 0;
    title_text->surface.parenty = 0;

    theme_pixmap_paint(title_text, bw, title_h);
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, title_text, x, y, bw, title_h);
#else
    pixmap = gdk_pixmap_foreign_new(title_text->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, x, y, bw, title_h);
#endif

    /* menu appears after title */
    y += theme->mbwidth + title_h;

    /* fill in menu appearance, used as the parent to every menu item's bg */
    menu = theme->a_menu;
    th = height - 3*theme->mbwidth - title_h;
    theme_pixmap_paint(menu, bw, th);
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, menu, x, y, bw, th);
#else
    pixmap = gdk_pixmap_foreign_new(menu->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, x, y, bw, th);
#endif

    /* fill in background appearance, used as the parent to text items */
    background = theme->a_menu_normal;
    background->surface.parent = menu;
    background->surface.parentx = 0;
    background->surface.parenty = 0;

    /* draw background for normal entry */
    theme_pixmap_paint(background, bw, bh);
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, background, x, y, bw, bh);
#else
    pixmap = gdk_pixmap_foreign_new(background->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, x, y, bw, bh);
#endif

    /* draw normal entry */
    normal->surface.parent = background;
    normal->surface.parentx = PADDING;
    normal->surface.parenty = PADDING;
    x += PADDING;
    y += PADDING;
    RrMinSize(normal, &tw, &th);
    theme_pixmap_paint(normal, tw, th);
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, normal, x, y, tw, th);
#else
    pixmap = gdk_pixmap_foreign_new(normal->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, x, y, tw, th);
#endif

    /* draw bullet */
    RrMinSize(normal, &tw, &th);
    bullet->surface.parent = background;
    bullet->surface.parentx = bw - th;
    bullet->surface.parenty = PADDING;
    theme_pixmap_paint(bullet, th, th);
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, bullet, width - theme->mbwidth - th, y, th, th);
#else
    pixmap = gdk_pixmap_foreign_new(bullet->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, width - theme->mbwidth - th, y,
                                          th, th);
#endif

    /* draw background for disabled entry */
    y += th + 2*PADDING;
    background->surface.parenty = bh;
    theme_pixmap_paint(background, bw, bh);
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, background,
                    x - PADDING, y - PADDING, bw, bh);
#else
    pixmap = gdk_pixmap_foreign_new(background->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, x - PADDING, y - PADDING,
                                          bw, bh);
#endif

    /* draw disabled entry */
    RrMinSize(disabled, &tw, &th);
    disabled->surface.parent = background;
    disabled->surface.parentx = PADDING;
    disabled->surface.parenty = PADDING;
    theme_pixmap_paint(disabled, tw, th);
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, disabled, x, y, tw, th);
#else
    pixmap = gdk_pixmap_foreign_new(disabled->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, x, y, tw, th);
#endif

    /* draw background for selected entry */
    y += th + 2*PADDING;
    background = theme->a_menu_selected;
    background->surface.parent = menu;
    background->surface.parentx = 2*bh;
    theme_pixmap_paint(background, bw, bh);
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, background,
                    x - PADDING, y - PADDING, bw, bh);
#else
    pixmap = gdk_pixmap_foreign_new(background->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, x - PADDING, y - PADDING,
                                          bw, bh);
#endif

    /* draw selected entry */
    RrMinSize(selected, &tw, &th);
    selected->surface.parent = background;
    selected->surface.parentx = PADDING;
    selected->surface.parenty = PADDING;
    theme_pixmap_paint(selected, tw, th);
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, selected, x, y, tw, th);
#else
    pixmap = gdk_pixmap_foreign_new(selected->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, x, y, tw, th);
#endif

#if GTK_CHECK_VERSION(3, 0, 0)
    // copy composed cairo surface to GDK pixbuf
    pixbuf = finalize_composition(&compose_params, surface, width, height);
#else
    ; // the GTK2 paths have composed onto the GDK pixbuf directly
#endif

    return pixbuf;
}

static GdkPixbuf* preview_window(RrTheme *theme, const gchar *titlelayout,
                                 gboolean focus, gint width, gint height)
{
    RrAppearance *title;
    RrAppearance *handle;
    RrAppearance *a;
#if GTK_CHECK_VERSION(3, 0, 0)
    cairo_surface_t *surface; /* compositing destination, source for pixbuf */
    CairoComposeParams compose_params; /* composite surface context */
#else
    GdkPixmap *pixmap;
#endif
    GdkPixbuf *pixbuf = NULL;
    GdkPixbuf *scratch;

    gint w, label_w, h, x, y;

    const gchar *layout;

#if GTK_CHECK_VERSION(3, 0, 0)
    /* initialize cairo context and composite destination surface */
    surface = initialize_composite(&compose_params, width, height);
#endif

    title = focus ? theme->a_focused_title : theme->a_unfocused_title;

    /* set border */
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
    gdk_pixbuf_fill(pixbuf,
                    rr_color_pixel(focus ?
                                   theme->frame_focused_border_color :
                                   theme->frame_unfocused_border_color));

    /* title */
    w = width - 2*theme->fbwidth;
    h = theme->title_height;
    theme_pixmap_paint(title, w, h);

    x = y = theme->fbwidth;
#if GTK_CHECK_VERSION(3, 0, 0)
    compose_surface(&compose_params, title, x, y, w, h);
#else
    pixmap = gdk_pixmap_foreign_new(title->pixmap);
    pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                          gdk_colormap_get_system(),
                                          0, 0, x, y, w, h);
#endif

    /* calculate label width */
    label_w = width - (theme->paddingx + theme->fbwidth + 1) * 2;

    for (layout = titlelayout; *layout; layout++) {
        switch (*layout) {
        case 'N':
            label_w -= theme->button_size + 2 + theme->paddingx + 1;
            break;
        case 'D':
        case 'S':
        case 'I':
        case 'M':
        case 'C':
            label_w -= theme->button_size + theme->paddingx + 1;
            break;
        default:
            break;
        }
    }

    x = theme->paddingx + theme->fbwidth + 1;
    y += theme->paddingy;
    for (layout = titlelayout; *layout; layout++) {
        if (*layout == 'N') {
            /* icon */
            a = theme->a_icon;
            a->texture[0].type = RR_TEXTURE_RGBA;
            a->texture[0].data.rgba.width = 48;
            a->texture[0].data.rgba.height = 48;
            a->texture[0].data.rgba.alpha = 0xff;
            a->texture[0].data.rgba.data = theme->def_win_icon;

            a->surface.parent = title;
            a->surface.parentx = x - theme->fbwidth;
            a->surface.parenty = theme->paddingy;

            w = h = theme->button_size + 2;

            theme_pixmap_paint(a, w, h);
#if GTK_CHECK_VERSION(3, 0, 0)
            compose_surface(&compose_params, a, x, y, w, h);
#else
            pixmap = gdk_pixmap_foreign_new(a->pixmap);
            pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                                  gdk_colormap_get_system(),
                                                  0, 0, x, y, w, h);
#endif

            x += theme->button_size + 2 + theme->paddingx + 1;
        } else if (*layout == 'L') {
            /* label */
            a = focus ? theme->a_focused_label : theme->a_unfocused_label;
            a->texture[0].data.text.string = focus ? "Active" : "Inactive";

            a->surface.parent = title;
            a->surface.parentx = x - theme->fbwidth;
            a->surface.parenty = theme->paddingy;
            w = label_w;
            h = theme->label_height;

            theme_pixmap_paint(a, w, h);
#if GTK_CHECK_VERSION(3, 0, 0)
            compose_surface(&compose_params, a, x, y, w, h);
#else
            pixmap = gdk_pixmap_foreign_new(a->pixmap);
            pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                                  gdk_colormap_get_system(),
                                                  0, 0, x, y, w, h);
#endif

            x += w + theme->paddingx + 1;
        } else {
            /* buttons */
            switch (*layout) {
            case 'D':
                a = focus ?
#if RR_CHECK_VERSION(3, 5, 28)
                    theme->btn_desk->a_focused_unpressed :
                    theme->btn_desk->a_unfocused_unpressed;
#else
                    theme->a_focused_unpressed_desk :
                    theme->a_unfocused_unpressed_desk;
#endif
                break;
            case 'S':
                a = focus ?
#if RR_CHECK_VERSION(3, 5, 28)
                    theme->btn_shade->a_focused_unpressed :
                    theme->btn_shade->a_unfocused_unpressed;
#else
                    theme->a_focused_unpressed_shade :
                    theme->a_unfocused_unpressed_shade;
#endif
                break;
            case 'I':
                a = focus ?
#if RR_CHECK_VERSION(3, 5, 28)
                    theme->btn_iconify->a_focused_unpressed :
                    theme->btn_iconify->a_unfocused_unpressed;
#else
                    theme->a_focused_unpressed_iconify :
                    theme->a_unfocused_unpressed_iconify;
#endif
                break;
            case 'M':
                a = focus ?
#if RR_CHECK_VERSION(3, 5, 28)
                    theme->btn_max->a_focused_unpressed :
                    theme->btn_max->a_unfocused_unpressed;
#else
                    theme->a_focused_unpressed_max :
                    theme->a_unfocused_unpressed_max;
#endif
                break;
            case 'C':
                a = focus ?
#if RR_CHECK_VERSION(3, 5, 28)
                    theme->btn_close->a_focused_unpressed :
                    theme->btn_close->a_unfocused_unpressed;
#else
                    theme->a_focused_unpressed_close :
                    theme->a_unfocused_unpressed_close;
#endif
                break;
            default:
                continue;
            }

            a->surface.parent = title;
            a->surface.parentx = x - theme->fbwidth;
            a->surface.parenty = theme->paddingy + 1;

            w = theme->button_size;
            h = theme->button_size;

            theme_pixmap_paint(a, w, h);
#if GTK_CHECK_VERSION(3, 0, 0)
            compose_surface(&compose_params, a, x, y + 1, w, h);
#else
            pixmap = gdk_pixmap_foreign_new(a->pixmap);
            /* use y + 1 so that these buttons are centered wrt the label */
            pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                                  gdk_colormap_get_system(),
                                                  0, 0, x, y + 1, w, h);
#endif

            x += theme->button_size + theme->paddingx + 1;
        }
    }

    if (theme->handle_height) {
        /* handle */
        handle = focus ? theme->a_focused_handle : theme->a_unfocused_handle;
        x = 2*theme->fbwidth + theme->grip_width;
        y = height - theme->fbwidth - theme->handle_height;
        w = width - 4*theme->fbwidth - 2*theme->grip_width;
        h = theme->handle_height;

        theme_pixmap_paint(handle, w, h);
#if GTK_CHECK_VERSION(3, 0, 0)
        compose_surface(&compose_params, handle, x, y, w, h);
#else
        pixmap = gdk_pixmap_foreign_new(handle->pixmap);
        pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                              gdk_colormap_get_system(),
                                              0, 0, x, y, w, h);
#endif

        /* openbox handles this drawing stuff differently (it fills the bottom
         * of the window with the handle), so it avoids this bug where
         * parentrelative grips are not fully filled. i'm avoiding it slightly
         * differently. */

        theme_pixmap_paint(handle, width, h);

        /* grips */
        a = focus ? theme->a_focused_grip : theme->a_unfocused_grip;
        a->surface.parent = handle;

        /* same y and h as the preceding 'handle' */
        x = theme->fbwidth;
        w = theme->grip_width;

        theme_pixmap_paint(a, w, h);
#if GTK_CHECK_VERSION(3, 0, 0)
        compose_surface(&compose_params, a, x, y, w, h);
#else
        pixmap = gdk_pixmap_foreign_new(a->pixmap);
        pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                              gdk_colormap_get_system(),
                                              0, 0, x, y, w, h);
#endif

        /* right grip */
        x = width - theme->fbwidth - theme->grip_width;
#if GTK_CHECK_VERSION(3, 0, 0)
        compose_surface(&compose_params, a, x, y, w, h);
#else
        pixbuf = gdk_pixbuf_get_from_drawable(pixbuf, pixmap,
                                              gdk_colormap_get_system(),
                                              0, 0, x, y, w, h);
#endif
    }

#if GTK_CHECK_VERSION(3, 0, 0)
    // copy composed cairo surface into GDK pixbuf
    pixbuf = finalize_composition(&compose_params, surface, width, height);
#else
    ; // the GTK2 paths have composed onto the GDK pixbuf directly
#endif

    /* title separator colour */
    x = theme->fbwidth;
    y = theme->fbwidth + theme->title_height;
    w = width - 2*theme->fbwidth;
    h = theme->fbwidth;

    if (h > 0)
    {
        scratch = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, w, h);
        gdk_pixbuf_fill(scratch, rr_color_pixel(focus ?
                                                theme->title_separator_focused_color :
                                                theme->title_separator_unfocused_color));

        gdk_pixbuf_copy_area(scratch, 0, 0, w, h, pixbuf, x, y);
    }

    /* retarded way of adding client colour */
    x = theme->fbwidth;
    y = theme->title_height + 2*theme->fbwidth;
    w = width - 2*theme->fbwidth;
    h = height - theme->title_height - 3*theme->fbwidth -
        (theme->handle_height ? (theme->fbwidth + theme->handle_height) : 0);

    scratch = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, w, h);
    gdk_pixbuf_fill(scratch, rr_color_pixel(focus ?
                                            theme->cb_focused_color :
                                            theme->cb_unfocused_color));

    gdk_pixbuf_copy_area(scratch, 0, 0, w, h, pixbuf, x, y);

    /* clear (no alpha!) the area where the client resides */
    gdk_pixbuf_fill(scratch, 0xffffffff);
    gdk_pixbuf_copy_area(scratch, 0, 0,
                         w - 2*theme->cbwidthx,
                         h - 2*theme->cbwidthy,
                         pixbuf,
                         x + theme->cbwidthx,
                         y + theme->cbwidthy);

    return pixbuf;
}

static gint theme_label_width(RrTheme *theme, gboolean active)
{
    RrAppearance *label;

    if (active) {
        label = theme->a_focused_label;
        label->texture[0].data.text.string = "Active";
    } else {
        label = theme->a_unfocused_label;
        label->texture[0].data.text.string = "Inactive";
    }

    return RrMinWidth(label);
}

static gint theme_window_min_width(RrTheme *theme, const gchar *titlelayout)
{
    gint numbuttons = strlen(titlelayout);
    gint w =  2 * theme->fbwidth + (numbuttons + 3) * (theme->paddingx + 1);

    if (g_strrstr(titlelayout, "L")) {
        numbuttons--;
        w += MAX(theme_label_width(theme, TRUE),
                 theme_label_width(theme, FALSE));
    }

    w += theme->button_size * numbuttons;

    return w;
}

GdkPixbuf *preview_theme(const gchar *name, const gchar *titlelayout,
                         RrFont *active_window_font,
                         RrFont *inactive_window_font,
                         RrFont *menu_title_font,
                         RrFont *menu_item_font,
                         RrFont *osd_active_font,
                         RrFont *osd_inactive_font)
{
    GdkPixbuf *preview;
    GdkPixbuf *menu;
    GdkPixbuf *window;

    gint window_w;
    gint menu_w;

    gint w, h;

    RrTheme *theme = RrThemeNew(rrinst, name, FALSE,
                                active_window_font, inactive_window_font,
                                menu_title_font, menu_item_font,
                                osd_active_font, osd_inactive_font);
    if (!theme)
        return NULL;

    menu = preview_menu(theme);

    window_w = theme_window_min_width(theme, titlelayout);

    menu_w = gdk_pixbuf_get_width(menu);
    h = gdk_pixbuf_get_height(menu);
    w = MAX(window_w, menu_w) + 20;

    /* we don't want windows disappearing on us */
    if (!window_w) window_w = menu_w;

    preview = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8,
                             w, h + 2*(theme->title_height +5) + 1);
    gdk_pixbuf_fill(preview, 0); /* clear */

    window = preview_window(theme, titlelayout, FALSE, window_w, h);
    gdk_pixbuf_copy_area(window, 0, 0, window_w, h, preview, 20, 0);

    window = preview_window(theme, titlelayout, TRUE, window_w, h);
    gdk_pixbuf_copy_area(window, 0, 0, window_w, h,
                         preview, 10, theme->title_height + 5);

    gdk_pixbuf_copy_area(menu, 0, 0, menu_w, h,
                         preview, 0, 2 * (theme->title_height + 5));

    RrThemeFree(theme);

    return preview;
}
