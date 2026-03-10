/*
 * pear-notch — notch în stil Apple, always on top, fără taskbar, fără Alt+Tab
 * Necesită X11 (compositor pentru transparență). Folosește notch.svg.
 */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cairo/cairo-xlib.h>
#include <cairo/cairo.h>
#include <librsvg/rsvg.h>
#include <cstdlib>
#include <csignal>
#include <cstring>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <sys/time.h>
#include <pwd.h>
#include <cmath>
#include <cerrno>
#include <atomic>
#include <thread>
#include <csignal>
#include <ctime>
#include <pulse/pulseaudio.h>
#include <glib.h>
#include <gio/gio.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <mutex>
#include <string>

namespace {

// Dimensiuni normale (hover = 1000ms pentru mărire)
const int NOTCH_WIDTH   = 170;
const int NOTCH_HEIGHT  =  30;

const int NOTCH_W_BIG   = 510;
const int NOTCH_H_BIG   =  90;

const int HOVER_MS      = 300;
const double ANIM_SPEED =  0.22;

// Music visualizer: 3 bare verticale pill, la dreapta notch-ului
const int VISUALIZER_WIDTH = 37;  // 4*BAR_WIDTH + 3*BAR_GAP + 2*BAR_MARGIN
const int BAR_WIDTH       =  3;
const int BAR_GAP         =  3;
const int BAR_MARGIN      =  4;
const int VISUALIZER_SHIFT_LEFT = 10;
const int ART_SIZE = 18;
const int ART_OFFSET_RIGHT = 13;
const int ART_OFFSET_RIGHT_HOVER = 12;   // la hover, album mai în dreapta (px extra)
const double ART_SCALE_HOVER = 1.15;      // la hover, album puțin mai mare (factor)
const int VISUALIZER_EXTRA_LEFT_HOVER = 22;  // la hover, bare mai în stânga (px)
const double ART_CORNER_RADIUS = 4.0;
const double BAR_PHASE[4] = { 0, 1.57, 3.14, 4.71 };
// Layout hover: titlu, artist, bară progres, butoane
const int PROGRESS_BAR_W = 200;
const int PROGRESS_BAR_H = 5;
const int CTRL_BUTTON_W = 36;
const int CTRL_BUTTON_H = 20;
const int CTRL_BUTTON_GAP = 8;
const int TITLE_FONT_SIZE = 11;
const int ARTIST_FONT_SIZE = 9;
const int MAX_TITLE_PX = 240;
const int MAX_ARTIST_PX = 220;
const double BAR_COLOR[4][3] = {
    { 0.75, 0.75, 0.75 },  // gri deschis
    { 0.85, 0.85, 0.85 },
    { 0.70, 0.70, 0.70 },
    { 0.80, 0.80, 0.80 },
};

static Display* g_dpy = nullptr;
static Window   g_win = 0;
static RsvgHandle* g_svg = nullptr;
static std::atomic<bool> g_running{true};
static std::atomic<bool> g_audio_playing{false};
static double g_visualizer_alpha = 0.0;
const double VISUALIZER_FADE_SPEED = 0.06;

static cairo_surface_t* g_art_surface = nullptr;
static int g_art_w = 0, g_art_h = 0;
static std::mutex g_art_mutex;
static std::string g_last_art_url;

// MPRIS metadata pentru hover (titlu, artist, bară progres, controale)
static std::string g_title;
static std::string g_artist;
static int64_t g_length_us = 0;
static int64_t g_position_us = 0;
static std::string g_playback_status;  // "Playing", "Paused", "Stopped"
static std::string g_mpris_name;       // ex: org.mpris.MediaPlayer2.spotify
static std::mutex g_meta_mutex;
static GDBusConnection* g_dbus_conn = nullptr;  // folosit în main pentru Previous/PlayPause/Next

static Cursor g_invisible_cursor = 0;
static bool   g_cursor_hidden = false;

static RsvgHandle* g_svg_prev  = nullptr;
static RsvgHandle* g_svg_next  = nullptr;
static RsvgHandle* g_svg_play  = nullptr;
static RsvgHandle* g_svg_pause = nullptr;
static RsvgHandle* g_svg_settings = nullptr;

static Window g_settings_win = 0;
static Atom   g_wm_delete_settings = 0;
static bool   g_settings_visible = false;
static bool   g_run_at_startup = false;
static std::string g_autostart_path;
static std::string g_autostart_backup_path;

// forward declarations pentru setări
static void paint_settings_window(Display* dpy);

void signal_handler(int) { g_running = false; }

static cairo_surface_t* pixbuf_to_cairo_surface(GdkPixbuf* pixbuf) {
    if (!pixbuf) return nullptr;
    int w = gdk_pixbuf_get_width(pixbuf);
    int h = gdk_pixbuf_get_height(pixbuf);
    gboolean has_alpha = gdk_pixbuf_get_has_alpha(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    const guint8* src = gdk_pixbuf_get_pixels(pixbuf);
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    if (!surface) return nullptr;
    unsigned char* dst = cairo_image_surface_get_data(surface);
    int dst_stride = cairo_image_surface_get_stride(surface);
    for (int y = 0; y < h; y++) {
        const guint8* p = src + y * rowstride;
        uint32_t* q = reinterpret_cast<uint32_t*>(dst + y * dst_stride);
        for (int x = 0; x < w; x++) {
            guint8 r = p[0], g = p[1], b = p[2], a = has_alpha && n_channels >= 4 ? p[3] : 255;
            *q++ = (static_cast<uint32_t>(a) << 24) | (r << 16) | (g << 8) | b;
            p += n_channels;
        }
    }
    cairo_surface_mark_dirty(surface);
    return surface;
}

static void mpris_album_art_thread_func() {
    GError* err = nullptr;
    GDBusConnection* conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
    if (!conn) { if (err) g_error_free(err); return; }
    while (g_running.load()) {
        do {
        err = nullptr;
        GVariant* list =         g_dbus_connection_call_sync(conn, "org.freedesktop.DBus", "/", "org.freedesktop.DBus",
            "ListNames", nullptr, G_VARIANT_TYPE("(as)"), G_DBUS_CALL_FLAGS_NONE, 2000, nullptr, &err);
        if (!list) { if (err) g_error_free(err); break; }
        GVariant* names_var = g_variant_get_child_value(list, 0);
        g_variant_unref(list);
        std::string mpris_name_storage;
        GVariantIter iter;
        g_variant_iter_init(&iter, names_var);
        const char* name;
        while (g_variant_iter_next(&iter, "&s", &name))
            if (g_str_has_prefix(name, "org.mpris.MediaPlayer2.")) { mpris_name_storage = name; break; }
        g_variant_unref(names_var);
        if (mpris_name_storage.empty()) {
            std::lock_guard<std::mutex> lock(g_meta_mutex);
            g_title.clear();
            g_artist.clear();
            g_length_us = 0;
            g_position_us = 0;
            g_playback_status.clear();
            g_mpris_name.clear();
            break;
        }
        GVariant* meta = nullptr;
        err = nullptr;
        meta = g_dbus_connection_call_sync(conn, mpris_name_storage.c_str(), "/org/mpris/MediaPlayer2",
            "org.freedesktop.DBus.Properties", "Get",
            g_variant_new("(ss)", "org.mpris.MediaPlayer2.Player", "Metadata"),
            G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, 2000, nullptr, &err);
        if (!meta) { if (err) g_error_free(err); break; }
        GVariant* inner = g_variant_get_child_value(meta, 0);
        g_variant_unref(meta);
        GVariant* dict = g_variant_get_variant(inner);
        g_variant_unref(inner);
        GVariant* art_var = g_variant_lookup_value(dict, "mpris:artUrl", G_VARIANT_TYPE_STRING);
        GVariant* title_var = g_variant_lookup_value(dict, "xesam:title", G_VARIANT_TYPE_STRING);
        GVariant* artist_var = g_variant_lookup_value(dict, "xesam:artist", G_VARIANT_TYPE("as"));
        GVariant* length_var = g_variant_lookup_value(dict, "mpris:length", G_VARIANT_TYPE_INT64);
        g_variant_unref(dict);

        char* art_url = art_var ? g_variant_dup_string(art_var, nullptr) : nullptr;
        if (art_var) g_variant_unref(art_var);

        std::string title_str, artist_str;
        int64_t length_us = 0;
        if (title_var) {
            char* t = g_variant_dup_string(title_var, nullptr);
            if (t) { title_str = t; g_free(t); }
            g_variant_unref(title_var);
        }
        if (artist_var && g_variant_n_children(artist_var) > 0) {
            GVariant* first = g_variant_get_child_value(artist_var, 0);
            char* a = g_variant_dup_string(first, nullptr);
            if (a) { artist_str = a; g_free(a); }
            g_variant_unref(first);
            g_variant_unref(artist_var);
        } else if (artist_var) g_variant_unref(artist_var);
        if (length_var) {
            length_us = g_variant_get_int64(length_var);
            g_variant_unref(length_var);
        }

        int64_t position_us = 0;
        std::string playback_status;
        err = nullptr;
        GVariant* pos_v = g_dbus_connection_call_sync(conn, mpris_name_storage.c_str(), "/org/mpris/MediaPlayer2",
            "org.freedesktop.DBus.Properties", "Get",
            g_variant_new("(ss)", "org.mpris.MediaPlayer2.Player", "Position"),
            G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, 1000, nullptr, &err);
        if (pos_v) {
            GVariant* inner_pos = g_variant_get_child_value(pos_v, 0);
            g_variant_unref(pos_v);
            GVariant* val = g_variant_get_variant(inner_pos);
            g_variant_unref(inner_pos);
            if (val && g_variant_classify(val) == G_VARIANT_CLASS_INT64)
                position_us = g_variant_get_int64(val);
            if (val) g_variant_unref(val);
        }
        if (err) { g_error_free(err); err = nullptr; }
        GVariant* status_v = g_dbus_connection_call_sync(conn, mpris_name_storage.c_str(), "/org/mpris/MediaPlayer2",
            "org.freedesktop.DBus.Properties", "Get",
            g_variant_new("(ss)", "org.mpris.MediaPlayer2.Player", "PlaybackStatus"),
            G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, 1000, nullptr, &err);
        if (status_v) {
            GVariant* inner_st = g_variant_get_child_value(status_v, 0);
            g_variant_unref(status_v);
            GVariant* val = g_variant_get_variant(inner_st);
            g_variant_unref(inner_st);
            if (val && g_variant_classify(val) == G_VARIANT_CLASS_STRING) {
                char* s = g_variant_dup_string(val, nullptr);
                if (s) { playback_status = s; g_free(s); }
            }
            if (val) g_variant_unref(val);
        }
        if (err) { g_error_free(err); err = nullptr; }

        {
            std::lock_guard<std::mutex> lock(g_meta_mutex);
            g_title = title_str;
            g_artist = artist_str;
            g_length_us = length_us;
            g_position_us = position_us;
            g_playback_status = playback_status;
            g_mpris_name = mpris_name_storage;
        }
        if (art_url && art_url[0]) {
            if (g_last_art_url != art_url) {
                g_last_art_url = art_url;
                gchar* path = nullptr;
                if (g_str_has_prefix(art_url, "file://"))
                    path = g_filename_from_uri(art_url, nullptr, &err);
                if (path) {
                    GdkPixbuf* pix = gdk_pixbuf_new_from_file_at_size(path, ART_SIZE, ART_SIZE, &err);
                    g_free(path);
                    if (pix) {
                        int pw = gdk_pixbuf_get_width(pix), ph = gdk_pixbuf_get_height(pix);
                        GdkPixbuf* square_pix = gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, ART_SIZE, ART_SIZE);
                        if (square_pix && pw > 0 && ph > 0) {
                            gdk_pixbuf_fill(square_pix, 0);
                            double scale = std::min(static_cast<double>(ART_SIZE) / pw, static_cast<double>(ART_SIZE) / ph);
                            int draw_w = static_cast<int>(pw * scale), draw_h = static_cast<int>(ph * scale);
                            int dest_x = (ART_SIZE - draw_w) / 2, dest_y = (ART_SIZE - draw_h) / 2;
                            gdk_pixbuf_scale(pix, square_pix, dest_x, dest_y, draw_w, draw_h, 0, 0, scale, scale, GDK_INTERP_BILINEAR);
                        }
                        g_object_unref(pix);
                        cairo_surface_t* new_surf = square_pix ? pixbuf_to_cairo_surface(square_pix) : nullptr;
                        if (square_pix) g_object_unref(square_pix);
                        if (new_surf) {
                            std::lock_guard<std::mutex> lock(g_art_mutex);
                            if (g_art_surface) cairo_surface_destroy(g_art_surface);
                            g_art_surface = new_surf;
                            g_art_w = cairo_image_surface_get_width(g_art_surface);
                            g_art_h = cairo_image_surface_get_height(g_art_surface);
                        }
                    }
                }
            }
        } else {
            g_last_art_url.clear();
            std::lock_guard<std::mutex> lock(g_art_mutex);
            if (g_art_surface) {
                cairo_surface_destroy(g_art_surface);
                g_art_surface = nullptr;
                g_art_w = g_art_h = 0;
            }
        }
        g_free(art_url);
        } while (0);
        for (int i = 0; i < 50 && g_running.load(); i++) {
            usleep(100000);
            if (i % 5 == 4) {
                std::string name;
                { std::lock_guard<std::mutex> lock(g_meta_mutex); name = g_mpris_name; }
                if (name.empty()) continue;
                GError* err2 = nullptr;
                GVariant* pos_v = g_dbus_connection_call_sync(conn, name.c_str(), "/org/mpris/MediaPlayer2",
                    "org.freedesktop.DBus.Properties", "Get",
                    g_variant_new("(ss)", "org.mpris.MediaPlayer2.Player", "Position"),
                    G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE, 500, nullptr, &err2);
                if (pos_v) {
                    GVariant* inner_pos = g_variant_get_child_value(pos_v, 0);
                    g_variant_unref(pos_v);
                    GVariant* val = g_variant_get_variant(inner_pos);
                    g_variant_unref(inner_pos);
                    int64_t pos = 0;
                    if (val && g_variant_classify(val) == G_VARIANT_CLASS_INT64)
                        pos = g_variant_get_int64(val);
                    if (val) g_variant_unref(val);
                    std::lock_guard<std::mutex> lock(g_meta_mutex);
                    g_position_us = pos;
                }
                if (err2) { g_error_free(err2); err2 = nullptr; }
            }
        }
    }
    g_object_unref(conn);
}

static void pa_sink_input_info_cb(pa_context* c, const pa_sink_input_info* info, int eol, void*) {
    static bool any_playing = false;
    if (eol == 0 && info != nullptr) {
        if (!info->corked)
            any_playing = true;
    } else {
        g_audio_playing = any_playing;
        any_playing = false;
    }
}

static void pa_subscribe_cb(pa_context* c, pa_subscription_event_type_t, uint32_t, void*) {
    pa_operation* op = pa_context_get_sink_input_info_list(c, pa_sink_input_info_cb, nullptr);
    if (op)
        pa_operation_unref(op);
}

static void pa_context_state_cb(pa_context* c, void*) {
    if (pa_context_get_state(c) != PA_CONTEXT_READY)
        return;
    pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK_INPUT, nullptr, nullptr);
    pa_operation* op = pa_context_get_sink_input_info_list(c, pa_sink_input_info_cb, nullptr);
    if (op)
        pa_operation_unref(op);
}

