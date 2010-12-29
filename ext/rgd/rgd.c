#include "ruby.h"

#include "gd.h"
#include "gdfontg.h"
#include "gdfontl.h"
#include "gdfontmb.h"
#include "gdfonts.h"
#include "gdfontt.h"
#ifndef HAVE_GDIMAGECREATEFROMBMP
// get from https://svn.php.net/viewvc/gd/trunk/playground/gdbmp/
#include "gd_playground/gd_bmp.c"
#endif

#ifdef WIN32
#define GDFUNC __stdcall
#else
#define GDFUNC 
#endif

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

#define SetIntIfQnil(v, i) if (v == Qnil) v = INT2NUM(i)
#define STR2SYM(v) ID2SYM(rb_intern(v))

typedef enum ImageFormat { FMT_UNKNOW, FMT_JPEG, FMT_PNG, FMT_GIF, FMT_GD, FMT_GD2, FMT_WBMP, FMT_XBM, FMT_XPM, FMT_BMP } ImageFormat;

static VALUE rb_mRGD, rb_eRGDError, rb_cFont, rb_cImage;

static ImageFormat m_image_detect_format_by_ext(const char* ext) {
    if (strcasecmp(ext, "jpg") == 0 || strcasecmp(ext, "jpeg") == 0) {
        return FMT_JPEG;
    } else if (strcasecmp(ext, "png") == 0) {
        return FMT_PNG;
    } else if (strcasecmp(ext, "gif") == 0) {
        return FMT_GIF;
    } else if (strcasecmp(ext, "bmp") == 0) {
        return FMT_BMP;
    } else if (strcasecmp(ext, "gd2") == 0) {
        return FMT_GD2;
    } else if (strcasecmp(ext, "gd") == 0) {
        return FMT_GD;
    } else if (strcasecmp(ext, "wbmp") == 0) {
        return FMT_WBMP;
    } else if (strcasecmp(ext, "xbm") == 0) {
        return FMT_XBM;
    } else if (strcasecmp(ext, "xpm") == 0) {
        return FMT_XPM;
    } else {
        return FMT_UNKNOW;
    }
}

static ImageFormat m_image_detect_format_by_magic(const char* buf) {
    if (strncmp(buf, "\377\330\377\340", 4) == 0 || strncmp(buf, "\377\330\377\341", 4) == 0 || strncmp(buf, "\377\330\377\356", 4) == 0) {
        return FMT_JPEG;
    } else if (strncmp(buf, "\x89PNG", 4) == 0) {
        return FMT_PNG;
    } else if (strncmp(buf, "GIF89a", 6) == 0) {
        return FMT_GIF;
    } else if (strncmp(buf, "BM", 2) == 0 || strncmp(buf, "BA", 2) == 0 || strncmp(buf, "CI", 2) == 0 || strncmp(buf, "CP", 2) == 0 || strncmp(buf, "IC", 2) == 0 || strncmp(buf, "PT", 2) == 0) {
        return FMT_BMP;
    } else if (strncmp(buf, "gd2\0", 4) == 0) {
        return FMT_GD2;
    } else if (strncmp(buf, "\xFF\xFF", 2) == 0 || strncmp(buf, "\xFF\xFE", 2) == 0) {
        return FMT_GD;
    } else if (strncmp(buf, "/* XPM */", 9) == 0) {
        return FMT_XPM;
    } else if (strncmp(buf, "\0\0", 2) == 0) {
        return FMT_WBMP;
    } else {
        return FMT_UNKNOW;
    }
}