static void pulse_audio_thread_func() {
    pa_mainloop* ml = pa_mainloop_new();
    if (!ml) return;
    pa_mainloop_api* api = pa_mainloop_get_api(ml);
    pa_context* ctx = pa_context_new(api, "pear-notch");
    if (!ctx) { pa_mainloop_free(ml); return; }
    pa_context_set_state_callback(ctx, pa_context_state_cb, nullptr);
    pa_context_set_subscribe_callback(ctx, pa_subscribe_cb, nullptr);
    if (pa_context_connect(ctx, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
        pa_context_unref(ctx);
        pa_mainloop_free(ml);
        return;
    }
    while (g_running)
        pa_mainloop_iterate(ml, 100000, nullptr);
    pa_context_disconnect(ctx);
    pa_context_unref(ctx);
    pa_mainloop_free(ml);
}

static long time_ms() {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<long>(tv.tv_sec) * 1000 + static_cast<long>(tv.tv_usec) / 1000;
}

// Caută un visual 32-bit pentru transparență (compositor)
bool find_argb_visual(Display* dpy, int screen, XVisualInfo* out_vinfo) {
    XVisualInfo templ;
    templ.screen = screen;
    templ.depth = 32;
    templ.c_class = TrueColor;
    int n;
    XVisualInfo* list = XGetVisualInfo(dpy, VisualScreenMask | VisualDepthMask | VisualClassMask, &templ, &n);
    if (!list || n == 0) return false;
    *out_vinfo = list[0];
    XFree(list);
    return true;
}

Cursor create_invisible_cursor(Display* dpy, Window win) {
    static char no_data[] = { 0,0,0,0,0,0,0,0 };
    XColor black;
    std::memset(&black, 0, sizeof(black));
    Pixmap bm_no = XCreateBitmapFromData(dpy, win, no_data, 8, 8);
    if (!bm_no) return 0;
    Cursor cursor = XCreatePixmapCursor(dpy, bm_no, bm_no, &black, &black, 0, 0);
    XFreePixmap(dpy, bm_no);
    return cursor;
}

static void init_autostart_paths() {
    if (!g_autostart_path.empty()) return;
    const char* home = std::getenv("HOME");
    if (!home || !*home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw && pw->pw_dir)
            home = pw->pw_dir;
    }
    if (!home) return;
    std::string base(home);
    g_autostart_path = base + "/.config/autostart/pearos-notch.desktop";
    g_autostart_backup_path = g_autostart_path + ".1";
    g_run_at_startup = (access(g_autostart_path.c_str(), F_OK) == 0);
}

static void set_run_at_startup(bool enable) {
    init_autostart_paths();
    if (g_autostart_path.empty()) return;
    if (enable) {
        if (access(g_autostart_backup_path.c_str(), F_OK) == 0 &&
            access(g_autostart_path.c_str(), F_OK) != 0) {
            rename(g_autostart_backup_path.c_str(), g_autostart_path.c_str());
        }
    } else {
        if (access(g_autostart_path.c_str(), F_OK) == 0) {
            rename(g_autostart_path.c_str(), g_autostart_backup_path.c_str());
        }
    }
    g_run_at_startup = (access(g_autostart_path.c_str(), F_OK) == 0);
}

static void open_settings_window() {
    if (!g_dpy || g_settings_win) return;
    init_autostart_paths();

    int screen = DefaultScreen(g_dpy);
    int sw = DisplayWidth(g_dpy, screen);
    int sh = DisplayHeight(g_dpy, screen);
    int w = 280;
    int h = 110;
    int x = (sw - w) / 2;
    int y = (sh - h) / 2;

    XSetWindowAttributes sa;
    sa.event_mask = ExposureMask | ButtonPressMask | StructureNotifyMask;
    sa.override_redirect = False;

    g_settings_win = XCreateWindow(
        g_dpy, RootWindow(g_dpy, screen),
        x, y, w, h,
        0,
        CopyFromParent,
        InputOutput,
        CopyFromParent,
        CWEventMask | CWOverrideRedirect,
        &sa
    );

    XStoreName(g_dpy, g_settings_win, "PearOS Notch Settings");

    // tratează corect închiderea ferestrei de către WM
    if (!g_wm_delete_settings)
        g_wm_delete_settings = XInternAtom(g_dpy, "WM_DELETE_WINDOW", False);
    if (g_wm_delete_settings)
        XSetWMProtocols(g_dpy, g_settings_win, &g_wm_delete_settings, 1);

    XMapWindow(g_dpy, g_settings_win);
    XFlush(g_dpy);

    g_settings_visible = true;
    paint_settings_window(g_dpy);
}