static VALUE m_named_colors() {
    VALUE h = rb_hash_new();
    rb_hash_aset(h, rb_str_new2("aliceblue"), rb_ary_new3(4, INT2NUM(0xF0), INT2NUM(0xF8), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("antiquewhite"), rb_ary_new3(4, INT2NUM(0xFA), INT2NUM(0xEB), INT2NUM(0xD7), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("aqua"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0xFF), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("aquamarine"), rb_ary_new3(4, INT2NUM(0x7F), INT2NUM(0xFF), INT2NUM(0xD4), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("azure"), rb_ary_new3(4, INT2NUM(0xF0), INT2NUM(0xFF), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("beige"), rb_ary_new3(4, INT2NUM(0xF5), INT2NUM(0xF5), INT2NUM(0xDC), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("bisque"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xE4), INT2NUM(0xC4), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("black"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0x00), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("blanchedalmond"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xEB), INT2NUM(0xCD), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("blue"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0x00), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("blueviolet"), rb_ary_new3(4, INT2NUM(0x8A), INT2NUM(0x2B), INT2NUM(0xE2), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("brown"), rb_ary_new3(4, INT2NUM(0xA5), INT2NUM(0x2A), INT2NUM(0x2A), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("burlywood"), rb_ary_new3(4, INT2NUM(0xDE), INT2NUM(0xB8), INT2NUM(0x87), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("cadetblue"), rb_ary_new3(4, INT2NUM(0x5F), INT2NUM(0x9E), INT2NUM(0xA0), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("chartreuse"), rb_ary_new3(4, INT2NUM(0x7F), INT2NUM(0xFF), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("chocolate"), rb_ary_new3(4, INT2NUM(0xD2), INT2NUM(0x69), INT2NUM(0x1E), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("coral"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0x7F), INT2NUM(0x50), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("cornflowerblue"), rb_ary_new3(4, INT2NUM(0x64), INT2NUM(0x95), INT2NUM(0xED), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("cornsilk"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xF8), INT2NUM(0xDC), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("crimson"), rb_ary_new3(4, INT2NUM(0xDC), INT2NUM(0x14), INT2NUM(0x3C), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("cyan"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0xFF), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkblue"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0x00), INT2NUM(0x8B), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkcyan"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0x8B), INT2NUM(0x8B), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkgoldenrod"), rb_ary_new3(4, INT2NUM(0xB8), INT2NUM(0x86), INT2NUM(0x0B), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkgray"), rb_ary_new3(4, INT2NUM(0xA9), INT2NUM(0xA9), INT2NUM(0xA9), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkgrey"), rb_ary_new3(4, INT2NUM(0xA9), INT2NUM(0xA9), INT2NUM(0xA9), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkgreen"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0x64), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkkhaki"), rb_ary_new3(4, INT2NUM(0xBD), INT2NUM(0xB7), INT2NUM(0x6B), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkmagenta"), rb_ary_new3(4, INT2NUM(0x8B), INT2NUM(0x00), INT2NUM(0x8B), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkolivegreen"), rb_ary_new3(4, INT2NUM(0x55), INT2NUM(0x6B), INT2NUM(0x2F), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkorange"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0x8C), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkorchid"), rb_ary_new3(4, INT2NUM(0x99), INT2NUM(0x32), INT2NUM(0xCC), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkred"), rb_ary_new3(4, INT2NUM(0x8B), INT2NUM(0x00), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darksalmon"), rb_ary_new3(4, INT2NUM(0xE9), INT2NUM(0x96), INT2NUM(0x7A), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkseagreen"), rb_ary_new3(4, INT2NUM(0x8F), INT2NUM(0xBC), INT2NUM(0x8F), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkslateblue"), rb_ary_new3(4, INT2NUM(0x48), INT2NUM(0x3D), INT2NUM(0x8B), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkslategray"), rb_ary_new3(4, INT2NUM(0x2F), INT2NUM(0x4F), INT2NUM(0x4F), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkslategrey"), rb_ary_new3(4, INT2NUM(0x2F), INT2NUM(0x4F), INT2NUM(0x4F), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkturquoise"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0xCE), INT2NUM(0xD1), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("darkviolet"), rb_ary_new3(4, INT2NUM(0x94), INT2NUM(0x00), INT2NUM(0xD3), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("deeppink"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0x14), INT2NUM(0x93), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("deepskyblue"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0xBF), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("dimgray"), rb_ary_new3(4, INT2NUM(0x69), INT2NUM(0x69), INT2NUM(0x69), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("dimgrey"), rb_ary_new3(4, INT2NUM(0x69), INT2NUM(0x69), INT2NUM(0x69), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("dodgerblue"), rb_ary_new3(4, INT2NUM(0x1E), INT2NUM(0x90), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("firebrick"), rb_ary_new3(4, INT2NUM(0xB2), INT2NUM(0x22), INT2NUM(0x22), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("floralwhite"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xFA), INT2NUM(0xF0), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("forestgreen"), rb_ary_new3(4, INT2NUM(0x22), INT2NUM(0x8B), INT2NUM(0x22), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("fuchsia"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0x00), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("gainsboro"), rb_ary_new3(4, INT2NUM(0xDC), INT2NUM(0xDC), INT2NUM(0xDC), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("ghostwhite"), rb_ary_new3(4, INT2NUM(0xF8), INT2NUM(0xF8), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("gold"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xD7), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("goldenrod"), rb_ary_new3(4, INT2NUM(0xDA), INT2NUM(0xA5), INT2NUM(0x20), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("gray"), rb_ary_new3(4, INT2NUM(0x80), INT2NUM(0x80), INT2NUM(0x80), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("grey"), rb_ary_new3(4, INT2NUM(0x80), INT2NUM(0x80), INT2NUM(0x80), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("green"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0x80), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("greenyellow"), rb_ary_new3(4, INT2NUM(0xAD), INT2NUM(0xFF), INT2NUM(0x2F), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("honeydew"), rb_ary_new3(4, INT2NUM(0xF0), INT2NUM(0xFF), INT2NUM(0xF0), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("hotpink"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0x69), INT2NUM(0xB4), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("indianred"), rb_ary_new3(4, INT2NUM(0xCD), INT2NUM(0x5C), INT2NUM(0x5C), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("indigo"), rb_ary_new3(4, INT2NUM(0x4B), INT2NUM(0x00), INT2NUM(0x82), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("ivory"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xFF), INT2NUM(0xF0), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("khaki"), rb_ary_new3(4, INT2NUM(0xF0), INT2NUM(0xE6), INT2NUM(0x8C), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lavender"), rb_ary_new3(4, INT2NUM(0xE6), INT2NUM(0xE6), INT2NUM(0xFA), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lavenderblush"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xF0), INT2NUM(0xF5), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lawngreen"), rb_ary_new3(4, INT2NUM(0x7C), INT2NUM(0xFC), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lemonchiffon"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xFA), INT2NUM(0xCD), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightblue"), rb_ary_new3(4, INT2NUM(0xAD), INT2NUM(0xD8), INT2NUM(0xE6), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightcoral"), rb_ary_new3(4, INT2NUM(0xF0), INT2NUM(0x80), INT2NUM(0x80), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightcyan"), rb_ary_new3(4, INT2NUM(0xE0), INT2NUM(0xFF), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightgoldenrodyellow"), rb_ary_new3(4, INT2NUM(0xFA), INT2NUM(0xFA), INT2NUM(0xD2), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightgray"), rb_ary_new3(4, INT2NUM(0xD3), INT2NUM(0xD3), INT2NUM(0xD3), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightgrey"), rb_ary_new3(4, INT2NUM(0xD3), INT2NUM(0xD3), INT2NUM(0xD3), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightgreen"), rb_ary_new3(4, INT2NUM(0x90), INT2NUM(0xEE), INT2NUM(0x90), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightpink"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xB6), INT2NUM(0xC1), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightsalmon"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xA0), INT2NUM(0x7A), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightseagreen"), rb_ary_new3(4, INT2NUM(0x20), INT2NUM(0xB2), INT2NUM(0xAA), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightskyblue"), rb_ary_new3(4, INT2NUM(0x87), INT2NUM(0xCE), INT2NUM(0xFA), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightslategray"), rb_ary_new3(4, INT2NUM(0x77), INT2NUM(0x88), INT2NUM(0x99), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightslategrey"), rb_ary_new3(4, INT2NUM(0x77), INT2NUM(0x88), INT2NUM(0x99), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightsteelblue"), rb_ary_new3(4, INT2NUM(0xB0), INT2NUM(0xC4), INT2NUM(0xDE), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lightyellow"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xFF), INT2NUM(0xE0), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("lime"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0xFF), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("limegreen"), rb_ary_new3(4, INT2NUM(0x32), INT2NUM(0xCD), INT2NUM(0x32), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("linen"), rb_ary_new3(4, INT2NUM(0xFA), INT2NUM(0xF0), INT2NUM(0xE6), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("magenta"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0x00), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("maroon"), rb_ary_new3(4, INT2NUM(0x80), INT2NUM(0x00), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mediumaquamarine"), rb_ary_new3(4, INT2NUM(0x66), INT2NUM(0xCD), INT2NUM(0xAA), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mediumblue"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0x00), INT2NUM(0xCD), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mediumorchid"), rb_ary_new3(4, INT2NUM(0xBA), INT2NUM(0x55), INT2NUM(0xD3), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mediumpurple"), rb_ary_new3(4, INT2NUM(0x93), INT2NUM(0x70), INT2NUM(0xD8), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mediumseagreen"), rb_ary_new3(4, INT2NUM(0x3C), INT2NUM(0xB3), INT2NUM(0x71), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mediumslateblue"), rb_ary_new3(4, INT2NUM(0x7B), INT2NUM(0x68), INT2NUM(0xEE), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mediumspringgreen"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0xFA), INT2NUM(0x9A), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mediumturquoise"), rb_ary_new3(4, INT2NUM(0x48), INT2NUM(0xD1), INT2NUM(0xCC), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mediumvioletred"), rb_ary_new3(4, INT2NUM(0xC7), INT2NUM(0x15), INT2NUM(0x85), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("midnightblue"), rb_ary_new3(4, INT2NUM(0x19), INT2NUM(0x19), INT2NUM(0x70), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mintcream"), rb_ary_new3(4, INT2NUM(0xF5), INT2NUM(0xFF), INT2NUM(0xFA), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("mistyrose"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xE4), INT2NUM(0xE1), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("moccasin"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xE4), INT2NUM(0xB5), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("navajowhite"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xDE), INT2NUM(0xAD), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("navy"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0x00), INT2NUM(0x80), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("oldlace"), rb_ary_new3(4, INT2NUM(0xFD), INT2NUM(0xF5), INT2NUM(0xE6), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("olive"), rb_ary_new3(4, INT2NUM(0x80), INT2NUM(0x80), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("olivedrab"), rb_ary_new3(4, INT2NUM(0x6B), INT2NUM(0x8E), INT2NUM(0x23), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("orange"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xA5), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("orangered"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0x45), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("orchid"), rb_ary_new3(4, INT2NUM(0xDA), INT2NUM(0x70), INT2NUM(0xD6), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("palegoldenrod"), rb_ary_new3(4, INT2NUM(0xEE), INT2NUM(0xE8), INT2NUM(0xAA), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("palegreen"), rb_ary_new3(4, INT2NUM(0x98), INT2NUM(0xFB), INT2NUM(0x98), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("paleturquoise"), rb_ary_new3(4, INT2NUM(0xAF), INT2NUM(0xEE), INT2NUM(0xEE), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("palevioletred"), rb_ary_new3(4, INT2NUM(0xD8), INT2NUM(0x70), INT2NUM(0x93), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("papayawhip"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xEF), INT2NUM(0xD5), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("peachpuff"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xDA), INT2NUM(0xB9), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("peru"), rb_ary_new3(4, INT2NUM(0xCD), INT2NUM(0x85), INT2NUM(0x3F), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("pink"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xC0), INT2NUM(0xCB), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("plum"), rb_ary_new3(4, INT2NUM(0xDD), INT2NUM(0xA0), INT2NUM(0xDD), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("powderblue"), rb_ary_new3(4, INT2NUM(0xB0), INT2NUM(0xE0), INT2NUM(0xE6), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("purple"), rb_ary_new3(4, INT2NUM(0x80), INT2NUM(0x00), INT2NUM(0x80), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("red"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0x00), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("rosybrown"), rb_ary_new3(4, INT2NUM(0xBC), INT2NUM(0x8F), INT2NUM(0x8F), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("royalblue"), rb_ary_new3(4, INT2NUM(0x41), INT2NUM(0x69), INT2NUM(0xE1), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("saddlebrown"), rb_ary_new3(4, INT2NUM(0x8B), INT2NUM(0x45), INT2NUM(0x13), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("salmon"), rb_ary_new3(4, INT2NUM(0xFA), INT2NUM(0x80), INT2NUM(0x72), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("sandybrown"), rb_ary_new3(4, INT2NUM(0xF4), INT2NUM(0xA4), INT2NUM(0x60), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("seagreen"), rb_ary_new3(4, INT2NUM(0x2E), INT2NUM(0x8B), INT2NUM(0x57), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("seashell"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xF5), INT2NUM(0xEE), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("sienna"), rb_ary_new3(4, INT2NUM(0xA0), INT2NUM(0x52), INT2NUM(0x2D), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("silver"), rb_ary_new3(4, INT2NUM(0xC0), INT2NUM(0xC0), INT2NUM(0xC0), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("skyblue"), rb_ary_new3(4, INT2NUM(0x87), INT2NUM(0xCE), INT2NUM(0xEB), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("slateblue"), rb_ary_new3(4, INT2NUM(0x6A), INT2NUM(0x5A), INT2NUM(0xCD), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("slategray"), rb_ary_new3(4, INT2NUM(0x70), INT2NUM(0x80), INT2NUM(0x90), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("slategrey"), rb_ary_new3(4, INT2NUM(0x70), INT2NUM(0x80), INT2NUM(0x90), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("snow"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xFA), INT2NUM(0xFA), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("springgreen"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0xFF), INT2NUM(0x7F), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("steelblue"), rb_ary_new3(4, INT2NUM(0x46), INT2NUM(0x82), INT2NUM(0xB4), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("tan"), rb_ary_new3(4, INT2NUM(0xD2), INT2NUM(0xB4), INT2NUM(0x8C), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("teal"), rb_ary_new3(4, INT2NUM(0x00), INT2NUM(0x80), INT2NUM(0x80), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("thistle"), rb_ary_new3(4, INT2NUM(0xD8), INT2NUM(0xBF), INT2NUM(0xD8), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("tomato"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0x63), INT2NUM(0x47), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("turquoise"), rb_ary_new3(4, INT2NUM(0x40), INT2NUM(0xE0), INT2NUM(0xD0), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("violet"), rb_ary_new3(4, INT2NUM(0xEE), INT2NUM(0x82), INT2NUM(0xEE), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("wheat"), rb_ary_new3(4, INT2NUM(0xF5), INT2NUM(0xDE), INT2NUM(0xB3), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("white"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xFF), INT2NUM(0xFF), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("whitesmoke"), rb_ary_new3(4, INT2NUM(0xF5), INT2NUM(0xF5), INT2NUM(0xF5), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("yellow"), rb_ary_new3(4, INT2NUM(0xFF), INT2NUM(0xFF), INT2NUM(0x00), INT2NUM(gdAlphaOpaque)));
    rb_hash_aset(h, rb_str_new2("yellowgreen"), rb_ary_new3(4, INT2NUM(0x9A), INT2NUM(0xCD), INT2NUM(0x32), INT2NUM(gdAlphaOpaque)));
    return h;
}

static VALUE m_scan_color_args(int argc, VALUE *argv) {
    VALUE r, g, b, a;
    int i;
    if (argc == 1) {
        i = TYPE(argv[0]);
        if (i == T_STRING) {
            a = rb_hash_aref(rb_cv_get(rb_cImage, "@@named_colors"), rb_funcall(argv[0], rb_intern("downcase"), 0));
            if (TYPE(a) == T_ARRAY) {
                return a;
            } else {
                rb_raise(rb_eArgError, "Unknow color name: %s", RSTRING_PTR(argv[0]));
            }
        } else if (i == T_FIXNUM) {
            i = NUM2INT(argv[0]);
            return rb_ary_new3(4, INT2NUM(gdTrueColorGetRed(i)), INT2NUM(gdTrueColorGetGreen(i)), INT2NUM(gdTrueColorGetBlue(i)), INT2NUM(gdTrueColorGetAlpha(i)));
        } else {
            rb_raise(rb_eArgError, "String or Fixnum expected");
        }
    } else if (argc == 3 || argc == 4) {
        rb_scan_args(argc, argv, "31", &r, &g, &b, &a);
        SetIntIfQnil(a, gdAlphaOpaque);
        return rb_ary_new3(4, r, g, b, a);
    } else {
        rb_raise(rb_eArgError, "Wrong # of arguments (1 or 3 or 4 for %d)", argc);
    }
}

// gd_gfp__X
static VALUE gd_font_get(VALUE klass, gdFontPtr (GDFUNC *pfunc)(void)) {
    gdFontPtr ft = (*pfunc)();
    return Data_Wrap_Struct(klass, 0, 0, ft);
}

/*
 * call-seq:
 *   Font.small()
 *
 * Returns the "small" gd font.
 *
 * See Image.string for more information, or Image.stringft for a
 * freetype-based alternative that supports truetype fonts.
 */
static VALUE font_s_small(VALUE klass) {
    return gd_font_get(klass, gdFontGetSmall);
}

/*
 * call-seq:
 *   Font.large()
 *
 * Returns the "large" gd font.
 *
 * See Image.string for more information, or Image.stringft for a
 * freetype-based alternative that supports truetype fonts.
 */
static VALUE font_s_large(VALUE klass) {
    return gd_font_get(klass, gdFontGetLarge);
}

/*
 * call-seq:
 *   Font.medium_bold()
 *
 * Returns the "medium bold" gd font.
 *
 * See Image.string for more information, or Image.stringft for a
 * freetype-based alternative that supports truetype fonts.
 */
static VALUE font_s_medium_bold(VALUE klass) {
    return gd_font_get(klass, gdFontGetMediumBold);
}

/*
 * call-seq:
 *   Font.giant()
 *
 * Returns the "giant" gd font.
 *
 * See Image.string for more information, or Image.stringft for a
 * freetype-based alternative that supports truetype fonts.
 */
static VALUE font_s_giant(VALUE klass) {
    return gd_font_get(klass, gdFontGetGiant);
}

/*
 * call-seq:
 *   Font.tiny()
 *
 * Returns the "tiny" gd font.
 *
 * See Image.string for more information, or Image.stringft for a
 * freetype-based alternative that supports truetype fonts.
 */
static VALUE font_s_tiny(VALUE klass) {
    return gd_font_get(klass, gdFontGetTiny);
}

// gd_H__H
/*
 * call-seq:
 *   use_fontconfig = boolean
 *
 * GD 2.0.29 introduced the ability to use fontconfig patterns rather than font
 * file names as parameters to Image.stringft, Image.stringft_circle. For
 * backwards compatibility reasons, the fontlist parameter to those functions is
 * still expected to be a full or partial font file path name or list thereof by
 * default. However, as a convenience, when use_fontconfig=true configures
 * gd to expect the fontlist parameter to be a fontconfig pattern.
 *
 * NOTE, if the fontconfig library is not available, use_fontconfig=true
 * will raise an error.
 */
static VALUE image_s_use_fontconfig(VALUE klass, VALUE flag) {
    if (!gdFTUseFontConfig(RTEST(flag) ? 1 : 0)) {
        rb_raise(rb_eRGDError, "The FontConfig library is not available.");
    }
}

// gd_H__H2
/*
 * call-seq:
 *   alpha_blend(dst, src)
 *
 * Accepts truecolor pixel values only. The source color is composited with the
 * destination color based on the alpha channel value of the source color.
 *
 * The resulting color is opaque.
 */
static VALUE image_s_alpha_blend(VALUE klass, VALUE dst, VALUE src) {
    return INT2NUM(gdAlphaBlend(NUM2INT(dst), NUM2INT(src)));
}

// gd_H__H4
/*
 * call-seq:
 *   truecolor(args)
 *
 * Returns an RGBA color value for use when drawing on a truecolor image.
 * 
 * Usage:
 * 
 * * truecolor(color_name)
 * * truecolor(r, g, b, a = ALPHA_OPAQUE)
 *
 * _color_name_ can be any one of 147 color names are defined in the HTML and
 * CSS color specification.
 * 
 * Red, green, and blue are all in the range between 0 (off) and 255 (maximum).
 * Alpha is in the range between ALPHA_OPAQUE (opaque) and ALPHA_TRANSPARENT
 * (fully transparent). This method should not be used with palette-based
 * images. If you need to write code which is compatible with both palette-based
 * and truecolor images, use Image.color_resolve.
 */
static VALUE image_s_truecolor(int argc, VALUE *argv, VALUE klass) {
    VALUE r = m_scan_color_args(argc, argv);
    return INT2NUM(gdTrueColorAlpha(NUM2INT(rb_ary_entry(r, 0)), NUM2INT(rb_ary_entry(r, 1)), NUM2INT(rb_ary_entry(r, 2)), NUM2INT(rb_ary_entry(r, 3))));
}

static void gd_image_free(gdImagePtr im) {
    if (im) gdImageDestroy(im);
}

// gd_gip__H2
static VALUE gd_image_create(VALUE klass, VALUE w, VALUE h, gdImagePtr (GDFUNC *pfunc)(int, int)) {
    gdImagePtr im = (*pfunc)(NUM2INT(w), NUM2INT(h));
    if (!im) rb_raise(rb_eRGDError, "unable to allocate the image");
    return Data_Wrap_Struct(klass, 0, gd_image_free, im);
}

/*
 * call-seq:
 *   Image.create(width, height)
 *
 * Creates a palette-based image, with no more than 256 colors.
 */
static VALUE image_s_create(VALUE klass, VALUE width, VALUE height) {
    return gd_image_create(klass, width, height, gdImageCreate);
}

/*
 * call-seq:
 *   create_truecolor(width, height)
 *
 * Creates a truecolor image, with an essentially unlimited number of colors.
 */
static VALUE image_s_create_truecolor(VALUE klass, VALUE width, VALUE height) {
    return gd_image_create(klass, width, height, gdImageCreateTrueColor);
}

// gd_gip__H_PAX
static VALUE gd_image_create_from_data(VALUE klass, VALUE data, gdImagePtr (GDFUNC *pfunc)(int, void*)) {
    gdImagePtr im;

    Check_Type(data, T_STRING);
    im = (*pfunc)(RSTRING_LEN(data), RSTRING_PTR(data));
    if (!im) rb_raise(rb_eRGDError, "Not valid Image data");

    return Data_Wrap_Struct(klass, 0, gd_image_free, im);
}

/*
 * call-seq:
 *   from_jpeg_data(data)
 *
 * Creates a truecolor image from a JPEG format byte-string.
 */
static VALUE image_s_from_jpeg_data(VALUE klass, VALUE data) {
    return gd_image_create_from_data(klass, data, gdImageCreateFromJpegPtr);
}

/*
 * call-seq:
 *   from_png_data(data)
 *
 * Creates a image from a PNG format byte-string.
 *
 * If the PNG image being loaded is a truecolor image, the result will be a
 * truecolor image. If the PNG image being loaded is a palette or grayscale
 * image, the result will be a palette image. gd retains only 8 bits of
 * resolution for each of the red, green and blue channels, and only 7 bits of
 * resolution for the alpha channel. The former restriction affects only a
 * handful of very rare 48-bit color and 16-bit grayscale PNG images. The second
 * restriction affects all semitransparent PNG images, but the difference is
 * essentially invisible to the eye. 7 bits of alpha channel resolution is, in
 * practice, quite a lot.
 */
static VALUE image_s_from_png_data(VALUE klass, VALUE data) {
    return gd_image_create_from_data(klass, data, gdImageCreateFromPngPtr);
}

/*
 * call-seq:
 *   from_gif_data(data)
 *
 * Creates a image from a GIF format byte-string.
 */
static VALUE image_s_from_gif_data(VALUE klass, VALUE data) {
    return gd_image_create_from_data(klass, data, gdImageCreateFromGifPtr);
}

/*
 * call-seq:
 *   from_gd_data(data)
 *
 * Creates a image from a GD format byte-string.
 */
static VALUE image_s_from_gd_data(VALUE klass, VALUE data) {
    return gd_image_create_from_data(klass, data, gdImageCreateFromGdPtr);
}

/*
 * call-seq:
 *   from_gd2_data(data)
 *
 * Creates a image from a GD2 format byte-string.
 */
static VALUE image_s_from_gd2_data(VALUE klass, VALUE data) {
    return gd_image_create_from_data(klass, data, gdImageCreateFromGd2Ptr);
}

/*
 * call-seq:
 *   from_wbmp_data(data)
 *
 * Creates a image from a WBMP format byte-string.
 */
static VALUE image_s_from_wbmp_data(VALUE klass, VALUE data) {
    return gd_image_create_from_data(klass, data, gdImageCreateFromWBMPPtr);
}

/*
 * call-seq:
 *   from_bmp_data(data)
 *
 * Creates a image from a BMP format byte-string.
 */
static VALUE image_s_from_bmp_data(VALUE klass, VALUE data) {
    return gd_image_create_from_data(klass, data, gdImageCreateFromBmpPtr);
}

// gd_gip__H_PAX_H4
/*
 * call-seq:
 *   from_gd2_part_data(data, x, y, width, height)
 *
 * Create a image from a GD2 format byte-string, with extra parameters
 * indicating the source (x, y) and width/height of the desired image.
 */
static VALUE image_s_from_gd2_part_data(VALUE klass, VALUE data, VALUE x, VALUE y, VALUE w, VALUE h) {
    gdImagePtr im;

    Check_Type(data, T_STRING);
    Check_Type(x, T_FIXNUM);Check_Type(y, T_FIXNUM);
    Check_Type(w, T_FIXNUM);Check_Type(h, T_FIXNUM);
    im = gdImageCreateFromGd2PartPtr(RSTRING_LEN(data), RSTRING_PTR(data), NUM2INT(x), NUM2INT(y), NUM2INT(w), NUM2INT(h));
    if (!im) rb_raise(rb_eRGDError, "Not valid Image data");
    return Data_Wrap_Struct(klass, 0, gd_image_free, im);
}

// gd_gip__PAD
/*
 * call-seq:
 *   from_xpm(filename)
 *
 * Creates a image from a XPM file.
 */
static VALUE image_s_from_xpm(VALUE klass, VALUE filename) {
    gdImagePtr im;

    if (rb_funcall(rb_cFile, rb_intern("readable?"), 1, filename) != Qtrue)
        rb_raise(rb_eIOError, "Cannot open %s for read", RSTRING_PTR(filename));
    im = gdImageCreateFromXpm(RSTRING_PTR(filename));
    if (!im) rb_raise(rb_eRGDError, "Not valid Image: %s", RSTRING_PTR(filename));
    return Data_Wrap_Struct(klass, 0, gd_image_free, im);
}

// gd_gip__fp
static VALUE gd_image_create_from_file(VALUE klass, VALUE filename, gdImagePtr (GDFUNC *pfunc)(FILE*)) {
    gdImagePtr im;
    FILE* fp;

    fp = fopen(RSTRING_PTR(filename), "rb");
    if (!fp) rb_raise(rb_eIOError, "Cannot open %s for read", RSTRING_PTR(filename));

    im = (*pfunc)(fp);
    fclose(fp);
    if (!im) rb_raise(rb_eRGDError, "Not valid Image: %s", RSTRING_PTR(filename));
    return Data_Wrap_Struct(klass, 0, gd_image_free, im);
}

/*
 * call-seq:
 *   from_jpeg(filename)
 *
 * Creates a truecolor image from a JPEG format file.
 */
static VALUE image_s_from_jpeg(VALUE klass, VALUE filename) {
    return gd_image_create_from_file(klass, filename, gdImageCreateFromJpeg);
}

/*
 * call-seq:
 *   from_png(filename)
 *
 * Creates a image from a PNG format file.
 *
 * If the PNG image being loaded is a truecolor image, the result will be a
 * truecolor image. If the PNG image being loaded is a palette or grayscale
 * image, the result will be a palette image. gd retains only 8 bits of
 * resolution for each of the red, green and blue channels, and only 7 bits of
 * resolution for the alpha channel. The former restriction affects only a
 * handful of very rare 48-bit color and 16-bit grayscale PNG images. The second
 * restriction affects all semitransparent PNG images, but the difference is
 * essentially invisible to the eye. 7 bits of alpha channel resolution is, in
 * practice, quite a lot.
 */
static VALUE image_s_from_png(VALUE klass, VALUE filename) {
    return gd_image_create_from_file(klass, filename, gdImageCreateFromPng);
}

/*
 * call-seq:
 *   from_gif(filename)
 *
 * Creates a image from a GIF format file.
 */
static VALUE image_s_from_gif(VALUE klass, VALUE filename) {
    return gd_image_create_from_file(klass, filename, gdImageCreateFromGif);
}

/*
 * call-seq:
 *   from_gd(filename)
 *
 * Creates a image from a GD format file.
 */
static VALUE image_s_from_gd(VALUE klass, VALUE filename) {
    return gd_image_create_from_file(klass, filename, gdImageCreateFromGd);
}

/*
 * call-seq:
 *   from_gd2(filename)
 *
 * Creates a image from a GD2 format file, with extra parameters indicating the
 * source (x, y) and width/height of the desired image.
 */
static VALUE image_s_from_gd2(VALUE klass, VALUE filename) {
    return gd_image_create_from_file(klass, filename, gdImageCreateFromGd2);
}

/*
 * call-seq:
 *   from_wbmp(filename)
 *
 * Creates a image from a WBMP format file.
 */
static VALUE image_s_from_wbmp(VALUE klass, VALUE filename) {
    return gd_image_create_from_file(klass, filename, gdImageCreateFromWBMP);
}

/*
 * call-seq:
 *   from_xbm(filename)
 *
 * Creates a image from a XBM format file.
 */
static VALUE image_s_from_xbm(VALUE klass, VALUE filename) {
    return gd_image_create_from_file(klass, filename, gdImageCreateFromXbm);
}

/*
 * call-seq:
 *   from_bmp(filename)
 *
 * Creates a image from a BMP format file.
 */
static VALUE image_s_from_bmp(VALUE klass, VALUE filename) {
    return gd_image_create_from_file(klass, filename, gdImageCreateFromBmp);
}

// gd_gip__fp_H4
/*
 * call-seq:
 *   from_gd2_part(filename, x, y, width, height)
 *
 * Create a image from a GD2 format file, with extra parameters indicating the
 * source (x, y) and width/height of the desired image.
 */
static VALUE image_s_from_gd2_part(VALUE klass, VALUE filename, VALUE x, VALUE y, VALUE w, VALUE h) {
    gdImagePtr im;
    FILE* fp;

    Check_Type(x, T_FIXNUM);Check_Type(y, T_FIXNUM);
    Check_Type(w, T_FIXNUM);Check_Type(h, T_FIXNUM);
    fp = fopen(RSTRING_PTR(filename), "rb");
    if (!fp) rb_raise(rb_eIOError, "Cannot open %s for read", RSTRING_PTR(filename));

    im = gdImageCreateFromGd2Part(fp, NUM2INT(x), NUM2INT(y), NUM2INT(w), NUM2INT(h));
    fclose(fp);

    if (!im) rb_raise(rb_eRGDError, "Not valid Image: %s", RSTRING_PTR(filename));
    return Data_Wrap_Struct(klass, 0, gd_image_free, im);
}

// gd_gip__gip_H2
/*
 * call-seq:
 *   create_palette_from_truecolor(img, dither = true, colors = MAX_COLORS)
 *
 * Create a new palette-based image from a truecolor image, by using a
 * high-quality two-pass quantization routine. If _dither_ is true, the image
 * will be dithered to approximate colors better, at the expense of some
 * obvious "speckling." _colors_ can be anything up to 256. If the original
 * source image includes photographic information or anything that came out of
 * a JPEG, 256 is strongly recommended. 100% transparency of a single
 * transparent color in the original truecolor image will be preserved. There is
 * no other support for preservation of alpha channel or transparency in the
 * destination image.
 */
static VALUE image_s_create_palette_from_truecolor(int argc, VALUE *argv, VALUE klass) {
    gdImagePtr im, im2;
    VALUE img2, dither, colors;
    rb_scan_args(argc, argv, "12", &img2, &dither, &colors);
    SetIntIfQnil(dither, Qtrue);
    Data_Get_Struct(img2, gdImage, im2);
    im = gdImageCreatePaletteFromTrueColor(im2, RTEST(dither) ? 1 : 0, RTEST(colors) ? NUM2INT(colors) : gdMaxColors);
    if (!im) rb_raise(rb_eRGDError, "Not valid Image data");
    return Data_Wrap_Struct(klass, 0, gd_image_free, im);
}

// gd_gip__gip_H
/*
 * call-seq:
 *   square_to_circle(img, radius)
 *
 * The argument _img_ MUST be square, but can have any size. Returns a new image
 * of width and height radius * 2, in which the X axis of the original has been
 * remapped to theta (angle) and the Y axis of the original has been remapped to
 * rho (distance from center). This is known as a "polar coordinate transform."
 *
 * See also Image.stringft_circle, which uses this function internally.
 */
static VALUE image_s_square_to_circle(VALUE klass, VALUE img2, VALUE radius) {
    gdImagePtr im, im2;
    Data_Get_Struct(img2, gdImage, im2);
    im = gdImageSquareToCircle(im2, NUM2INT(radius));
    if (!im) rb_raise(rb_eRGDError, "Not valid Image data");
    return Data_Wrap_Struct(klass, 0, gd_image_free, im);
}

// gd_X__gip_fp
static VALUE gd_image_to_file(VALUE klass, VALUE filename, void (GDFUNC *pfunc)(gdImagePtr, FILE*)) {
    gdImagePtr im;
    FILE* fp;

    fp = fopen(RSTRING_PTR(filename), "wb");
    if (!fp) rb_raise(rb_eIOError, "Cannot open %s for write", RSTRING_PTR(filename));

    Data_Get_Struct(klass, gdImage, im);
    (*pfunc)(im, fp);
    fclose(fp);
    return klass;
}

/*
 * call-seq:
 *   gif(filename)
 *
 * Write the image to the specified file in GIF format.
 */
static VALUE image_gif(VALUE klass, VALUE filename) {
    return gd_image_to_file(klass, filename, gdImageGif);
}

/*
 * call-seq:
 *   gd(filename)
 *
 * Write the image to the specified file in GD format.
 */
static VALUE image_gd(VALUE klass, VALUE filename) {
    return gd_image_to_file(klass, filename, gdImageGd);
}

// gd_X__gip_fp_H
// gd_X__gip_H_fp
static VALUE gd_image_to_file_H(VALUE klass, VALUE filename, VALUE a, void (GDFUNC *pfunc)(gdImagePtr, FILE*, int)) {
    gdImagePtr im;
    FILE* fp;

    Check_Type(a, T_FIXNUM);

    fp = fopen(RSTRING_PTR(filename), "wb");
    if (!fp) rb_raise(rb_eIOError, "Cannot open %s for write", RSTRING_PTR(filename));

    Data_Get_Struct(klass, gdImage, im);
    (*pfunc)(im, fp, NUM2INT(a));
    fclose(fp);
    return klass;
}

/*
 * call-seq:
 *   jpeg(filename, quality = -1)
 *
 * Write the image to the specified file in JPEG format.
 *
 * If quality is negative, the default IJG JPEG quality value (which should
 * yield a good general quality / size tradeoff for most situations) is used.
 * Otherwise, for practical purposes, quality should be a value in the range
 * 0-95, higher quality values usually implying both higher quality and larger
 * image sizes.
 */
static VALUE image_jpeg(int argc, VALUE *argv, VALUE klass) {
    VALUE filename, quality;
    rb_scan_args(argc, argv, "11", &filename, &quality);
    SetIntIfQnil(quality, -1);
    return gd_image_to_file_H(klass, filename, quality, gdImageJpeg);
}

/*
 * call-seq:
 *   png(filename, level = -1)
 *
 * Write the image to the specified file in PNG format.
 *
 * _level_ specified the level of compression to be specified. A compression
 * level of 0 means "no compression." A compression level of 1 means
 * "compressed, but as quickly as possible." A compression level of 9 means
 * "compressed as much as possible to produce the smallest possible file." A
 * compression level of -1 will use the default compression level at the time
 * zlib was compiled on your system.
 */
static VALUE image_png(int argc, VALUE *argv, VALUE klass) {
    VALUE filename, level;
    rb_scan_args(argc, argv, "11", &filename, &level);
    SetIntIfQnil(level, -1);
    return gd_image_to_file_H(klass, filename, level, gdImagePngEx);
}

/*
 * call-seq:
 *   bmp(filename)
 *
 * Write the image to the specified file in BMP format.
 */
static VALUE image_bmp(VALUE klass, VALUE filename) {
    return gd_image_to_file_H(klass, filename, 1, gdImageBmp);
}

void GDFUNC gd_gdImageWBMP(gdImagePtr im, FILE* out, int fg) {
    gdImageWBMP(im, fg, out);
}

/*
 * call-seq:
 *   wbmp(filename, fg = 1)
 *
 * Write the image to the specified file in WBMP format.
 *
 * WBMP file support is black and white only. The color index specified by
 * the _fg_ argument is the "foreground," and only pixels of this color will be
 * set in the WBMP file. All other pixels will be considered "background."
 */
static VALUE image_wbmp(int argc, VALUE *argv, VALUE klass) {
    VALUE filename, fg;
    rb_scan_args(argc, argv, "11", &filename, &fg);
    SetIntIfQnil(fg, 1);
    return gd_image_to_file_H(klass, filename, fg, gd_gdImageWBMP);
}

// gd_X__gip_fp_H2
/*
 * call-seq:
 *   gd2(filename, chunk_size = 0, fmt = GD2_FMT_COMPRESSED)
 *
 * Write the image to the specified file in GD2 format.
 *
 * The gd2 image format is intended for fast reads and writes of parts of
 * images. It is a compressed format (when _fmt_ is GD2_FMT_COMPRESSED), and
 * well suited to retrieving smll sections of much larger images.
 *
 * The file is stored as a series of compressed subimages, and the _chunk_size_
 * determines the sub-image size - a value of zero causes the GD library to use
 * the default.
 *
 * It is also possible to store GD2 files in an uncompressed format, in which
 * case _fmt_ should be GD2_FMT_RAW.
 */
static VALUE image_gd2(int argc, VALUE *argv, VALUE klass) {
    VALUE filename, chunk_size, fmt;
    gdImagePtr im;
    FILE* fp;

    rb_scan_args(argc, argv, "12", &filename, &chunk_size, &fmt);
    SetIntIfQnil(chunk_size, 0);
    SetIntIfQnil(fmt, GD2_FMT_COMPRESSED);
    Check_Type(chunk_size, T_FIXNUM);
    Check_Type(fmt, T_FIXNUM);

    fp = fopen(RSTRING_PTR(filename), "wb");
    if (!fp) rb_raise(rb_eIOError, "Cannot open %s for write", RSTRING_PTR(filename));

    Data_Get_Struct(klass, gdImage, im);
    gdImageGd2(im, fp, NUM2INT(chunk_size), NUM2INT(fmt));
    fclose(fp);
    return klass;
}

// gd_PAX__gip_PAH
static VALUE gd_image_to_data(VALUE klass, void* (GDFUNC *pfunc)(gdImagePtr, int*)) {
    VALUE data;
    gdImagePtr im;
    char* buf;
    int len;

    Data_Get_Struct(klass, gdImage, im);
    buf = (*pfunc)(im, &len);
    data = rb_str_new(buf, len);
    gdFree(buf);
    return data;
}

/*
 * call-seq:
 *   gif_data()
 *
 * Convert the image to a GIF format byte-string.
 */
static VALUE image_gif_data(VALUE klass) {
    return gd_image_to_data(klass, gdImageGifPtr);
}

/*
 * call-seq:
 *   gd_data()
 *
 * Convert the image to a GD format byte-string.
 */
static VALUE image_gd_data(VALUE klass) {
    return gd_image_to_data(klass, gdImageGdPtr);
}

// gd_PAX__gip_PAH_H
static VALUE gd_image_to_data_H(VALUE klass, VALUE a, void* (GDFUNC *pfunc)(gdImagePtr, int*, int)) {
    VALUE data;
    gdImagePtr im;
    char* buf;
    int len;

    Data_Get_Struct(klass, gdImage, im);
    buf = (*pfunc)(im, &len, NUM2INT(a));
    data = rb_str_new(buf, len);
    gdFree(buf);
    return data;
}

/*
 * call-seq:
 *   jpeg_data(quality = -1)
 *
 * Convert the image to a JPEG format byte-string.
 *
 * Read Image.jpeg for more.
 */
static VALUE image_jpeg_data(int argc, VALUE *argv, VALUE klass) {
    VALUE a;
    rb_scan_args(argc, argv, "01", &a);
    SetIntIfQnil(a, -1);
    return gd_image_to_data_H(klass, a, gdImageJpegPtr);
}

/*
 * call-seq:
 *   png_data(level = -1)
 *
 * Convert the image to a PNG format byte-string.
 *
 * Read Image.png for more.
 */
static VALUE image_png_data(int argc, VALUE *argv, VALUE klass) {
    VALUE a;
    rb_scan_args(argc, argv, "01", &a);
    SetIntIfQnil(a, -1);
    return gd_image_to_data_H(klass, a, gdImagePngPtrEx);
}

/*
 * call-seq:
 *   wbmp_data(fg = 1)
 *
 * Convert the image to a WBMP format byte-string.
 *
 * Read Image.wbmp for more.
 */
static VALUE image_wbmp_data(int argc, VALUE *argv, VALUE klass) {
    VALUE a;
    rb_scan_args(argc, argv, "01", &a);
    SetIntIfQnil(a, 1);
    return gd_image_to_data_H(klass, a, gdImageWBMPPtr);
}

/*
 * call-seq:
 *   bmp_data()
 *
 * Convert the image to a BMP format byte-string.
 */
static VALUE image_bmp_data(VALUE klass) {
    return gd_image_to_data_H(klass, INT2NUM(1), gdImageBmpPtr);
}

// gd_PAX__gip_PAH_H2
// gd_PAX__gip_H2_PAH
static VALUE gd_image_to_data_H2(VALUE klass, VALUE a, VALUE b, void* (GDFUNC *pfunc)(gdImagePtr, int*, int, int)) {
    VALUE data;
    gdImagePtr im;
    char* buf;
    int len;

    Data_Get_Struct(klass, gdImage, im);
    buf = (*pfunc)(im, &len, NUM2INT(a), NUM2INT(b));
    data = rb_str_new(buf, len);
    gdFree(buf);
    return data;
}

void* GDFUNC gd_gdImageGd2Ptr(gdImagePtr im, int *size, int cs, int fmt) {
    gdImageGd2Ptr(im, cs, fmt, size);
}

/*
 * call-seq:
 *   gd2_data(chunk_size = 0, fmt = GD2_FMT_COMPRESSED)
 *
 * Convert the image to a GD2 format byte-string.
 *
 * Read Image.gd2 for more.
 */
static VALUE image_gd2_data(int argc, VALUE *argv, VALUE klass) {
    VALUE a, b;
    rb_scan_args(argc, argv, "02", &a, &b);
    SetIntIfQnil(a, 0);
    SetIntIfQnil(b, GD2_FMT_COMPRESSED);
    return gd_image_to_data_H2(klass, a, b, gd_gdImageGd2Ptr);
}

/*
 * call-seq:
 *   gif_anim_begin_data(global_cm = -1, loops = 0)
 *
 * This function must be called as the first function when creating a GIF
 * animation. It returns a byte-string for the correct GIF file headers,
 * and prepares for frames to be added for the animation. It's *NOT* used to
 * produce an image frame of GIF file, it is only used to establish the GIF
 * animation frame size, interlacing options and the color palette.
 * Image.gif_anim_add_data is used to make the first and subsequent frames
 * to the animation, and the animation must be terminated by writing a semicolon
 * character (;) to it or by using Image.gif_anim_end_data to get that.
 *
 * The global_cm flag indicates if a global color map (or palette) is used in
 * the GIF89A header. A nonzero value specifies that a global color map should
 * be used to reduce the size of the animation. Of course, if the color maps of
 * individual frames differ greatly, a global color map may not be a good idea.
 * global_cm=1 means write global color map, global_cm=0 means do not, and
 * global_cm=-1 means to do the default, which currently is to use a global
 * color map.
 *
 * If loops is 0 or greater, the Netscape 2.0 extension for animation loop count
 * is written. 0 means infinite loop count. -1 means that the extension is not
 * added which results in no looping.
 */
static VALUE image_gif_anim_begin_data(int argc, VALUE *argv, VALUE klass) {
    VALUE a, b;
    rb_scan_args(argc, argv, "02", &a, &b);
    SetIntIfQnil(a, -1);
    SetIntIfQnil(b, 0);
    return gd_image_to_data_H2(klass, a, b, gdImageGifAnimBeginPtr);
}

// gd_PAX__gip_PAzH_H5_gip
/*
 * call-seq:
 *   gif_anim_add_data(local_cm = 1, left = 0, top = 0, delay = 5, disposal = DISPOSAL_NONE, prev_image = nil)
 *
 * This function returns GIF animation frames to GIF animation, which was
 * initialized with Image.gif_anim_begin_data.
 *
 * With _left_ and _top_ you can place this frame in different offset than (0,0)
 * inside the image screen as defined in Image.gif_anim_begin_data.
 *
 * The argument _delay_ between the previous frame and this frame is in 1/100s
 * units.
 *
 * The argument _disposal_ is usually DISPOSAL_NONE, meaning that the pixels
 * changed by this frame should remain on the display when the next frame begins
 * to render, but can also be DISPOSAL_RESTORE_BACKGROUND (restores the first
 * allocated color of the global palette), or DISPOSAL_RESTORE_PREVIOUS
 * (restores the appearance of the affected area before the frame was rendered).
 * Only DISPOSAL_NONE is a sensible choice for the first frame.
 *
 * If _prev_im_ is not nil, the built-in GIF optimizer will always use
 * DISPOSAL_NONE regardless of the Disposal parameter.
 */
static VALUE image_gif_anim_add_data(int argc, VALUE *argv, VALUE klass) {
    VALUE data, local_cm, left, top, delay, disposal, prev_image;
    gdImagePtr im, im2 = NULL;
    char* buf;
    int len;

    rb_scan_args(argc, argv, "06", &local_cm, &left, &top, &delay, &disposal, &prev_image);
    SetIntIfQnil(local_cm, 1);
    SetIntIfQnil(left, 0);
    SetIntIfQnil(top, 0);
    SetIntIfQnil(delay, 5);
    SetIntIfQnil(disposal, gdDisposalNone);
    if RTEST(prev_image) Data_Get_Struct(prev_image, gdImage, im2);

    Data_Get_Struct(klass, gdImage, im);
    buf = gdImageGifAnimAddPtr(im, &len, NUM2INT(local_cm), NUM2INT(left), NUM2INT(top), NUM2INT(delay), NUM2INT(disposal), im2);
    data = rb_str_new(buf, len);
    gdFree(buf);
    return data;
}

// gd_PAX__PAH
/*
 * call-seq:
 *   gif_anim_end_data()
 *
 * Returns a one byte string containing the semicolon character (;). The string
 * ";" can be used in place of this function.
 */
static VALUE image_gif_anim_end_data(VALUE klass) {
    VALUE data;
    char* buf;
    int len;

    buf = gdImageGifAnimEndPtr(&len);
    data = rb_str_new(buf, len);
    gdFree(buf);
    return data;
}

// gd_H__gip2
/*
 * call-seq:
 *   compare(image)
 *
 * Returns a bitmap indicating if the two images are different. The members of
 * the bitmap are defined as GD_CMP_*, but the most important is GD_CMP_IMAGE,
 * which indicated that the images will actually appear different when displayed.
 * Other, less important, differences relate to pallette entries. Any difference
 * in the transparent color is assumed to make images display differently, even
 * if the transparent color is not used.
 */
static VALUE image_compare(VALUE klass, VALUE image) {
    gdImagePtr im, im2;
    Data_Get_Struct(klass, gdImage, im);
    Data_Get_Struct(image, gdImage, im2);
    return INT2NUM(gdImageCompare(im, im2));
}

/*
 * call-seq:
 *   width()
 *
 * Get the width of the image in pixels.
 */
static VALUE image_sx(VALUE klass) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    return INT2NUM(gdImageSX(im));
}

/*
 * call-seq:
 *   height()
 *
 * Get the height of the image in pixels.
 */
static VALUE image_sy(VALUE klass) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    return INT2NUM(gdImageSY(im));
}

/*
 * call-seq:
 *   truecolor?()
 *
 * Return true if the image is a truecolor image, false for palette-based image.
 */
static VALUE image_is_truecolor(VALUE klass) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    return gdImageTrueColor(im) != 0 ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *   colors_total()
 *
 * Returns the number of colors currently allocated in a palette image.
 */
static VALUE image_colors_total(VALUE klass) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    return INT2NUM(gdImageColorsTotal(im));
}

/*
 * call-seq:
 *   interlace()
 *
 * Read Image.interlace= for details.
 */
static VALUE image_get_interlace(VALUE klass) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    return gdImageGetInterlaced(im) ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *   transparent()
 *
 * Returns the current transparent color index in the image. If there is no
 * transparent color returns -1.
 *
 */
static VALUE image_transparent_get(VALUE klass) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    return INT2NUM(gdImageGetTransparent(im));
}

// gd_H__gip_H
/*
 * call-seq:
 *   r, g, b, a = rgba(color)
 *
 * Returns a array containing the red, green, blue, alpha components of the
 * specified color (for truecolor image) or color index (for palette-based
 * image).
 */
static VALUE image_rgba(VALUE klass, VALUE color) {
    gdImagePtr im;
    int c = NUM2INT(color);
    Data_Get_Struct(klass, gdImage, im);
    return rb_ary_new3(4, INT2NUM(gdImageRed(im, c)), INT2NUM(gdImageGreen(im, c)), INT2NUM(gdImageBlue(im, c)), INT2NUM(gdImageAlpha(im, c)));
}

static VALUE gd_H__gip_H2(VALUE klass, VALUE a, VALUE b, int (GDFUNC *pfunc)(gdImagePtr, int, int)) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    return INT2NUM((*pfunc)(im, NUM2INT(a), NUM2INT(b)));
}

/*
 * call-seq:
 *   slef[x, y]
 *
 * Get the color index or the color values of a particular pixel.
 *
 * A color index is returned when the image is palette based (created by
 * Image.create), the color value is returned when the image is a true
 * color image (created by Image.create_truecolor). To fetch the value of
 * each channel, you can use Image.rgba.
 */
static VALUE image_get_pixel(VALUE klass, VALUE x, VALUE y) {
    return gd_H__gip_H2(klass, x, y, gdImageGetPixel);
}

/*
 * call-seq:
 *   bounds_safe(x, y)
 *
 * Returns true if the specified point is within the current clipping rectangle,
 * false if not. The clipping rectangle is set by Image.clip= and defaults to
 * the entire image. This function is intended primarily for use by those who
 * wish to add functions to gd.
 */
static VALUE image_bounds_safe(VALUE klass, VALUE x, VALUE y) {
    return gd_H__gip_H2(klass, x, y, gdImageBoundsSafe) == INT2NUM(0) ? Qfalse : Qtrue;
}

// gd_H__gip_H3
/*
 * call-seq:
 *   color_closest_hwb(args)
 *
 * Usage:
 * 
 * * color_closest_hwb(r, g, b)
 * * color_closest_hwb(integer)
 * * color_closest_hwb(color_name)
 * 
 * Read Image.color_allocate for more about usage.
 *
 * Searches the colors which have been defined thus far in the image specified
 * and returns the index of the color with hue, whiteness and blackness closest
 * to the requested color. This scheme is typically superior to the Euclidian
 * distance scheme used by Image.color_closest.
 *
 * If no colors have yet been allocated in the image, this method returns -1.
 * When applied to a truecolor image, this function always succeeds in
 * returning the desired color.
 *
 * This function is most useful as a backup method for choosing a drawing color
 * when an image already contains MAX_COLORS (256) colors and no more can be
 * allocated. (This is not uncommon when working with existing PNG files that
 * already use many colors.) See Image.color_exact for a method of
 * locating exact matches only.
 */
static VALUE image_color_closest_hwb(int argc, VALUE *argv, VALUE klass) {
    gdImagePtr im;
    VALUE r = m_scan_color_args(argc, argv);
    Data_Get_Struct(klass, gdImage, im);
    return INT2NUM(gdImageColorClosestHWB(im, NUM2INT(rb_ary_entry(r, 0)), NUM2INT(rb_ary_entry(r, 1)), NUM2INT(rb_ary_entry(r, 2))));
}

// gd_H__gip_H4
static VALUE gd_image_color_func_alpha(int argc, VALUE *argv, VALUE klass, int (GDFUNC *pfunc)(gdImagePtr, int, int, int, int)) {
    gdImagePtr im;
    VALUE r = m_scan_color_args(argc, argv);
    Data_Get_Struct(klass, gdImage, im);
    return INT2NUM((*pfunc)(im, NUM2INT(rb_ary_entry(r, 0)), NUM2INT(rb_ary_entry(r, 1)), NUM2INT(rb_ary_entry(r, 2)), NUM2INT(rb_ary_entry(r, 3))));
}

/*
 * call-seq:
 *   color_allocate(args)
 *
 * Usage:
 *
 * * color_allocate(r, g, b, a = ALPHA_OPAQUE)
 * * color_allocate(integer)
 * * color_allocate(color_name)
 *
 * Red, green and blue can be anything upto 255, and 127 represents full
 * transparency for alpha.
 *
 * _color_name_ can be any one of 147 color names are defined in the HTML and
 * CSS color specification.
 *
 * This method finds the first available color index in the image specified,
 * sets its RGBA values to those requested, and returns the index of the
 * new color table entry, or an RGBA value in the case of a truecolor image;
 * in either case you can then use the returned value as a parameter to drawing
 * functions. When creating a new palette-based image, the first time you
 * invoke this function, you are setting the background color for that image.
 *
 * In the event that all MAX_COLORS colors (256) have already been allocated,
 * this method will return -1 to indicate failure. (This is not uncommon when
 * working with existing palette-based PNG files that already use 256 colors.)
 * Note that this method does not check for existing colors that match your
 * request; see Image.color_exact and Image.color_closest
 * for ways to locate existing colors that approximate the color desired in
 * situations where a new color is not available.Also see
 * Image.color_resolve.
 */
static VALUE image_color_allocate(int argc, VALUE *argv, VALUE klass) {
    return gd_image_color_func_alpha(argc, argv, klass, gdImageColorAllocateAlpha);
}

/*
 * call-seq:
 *   color_closest(args)
 *
 * Read Image.color_allocate for more about args.
 *
 * Searches the colors which have been defined thus far in the image specified
 * and returns the index of the color with RGB values closest to those of the
 * request. (Closeness is determined by Euclidian distance, which is used to
 * determine the distance in three-dimensional color space between colors.)
 *
 * If no colors have yet been allocated in the image, this method returns -1.
 * When applied to a truecolor image, this function always succeeds in
 * returning the desired color.
 *
 * This function is most useful as a backup method for choosing a drawing color
 * when an image already contains MAX_COLORS (256) colors and no more can be
 * allocated. (This is not uncommon when working with existing PNG files that
 * already use many colors.) See Image.color_exact for a method of
 * locating exact matches only.
 *
 */
static VALUE image_color_closest(int argc, VALUE *argv, VALUE klass) {
    return gd_image_color_func_alpha(argc, argv, klass, gdImageColorClosestAlpha);
}

/*
 * call-seq:
 *   color_exact(args)
 *
 * Read Image.color_allocate for more about args.
 *
 * Searches the colors which have been defined thus far in the image specified
 * and returns the index of the color with RGBA values closest to those of the
 * request. (Closeness is determined by Euclidian distance, which is used to
 * determine the distance in four-dimensional color/alpha space between colors.)
 *
 * If no colors have yet been allocated in the image, this method returns -1.
 * When applied to a truecolor image, this function always succeeds in returning
 * the desired color.
 *
 * This function is most useful as a backup method for choosing a drawing color
 * when a palette-based image already contains MAX_COLORS (256) colors and no
 * more can be allocated. (This is not uncommon when working with existing
 * palette-based PNG files that already use many colors.)
 */
static VALUE image_color_exact(int argc, VALUE *argv, VALUE klass) {
    return gd_image_color_func_alpha(argc, argv, klass, gdImageColorExactAlpha);
}

/*
 * call-seq:
 *   color_resolve(args)
 *
 * Read Image.color_allocate for more about args.
 *
 * Searches the colors which have been defined thus far in the image specified
 * and returns the index of the first color with RGBA values which exactly match
 * those of the request. If no allocated color matches the request precisely,
 * then thid medhos tries to allocate the exact color. If there is no space left
 * in the color table then this method returns the closest color (as in
 * Image.color_closest). This function always returns an index of a
 * color.
 *
 * When applied to a truecolor image, this function always succeeds in returning
 * the desired color.
 */
static VALUE image_color_resolve(int argc, VALUE *argv, VALUE klass) {
    return gd_image_color_func_alpha(argc, argv, klass, gdImageColorResolveAlpha);
}

static VALUE gd_X__gip2(VALUE klass, VALUE img2, void (GDFUNC *pfunc)(gdImagePtr, gdImagePtr)) {
    gdImagePtr im, im2;
    Data_Get_Struct(klass, gdImage, im);
    Data_Get_Struct(img2, gdImage, im2);
    (*pfunc)(im, im2);
    return klass;
}

/*
 * call-seq:
 *   brush = img
 *
 * A "brush" is an image used to draw wide, shaped strokes in another image.
 * Just as a paintbrush is not a single point, a brush image need not be a
 * single pixel. Any gd image can be used as a brush, and by setting the
 * transparent color index of the brush image with Image.transparent=, a brush
 * of any shape can be created. All line-drawing functions, such as Image.line,
 * Image.open_polygon and Image.polygon, will use the current brush if the
 * special "color" COLOR_BRUSHED or COLOR_STYLED_BRUSHED is used when calling
 * them.
 *
 * This method is used to specify the brush to be used in a particular image.
 * You can set any image to be the brush. If the brush image does not have the
 * same color map as the first image, any colors missing from the first image
 * will be allocated. If not enough colors can be allocated, the closest colors
 * already available will be used. This allows arbitrary PNGs to be used as
 * brush images. It also means, however, that you should not set a brush unless
 * you will actually use it; if you set a rapid succession of different brush
 * images, you can quickly fill your color map, and the results will not be
 * optimal.
 */
static VALUE image_set_brush(VALUE klass, VALUE img2) {
    return gd_X__gip2(klass, img2, gdImageSetBrush);
}

/*
 * call-seq:
 *   tile = img
 *
 * A "tile" is an image used to fill an area with a repeated pattern. Any gd
 * image can be used as a tile, and by setting the transparent color index of
 * the tile image with Image.transparent=, a tile that allows certain parts of
 * the underlying area to shine through can be created. All region-filling
 * functions, such as Image.fill and Image.filled_polygon, will use the current
 * tile if the special "color" COLOR_TILED is used when calling them.
 *
 * This is method is used to specify the tile to be used in a particular image.
 * You can set any image to be the tile. If the tile image does not have the
 * same color map as the first image, any colors missing from the first image
 * will be allocated. If not enough colors can be allocated, the closest colors
 * already available will be used. This allows arbitrary PNGs to be used as tile
 * images. It also means, however, that you should not set a tile unless you
 * will actually use it; if you set a rapid succession of different tile images,
 * you can quickly fill your color map, and the results will not be optimal.
 */
static VALUE image_set_tile(VALUE klass, VALUE img2) {
    return gd_X__gip2(klass, img2, gdImageSetTile);
}

/*
 * call-seq:
 *   copy_palette(src)
 *
 * Copies a palette from an image, attempting to match the colors in the target
 * image to the colors in the source palette.
 */
static VALUE image_copy_palette(VALUE klass, VALUE img2) {
    return gd_X__gip2(klass, img2, gdImagePaletteCopy);
}

// gd_X__gip2_H6
/*
 * call-seq:
 *   copy(src, dstX, dstY, srcX, srcY, w, h)
 *
 * Used to copy a rectangular portion of one image to another image. (For a way
 * of stretching or shrinking the image in the process, see
 * Image.copy_resized.)
 *
 * The dst argument is the destination image to which the region will be copied.
 * The src argument is the source image from which the region is copied. The
 * dstX and dstY arguments specify the point in the destination image to which
 * the region will be copied. The srcX and srcY arguments specify the upper left
 * corner of the region in the source image. The w and h arguments specify the
 * width and height of the region.
 *
 * When you copy a region from one location in an image to another location in
 * the same image, this method will perform as expected unless the regions
 * overlap, in which case the result is unpredictable.
 *
 * Important note on copying between images: since different images do not
 * necessarily have the same color tables, pixels are not simply set to the
 * same color index values to copy them. This method will attempt to find an
 * identical RGB value in the destination image for each pixel in the copied
 * portion of the source image by invoking Image.color_exact. If such a
 * value is not found, this method will attempt to allocate colors as needed
 * using Image.color_allocate. If both of these methods fail, this method
 * will invoke Image.color_closest to find the color in the destination
 * image which most closely approximates the color of the pixel being copied.
 */
static VALUE image_copy(VALUE klass, VALUE srcImg, VALUE dstX, VALUE dstY, VALUE srcX, VALUE srcY, VALUE w, VALUE h) {
    gdImagePtr dst, src;
    Data_Get_Struct(klass, gdImage, dst);
    Data_Get_Struct(srcImg, gdImage, src);
    gdImageCopy(dst, src, NUM2INT(dstX), NUM2INT(dstY), NUM2INT(srcX), NUM2INT(srcY), NUM2INT(w), NUM2INT(h));
    return klass;
}

static VALUE gd_X__gip2_H7(VALUE klass, VALUE img2, VALUE a, VALUE b, VALUE c, VALUE d, VALUE e, VALUE f, VALUE g, void (GDFUNC *pfunc)(gdImagePtr, gdImagePtr, int, int, int, int, int, int, int)) {
    gdImagePtr im, im2;
    Data_Get_Struct(klass, gdImage, im);
    Data_Get_Struct(img2, gdImage, im2);
    (*pfunc)(im, im2, NUM2INT(a), NUM2INT(b), NUM2INT(c), NUM2INT(d), NUM2INT(e), NUM2INT(f), NUM2INT(g));
    return klass;
}

/*
 * call-seq:
 *   copy_merge(src, dstX, dstY, srcX, srcY, w, h, pct)
 *
 * This method is almost identical to GD::Image.copy, except that it 'merges'
 * the two images by an amount specified in the last parameter. If the last
 * parameter is 100, then it will function identically to GD::Image.copy - the
 * source image replaces the pixels in the destination.
 *
 * If, however, the pct parameter is less than 100, then the two images are
 * merged. With pct = 0, no action is taken.
 *
 * This feature is most useful to 'highlight' sections of an image by merging a
 * solid color with pct = 50:
 *
 *   dst.copy_merge(src, 100, 200, 0, 0, 30, 50, 50)
 */
static VALUE image_copy_merge(VALUE klass, VALUE srcImg, VALUE dstX, VALUE dstY, VALUE srcX, VALUE srcY, VALUE w, VALUE h, VALUE pct) {
    return gd_X__gip2_H7(klass, srcImg, dstX, dstY, srcX, srcY, w, h, pct, gdImageCopyMerge);
}

/*
 * call-seq:
 *   copy_merge_gray(src, dstX, dstY, srcX, srcY, w, h, pct)
 *
 * This method is almost identical to GD::Image.copy_merge, except that when
 * merging images it preserves the hue of the source by converting the
 * destination pixels to grey scale before the copy operation.
 */
static VALUE image_copy_merge_gray(VALUE klass, VALUE srcImg, VALUE dstX, VALUE dstY, VALUE srcX, VALUE srcY, VALUE w, VALUE h, VALUE pct) {
    return gd_X__gip2_H7(klass, srcImg, dstX, dstY, srcX, srcY, w, h, pct, gdImageCopyMergeGray);
}

static VALUE gd_X__gip2_H8(VALUE klass, VALUE img2, VALUE a, VALUE b, VALUE c, VALUE d, VALUE e, VALUE f, VALUE g, VALUE h, void (GDFUNC *pfunc)(gdImagePtr, gdImagePtr, int, int, int, int, int, int, int, int)) {
    gdImagePtr im, im2;
    Data_Get_Struct(klass, gdImage, im);
    Data_Get_Struct(img2, gdImage, im2);
    (*pfunc)(im, im2, NUM2INT(a), NUM2INT(b), NUM2INT(c), NUM2INT(d), NUM2INT(e), NUM2INT(f), NUM2INT(g), NUM2INT(h));
    return klass;
}

/*
 * call-seq:
 *   copy_resized(src, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH)
 *
 * Used to copy a rectangular portion of one image to another image. The X and Y
 * dimensions of the original region and the destination region can vary,
 * resulting in stretching or shrinking of the region as appropriate. (For a
 * simpler version of this function which does not deal with resizing,
 * see Image.copy.)
 *
 * The dst argument is the destination image to which the region will be copied.
 * The src argument is the source image from which the region is copied. The
 * dstX and dstY arguments specify the point in the destination image to which
 * the region will be copied. The srcX and srcY arguments specify the upper left
 * corner of the region in the source image. The dstW and dstH arguments specify
 * the width and height of the destination region. The srcW and srcH arguments
 * specify the width and height of the source region and can differ from the
 * destination size, allowing a region to be scaled during the copying process.
 *
 * When you copy a region from one location in an image to another location in
 * the same image, Image.copy will perform as expected unless the regions
 * overlap, in which case the result is unpredictable. If this presents a
 * problem, create a scratch image in which to keep intermediate results.
 *
 * Important note on copying between images: since images do not necessarily
 * have the same color tables, pixels are not simply set to the same color index
 * values to copy them. Image.copy will attempt to find an identical RGB
 * value in the destination image for each pixel in the copied portion of the
 * source image by invoking Image.color_exact. If such a value is not
 * found, Image.copy will attempt to allocate colors as needed
 * using Image.color_allocate. If both of these methods fail,
 * Image.copy will invoke Image.color_closest to find the color in the
 * destination image which most closely approximates the color of the pixel
 * being copied.

 */
static VALUE image_copy_resized(VALUE klass, VALUE srcImg, VALUE dstX, VALUE dstY, VALUE srcX, VALUE srcY, VALUE dstW, VALUE dstH, VALUE srcW, VALUE srcH) {
    return gd_X__gip2_H8(klass, srcImg, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH, gdImageCopyResized);
}

/*
 * call-seq:
 *   copy_resampled(src, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH)
 *
 * Used to copy a rectangular portion of one image to another image, smoothly
 * interpolating pixel values so that, in particular, reducing the size of an
 * image still retains a great deal of clarity. The X and Y dimensions of the
 * original region and the destination region can vary, resulting in stretching
 * or shrinking of the region as appropriate. (For a simpler version of this
 * function which does not deal with resizing, see GD::Image.copy. For a version
 * which does not interpolate pixel values, see GD::Image.copy_resized.
 *
 * Pixel values are only interpolated if the destination image is a truecolor
 * image. Otherwise, GD::Image.copy_resized is automatically invoked.
 *
 * The dst argument is the destination image to which the region will be copied.
 * The src argument is the source image from which the region is copied. The
 * dstX and dstY arguments specify the point in the destination image to which
 * the region will be copied. The srcX and srcY arguments specify the upper left
 * corner of the region in the source image. The dstW and dstH arguments specify
 * the width and height of the destination region. The srcW and srcH arguments
 * specify the width and height of the source region and can differ from the
 * destination size, allowing a region to be scaled during the copying process.
 *
 * When you copy a region from one location in an image to another location in
 * the same image, GD::Image.copy will perform as expected unless the regions
 * overlap, in which case the result is unpredictable. If this presents a
 * problem, create a scratch image in which to keep intermediate results.
 *
 * Important note on copying between images: since images do not necessarily
 * have the same color tables, pixels are not simply set to the same color
 * index values to copy them. If the destination image is a palette image,
 * gd will use the Image.color_resolve function to determine the best color
 * available.
 */
static VALUE image_copy_resampled(VALUE klass, VALUE srcImg, VALUE  dstX, VALUE  dstY, VALUE  srcX, VALUE  srcY, VALUE  dstW, VALUE  dstH, VALUE  srcW, VALUE  srcH) {
    return gd_X__gip2_H8(klass, srcImg, dstX, dstY, srcX, srcY, dstW, dstH, srcW, srcH, gdImageCopyResampled);
}

// gd_X__gip2_N2_H5
/*
 * call-seq:
 *   copy_rotated(src, dstX, dstY, srcX, srcY, srcW, srcH, angle)
 *
 * Used to copy a rectangular portion of one image to another image, or to
 * another region of the same image. The srcX and srcY coordinates specify the
 * upper left corner of the source area; however, the dstX and dstY coordinates
 * specify the CENTER of the destination area. This important distinction is
 * made because the rotated rectangle may may or may not be parallel to the X
 * and Y axes. The destination coordinates may be floating point, as the center
 * of the desired destination area may lie at the center of a pixel (0.5 pixels)
 * rather than its upper left corner. The angle specified is an integer number
 * of degrees, between 0 and 360, with 0 degrees causing no change, and
 * counterclockwise rotation as the angle increases.
 *
 * When you copy a region from one location in an image to another location in
 * the same image, this method will perform as expected unless the regions
 * overlap, in which case the result is unpredictable. If this presents a
 * problem, create a scratch image in which to keep intermediate results.
 *
 * Important note on copying between images: since palette-based images do not
 * necessarily have the same color tables, pixels are not simply set to the same
 * color index values to copy them. If the destination image is not a truecolor
 * image, Image.color_resolve is used to choose the destination pixel.
 */
static VALUE image_copy_rotated(VALUE klass, VALUE srcImg, VALUE dstX, VALUE dstY, VALUE srcX, VALUE srcY, VALUE srcW, VALUE srcH, VALUE angle) {
    gdImagePtr dst, src;
    Data_Get_Struct(klass, gdImage, dst);
    Data_Get_Struct(srcImg, gdImage, src);
    gdImageCopyRotated(dst, src, NUM2DBL(dstX), NUM2DBL(dstY), NUM2INT(srcX), NUM2INT(srcY), NUM2INT(srcW), NUM2INT(srcH), NUM2INT(angle));
    return klass;
}

static VALUE gd_X__gip_H(VALUE klass, VALUE a, void (GDFUNC *pfunc)(gdImagePtr, int)) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    (*pfunc)(im, NUM2INT(a));
    return a;
}

/*
 * call-seq:
 *   interlace = boolean
 *
 * Used to determine whether an image should be stored in a linear fashion, in
 * which lines will appear on the display from first to last, or in an
 * interlaced fashion, in which the image will "fade in" over several passes.
 * By default, images are not interlaced. (When writing JPEG images, interlacing
 * implies generating progressive JPEG files, which are represented as a series
 * of scans of increasing quality. Noninterlaced gd images result in regular
 * (sequential) JPEG data streams.)
 *
 * Note that interlace has no effect on other functions, and has no meaning
 * unless you save the image in PNG or JPEG format.
 */
static VALUE image_interlace_set(VALUE klass, VALUE interlace) {
    return gd_X__gip_H(klass, RTEST(interlace) ? INT2NUM(1) : INT2NUM(0), gdImageInterlace);
}

/*
 * call-seq:
 *   antialiased = color
 *
 * "Antialiasing" is a process by which jagged edges associated with line
 * drawing can be reduced by blending the foreground color with an appropriate
 * percentage of the background, depending on how much of the pixel in question
 * is actually within the boundaries of the line being drawn. All line-drawing
 * functions, such as Image.line, Image.open_polygon and Image.polygon, will
 * draw antialiased lines if the special "color" COLOR_ANTIALIASED is used when
 * calling them.
 *
 * this method is used to specify the actual foreground color to be used when
 * drawing antialiased lines. You may set any color to be the foreground,
 * however as of version 2.0.12 an alpha channel component is not supported.
 *
 * Antialiased lines can be drawn on both truecolor and palette-based images.
 * However, attempts to draw antialiased lines on highly complex palette-based
 * backgrounds may not give satisfactory results, due to the limited number of
 * colors available in the palette. Antialiased line-drawing on simple
 * backgrounds should work well with palette-based images; otherwise create or
 * fetch a truecolor image instead.
 */
static VALUE image_set_antialiased(VALUE klass, VALUE color){
    return gd_X__gip_H(klass, color, gdImageSetAntiAliased);
}

/*
 * call-seq:
 *   thickness = width
 *
 * This method determines the width of lines drawn by the Image.line,
 * Image.polygon, Image.open_polygon and related methods, in pixels.
 */
static VALUE image_set_thickness(VALUE klass, VALUE thickness) {
    return gd_X__gip_H(klass, thickness, gdImageSetThickness);
}

/*
 * call-seq:
 *   alpha_blending = boolean
 *
 * This method allows for two different modes of drawing on truecolor images. In
 * blending mode, which is on by default (gd 2.0.2 and above), the alpha channel
 * component of the color supplied to all drawing functions, such as
 * Image.pixels[]=, determines how much of the underlying color should be
 * allowed to shine through. As a result, gd automatically blends the existing
 * color at that point with the drawing color, and stores the result in the
 * image. The resulting pixel is opaque. In non-blending mode, the drawing color
 * is copied literally with its alpha channel information, replacing the
 * destination pixel. Blending mode is not available when drawing on palette
 * images.
 */
static VALUE image_set_alpha_blending(VALUE klass, VALUE blending) {
    return gd_X__gip_H(klass, RTEST(blending) ? INT2NUM(1) : INT2NUM(0) , gdImageAlphaBlending);
}

/*
 * call-seq:
 *   save_alpha = boolean
 *
 * By default, gd 2.0.2 and above do not attempt to save full alpha channel
 * information (as opposed to single-color transparency) when saving PNG images.
 * (PNG is currently the only output format supported by gd which can
 * accommodate alpa channel information.) This saves space in the output file.
 * If you wish to create an image with alpha channel information for use with
 * tools that support it, set save_alpha=true to turn on saving of such
 * information, and set alpha_blending=false to turn off alpha blending within
 * the library so that alpha channel information is actually stored in the image
 * rather than being composited immediately at the time that drawing functions
 * are invoked.
 */
static VALUE image_save_alpha(VALUE klass, VALUE save) {
    return gd_X__gip_H(klass, RTEST(save) ? INT2NUM(1) : INT2NUM(0), gdImageSaveAlpha);
}

/*
 * call-seq:
 *   transparent = color
 *
 * Set the transparent color index for the specified image to the specified
 * index. To indicate that there should be no transparent color, invoke this
 * method with a color index of -1. Note that JPEG images do not support
 * transparency, so this setting has no effect when writing JPEG images.
 *
 * The color index used should be an index allocated by Image.color_allocate,
 * whether explicitly invoked by your code or implicitly invoked by loading an
 * image. In order to ensure that your image has a reasonable appearance when
 * viewed by users who do not have transparent background capabilities (or when
 * you are writing a JPEG-format file, which does not support transparency), be
 * sure to give reasonable RGB values to the color you allocate for use as a
 * transparent color, even though it will be transparent on systems that support
 * PNG transparency.
 */
static VALUE image_color_transparent(VALUE klass, VALUE color) {
    return gd_X__gip_H(klass, color, gdImageColorTransparent);
}

/*
 * call-seq:
 *   color_deallocate(color)
 *
 * Marks the specified color as being available for reuse. It does not attempt
 * to determine whether the color index is still in use in the image. After a
 * call to this function, the next call to Image.color_allocate for the same
 * image will set new RGB values for that color index, changing the color of any
 * pixels which have that index as a result. If multiple calls to this method
 * are made consecutively, the lowest-numbered index among them will be reused
 * by the next Image.color_allocate call.
 */
static VALUE image_color_deallocate(VALUE klass, VALUE color) {
    return gd_X__gip_H(klass, color, gdImageColorDeallocate);
}

/*
 * call-seq:
 *   sharepen(pct)
 *
 * Sharpens the specified image. pct is a sharpening percentage, and can be
 * greater than 100. Silently does nothing to non-truecolor images. Silently
 * does nothing for pct<0. Transparency/alpha channel are not altered.
 */
static VALUE image_sharepen(VALUE klass, VALUE pct) {
    return gd_X__gip_H(klass, pct, gdImageSharpen);
}

static VALUE gd_X__gip_H2(VALUE klass, VALUE a, VALUE b, void (GDFUNC *pfunc)(gdImagePtr, int, int)) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    (*pfunc)(im, NUM2INT(a), NUM2INT(b));
    return klass;
}

/*
 * call-seq:
 *   to_palette!(dither = true, colors = MAX_COLORS)
 *
 * Convert a truecolor image to a palette-based image, using a high-quality
 * two-pass quantization routine.
 *
 * If _dither_ is true, the image will be dithered to approximate colors better,
 * at the expense of some obvious "speckling." _colors_ can be anything up to 256.
 * If the original source image includes photographic information or
 * anything that came out of a JPEG, 256 is strongly recommended. 100%
 * transparency of a single transparent color in the original truecolor image
 * will be preserved. There is no other support for preservation of alpha
 * channel or transparency in the destination image.
 */
static VALUE image_truecolor_to_palette(int argc, VALUE* argv, VALUE klass) {
    VALUE dither, colors;

    if (image_is_truecolor(klass) == Qfalse) return klass;

    rb_scan_args(argc, argv, "02", &dither, &colors);
    return gd_X__gip_H2(klass,
            (dither == Qnil || dither == Qtrue ? INT2NUM(1) : INT2NUM(0)),
            (colors == Qnil ? INT2NUM(gdMaxColors) : colors),
            gdImageTrueColorToPalette);
}

/*
 * call-seq:
 *   antialiased_dont_blend(color, boolean)
 *
 * Normally, when drawing lines with the special COLOR_ANTIALIASED "color,"
 * blending with the background to reduce jagged edges is the desired behavior.
 * However, when it is desired that lines not be blended with one particular
 * color when it is encountered in the background, this method can be used to
 * indicate the special color that the foreground should stand out more clearly
 * against.
 */
static VALUE image_set_antialiased_dont_blend(VALUE klass, VALUE color, VALUE dont_blend) {
    return gd_X__gip_H2(klass, color, RTEST(dont_blend) ? INT2NUM(1) : INT2NUM(0), gdImageSetAntiAliasedDontBlend);
}

static VALUE gd_X__gip_H3(VALUE klass, VALUE a, VALUE b, VALUE c, void (GDFUNC *pfunc)(gdImagePtr, int, int, int)) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    (*pfunc)(im, NUM2INT(a), NUM2INT(b), NUM2INT(c));
    return klass;
}

/*
 * call-seq:
 *   slef[x, y] = color
 *
 * Sets a pixel to a particular color index.
 */
static VALUE image_set_pixel(VALUE klass, VALUE x, VALUE y, VALUE color) {
    return gd_X__gip_H3(klass, x, y, color, gdImageSetPixel);
}

/*
 * call-seq:
 *   fill(x, y, color)
 *
 * Floods a portion of the image with the specified color, beginning at the
 * specified point and flooding the surrounding region of the same color as the
 * starting point. For a way of flooding a region defined by a specific border
 * color rather than by its interior color, see Image.fill_to_border.
 *
 * The fill color can be COLOR_TILED, resulting in a tile fill using another
 * image as the tile. However, the tile image cannot be transparent. If the
 * image you wish to fill with has a transparent color index, call
 * Image.transparent= and set the transparent color index to -1 to turn off its
 * transparency.
 *
 * Note, this method is recursive. It is not the most naive implementation
 * possible, and the implementation is expected to improve, but there will
 * always be degenerate cases in which the stack can become very deep. This can
 * be a problem in MSDOS and MS Windows environments. (Of course, in a Unix or
 * Windows 95/98/NT environment with a proper stack, this is not a problem at
 * all.)
 */
static VALUE image_fill(VALUE klass, VALUE x, VALUE y, VALUE color) {
    return gd_X__gip_H3(klass, x, y, color, gdImageFill);
}

static VALUE gd_X__gip_H4(VALUE klass, VALUE a, VALUE b, VALUE c, VALUE d, void (GDFUNC *pfunc)(gdImagePtr, int, int, int, int)) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    (*pfunc)(im, NUM2INT(a), NUM2INT(b), NUM2INT(c), NUM2INT(d));
    return klass;
}

/*
 * call-seq:
 *   fill_to_border(x, y, border, color)
 *
 * Floods a portion of the image with the specified color, beginning at the
 * specified point and stopping at the specified border color. For a way of
 * flooding an area defined by the color of the starting point, see Image.fill.
 *
 * The border color cannot be a special color such as COLOR_TILED; it must be a
 * proper solid color. The fill color can be, however.
 *
 * Note, this method is recursive. It is not the most naive implementation
 * possible, and the implementation is expected to improve, but there will
 * always be degenerate cases in which the stack can become very deep. This can
 * be a problem in MSDOS and MS Windows 3.1 environments. (Of course, in a Unix
 * or Windows 95/98/NT environment with a proper stack, this is not a problem at
 * all.)
 */
static VALUE image_fill_to_border(VALUE klass, VALUE x, VALUE y, VALUE border, VALUE color) {
    return gd_X__gip_H4(klass, x, y, border, color, gdImageFillToBorder);
}

/*
 * call-seq:
 *   clip = x1, y1, x2, y2
 *
 * Establishes a clipping rectangle. Once this method has been called, all
 * future drawing operations will remain within the specified clipping area,
 * until a new call takes place. For instance, if a clipping rectangle
 * of 25, 25, 75, 75 has been set within a 100x100 image, a diagonal line
 * from 0,0 to 99,99 will appear only between 25,25 and 75,75.
 *
 * If this method is never called, the clipping area will be the entire image.
 *
 * The parameters passed in are checked against the dimensions of the image and
 * limited to "safe" values.
 */
static VALUE image_set_clip(VALUE klass, VALUE x1, VALUE y1, VALUE x2, VALUE y2) {
    return gd_X__gip_H4(klass, x1, y1, x2, y2, gdImageSetClip);
}

static VALUE gd_X__gip_H5(VALUE klass, VALUE a, VALUE b, VALUE c, VALUE d, VALUE e, void (GDFUNC *pfunc)(gdImagePtr, int, int, int, int, int)) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    (*pfunc)(im, NUM2INT(a), NUM2INT(b), NUM2INT(c), NUM2INT(d), NUM2INT(e));
    return klass;
}

/*
 * call-seq:
 *   line(x1, y1, x2, y2, color)
 *
 * Used to draw a line between two endpoints (x1,y1 and x2, y2). The line is
 * drawn using the color index specified. Note that the color index can be an
 * actual color returned by Image.color_allocate or one of COLOR_STYLED,
 * COLOR_BRUSHED or COLOR_STYLED_BRUSHED.
 */
static VALUE image_line(VALUE klass, VALUE x1, VALUE y1, VALUE x2, VALUE y2, VALUE color) {
    return gd_X__gip_H5(klass, x1, y1, x2, y2, color, gdImageLine);
}

/*
 * call-seq:
 *   dashed_line(x1, y1, x2, y2, color)
 *
 * Provided solely for backwards compatibility with gd 1.0. New programs should
 * draw dashed lines using the normal Image.line and the new Image.set_style.
 *
 * This method is used to draw a dashed line between two endpoints (x1,y1 and
 * x2, y2). The line is drawn using the color index specified. The portions of
 * the line that are not drawn are left transparent so the background is
 * visible.
 */
static VALUE image_dashed_line(VALUE klass, VALUE x1, VALUE y1, VALUE x2, VALUE y2, VALUE color) {
    return gd_X__gip_H5(klass, x1, y1, x2, y2, color, gdImageDashedLine);
}

/*
 * call-seq:
 *   rectangle(x1, y1, x2, y2, color)
 *
 * Used to draw a rectangle with the two corners (upper left first, then lower
 * right) specified, using the color index specified.
 */
static VALUE image_rectangle(VALUE klass, VALUE x1, VALUE y1, VALUE x2, VALUE y2, VALUE color) {
    return gd_X__gip_H5(klass, x1, y1, x2, y2, color, gdImageRectangle);
}

/*
 * call-seq:
 *   filled_rectangle(x1, y1, x2, y2, color)
 *
 * Used to fill a polygon with the verticies (at least 3) specified, using the
 * color index specified. See also Image.polygon.
 */
static VALUE image_filled_rectangle(VALUE klass, VALUE x1, VALUE y1, VALUE x2, VALUE y2, VALUE color) {
    return gd_X__gip_H5(klass, x1, y1, x2, y2, color, gdImageFilledRectangle);
}

/*
 * call-seq:
 *   filled_ellipse(x1, y1, x2, y2, color)
 *
 * Used to draw an ellipse centered at the given point, with the specified width
 * and height in pixels. The ellipse is filled in the color specified by the
 * last argument.
 */
static VALUE image_filled_ellipse(VALUE klass, VALUE x1, VALUE y1, VALUE x2, VALUE y2, VALUE color) {
    return gd_X__gip_H5(klass, x1, y1, x2, y2, color, gdImageFilledEllipse);
}

// gd_X__gip_H7
/*
 * call-seq:
 *   arc(cx, cy, w, h, s, e, color)
 *
 * Used to draw a partial ellipse centered at the given point, with the
 * specified width and height in pixels. The arc begins at the position in
 * degrees specified by _s_ and ends at the position specified by _e_. The arc
 * is drawn in the color specified by the last argument. A circle can be drawn
 * by beginning from 0 degrees and ending at 360 degrees, with width and height
 * being equal. _e_ must be greater than _s_. Values greater than 360 are
 * interpreted modulo 360.
 */
static VALUE image_arc(VALUE klass, VALUE cx, VALUE cy, VALUE w, VALUE h, VALUE s, VALUE e, VALUE color) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    gdImageArc(im, NUM2INT(cx), NUM2INT(cy), NUM2INT(w), NUM2INT(h), NUM2INT(s), NUM2INT(e), NUM2INT(color));
}

// gd_X__gip_H8
/*
 * call-seq:
 *   filled_arc(cx, cy, w, h, s, e, color, style)
 *
 * Used to draw a partial ellipse centered at the given point, with the
 * specified width and height in pixels. The arc begins at the position in
 * degrees specified by _s_ and ends at the position specified by _e_. The arc
 * is filled in the color specified by the second to last argument. A circle can
 * be drawn by beginning from 0 degrees and ending at 360 degrees, with width
 * and height being equal. _e_ must be greater than _s_. Values greater than 360
 * are interpreted modulo 360. The last argument is a bitwise OR of the
 * following possibilities:
 *
 * * STYLE_ARC
 * * STYLE_CHORD
 * * STYLE_PIE (synonym for STYLE_CHORD)
 * * STYLE_NO_FILL
 * * STYLE_EDGED
 *
 * STYLE_ARC and STYLE_CHORD are mutually exclusive; STYLE_CHORD just connects
 * the starting and ending angles with a straight line, while STYLE_ARC produces
 * a rounded edge. STYLE_PIE is a synonym for STYLE_ARC. STYLE_NO_FILL indicates
 * that the arc or chord should be outlined, not filled. STYLE_EDGED, used
 * together with STYLE_NO_FILL, indicates that the beginning and ending angles
 * should be connected to the center; this is a good way to outline (rather than
 * fill) a 'pie slice'.
 *
 */
static VALUE image_filled_arc(VALUE klass, VALUE cx, VALUE cy, VALUE w, VALUE h, VALUE s, VALUE e, VALUE color, VALUE style) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    gdImageFilledArc(im, NUM2INT(cx), NUM2INT(cy), NUM2INT(w), NUM2INT(h), NUM2INT(s), NUM2INT(e), NUM2INT(color), NUM2INT(style));
}

// gd_X__gip_PAH4
/*
 * call-seq:
 *   x1, y1, x2, y2 = clip
 *
 * Fetches the boundaries of the current clipping rectangle.
 */
static VALUE image_get_clip(VALUE klass) {
    int x1, y1, x2, y2;
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    gdImageGetClip(im, &x1, &y1, &x2, &y2);
    return rb_ary_new3(4, INT2NUM(x1), INT2NUM(y1), INT2NUM(x2), INT2NUM(y2));
}

// gd_X__gip_PAH_H
/*
 * call-seq:
 *   styles = array
 *
 * It is often desirable to draw dashed lines, dotted lines, and other
 * variations on a broken line. This method can be used to set any desired
 * series of colors, including a special color that leaves the background
 * intact, to be repeated during the drawing of a line.
 *
 * To use this method, create an array of integers and assign them the desired
 * series of color values to be repeated. You can assign the special color value
 * COLOR_TRANSPARENT to indicate that the existing color should be left
 * unchanged for that particular pixel (allowing a dashed line to be
 * attractively drawn over an existing image).
 *
 * Then, to draw a line using the style, use the normal Image.line with the
 * special color value COLOR_STYLED.
 *
 * You can also combine styles and brushes to draw the brush image at intervals
 * instead of in a continuous stroke. When creating a style for use with a
 * brush, the style values are interpreted differently: zero (0) indicates
 * pixels at which the brush should not be drawn, while one (1) indicates pixels
 * at which the brush should be drawn. To draw a styled, brushed line, you must
 * use the special color value COLOR_STYLED_BRUSHED.
 */
static VALUE image_set_style(VALUE klass, VALUE styles) {
    gdImagePtr im;
    int *ary;
    int i, len;

    Check_Type(styles, T_ARRAY);
    len = RARRAY_LEN(styles);
    ary = malloc(sizeof(int) * len);
    for (i=0;i<len;i++) ary[i] = NUM2INT(rb_ary_entry(styles, i));

    Data_Get_Struct(klass, gdImage, im);
    gdImageSetStyle(im, ary, len);
    free(ary);
    return klass;
}

static VALUE gd_X__gip_gptp_H2(VALUE klass, VALUE points, VALUE color, void (GDFUNC *pfunc)(gdImagePtr, gdPointPtr, int, int)) {
    gdImagePtr im;
    gdPointPtr ary;
    int i, len;
    VALUE t;

    Check_Type(points, T_ARRAY);
    Check_Type(color, T_FIXNUM);
    Data_Get_Struct(klass, gdImage, im);
    len = RARRAY_LEN(points);
    ary = malloc(sizeof(gdPoint) * len);
    for (i=0;i<len;i++) {
        t = rb_ary_entry(points, i);
        if (TYPE(t) == T_ARRAY && RARRAY_LEN(t) >= 2 && TYPE(rb_ary_entry(t, 0)) == T_FIXNUM && TYPE(rb_ary_entry(t, 1)) == T_FIXNUM) {
            ary[i].x = NUM2INT(rb_ary_entry(t, 0));
            ary[i].y = NUM2INT(rb_ary_entry(t, 1));
        } else {
            free(ary);
            rb_raise(rb_eArgError, "array of points please.");
        }
    }
    (*pfunc)(im, ary, len, NUM2INT(color));
    free(ary);
    return klass;
}

/*
 * call-seq:
 *   polygon(points, color)
 *
 * Used to draw a polygon with the verticies (at least 3) specified, using the
 * color index specified. _points_ is an array looks like [[1, 1], [2, 2], [3, 3]].
 *
 * See also Image.filled_polygon.
 */
static VALUE image_polygon(VALUE klass, VALUE points, VALUE color) {
    return gd_X__gip_gptp_H2(klass, points, color, gdImagePolygon);
}

/*
 * call-seq:
 *   open_polygon(points, color)
 *
 * Used to draw a sequence of lines with the verticies (at least 3) specified,
 * using the color index specified. _points_ is an array looks
 * like [[1, 1], [2, 2], [3, 3]]. Unlike Image.polygon, the enpoints of the line
 * sequence are not connected to a closed polygon.
 */
static VALUE image_open_polygon(VALUE klass, VALUE points, VALUE color) {
    return gd_X__gip_gptp_H2(klass, points, color, gdImageOpenPolygon);
}

/*
 * call-seq:
 *   filled_polygon(points, color)
 *
 * Used to fill a polygon with the verticies (at least 3) specified, using the
 * color index specified. _points_ is an array looks like [[1, 1], [2, 2], [3, 3]].
 *
 * See also Image.polygon.
 */
static VALUE image_filled_polygon(VALUE klass, VALUE points, VALUE color) {
    return gd_X__gip_gptp_H2(klass, points, color, gdImageFilledPolygon);
}

// gd_X__gip
/*
 * call-seq:
 *   aa_blend()
 *
 * TODO: document for aa_blend.
 */
static VALUE image_aa_blend(VALUE klass) {
    gdImagePtr im;
    Data_Get_Struct(klass, gdImage, im);
    gdImageAABlend(im);
    return klass;
}

// gd_X__gip_gfp_H2_PAE_H
// TODO: gdImageString16 and gdImageStringUp16
static VALUE gd_image_string(VALUE klass, VALUE font, VALUE x, VALUE y, VALUE str, VALUE color, void (GDFUNC *pfunc)(gdImagePtr, gdFontPtr, int, int, unsigned char*, int)) {
    gdImagePtr im;
    gdFontPtr ft;
    Data_Get_Struct(klass, gdImage, im);
    Data_Get_Struct(font, gdFont, ft);
    (*pfunc)(im, ft, NUM2INT(x), NUM2INT(y), RSTRING_PTR(str), NUM2INT(color));
    return klass;
}

/*
 * call-seq:
 *   string(font, x, y, str, color)
 *
 * Used to draw multiple characters on the image. (To draw single characters,
 * use #char.)
 *
 * The string specified by the fifth argument _str_ is drawn from left to right
 * in the specified _color_. (See #string_up for a way of drawing vertical text.
 * See also #stringft for a high quality solution.) Pixels not set by a
 * particular character retain their previous color.
 */
static VALUE image_string(VALUE klass, VALUE font, VALUE x, VALUE y, VALUE str, VALUE color) {
    return gd_image_string(klass, font, x, y, str, color, gdImageString);
}

/*
 * call-seq:
 *   string_up(font, x, y, str, color)
 *
 * Used to draw multiple characters on the image, rotated 90 degrees. (To draw
 * single characters, use Image.char_up.)
 *
 * The argument _str_ is drawn from bottom to top (rotated 90 degrees) in the
 * specified color. (See Image.string for a way of drawing horizontal text.)
 * Pixels not set by a particular character retain their previous color.
 */
static VALUE image_string_up(VALUE klass, VALUE font, VALUE x, VALUE y, VALUE str, VALUE color) {
    return gd_image_string(klass, font, x, y, str, color, gdImageStringUp);
}

// gd_X__gip_gfp_H4
static VALUE gd_image_char(VALUE klass, VALUE font, VALUE x, VALUE y, VALUE c, VALUE color, void (GDFUNC *pfunc)(gdImagePtr, gdFontPtr, int, int, int, int)) {
    gdImagePtr im;
    gdFontPtr ft;
    Data_Get_Struct(klass, gdImage, im);
    Data_Get_Struct(font, gdFont, ft);
    (*pfunc)(im, ft, NUM2INT(x), NUM2INT(y), NUM2INT(rb_funcall(c, rb_intern("rnd"), 0)), NUM2INT(color));
    return klass;
}

/*
 * call-seq:
 *   char(font, x, y, c, color)
 *
 * Used to draw single characters on the image. (To draw multiple characters,
 * use Image.string. See also Image.stringft for a high quality solution.)
 *
 * The character specified by the fifth argument _c_ is drawn from left to right
 * in the specified color. (See Image.char_up for a way of drawing vertical
 * text.) Pixels not set by a particular character retain their previous color.
 */
static VALUE image_char(VALUE klass, VALUE font, VALUE x, VALUE y, VALUE c, VALUE color) {
    return gd_image_char(klass, font, x, y, c, color, gdImageChar);
}

/*
 * call-seq:
 *   char_up(font, x, y, c, color)
 *
 * Used to draw single characters on the image, rotated 90 degrees. (To draw
 * multiple characters, use Image.string_up.)
 *
 * The character specified by the fifth argument _c_ is drawn from bottom to
 * top, rotated at a 90-degree angle, in the specified color. (See Image.char
 * for a way of drawing horizontal text.) Pixels not set by a particular
 * character retain their previous color.
 */
static VALUE image_char_up(VALUE klass, VALUE font, VALUE x, VALUE y, VALUE c, VALUE color) {
    return gd_image_char(klass, font, x, y, c, color, gdImageCharUp);
}

// gd_PAD__gip_PAH_H_PAD_N2_H2_PAD
// gd_PAD__gip_PAH_H_PAD_N2_H2_PAD_gftexp
/*
 * call-seq:
 *   stringft(fg, fontname, ptsize, angle, x, y, str, opts = {})
 *
 * Draws a string of anti-aliased characters on the image using the FreeType
 * library to render user-supplied TrueType fonts. The string is anti-aliased,
 * meaning that there should be fewer "jaggies" visible. The fontname is the
 * full pathname to a TrueType font file, or a font face name if the GDFONTPATH
 * environment variable or the compiled-in DEFAULT_FONTPATH macro of gdft.c have
 * been set intelligently. In the absence of a full path, the font face name may
 * be presented with or without extension (2.0.26).
 *
 * The null-terminated string argument is considered to be encoded via
 * the UTF_8 standard; also, HTML entities are supported, including decimal,
 * hexadecimal, and named entities (2.0.26). Those who are passing ordinary
 * ASCII strings may have difficulty with the & character unless encoded
 * correctly as & but should have no other difficulties.
 *
 * The string may be arbitrarily scaled (_ptsize_) and rotated (_angle_ in
 * radians). The direction of rotation is counter-clockwise, with 0 radians (0
 * degrees) at 3 o'clock and PI/2 radians (90 degrees) at 12 o'clock.
 *
 * The string is rendered in the color indicated by the _fg_ color index. Use
 * the negative of the desired color index to disable anti-aliasing.
 *
 * The string may contain UTF-8 sequences like: "À"
 *
 * This method returns a hash object, and the element with key "brect" is an
 * array filled with 4 elements representing the 4 corner coordinates of the
 * bounding rectangle (the smallest rectangle that completely surrounds the
 * rendered string and does not intersect any pixel of the rendered string).
 *
 * [ [lower_left_X, lower_left_Y], [lower_right_X, lower_right_Y],
 * \[upper_right_X, upper_right_Y], [upper_left_X, upper_left_Y] ]
 * 
 * Use Image::stringft to get the bounding rectangle without rendering. This is
 * a relatively cheap operation if followed by a rendering of the same string,
 * because of the caching of the partial rendering during bounding rectangle
 * calculation.
 *
 * Options:
 *
 * * :linespcing => double
 * * :charmap => CHARMAP_*
 * * :hdpi => integer
 * * :vdpi => integer
 * * :kerning => boolean
 * * :xshow => boolean
 * * :fontpath => boolean
 * * :fontconfig => boolean
 *
 * To output multiline text with a specific line spacing, set the option
 * _:linespacing_ to the desired spacing, expressed as a multiple of the font
 * height. Thus a line spacing of 1.0 is the minimum to guarantee that lines of
 * text do not collide. If _:linespacing_ is not present, linespacing defaults
 * to 1.05.
 *
 * To specify a preference for Unicode, Shift_JIS Big5 character encoding, set
 * the option _:charmap_ to CHARMAP_*. If you do not specify a preference,
 * Unicode will be tried first. If the preferred character mapping is not found
 * in the font, other character mappings are attempted.
 *
 * GD operates on the assumption that the output image will be rendered to a
 * computer screen. By default, gd passes a resolution of 96 dpi to the freetype
 * text rendering engine. This influences the "hinting" decisions made by the
 * renderer. To specify a different resolution, set _:hdpi_ and _:vdpi_
 * accordingly (in dots per inch).
 *
 * GD 2.0.29 and later will normally attempt to apply kerning tables, if
 * fontconfig is available, to adjust the relative positions of consecutive
 * characters more ideally for that pair of characters. This can be turn off by
 * set the option _:kerning_ to false;
 *
 * GD 2.0.29 and later can return a vector of individual character position
 * advances by set the option _:xshow_ to true, occasionally useful in
 * applications that must know exactly where each character begins. This is
 * returned in the element with key "xshow".
 *
 * GD 2.0.29 and later can also return the path to the actual font file used if
 * the option _:returnfontpath_ is true. This is useful because GD 2.0.29 and
 * above are capable of selecting a font automatically based on a fontconfig
 * font pattern when fontconfig is available. This information is returned in
 * the element with key "fontpath".
 *
 * GD 2.0.29 and later can use fontconfig to resolve font names, including
 * fontconfig patterns, if the option _:fontconfig_ is true. As a convenience,
 * this behavior can be made the default by calling Image.use_fontconfig=true.
 * In that situation it is not necessary to set the option _:fontconfig_ on
 * every call; however explicit font path names can still be used if the option
 * _:fontpathname_ is true.
 * 
 * Unless Image.use_fontconfig=true has been called, GD 2.0.29 and later will
 * still expect the fontlist argument to the freetype text output functions to
 * be a font file name or list thereof as in previous versions. If you do not
 * wish to make fontconfig the default, it is still possible to force the use
 * of fontconfig for a single call to the freetype text output functions by
 * setting the option _:fontconfig_ to ture.
 */
static VALUE image_stringft(int argc, VALUE *argv, VALUE klass) {
    VALUE fg, fontname, ptsize, angle, x, y, str, opts;
    VALUE ret, m, n;
    gdImagePtr im = NULL;
    gdFTStringExtra ftex;
    int brect[8];
    char *err;

    if (TYPE(klass) == T_DATA) Data_Get_Struct(klass, gdImage, im);
    memset(&ftex, 0, sizeof(gdFTStringExtra));
    if (rb_scan_args(argc, argv, "71", &fg, &fontname, &ptsize, &angle, &x, &y, &str, &opts) == 7) {
        err = gdImageStringFT(im, &brect[0], NUM2INT(fg), RSTRING_PTR(fontname), NUM2DBL(ptsize), NUM2DBL(angle), NUM2INT(x), NUM2INT(y), RSTRING_PTR(str));
    } else {
        Check_Type(opts, T_HASH);

        m = rb_hash_aref(opts, STR2SYM("linespacing"));
        if (RTEST(m)) {
            ftex.flags |= gdFTEX_LINESPACE;
            ftex.linespacing = NUM2DBL(m);
        }

        m = rb_hash_aref(opts, STR2SYM("charmp"));
        if (RTEST(m)) {
            ftex.flags |= gdFTEX_CHARMAP;
            ftex.charmap = NUM2INT(m);
        }

        m = rb_hash_aref(opts, STR2SYM("hdpi"));
        n = rb_hash_aref(opts, STR2SYM("vdpi"));
        if (RTEST(m) && RTEST(n)) {
            ftex.flags |= gdFTEX_RESOLUTION;
            ftex.hdpi = NUM2INT(m);
            ftex.vdpi = NUM2INT(n);
        }

        m = rb_hash_aref(opts, STR2SYM("kerning"));
        if (m == Qfalse) ftex.flags |= gdFTEX_DISABLE_KERNING;

        m = rb_hash_aref(opts, STR2SYM("xshow"));
        if (RTEST(m)) ftex.flags |= gdFTEX_XSHOW;

        m = rb_hash_aref(opts, STR2SYM("returnfontpath"));
        if (RTEST(m)) ftex.flags |= gdFTEX_RETURNFONTPATHNAME;

        m = rb_hash_aref(opts, STR2SYM("fontpathname"));
        if (RTEST(m)) ftex.flags |= gdFTEX_FONTPATHNAME;

        m = rb_hash_aref(opts, STR2SYM("fontconfig"));
        if (RTEST(m)) ftex.flags |= gdFTEX_FONTCONFIG;

        err = gdImageStringFTEx(im, &brect[0], NUM2INT(fg), RSTRING_PTR(fontname), NUM2DBL(ptsize), NUM2DBL(angle), NUM2INT(x), NUM2INT(y), RSTRING_PTR(str), &ftex);
    }

    if (err) rb_raise(rb_eRGDError, "%s", err);

    ret = rb_hash_new();
    rb_hash_aset(ret, rb_str_new2("brect"),
        rb_ary_new3(4,
            rb_ary_new3(2, INT2NUM(brect[0]), INT2NUM(brect[1])),
            rb_ary_new3(2, INT2NUM(brect[2]), INT2NUM(brect[3])),
            rb_ary_new3(2, INT2NUM(brect[4]), INT2NUM(brect[5])),
            rb_ary_new3(2, INT2NUM(brect[6]), INT2NUM(brect[7]))
        ));

    if (ftex.flags != 0) {
        if (ftex.xshow) {
            rb_hash_aset(ret, rb_str_new2("xshow"), rb_str_new2(ftex.xshow));
            gdFree(ftex.xshow);
        }

        if (ftex.fontpath) {
            rb_hash_aset(ret, rb_str_new2("fontpath"), rb_str_new2(ftex.fontpath));
            gdFree(ftex.fontpath);
        }
    }

    return ret;
}

// gd_PAD__gip_H2_N3_PAD_N_PAD2_H
/*
 * call-seq:
 *   stringft_circle(cx, cy, radius, textRadius, fillPortion, fontname, points, top, bottom, fg)
 *
 * Draws the text strings specified by _top_ and _bottom_ on the image, curved
 * along the edge of a circle of radius _radius_, with its center at _cx_
 * and _cy_. _top_ is written clockwise along the top; _bottom_ is written
 * counterclockwise along the bottom. _textRadius_ determines the "height" of
 * each character; if _textRadius_ is 1/2 of _radius_, characters extend halfway
 * from the edge to the center. _fillPortion_ varies from 0 to 1.0, with useful
 * values from about 0.4 to 0.9, and determines how much of the 180 degrees of
 * arc assigned to each section of text is actually occupied by text; 0.9 looks
 * better than 1.0 which is rather crowded. _fontname_ is a freetype font; see
 * Image.stringft. _points_ is passed to the freetype engine and has an effect
 * on hinting; although the size of the text is determined by _radius_,
 * _textRadius_, and _fillPortion_, you should pass a point size that "hints"
 * appropriately -- if you know the text will be large, pass a large point size
 * such as 24.0 to get the best results. _fg_ can be any color, and may have an
 * alpha component, do blending, etc.
 */
static VALUE image_stringft_circle(VALUE klass, VALUE cx, VALUE cy, VALUE radius, VALUE textRadius, VALUE fillPortion, VALUE fontname, VALUE points, VALUE top, VALUE bottom, VALUE fg) {
    gdImagePtr im;
    char *err;
    Data_Get_Struct(klass, gdImage, im);
    err = gdImageStringFTCircle(im, NUM2INT(cx), NUM2INT(cy), NUM2DBL(radius), NUM2DBL(textRadius), NUM2DBL(fillPortion), RSTRING_PTR(fontname), NUM2DBL(points), RSTRING_PTR(top), RSTRING_PTR(bottom), NUM2INT(fg));
    if (err) rb_raise(rb_eRGDError, "%s", err);
    return klass;
}

/*
 * call-seq:
 *   new(filename, format = nil)
 *
 * Create a new image from a file.
 *
 * The argument _format_ should be a image format name, like "jpeg", "png", etc.
 * If it's nil, the image type will be detected automatically by the extension
 * of _filename_.
 */
static VALUE image_s_new(int argc, VALUE* argv, VALUE klass) {
    VALUE filename, format;
    char *ext;
    ImageFormat i_fmt = FMT_UNKNOW;

    if (rb_scan_args(argc, argv, "11", &filename, &format) == 2) Check_Type(format, T_STRING);
    Check_Type(filename, T_STRING);
    if (rb_funcall(rb_cFile, rb_intern("readable?"), 1, filename) != Qtrue) rb_raise(rb_eArgError, "%s is not readable", RSTRING_PTR(filename));

    if (RTEST(format)) {
        i_fmt = m_image_detect_format_by_ext(RSTRING_PTR(format));
    } else {
        ext = strrchr(RSTRING_PTR(filename), '.');
        if (ext && ++ext) i_fmt = m_image_detect_format_by_ext(ext);
    }

    switch (i_fmt) {
        case FMT_JPEG:
            return image_s_from_jpeg(klass, filename);
        case FMT_PNG:
        	return image_s_from_png(klass, filename);
        case FMT_GIF:
        	return image_s_from_gif(klass, filename);
        case FMT_BMP:
        	return image_s_from_bmp(klass, filename);
        case FMT_GD2:
        	return image_s_from_gd2(klass, filename);
        case FMT_GD:
        	return image_s_from_gd(klass, filename);
        case FMT_WBMP:
        	return image_s_from_wbmp(klass, filename);
        case FMT_XBM:
            return image_s_from_xbm(klass, filename);
        case FMT_XPM:
            return image_s_from_xpm(klass, filename);
        default:
            if (RTEST(format)) {
                rb_raise(rb_eRGDError, "Unknown image format: %s", RSTRING_PTR(format));
            } else {
                rb_raise(rb_eRGDError, "Cannot detect image format: %s", RSTRING_PTR(filename));
            }
    }
}

/*
 * call-seq:
 *   from_data(data, format = nil)
 *
 * Create a new image from a byte-string.
 *
 * The argument _format_ should be a image format name, like "jpeg", "png", etc.
 * If it's nil, the image type will be detected automatically by the MAGIC of
 * _data_ (work for JPEG, PNG, GIF, BMP, GD2 and GD).
 */
static VALUE image_s_from_data(int argc, VALUE* argv, VALUE klass) {
    VALUE data, format;
    char *ext;
    ImageFormat i_fmt = FMT_UNKNOW;

    if (rb_scan_args(argc, argv, "11", &data, &format) == 2) Check_Type(format, T_STRING);
    Check_Type(data, T_STRING);

    if (RTEST(format)) {
        i_fmt = m_image_detect_format_by_ext(RSTRING_PTR(format));
    } else {
    	i_fmt = m_image_detect_format_by_magic(RSTRING_PTR(data));
    }

    switch (i_fmt) {
        case FMT_JPEG:
            return image_s_from_jpeg_data(klass, data);
        case FMT_PNG:
            return image_s_from_png_data(klass, data);
        case FMT_GIF:
            return image_s_from_gif_data(klass, data);
        case FMT_BMP:
            return image_s_from_bmp_data(klass, data);
        case FMT_GD2:
            return image_s_from_gd2_data(klass, data);
        case FMT_GD:
            return image_s_from_gd_data(klass, data);
        case FMT_WBMP:
            return image_s_from_wbmp_data(klass, data);
        case FMT_XBM:
            rb_raise(rb_eRGDError, "Cannot load a XBM image from a byte-string.");
        case FMT_XPM:
            rb_raise(rb_eRGDError, "Cannot load a XPM image from a byte-string.");
        default:
            if (RTEST(format)) {
                rb_raise(rb_eRGDError, "Unknown image format: %s", RSTRING_PTR(format));
            } else {
                rb_raise(rb_eRGDError, "Cannot detect image format");
            }
    }
}



/*
 * call-seq:
 *   file(filename, format = nil, [more params])
 *
 * Write the image to _filename_ in specified _format_. If _format_ is nil, the
 * file format will be detected automatically by the extension of _filename_.
 *
 * Usage:
 *
 * * file(filename, "jpeg", quality = -1)
 * * file(filename, "png", level = -1)
 * * file(filename, "gif")
 * * file(filename, "bmp")
 * * file(filename, "gd2", chunk_size = 0, fmt = GD2_FMT_COMPRESSED)
 * * file(filename, "gd")
 * * file(filename, "wbmp", fg = 1)
 */
static VALUE image_file(int argc, VALUE* argv, VALUE klass) {
    VALUE filename, format, a1, a2;
    char *ext;
    ImageFormat i_fmt = FMT_UNKNOW;

    rb_scan_args(argc, argv, "13", &filename, &format, &a1, &a2);
    Check_Type(filename, T_STRING);
    if (RTEST(a1)) Check_Type(a1, T_FIXNUM);
    if (RTEST(a2)) Check_Type(a2, T_FIXNUM);

    if (RTEST(format)) {
        Check_Type(format, T_STRING);
        i_fmt = m_image_detect_format_by_ext(RSTRING_PTR(format));
    } else {
        ext = strrchr(RSTRING_PTR(filename), '.');
        if (ext && strlen(ext) > 1) {
            ext += 1;
            i_fmt = m_image_detect_format_by_ext(ext);
        }
    }

    switch (i_fmt) {
        case FMT_JPEG:
            SetIntIfQnil(a1, -1);
            return rb_funcall(klass, rb_intern("jpeg"), 2, filename, a1);
        case FMT_GIF:
            return image_gif(klass, filename);
        case FMT_PNG:
            SetIntIfQnil(a1, -1);
            return rb_funcall(klass, rb_intern("png"), 2, filename, a1);
        case FMT_BMP:
            return image_bmp(klass, filename);
        case FMT_GD2:
            SetIntIfQnil(a1, 0);
            SetIntIfQnil(a2, GD2_FMT_COMPRESSED);
            return rb_funcall(klass, rb_intern("gd2"), 3, filename, a1, a2);
        case FMT_GD:
            return image_gd(klass, filename);
        case FMT_WBMP:
            SetIntIfQnil(a1, 1);
            return rb_funcall(klass, rb_intern("wbmp"), 2, filename, a1);
        case FMT_XBM:
            rb_raise(rb_eRGDError, "This method doesn't support XBM format");
        case FMT_XPM:
            rb_raise(rb_eRGDError, "This method doesn't support XPM format");
        default:
            rb_raise(rb_eRGDError, "Unknown image format.");
    }
}

/*
 * call-seq:
 *   data(format, [more params])
 *
 * Convert the image to a byte-string in specified _format_.
 *
 * Usage:
 *
 * * data("jpeg", quality = -1)
 * * data("png", level = -1)
 * * data("gif")
 * * data("bmp")
 * * data("gd2", chunk_size = 0, fmt = GD2_FMT_COMPRESSED)
 * * data("gd")
 * * data("wbmp", fg = 1)
 */
static VALUE image_data(int argc, VALUE* argv, VALUE klass) {
    VALUE format, a1, a2;
    ImageFormat i_fmt;

    rb_scan_args(argc, argv, "12", &format, &a1, &a2);
    Check_Type(format, T_STRING);
    if (RTEST(a1)) Check_Type(a1, T_FIXNUM);
    if (RTEST(a2)) Check_Type(a2, T_FIXNUM);

    i_fmt = m_image_detect_format_by_ext(RSTRING_PTR(format));
    switch (i_fmt) {
        case FMT_JPEG:
            SetIntIfQnil(a1, -1);
            return rb_funcall(klass, rb_intern("jpeg_data"), 1, a1);
        case FMT_GIF:
            return image_gif_data(klass);
        case FMT_PNG:
            SetIntIfQnil(a1, -1);
            return rb_funcall(klass, rb_intern("png_data"), 1, a1);
        case FMT_BMP:
            return image_bmp_data(klass);
        case FMT_GD2:
            SetIntIfQnil(a1, 0);
            SetIntIfQnil(a2, GD2_FMT_COMPRESSED);
            return rb_funcall(klass, rb_intern("gd2_data"), 2, a1, a2);
        case FMT_GD:
            return image_gd_data(klass);
        case FMT_WBMP:
            SetIntIfQnil(a1, 1);
            return rb_funcall(klass, rb_intern("wbmp_data"), 1, a1);
        case FMT_XBM:
            rb_raise(rb_eRGDError, "This method doesn't support XBM format");
        case FMT_XPM:
            rb_raise(rb_eRGDError, "This method doesn't support XPM format");
        default:
            rb_raise(rb_eRGDError, "Unknown image format.");
    }
}

/*
 * call-seq:
 *   clone()
 *
 * Return a new image clone from current.
 */
static VALUE image_clone(VALUE klass) {
    VALUE nimage, data;

    data = image_gd2_data(klass, 0, GD2_FMT_RAW);
    nimage = rb_funcall(rb_cImage, rb_intern("from_gd2_data"), 1, data);

    return nimage;
}

/*
 * call-seq:
 *   to_palette(dither = true, colors = MAX_COLORS)
 *
 * Create a new palette-based image from current. If the image is a
 * palette-based image already, this method equals to RGD::Image.clone.
 */
static VALUE image_to_palette(int argc, VALUE* argv, VALUE klass) {
    VALUE nimage = image_clone(klass);
    if (image_is_truecolor(nimage) == Qfalse) {
        return nimage;
    } else {
        return image_truecolor_to_palette(argc, argv, nimage);
    }
}

void Init_rgd() {
    rb_mRGD = rb_define_module("RGD");

    rb_eRGDError = rb_define_class_under(rb_mRGD, "RGDError", rb_eException);

    rb_cFont = rb_define_class_under(rb_mRGD, "Font", rb_cObject);
    rb_define_singleton_method(rb_cFont, "small", font_s_small, 0);
    rb_define_singleton_method(rb_cFont, "large", font_s_large, 0);
    rb_define_singleton_method(rb_cFont, "medium_bold", font_s_medium_bold, 0);
    rb_define_singleton_method(rb_cFont, "giant", font_s_giant, 0);
    rb_define_singleton_method(rb_cFont, "tiny", font_s_tiny, 0);

    rb_cImage = rb_define_class_under(rb_mRGD, "Image", rb_cObject);
    /* gdAlphaMax: Same as ALPHA_TRANSPARENT. */
    rb_define_const(rb_cImage, "ALPHA_MAX", INT2NUM(gdAlphaMax));
    /* gdAlphaOpaque: Does not blend at all with the background. */
    rb_define_const(rb_cImage, "ALPHA_OPAQUE", INT2NUM(gdAlphaOpaque));
    /* gdAlphaTransparent: Allows the background to shine through 100%. */
    rb_define_const(rb_cImage, "ALPHA_TRANSPARENT", INT2NUM(gdAlphaTransparent));
    /* gdMaxColors: Max colors can be used in palette-based image. */
    rb_define_const(rb_cImage, "MAX_COLORS", INT2NUM(gdMaxColors));
     /* GD2_FMT_RAW: Uncompressed \GD2 format. */
    rb_define_const(rb_cImage, "GD2_FMT_RAW", INT2NUM(GD2_FMT_RAW));
    /* GD2_FMT_COMPRESSED: Compressed \GD2 format. */
    rb_define_const(rb_cImage, "GD2_FMT_COMPRESSED", INT2NUM(GD2_FMT_COMPRESSED));
    //rb_define_const(rb_cImage, "DISPOSAL_UNKNOWN", INT2NUM(gdDisposalUnknown));
    /* gdDisposalNone: Restores the first allocated color of the global palette. */
    rb_define_const(rb_cImage, "DISPOSAL_NONE", INT2NUM(gdDisposalNone));
    /* gdDisposalRestoreBackground: Restores the appearance of the affected area before the frame was rendered. */
    rb_define_const(rb_cImage, "DISPOSAL_RESTORE_BACKGROUND", INT2NUM(gdDisposalRestoreBackground));
    /* gdDisposalRestorePrevious: The pixels changed by this frame should remain on the display when the next frame begins to render. */
    rb_define_const(rb_cImage, "DISPOSAL_RESTORE_PREVIOUS", INT2NUM(gdDisposalRestorePrevious));
    /* GD_CMP_IMAGE: Actual image IS different */
    rb_define_const(rb_cImage, "GD_CMP_IMAGE", INT2NUM(GD_CMP_IMAGE));
    /* GD_CMP_NUM_COLORS: Number of Colors in pallette differ */
    rb_define_const(rb_cImage, "GD_CMP_NUM_COLORS", INT2NUM(GD_CMP_NUM_COLORS));
    /* GD_CMP_COLOR: \Image Colors differ */
    rb_define_const(rb_cImage, "GD_CMP_COLOR", INT2NUM(GD_CMP_COLOR));
    /* GD_CMP_SIZE_X: \Image width differs */
    rb_define_const(rb_cImage, "GD_CMP_SIZE_X", INT2NUM(GD_CMP_SIZE_X));
    /* GD_CMP_SIZE_Y: \Image heights differ */
    rb_define_const(rb_cImage, "GD_CMP_SIZE_Y", INT2NUM(GD_CMP_SIZE_Y));
    /* GD_CMP_TRANSPARENT: Transparent color */
    rb_define_const(rb_cImage, "GD_CMP_TRANSPARENT", INT2NUM(GD_CMP_TRANSPARENT));
    /* GD_CMP_BACKGROUND: Background color */
    rb_define_const(rb_cImage, "GD_CMP_BACKGROUND", INT2NUM(GD_CMP_BACKGROUND));
    /* GD_CMP_INTERLACE: Interlaced setting */
    rb_define_const(rb_cImage, "GD_CMP_INTERLACE", INT2NUM(GD_CMP_INTERLACE));
    /* GD_CMP_TRUECOLOR: Truecolor vs palette differs */
    rb_define_const(rb_cImage, "GD_CMP_TRUECOLOR", INT2NUM(GD_CMP_TRUECOLOR));
    /* gdStyled: Special color. */
    rb_define_const(rb_cImage, "COLOR_STYLED", INT2NUM(gdStyled));
    /* gdBrushed: Special color. */
    rb_define_const(rb_cImage, "COLOR_BRUSHED", INT2NUM(gdBrushed));
    /* gdStyledBrushed: Special color. */
    rb_define_const(rb_cImage, "COLOR_STYLED_BRUSHED", INT2NUM(gdStyledBrushed));
    /* gdTiled: Special color. */
    rb_define_const(rb_cImage, "COLOR_TILED", INT2NUM(gdTiled));
    /* gdTransparent: Special color. NOT the same as the transparent color index. This is used in line styles only. */
    rb_define_const(rb_cImage, "COLOR_TRANSPARENT", INT2NUM(gdTransparent));
    /* gdAntiAliased: Special color. */
    rb_define_const(rb_cImage, "COLOR_ANTIALIASED", INT2NUM(gdAntiAliased));
    /* gdArc: See Image.filled_arc. */
    rb_define_const(rb_cImage, "STYLE_ARC", INT2NUM(gdArc));
    /* gdChord: See Image.filled_arc. */
    rb_define_const(rb_cImage, "STYLE_CHORD", INT2NUM(gdChord));
    /* gdPie: See Image.filled_arc. */
    rb_define_const(rb_cImage, "STYLE_PIE", INT2NUM(gdPie));
    /* gdNoFill: See Image.filled_arc. */
    rb_define_const(rb_cImage, "STYLE_NO_FILL", INT2NUM(gdNoFill));
    /* gdEdged: See Image.filled_arc. */
    rb_define_const(rb_cImage, "STYLE_EDGED", INT2NUM(gdEdged));
    /* gdFTEX_Unicode: See Image.stringft. */
    rb_define_const(rb_cImage, "CHARMAP_UNICODE", INT2NUM(gdFTEX_Unicode));
    /* gdFTEX_Shift_JIS: See Image.stringft. */
    rb_define_const(rb_cImage, "CHARMAP_SHIFT_JIS", INT2NUM(gdFTEX_Shift_JIS));
    /* gdFTEX_Big5: See Image.stringft. */
    rb_define_const(rb_cImage, "CHARMAP_BIG5", INT2NUM(gdFTEX_Big5));
    /* gdFTEX_Adobe_Custom: See Image.stringft. */
    rb_define_const(rb_cImage, "CHARMAP_ADOBE_CUSTOM", INT2NUM(gdFTEX_Adobe_Custom));
    rb_define_singleton_method(rb_cImage, "use_fontconfig=", image_s_use_fontconfig, 1);
    rb_define_singleton_method(rb_cImage, "alpha_blend", image_s_alpha_blend, 2);
    rb_define_singleton_method(rb_cImage, "truecolor", image_s_truecolor, -1);
    rb_define_singleton_method(rb_cImage, "create", image_s_create, 2);
    rb_define_singleton_method(rb_cImage, "create_truecolor", image_s_create_truecolor, 2);
    rb_define_singleton_method(rb_cImage, "from_jpeg_data", image_s_from_jpeg_data, 1);
    rb_define_singleton_method(rb_cImage, "from_png_data", image_s_from_png_data, 1);
    rb_define_singleton_method(rb_cImage, "from_gif_data", image_s_from_gif_data, 1);
    rb_define_singleton_method(rb_cImage, "from_gd_data", image_s_from_gd_data, 1);
    rb_define_singleton_method(rb_cImage, "from_gd2_data", image_s_from_gd2_data, 1);
    rb_define_singleton_method(rb_cImage, "from_wbmp_data", image_s_from_wbmp_data, 1);
    rb_define_singleton_method(rb_cImage, "from_bmp_data", image_s_from_bmp_data, 1);
    rb_define_singleton_method(rb_cImage, "from_gd2_part_data", image_s_from_gd2_part_data, 5);
    rb_define_singleton_method(rb_cImage, "from_xpm", image_s_from_xpm, 1);
    rb_define_singleton_method(rb_cImage, "from_jpeg", image_s_from_jpeg, 1);
    rb_define_singleton_method(rb_cImage, "from_png", image_s_from_png, 1);
    rb_define_singleton_method(rb_cImage, "from_gif", image_s_from_gif, 1);
    rb_define_singleton_method(rb_cImage, "from_gd", image_s_from_gd, 1);
    rb_define_singleton_method(rb_cImage, "from_gd2", image_s_from_gd2, 1);
    rb_define_singleton_method(rb_cImage, "from_wbmp", image_s_from_wbmp, 1);
    rb_define_singleton_method(rb_cImage, "from_xbm", image_s_from_xbm, 1);
    rb_define_singleton_method(rb_cImage, "from_bmp", image_s_from_bmp, 1);
    rb_define_singleton_method(rb_cImage, "from_gd2_part", image_s_from_gd2_part, 5);
    rb_define_singleton_method(rb_cImage, "create_palette_from_truecolor", image_s_create_palette_from_truecolor, -1);
    rb_define_singleton_method(rb_cImage, "square_to_circle", image_s_square_to_circle, 2);
    // TODO: document for Image.stringft, RDOC doesn't work.
    rb_define_singleton_method(rb_cImage, "stringft", image_stringft, -1);

    rb_define_method(rb_cImage, "gif", image_gif, 1);
    rb_define_method(rb_cImage, "gd", image_gd, 1);
    rb_define_method(rb_cImage, "jpeg", image_jpeg, -1);
    rb_define_method(rb_cImage, "png", image_png, -1);
    rb_define_method(rb_cImage, "bmp", image_bmp, 1);
    rb_define_method(rb_cImage, "wbmp", image_wbmp, -1);
    rb_define_method(rb_cImage, "gif_data", image_gif_data, 0);
    rb_define_method(rb_cImage, "gd_data", image_gd_data, 1);
    rb_define_method(rb_cImage, "jpeg_data", image_jpeg_data, -1);
    rb_define_method(rb_cImage, "png_data", image_png_data, -1);
    rb_define_method(rb_cImage, "wbmp_data", image_wbmp_data, -1);
    rb_define_method(rb_cImage, "bmp_data", image_bmp_data, 0);
    rb_define_method(rb_cImage, "gd2_data", image_gd2_data, -1);
    rb_define_method(rb_cImage, "gif_anim_begin_data", image_gif_anim_begin_data, -1);
    rb_define_method(rb_cImage, "gif_anim_add_data", image_gif_anim_add_data, -1);
    rb_define_method(rb_cImage, "gif_anim_end_data", image_gif_anim_end_data, 0);
    rb_define_method(rb_cImage, "gd2", image_gd2, -1);
    rb_define_method(rb_cImage, "compare", image_compare, 1);
    rb_define_method(rb_cImage, "width", image_sx, 0);
    rb_define_method(rb_cImage, "height", image_sy, 0);
    rb_define_method(rb_cImage, "truecolor?", image_is_truecolor, 0);
    rb_define_method(rb_cImage, "colors_total", image_colors_total, 0);
    rb_define_method(rb_cImage, "interlace", image_get_interlace, 0);
    rb_define_method(rb_cImage, "transparent", image_transparent_get, 0);
    rb_define_method(rb_cImage, "rgba", image_rgba, 1);
    rb_define_method(rb_cImage, "[]", image_get_pixel, 2);
    rb_define_method(rb_cImage, "bounds_safe?", image_bounds_safe, 2);
    rb_define_method(rb_cImage, "color_closest_hwb", image_color_closest_hwb, -1);
    rb_define_method(rb_cImage, "color_allocate", image_color_allocate, -1);
    rb_define_method(rb_cImage, "color_closest", image_color_closest, -1);
    rb_define_method(rb_cImage, "color_exact", image_color_exact, -1);
    rb_define_method(rb_cImage, "color_resolve", image_color_resolve, -1);
    rb_define_method(rb_cImage, "brush=", image_set_brush, 1);
    rb_define_method(rb_cImage, "tile=", image_set_tile, 1);
    rb_define_method(rb_cImage, "copy_palette", image_copy_palette, 1);
    rb_define_method(rb_cImage, "copy", image_copy, 7);
    rb_define_method(rb_cImage, "copy_merge", image_copy_merge, 8);
    rb_define_method(rb_cImage, "copy_merge_gray", image_copy_merge_gray, 8);
    rb_define_method(rb_cImage, "copy_resized", image_copy_resized, 9);
    rb_define_method(rb_cImage, "copy_resampled", image_copy_resampled, 9);
    rb_define_method(rb_cImage, "copy_rotated", image_copy_rotated, 8);
    rb_define_method(rb_cImage, "interlace=", image_interlace_set, 1);
    rb_define_method(rb_cImage, "antialiased=", image_set_antialiased, 1);
    rb_define_method(rb_cImage, "thickness=", image_set_thickness, 1);
    rb_define_method(rb_cImage, "alpha_blending=", image_set_alpha_blending, 1);
    rb_define_method(rb_cImage, "save_alpha=", image_save_alpha, 1);
    rb_define_method(rb_cImage, "color_deallocate", image_color_deallocate, 1);
    rb_define_method(rb_cImage, "transparent=", image_color_transparent, 1);
    rb_define_method(rb_cImage, "sharepen", image_sharepen, 1);
    rb_define_method(rb_cImage, "to_palette!", image_truecolor_to_palette, -1);
    rb_define_method(rb_cImage, "antialiased_dont_blend", image_set_antialiased_dont_blend, 2);
    rb_define_method(rb_cImage, "[]=", image_set_pixel, 3);
    rb_define_method(rb_cImage, "fill", image_fill, 3);
    rb_define_method(rb_cImage, "fill_to_border", image_fill_to_border, 4);
    rb_define_method(rb_cImage, "clip=", image_set_clip, 4);
    rb_define_method(rb_cImage, "line", image_line, 5);
    rb_define_method(rb_cImage, "dashed_line", image_dashed_line, 5);
    rb_define_method(rb_cImage, "rectangle", image_rectangle, 5);
    rb_define_method(rb_cImage, "filled_rectangle", image_filled_rectangle, 5);
    rb_define_method(rb_cImage, "filled_ellipse", image_filled_ellipse, 5);
    rb_define_method(rb_cImage, "arc", image_arc, 7);
    rb_define_method(rb_cImage, "filled_arc", image_filled_arc, 8);
    rb_define_method(rb_cImage, "clip", image_get_clip, 0);
    rb_define_method(rb_cImage, "styles=", image_set_style, 1);
    rb_define_method(rb_cImage, "polygon", image_polygon, 2);
    rb_define_method(rb_cImage, "open_polygon", image_open_polygon, 2);
    rb_define_method(rb_cImage, "filled_polygon", image_filled_polygon, 2);
    rb_define_method(rb_cImage, "aa_blend", image_aa_blend, 0);
    rb_define_method(rb_cImage, "string", image_string, 5);
    rb_define_method(rb_cImage, "string_up", image_string_up, 5);
    rb_define_method(rb_cImage, "char", image_char, 5);
    rb_define_method(rb_cImage, "char_up", image_char_up, 5);
    rb_define_method(rb_cImage, "stringft", image_stringft, -1);
    rb_define_method(rb_cImage, "stringft_circle", image_stringft_circle, 10);

    rb_define_class_variable(rb_cImage, "@@named_colors", m_named_colors());
    rb_define_singleton_method(rb_cImage, "new", image_s_new, -1);
    rb_define_singleton_method(rb_cImage, "from_data", image_s_from_data, -1);
    rb_define_method(rb_cImage, "file", image_file, -1);
    rb_define_method(rb_cImage, "data", image_data, -1);
    rb_define_method(rb_cImage, "clone", image_clone, 0);
    rb_define_method(rb_cImage, "to_palette", image_to_palette, -1);
    rb_define_alias(rb_cImage, "text", "stringft");

    // Init
    gdFontCacheSetup();
}