static void close_settings_window() {
    if (!g_dpy || !g_settings_win) return;
    XDestroyWindow(g_dpy, g_settings_win);
    g_settings_win = 0;
    g_settings_visible = false;
}

static void paint_settings_window(Display* dpy) {
    if (!g_settings_win) return;
    XWindowAttributes xwa;
    if (!XGetWindowAttributes(dpy, g_settings_win, &xwa)) return;

    int w = xwa.width;
    int h = xwa.height;
    if (w <= 0 || h <= 0) return;

    cairo_surface_t* surf = cairo_xlib_surface_create(dpy, g_settings_win, xwa.visual, w, h);
    if (!surf) return;
    cairo_t* cr = cairo_create(surf);
    if (!cr) { cairo_surface_destroy(surf); return; }

    cairo_set_source_rgb(cr, 0.10, 0.10, 0.10);
    cairo_paint(cr);

    const char* title = "PearOS Notch Settings";
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 0.95, 0.95, 0.95);
    cairo_move_to(cr, 14, 22);
    cairo_show_text(cr, title);

    // checkbox „Run Notch at startup”
    double cb_x = 18;
    double cb_y = 38;
    double cb_size = 14;

    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, cb_x, cb_y, cb_size, cb_size);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_stroke(cr);

    if (g_run_at_startup) {
        cairo_move_to(cr, cb_x + 3, cb_y + cb_size / 2);
        cairo_line_to(cr, cb_x + cb_size / 2, cb_y + cb_size - 3);
        cairo_line_to(cr, cb_x + cb_size - 3, cb_y + 3);
        cairo_set_source_rgb(cr, 0.3, 0.9, 0.3);
        cairo_set_line_width(cr, 2.0);
        cairo_stroke(cr);
    }

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 11);
    cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
    const char* label = "Run Notch at startup";
    cairo_move_to(cr, cb_x + cb_size + 8, cb_y + cb_size - 2);
    cairo_show_text(cr, label);

    // buton „Terminate Notch” (jos dreapta)
    double btn_w = 120;
    double btn_h = 24;
    double margin = 16;
    double btn_x = w - btn_w - margin;
    double btn_y = h - btn_h - margin;

    cairo_new_path(cr);
    cairo_rectangle(cr, btn_x, btn_y, btn_w, btn_h);
    cairo_set_source_rgb(cr, 0.35, 0.10, 0.10);
    cairo_fill_preserve(cr);
    cairo_set_source_rgb(cr, 0.85, 0.35, 0.35);
    cairo_set_line_width(cr, 1.2);
    cairo_stroke(cr);

    const char* quit_label = "Terminate Notch";
    cairo_set_source_rgb(cr, 0.98, 0.98, 0.98);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 10.5);
    cairo_text_extents_t te_quit;
    cairo_text_extents(cr, quit_label, &te_quit);
    double tx = btn_x + (btn_w - te_quit.x_advance) / 2.0;
    double ty = btn_y + (btn_h + te_quit.height) / 2.0;
    cairo_move_to(cr, tx, ty);
    cairo_show_text(cr, quit_label);

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

static void draw_svg_icon_white(cairo_t* cr, RsvgHandle* handle,
                                double x, double y, double w, double h) {
    if (!handle || w <= 0 || h <= 0) return;

    double svg_w = 24, svg_h = 24;
    rsvg_handle_get_intrinsic_size_in_pixels(handle, &svg_w, &svg_h);
    if (svg_w <= 0 || svg_h <= 0) {
        svg_w = 24;
        svg_h = 24;
    }

    cairo_surface_t* mask = cairo_image_surface_create(CAIRO_FORMAT_A8,
                                                       static_cast<int>(w),
                                                       static_cast<int>(h));
    if (!mask) return;

    cairo_t* cr_mask = cairo_create(mask);
    if (!cr_mask) {
        cairo_surface_destroy(mask);
        return;
    }

    cairo_set_operator(cr_mask, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr_mask);

    cairo_set_operator(cr_mask, CAIRO_OPERATOR_OVER);
    cairo_scale(cr_mask, w / svg_w, h / svg_h);
    RsvgRectangle viewport = { 0, 0, svg_w, svg_h };
    GError* err = nullptr;
    rsvg_handle_render_document(handle, cr_mask, &viewport, &err);
    if (err) g_error_free(err);

    cairo_destroy(cr_mask);

    cairo_save(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_mask_surface(cr, mask, x, y);
    cairo_restore(cr);

    cairo_surface_destroy(mask);
}

void set_always_on_top(Display* dpy, Window win) {
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom above    = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
    Atom stays_on_top = XInternAtom(dpy, "_NET_WM_STATE_STAYS_ON_TOP", False);
    if (!wm_state || !above) return;

    XClientMessageEvent ev{};
    ev.type         = ClientMessage;
    ev.window       = win;
    ev.message_type = wm_state;
    ev.format       = 32;
    ev.data.l[0]    = 1; // _NET_WM_STATE_ADD
    ev.data.l[1]    = static_cast<long>(above);
    ev.data.l[2]    = static_cast<long>(stays_on_top);
    ev.data.l[3]    = 0;
    ev.data.l[4]    = 0;

    Window root = DefaultRootWindow(dpy);
    XSendEvent(dpy, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               reinterpret_cast<XEvent*>(&ev));
    XFlush(dpy);
}

void set_skip_taskbar_and_pager(Display* dpy, Window win) {
    Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
    Atom skip_taskbar = XInternAtom(dpy, "_NET_WM_STATE_SKIP_TASKBAR", False);
    Atom skip_pager   = XInternAtom(dpy, "_NET_WM_STATE_SKIP_PAGER", False);
    if (!wm_state || !skip_taskbar) return;

    XClientMessageEvent ev{};
    ev.type         = ClientMessage;
    ev.window       = win;
    ev.message_type = wm_state;
    ev.format       = 32;
    ev.data.l[0]    = 1;
    ev.data.l[1]    = static_cast<long>(skip_taskbar);
    ev.data.l[2]    = static_cast<long>(skip_pager);
    ev.data.l[3]    = 0;
    ev.data.l[4]    = 0;

    Window root = DefaultRootWindow(dpy);
    XSendEvent(dpy, root, False,
               SubstructureRedirectMask | SubstructureNotifyMask,
               reinterpret_cast<XEvent*>(&ev));
    XFlush(dpy);
}

// Caută notch.svg: mai întâi cwd, apoi lângă executabil, apoi parent.
bool find_notch_svg(char* out_path, size_t size) {
    if (size < 8) return false;
    if (access("notch.svg", R_OK) == 0) {
        strncpy(out_path, "notch.svg", size - 1);
        out_path[size - 1] = '\0';
        return true;
    }
    char exe[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n > 0) {
        exe[n] = '\0';
        char* dir = dirname(exe);
        snprintf(out_path, size, "%s/notch.svg", dir);
        if (access(out_path, R_OK) == 0) return true;
        snprintf(out_path, size, "%s/../notch.svg", dir);
        if (access(out_path, R_OK) == 0) return true;
    }
    return false;
}

void paint(Display* dpy, Window win, int w, int h, int notch_w, int notch_h) {
    if (w <= 0 || h <= 0) return;
    XWindowAttributes xwa;
    if (!XGetWindowAttributes(dpy, win, &xwa)) return;

    cairo_surface_t* surf = cairo_xlib_surface_create(dpy, win, xwa.visual, w, h);
    if (!surf) return;

    cairo_t* cr = cairo_create(surf);
    if (!cr) { cairo_surface_destroy(surf); return; }

    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 0, 0, 0, 0);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_save(cr);  // stare „curată” pentru a restaura înainte de controale

    if (g_svg && notch_w > 0 && notch_h > 0) {
        double svg_w = 372, svg_h = 52;
        rsvg_handle_get_intrinsic_size_in_pixels(g_svg, &svg_w, &svg_h);
        if (svg_w <= 0) svg_w = 372;
        if (svg_h <= 0) svg_h = 52;
        cairo_save(cr);
        cairo_rectangle(cr, 0, 0, notch_w, notch_h);
        cairo_clip(cr);
        cairo_scale(cr, static_cast<double>(notch_w) / svg_w, static_cast<double>(notch_h) / svg_h);
        RsvgRectangle viewport = { 0, 0, svg_w, svg_h };
        GError* err = nullptr;
        rsvg_handle_render_document(g_svg, cr, &viewport, &err);
        if (err) g_error_free(err);
        cairo_restore(cr);
    }

    {
        std::lock_guard<std::mutex> lock(g_art_mutex);
        if (g_art_surface && g_art_w > 0 && g_art_h > 0) {
            double scale_art = static_cast<double>(h) / NOTCH_HEIGHT;
            double art_display = ART_SIZE * scale_art * (scale_art > 1.5 ? ART_SCALE_HOVER : 1.0);
            double ax = static_cast<double>(BAR_MARGIN + ART_OFFSET_RIGHT);
            if (scale_art > 1.5) ax += static_cast<double>(ART_OFFSET_RIGHT_HOVER);
            double ay = (static_cast<double>(h) - art_display) / 2.0;
            double r = std::min(ART_CORNER_RADIUS * scale_art, art_display / 2.0);
            cairo_save(cr);
            cairo_new_path(cr);
            cairo_move_to(cr, ax + r, ay);
            cairo_line_to(cr, ax + art_display - r, ay);
            cairo_arc(cr, ax + art_display - r, ay + r, r, 3 * M_PI / 2, 0);
            cairo_line_to(cr, ax + art_display, ay + art_display - r);
            cairo_arc(cr, ax + art_display - r, ay + art_display - r, r, 0, M_PI / 2);
            cairo_line_to(cr, ax + r, ay + art_display);
            cairo_arc(cr, ax + r, ay + art_display - r, r, M_PI / 2, M_PI);
            cairo_line_to(cr, ax, ay + r);
            cairo_arc(cr, ax + r, ay + r, r, M_PI, 3 * M_PI / 2);
            cairo_close_path(cr);
            cairo_clip(cr);
            cairo_translate(cr, ax, ay);
            cairo_scale(cr, art_display / g_art_w, art_display / g_art_h);
            cairo_set_source_surface(cr, g_art_surface, 0, 0);
            cairo_paint(cr);
            cairo_restore(cr);
        }
    }

    if (g_visualizer_alpha > 0.001) {
    double t_sec = time_ms() / 1000.0;
    double cy_center = h / 2.0;
    double bar_amp = (h / 2.0 - 4.0) * 0.85;
    double bar_min = static_cast<double>(BAR_WIDTH);
    for (int i = 0; i < 4; i++) {
        double v = 0.5 + 0.5 * std::sin(t_sec * 3.5 + BAR_PHASE[i]);
        double bar_h = bar_min + bar_amp * v;
        double shift_left = VISUALIZER_SHIFT_LEFT;
        if (static_cast<double>(h) / NOTCH_HEIGHT > 1.5) shift_left += VISUALIZER_EXTRA_LEFT_HOVER;
        double cx = notch_w - BAR_MARGIN - shift_left - (3 - i) * (BAR_WIDTH + BAR_GAP) - BAR_WIDTH / 2.0;
        double rx = BAR_WIDTH / 2.0;
        double ry = bar_h / 2.0;
        double y_top = cy_center - ry, y_bot = cy_center + ry;
        cairo_new_path(cr);
        cairo_move_to(cr, cx - rx, y_top + rx);
        cairo_arc(cr, cx, y_top + rx, rx, M_PI, 2 * M_PI);
        cairo_line_to(cr, cx + rx, y_bot - rx);
        cairo_arc(cr, cx, y_bot - rx, rx, 0, M_PI);
        cairo_close_path(cr);
        cairo_set_source_rgba(cr, BAR_COLOR[i][0], BAR_COLOR[i][1], BAR_COLOR[i][2], g_visualizer_alpha);
        cairo_fill(cr);
    }
    }

        if (h > NOTCH_HEIGHT) {
        cairo_restore(cr);  // înapoi la stare curată (fără clip/transform/sursă de la album art)
        cairo_save(cr);
        cairo_reset_clip(cr);  // asigură că nu mai există niciun clip activ înainte de UI player
        std::string title, artist;
        int64_t length_us = 0, position_us = 0;
        bool playing = false;
        {
            std::lock_guard<std::mutex> lock(g_meta_mutex);
            title = g_title;
            artist = g_artist;
            length_us = g_length_us;
            position_us = g_position_us;
            playing = (g_playback_status == "Playing");
        }
        double cx = notch_w / 2.0;
        double title_y = h * 0.22;
        double artist_y = h * 0.38;
        double progress_y = h * 0.52;
        double button_y = h * 0.72;

        auto truncate = [](cairo_t* cr, const std::string& s, int max_px) -> std::string {
            if (s.empty()) return "";
            cairo_text_extents_t te;
            cairo_text_extents(cr, (s + "...").c_str(), &te);
            if (te.x_advance <= max_px) return s;
            std::string t = s;
            while (!t.empty()) {
                t.pop_back();
                cairo_text_extents(cr, (t + "...").c_str(), &te);
                if (te.x_advance <= max_px) return t + "...";
            }
            return "...";
        };

        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, TITLE_FONT_SIZE);
        cairo_set_source_rgba(cr, 0.95, 0.95, 0.95, 1);
        if (!title.empty()) {
            std::string show = truncate(cr, title, MAX_TITLE_PX);
            cairo_text_extents_t te;
            cairo_text_extents(cr, show.c_str(), &te);
            cairo_move_to(cr, cx - te.x_advance / 2, title_y);
            cairo_show_text(cr, show.c_str());
        }

        cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
        cairo_set_font_size(cr, ARTIST_FONT_SIZE);
        cairo_set_source_rgba(cr, 0.75, 0.75, 0.75, 1);
        if (!artist.empty()) {
            std::string show = truncate(cr, artist, MAX_ARTIST_PX);
            cairo_text_extents_t te;
            cairo_text_extents(cr, show.c_str(), &te);
            cairo_move_to(cr, cx - te.x_advance / 2, artist_y);
            cairo_show_text(cr, show.c_str());
        }

        if (playing) {
            cairo_reset_clip(cr);  // siguranță: controalele nu sunt decupate de nimic
            cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

            // fundal semi-transparent pentru zona de player (pt. contrast peste album art)
            double bg_top = artist_y - 4;
            double bg_height = (button_y + CTRL_BUTTON_H) - bg_top + 6;
            if (bg_top < 0) bg_top = 0;
            if (bg_top + bg_height > h) bg_height = h - bg_top;
            cairo_new_path(cr);
            cairo_rectangle(cr, 0, bg_top, notch_w, bg_height);
            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.55);
            cairo_fill(cr);

            double bar_x = cx - PROGRESS_BAR_W / 2.0;
            double r = PROGRESS_BAR_H / 2.0;
            cairo_new_path(cr);
            cairo_rectangle(cr, bar_x, progress_y - r, PROGRESS_BAR_W, PROGRESS_BAR_H);
            cairo_set_source_rgba(cr, 0.35, 0.35, 0.35, 0.9);
            cairo_fill(cr);

            if (length_us > 0) {
                double frac = std::min(1.0, std::max(0.0, static_cast<double>(position_us) / length_us));
                cairo_new_path(cr);
                cairo_rectangle(cr, bar_x, progress_y - r, PROGRESS_BAR_W * frac, PROGRESS_BAR_H);
                cairo_set_source_rgba(cr, 0.85, 0.85, 0.85, 1);
                cairo_fill(cr);
            }

            int total_w = 3 * CTRL_BUTTON_W + 2 * CTRL_BUTTON_GAP;
            double btn_left = cx - total_w / 2.0;
            double by = button_y;
            for (int i = 0; i < 3; i++) {
                double bx = btn_left + i * (CTRL_BUTTON_W + CTRL_BUTTON_GAP);

                // spațiu interior pentru icon (lăsăm un mic padding)
                double pad_x = 4.0;
                double pad_y = 3.0;
                double iw = CTRL_BUTTON_W - 2 * pad_x;
                double ih = CTRL_BUTTON_H - 2 * pad_y;

                RsvgHandle* icon = nullptr;
                if (i == 0)
                    icon = g_svg_prev;
                else if (i == 1)
                    icon = playing ? g_svg_pause : g_svg_play;
                else
                    icon = g_svg_next;

                if (icon) {
                    draw_svg_icon_white(cr, icon, bx + pad_x, by + pad_y, iw, ih);
                }
            }
        }
        // buton de setări (mereu vizibil la hover mare) - dreapta sus
        if (g_svg_settings) {
            double icon_size = 14.0;
            double margin = 6.0;
            double sx = notch_w - icon_size - margin;
            double sy = margin;
            draw_svg_icon_white(cr, g_svg_settings, sx, sy, icon_size, icon_size);
        }

        cairo_restore(cr);
    }

    cairo_destroy(cr);
    cairo_surface_destroy(surf);
}

} // namespace

int main() {
    struct sigaction sa = {};
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    std::thread pulse_thread(pulse_audio_thread_func);
    pulse_thread.detach();
    std::thread mpris_thread(mpris_album_art_thread_func);
    mpris_thread.detach();

    char svg_path[PATH_MAX];
    if (!find_notch_svg(svg_path, sizeof(svg_path))) {
        return 1;
    }
    GError* err = nullptr;
    g_svg = rsvg_handle_new_from_file(svg_path, &err);
    if (!g_svg) {
        if (err) g_error_free(err);
        return 1;
    }

    // încarcă SVG-urile pentru controale (dacă există)
    GError* err_ctrl = nullptr;
    g_svg_prev = rsvg_handle_new_from_file("prev.svg", &err_ctrl);
    if (err_ctrl) { g_error_free(err_ctrl); err_ctrl = nullptr; }
    g_svg_next = rsvg_handle_new_from_file("next.svg", &err_ctrl);
    if (err_ctrl) { g_error_free(err_ctrl); err_ctrl = nullptr; }
    g_svg_play = rsvg_handle_new_from_file("play.svg", &err_ctrl);
    if (err_ctrl) { g_error_free(err_ctrl); err_ctrl = nullptr; }
    g_svg_pause = rsvg_handle_new_from_file("pause.svg", &err_ctrl);
    if (err_ctrl) { g_error_free(err_ctrl); err_ctrl = nullptr; }
    g_svg_settings = rsvg_handle_new_from_file("settings.svg", &err_ctrl);
    if (err_ctrl) { g_error_free(err_ctrl); err_ctrl = nullptr; }

    g_dpy = XOpenDisplay(nullptr);
    if (!g_dpy) {
        g_object_unref(g_svg);
        return 1;
    }

    int screen = DefaultScreen(g_dpy);
    int sw = DisplayWidth(g_dpy, screen);
    int win_w = NOTCH_WIDTH;
    int win_h = NOTCH_HEIGHT;
    int x = (sw - win_w) / 2;
    int y = 0;

    XVisualInfo vinfo;
    bool use_alpha = find_argb_visual(g_dpy, screen, &vinfo);

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.event_mask = ExposureMask | StructureNotifyMask | EnterWindowMask | LeaveWindowMask | ButtonPressMask;
    attrs.background_pixel = 0;
    attrs.border_pixel = 0;
    attrs.colormap = use_alpha ? XCreateColormap(g_dpy, RootWindow(g_dpy, screen), vinfo.visual, AllocNone) : CopyFromParent;

    g_win = XCreateWindow(
        g_dpy, RootWindow(g_dpy, screen),
        x, y, win_w, win_h,
        0,
        use_alpha ? vinfo.depth : CopyFromParent,
        InputOutput,
        use_alpha ? vinfo.visual : CopyFromParent,
        CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWEventMask | (use_alpha ? CWColormap : 0),
        &attrs
    );

    // Fără titlu (no window name)
    XStoreName(g_dpy, g_win, "");

    // Nu apărea în taskbar / Alt+Tab (pe lângă override_redirect)
    set_skip_taskbar_and_pager(g_dpy, g_win);

    XMapWindow(g_dpy, g_win);
    XFlush(g_dpy);

    set_always_on_top(g_dpy, g_win);

    g_invisible_cursor = create_invisible_cursor(g_dpy, g_win);

    {
        GError* err = nullptr;
        g_dbus_conn = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &err);
        if (!g_dbus_conn && err) g_error_free(err);
    }

    double cur_w = NOTCH_WIDTH, cur_h = NOTCH_HEIGHT;
    int target_w = NOTCH_WIDTH, target_h = NOTCH_HEIGHT;
    long hover_since = 0;
    const int frame_ms = 16;  // ~60 FPS
    Window root = RootWindow(g_dpy, screen);
    int last_w = -1, last_h = -1, last_sx = -1;

    while (g_running.load()) {
        while (XPending(g_dpy)) {
            XEvent ev;
            XNextEvent(g_dpy, &ev);

            if (ev.type == Expose && ev.xexpose.count == 0) {
                if (ev.xexpose.window == g_win)
                    paint(g_dpy, g_win, static_cast<int>(cur_w), static_cast<int>(cur_h), static_cast<int>(cur_w), static_cast<int>(cur_h));
                else if (ev.xexpose.window == g_settings_win)
                    paint_settings_window(g_dpy);
            }
            else if (ev.type == ClientMessage && ev.xclient.window == g_settings_win) {
                if (g_wm_delete_settings &&
                    static_cast<Atom>(ev.xclient.data.l[0]) == g_wm_delete_settings) {
                    close_settings_window();
                }
            }
            else if (ev.type == ButtonPress && ev.xbutton.button == Button1) {
                if (ev.xbutton.window == g_win && static_cast<int>(cur_h) > NOTCH_HEIGHT) {
                    int cw = static_cast<int>(cur_w), ch = static_cast<int>(cur_h);
                    double cx = cw / 2.0;
                    double button_y = ch * 0.72;
                    int total_w = 3 * CTRL_BUTTON_W + 2 * CTRL_BUTTON_GAP;
                    double btn_left = cx - total_w / 2.0;
                    int px = ev.xbutton.x, py = ev.xbutton.y;

                    // click pe controalele media (dacă avem MPRIS)
                    if (g_dbus_conn) {
                        for (int i = 0; i < 3; i++) {
                            double bx = btn_left + i * (CTRL_BUTTON_W + CTRL_BUTTON_GAP);
                            if (px >= static_cast<int>(bx) && px < static_cast<int>(bx) + CTRL_BUTTON_W &&
                                py >= static_cast<int>(button_y) && py < static_cast<int>(button_y) + CTRL_BUTTON_H) {
                                const char* method = (i == 0) ? "Previous" : (i == 1) ? "PlayPause" : "Next";
                                std::string name;
                                { std::lock_guard<std::mutex> lock(g_meta_mutex); name = g_mpris_name; }
                                if (!name.empty()) {
                                    GError* err = nullptr;
                                    g_dbus_connection_call_sync(g_dbus_conn, name.c_str(), "/org/mpris/MediaPlayer2",
                                        "org.mpris.MediaPlayer2.Player", method, g_variant_new("()"),
                                        G_VARIANT_TYPE("()"), G_DBUS_CALL_FLAGS_NONE, 1000, nullptr, &err);
                                    if (err) g_error_free(err);
                                }
                                break;
                            }
                        }
                    }

                    // click pe iconița de setări (dreapta sus)
                    if (g_svg_settings) {
                        double icon_size = 14.0;
                        double margin = 6.0;
                        double sx = cw - icon_size - margin;
                        double sy = margin;
                        if (px >= static_cast<int>(sx) && px <= static_cast<int>(sx + icon_size) &&
                            py >= static_cast<int>(sy) && py <= static_cast<int>(sy + icon_size)) {
                            if (g_settings_visible)
                                close_settings_window();
                            else
                                open_settings_window();
                        }
                    }
                } else if (ev.xbutton.window == g_settings_win) {
                    int px = ev.xbutton.x;
                    int py = ev.xbutton.y;

                    // toggle checkbox Run Notch at startup
                    double cb_x = 18;
                    double cb_y = 38;
                    double cb_size = 14;
                    if (px >= static_cast<int>(cb_x) && px <= static_cast<int>(cb_x + cb_size) &&
                        py >= static_cast<int>(cb_y) && py <= static_cast<int>(cb_y + cb_size)) {
                        set_run_at_startup(!g_run_at_startup);
                        paint_settings_window(g_dpy);
                    }

                    // buton Terminate Notch
                    XWindowAttributes xwa;
                    if (XGetWindowAttributes(g_dpy, g_settings_win, &xwa)) {
                        int w = xwa.width;
                        int h = xwa.height;
                        double btn_w = 120;
                        double btn_h = 24;
                        double margin = 16;
                        double btn_x = w - btn_w - margin;
                        double btn_y = h - btn_h - margin;
                        if (px >= static_cast<int>(btn_x) && px <= static_cast<int>(btn_x + btn_w) &&
                            py >= static_cast<int>(btn_y) && py <= static_cast<int>(btn_y + btn_h)) {
                            g_running = false;
                        }
                    }
                }
            }
            else if (ev.type == LeaveNotify) {
                hover_since = 0;
                target_w = NOTCH_WIDTH;
                target_h = NOTCH_HEIGHT;
                if (g_cursor_hidden) {
                    XUndefineCursor(g_dpy, g_win);
                    g_cursor_hidden = false;
                }
            }
            else if (ev.type == EnterNotify) {
                hover_since = 0;
                if (!g_cursor_hidden && static_cast<int>(cur_h) <= NOTCH_HEIGHT && g_invisible_cursor) {
                    XDefineCursor(g_dpy, g_win, g_invisible_cursor);
                    g_cursor_hidden = true;
                }
            }
        }

        int root_x, root_y, win_x, win_y; Window child; unsigned mask;
        Bool over = XQueryPointer(g_dpy, root, &child, &child, &root_x, &root_y, &win_x, &win_y, &mask);
        bool inside = over && (child == g_win);

        long now = time_ms();
        if (inside) {
            if (hover_since == 0) hover_since = now;
            if (now - hover_since >= HOVER_MS)
                target_w = NOTCH_W_BIG, target_h = NOTCH_H_BIG;
        }

        cur_w += (target_w - cur_w) * ANIM_SPEED;
        cur_h += (target_h - cur_h) * ANIM_SPEED;
        if (std::abs(cur_w - target_w) < 0.5) cur_w = target_w;
        if (std::abs(cur_h - target_h) < 0.5) cur_h = target_h;

        if (g_cursor_hidden && cur_h > NOTCH_HEIGHT + 1) {
            XUndefineCursor(g_dpy, g_win);
            g_cursor_hidden = false;
        }

        if (g_audio_playing)
            g_visualizer_alpha = std::min(1.0, g_visualizer_alpha + VISUALIZER_FADE_SPEED);
        else
            g_visualizer_alpha = std::max(0.0, g_visualizer_alpha - VISUALIZER_FADE_SPEED);

        int nw = static_cast<int>(std::round(cur_w));
        int nh = static_cast<int>(std::round(cur_h));
        const int step = 1;
        nw = (nw / step) * step;
        nh = (nh / step) * step;
        if (nw < NOTCH_WIDTH) nw = NOTCH_WIDTH;
        if (nh < NOTCH_HEIGHT) nh = NOTCH_HEIGHT;
        int w_cur = nw;
        int h_cur = nh;
        int sx = (DisplayWidth(g_dpy, screen) - w_cur) / 2;

        if (w_cur != last_w || h_cur != last_h) {
            XResizeWindow(g_dpy, g_win, w_cur, h_cur);
            last_w = w_cur;
            last_h = h_cur;
        }
        if (sx != last_sx) {
            XMoveWindow(g_dpy, g_win, sx, 0);
            last_sx = sx;
        }

        paint(g_dpy, g_win, w_cur, h_cur, nw, nh);
        XFlush(g_dpy);
        XRaiseWindow(g_dpy, g_win);
        struct timespec ts = { 0, frame_ms * 1000 * 1000 };
        while (nanosleep(&ts, &ts) < 0 && errno == EINTR && g_running.load())
            ;
    }

    if (g_svg) {
        g_object_unref(g_svg);
        g_svg = nullptr;
    }
    if (g_svg_prev) {
        g_object_unref(g_svg_prev);
        g_svg_prev = nullptr;
    }
    if (g_svg_next) {
        g_object_unref(g_svg_next);
        g_svg_next = nullptr;
    }
    if (g_svg_play) {
        g_object_unref(g_svg_play);
        g_svg_play = nullptr;
    }
    if (g_svg_pause) {
        g_object_unref(g_svg_pause);
        g_svg_pause = nullptr;
    }
    if (g_svg_settings) {
        g_object_unref(g_svg_settings);
        g_svg_settings = nullptr;
    }
    if (g_settings_win) {
        XDestroyWindow(g_dpy, g_settings_win);
        g_settings_win = 0;
    }
    if (g_svg_prev) {
        g_object_unref(g_svg_prev);
        g_svg_prev = nullptr;
    }
    if (g_svg_next) {
        g_object_unref(g_svg_next);
        g_svg_next = nullptr;
    }
    if (g_svg_play) {
        g_object_unref(g_svg_play);
        g_svg_play = nullptr;
    }
    if (g_svg_pause) {
        g_object_unref(g_svg_pause);
        g_svg_pause = nullptr;
    }
    if (g_dbus_conn) {
        g_object_unref(g_dbus_conn);
        g_dbus_conn = nullptr;
    }
    if (g_invisible_cursor) {
        XFreeCursor(g_dpy, g_invisible_cursor);
        g_invisible_cursor = 0;
    }
    XDestroyWindow(g_dpy, g_win);
    XCloseDisplay(g_dpy);
    return 0;
}
