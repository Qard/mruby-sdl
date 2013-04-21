/**
 * mruby-sdl
 *
 * Copyright 2013 Stephen Belanger
 *
 * API Reference: http://www.libsdl.org/cgi/docwiki.fcg/SDL_API
 */
#define _GNU_SOURCE
#include <SDL/SDL.h>
#include <mruby.h>
#include <mruby/proc.h>
#include <mruby/data.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/hash.h>
#include <mruby/class.h>
#include <mruby/variable.h>

/*******************************************************************************
 * Expose SDL struct types through a context union
 ******************************************************************************/
typedef struct {
  union {
    SDL_Surface* surface;
    const SDL_VideoInfo* video_info;
    SDL_Rect* rect;
    SDL_Rect** modes;
    SDL_Color* color;
    SDL_PixelFormat* pixel_format;
    SDL_GLattr* gl_attr;
    SDL_Overlay* overlay;
    SDL_Palette* palette;
  } any;
  mrb_value instance;
  mrb_state* mrb;
} mrb_sdl_context;

static mrb_sdl_context* sdl_context_alloc (mrb_state* mrb) {
  mrb_sdl_context* context = (mrb_sdl_context*) malloc(sizeof(mrb_sdl_context));
  if ( ! context) return NULL;
  memset(context, 0, sizeof(mrb_sdl_context));
  context->mrb = mrb;
  return context;
}

static void sdl_context_free (mrb_state *mrb, void *p) {
  mrb_sdl_context* context = (mrb_sdl_context*) p;
  if (context) {
    context->instance = mrb_nil_value();
    context->mrb = NULL;
  }
  free(p);
}

static const struct mrb_data_type sdl_context_type = {
  "sdl_context", sdl_context_free,
};


/*******************************************************************************
 * Use macros to construct to/from mrb value converters
 ******************************************************************************/
#define SDL_TO_MRB(type, key)\
  static mrb_value sdl_##key##_to_mrb_value (mrb_state *mrb, mrb_value self, type* key) {\
    mrb_sdl_context* context = sdl_context_alloc(mrb);\
    if ( ! context) mrb_raise(mrb, E_RUNTIME_ERROR, "can't alloc memory");\
    context->any.key = key;\
    context->instance = self;\
    mrb_iv_set(mrb, self, mrb_intern(mrb, "context"), mrb_obj_value(\
      Data_Wrap_Struct(mrb, mrb->object_class,\
      &sdl_context_type, (void*) context)));\
    return self;\
  }
#define MRB_TO_SDL(type, key)\
  type* mrb_value_to_sdl_##key (mrb_state *mrb, mrb_value self) {\
    mrb_sdl_context* context = NULL;\
    mrb_value value_context = mrb_iv_get(mrb, self, mrb_intern(mrb, "context"));\
    Data_Get_Struct(mrb, value_context, &sdl_context_type, context);\
    if ( ! context) {\
      mrb_raise(mrb, E_ARGUMENT_ERROR, "invalid argument");\
    }\
    return context->any.key;\
  }


SDL_TO_MRB(SDL_Surface, surface);
MRB_TO_SDL(SDL_Surface, surface);

SDL_TO_MRB(const SDL_VideoInfo, video_info);
MRB_TO_SDL(const SDL_VideoInfo, video_info);

SDL_TO_MRB(SDL_Rect, rect);
MRB_TO_SDL(SDL_Rect, rect);

SDL_TO_MRB(SDL_Rect*, modes);
MRB_TO_SDL(SDL_Rect*, modes);

SDL_TO_MRB(SDL_Color, color);
MRB_TO_SDL(SDL_Color, color);

SDL_TO_MRB(SDL_PixelFormat, pixel_format);
MRB_TO_SDL(SDL_PixelFormat, pixel_format);

SDL_TO_MRB(SDL_GLattr, gl_attr);
MRB_TO_SDL(SDL_GLattr, gl_attr);

SDL_TO_MRB(SDL_Overlay, overlay);
MRB_TO_SDL(SDL_Overlay, overlay);

SDL_TO_MRB(SDL_Palette, palette);
MRB_TO_SDL(SDL_Palette, palette);





/*******************************************************************************
 * Core module
 *
 * TODO:
 *  - Implement SDL_SetError(fmt, ...);
 *  - Implement SDL_LoadObject(sofile);
 *  - Implement SDL_LoadFunction(handle, name);
 *  - Implement SDL_UnloadObject(handle);
 ******************************************************************************/
// Init
static mrb_value mrb_sdl_init (mrb_state *mrb, mrb_value self) {
  mrb_int flags = SDL_INIT_EVERYTHING;
  mrb_get_args(mrb, "|i", &flags);
  return mrb_fixnum_value(SDL_Init(flags));
}
static mrb_value mrb_sdl_init_sub_system (mrb_state *mrb, mrb_value self) {
  mrb_int flags;
  mrb_get_args(mrb, "|i", &flags);
  return mrb_fixnum_value(SDL_InitSubSystem(flags));
}

// Quit
static mrb_value mrb_sdl_quit (mrb_state *mrb, mrb_value self) {
  SDL_Quit();
  return mrb_nil_value();
}
static mrb_value mrb_sdl_quit_sub_system (mrb_state *mrb, mrb_value self) {
  mrb_int flags;
  mrb_get_args(mrb, "|i", &flags);
  SDL_QuitSubSystem(flags);
  return mrb_nil_value();
}

// Checking
static mrb_value mrb_sdl_was_init (mrb_state *mrb, mrb_value self) {
  mrb_int flags;
  mrb_get_args(mrb, "|i", &flags);
  return mrb_fixnum_value(SDL_WasInit(flags));
}

// Errors
static mrb_value mrb_sdl_get_error (mrb_state *mrb, mrb_value self) {
  return mrb_str_new_cstr(mrb, SDL_GetError());
}
static mrb_value mrb_sdl_error (mrb_state *mrb, mrb_value self) {
  mrb_int code;
  mrb_get_args(mrb, "|i", &code);
  SDL_Error(code);
  return mrb_nil_value();
}
static mrb_value mrb_sdl_clear_error (mrb_state *mrb, mrb_value self) {
  SDL_ClearError();
  return mrb_nil_value();
}


/*******************************************************************************
 * Video module
 *
 * TODO:
 *  - Figure out SDL_CreateRGBSurfaceFrom
 ******************************************************************************/
// Video surface
static mrb_value mrb_sdl_get_video_surface (mrb_state *mrb, mrb_value self) {
  return sdl_surface_to_mrb_value(mrb, self, SDL_GetVideoSurface());
}

// Display info
static mrb_value mrb_sdl_get_video_info (mrb_state *mrb, mrb_value self) {
  return sdl_video_info_to_mrb_value(mrb, self, SDL_GetVideoInfo());
}
static mrb_value mrb_sdl_video_driver_name (mrb_state *mrb, mrb_value self) {
  char* name_buf;
  mrb_int max_len;
  mrb_get_args(mrb, "s", &name_buf);
  mrb_get_args(mrb, "|i", &max_len);
  return mrb_str_new_cstr(mrb, SDL_VideoDriverName(name_buf, max_len));
}
static mrb_value mrb_sdl_video_list_modes (mrb_state *mrb, mrb_value self) {
  mrb_value arg_format = mrb_nil_value();
  mrb_int flags;

  mrb_get_args(mrb, "|o", &arg_format);
  mrb_get_args(mrb, "|i", &flags);
  
  SDL_PixelFormat* format = mrb_value_to_sdl_pixel_format(mrb, arg_format);
  return sdl_modes_to_mrb_value(mrb, self, SDL_ListModes(format, flags));
}
static mrb_value mrb_sdl_video_mode_ok (mrb_state *mrb, mrb_value self) {
  mrb_int width;
  mrb_int height;
  mrb_int depth;
  mrb_int flags;
  mrb_get_args(mrb, "|i", &width);
  mrb_get_args(mrb, "|i", &height);
  mrb_get_args(mrb, "|i", &depth);
  mrb_get_args(mrb, "|i", &flags);
  return mrb_fixnum_value(SDL_VideoModeOK(width, height, depth, flags));
}

// Display settings
static mrb_value mrb_sdl_video_set_mode (mrb_state *mrb, mrb_value self) {
  mrb_int w;
  mrb_int h;
  mrb_int d;
  mrb_int f;
  mrb_get_args(mrb, "|i", &w);
  mrb_get_args(mrb, "|i", &h);
  mrb_get_args(mrb, "|i", &d);
  mrb_get_args(mrb, "|i", &f);
  return sdl_surface_to_mrb_value(mrb, self, SDL_SetVideoMode(w, h, d, f));
}

// Screen buffer
static mrb_value mrb_sdl_video_update_rect (mrb_state *mrb, mrb_value self) {
  mrb_value surface = mrb_nil_value();
  mrb_int x;
  mrb_int y;
  mrb_int w;
  mrb_int h;

  mrb_get_args(mrb, "|o", &surface);
  mrb_get_args(mrb, "|i", &x);
  mrb_get_args(mrb, "|i", &y);
  mrb_get_args(mrb, "|i", &w);
  mrb_get_args(mrb, "|i", &h);

  SDL_UpdateRect(mrb_value_to_sdl_surface(mrb, surface), x, y, w, h);
  return mrb_nil_value();
}
static mrb_value mrb_sdl_video_update_rects (mrb_state *mrb, mrb_value self) {
  mrb_value surface = mrb_nil_value();
  mrb_int num;
  mrb_value rects = mrb_nil_value();

  mrb_get_args(mrb, "|o", &surface);
  mrb_get_args(mrb, "|i", &num);
  mrb_get_args(mrb, "|o", &rects);

  SDL_UpdateRects(mrb_value_to_sdl_surface(mrb, surface), num, mrb_value_to_sdl_rect(mrb, rects));
  return mrb_nil_value();
}
static mrb_value mrb_sdl_video_flip (mrb_state *mrb, mrb_value self) {
  mrb_value surface = mrb_nil_value();
  mrb_get_args(mrb, "|o", &surface);
  return mrb_fixnum_value(SDL_Flip(mrb_value_to_sdl_surface(mrb, surface)));
}

// Colors
static mrb_value mrb_sdl_video_set_colors (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_value arg_colors = mrb_nil_value();
  mrb_int first_color;
  mrb_int n_colors;

  mrb_get_args(mrb, "|o", &arg_surface);
  mrb_get_args(mrb, "|o", &arg_colors);
  mrb_get_args(mrb, "|i", &first_color);
  mrb_get_args(mrb, "|i", &n_colors);

  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  SDL_Color* colors = mrb_value_to_sdl_color(mrb, arg_colors);

  return mrb_fixnum_value(SDL_SetColors(surface, colors, first_color, n_colors));
}
static mrb_value mrb_sdl_video_set_palette (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_int flags;
  mrb_value arg_colors = mrb_nil_value();
  mrb_int first_color;
  mrb_int n_colors;

  mrb_get_args(mrb, "|o", &arg_surface);
  mrb_get_args(mrb, "|i", &flags);
  mrb_get_args(mrb, "|o", &arg_colors);
  mrb_get_args(mrb, "|i", &first_color);
  mrb_get_args(mrb, "|i", &n_colors);

  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  SDL_Color* colors = mrb_value_to_sdl_color(mrb, arg_colors);

  return mrb_fixnum_value(SDL_SetPalette(surface, flags, colors, first_color, n_colors));
}
static mrb_value mrb_sdl_video_set_color_key (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_int flag;
  mrb_int key;

  mrb_get_args(mrb, "|o", &arg_surface);
  mrb_get_args(mrb, "|i", &flag);
  mrb_get_args(mrb, "|i", &key);
  
  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  return mrb_fixnum_value(SDL_SetColorKey(surface, flag, key));
}
static mrb_value mrb_sdl_video_set_alpha (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_int flag;
  mrb_int key;

  mrb_get_args(mrb, "|o", &arg_surface);
  mrb_get_args(mrb, "|i", &flag);
  mrb_get_args(mrb, "|i", &key);
  
  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  return mrb_fixnum_value(SDL_SetAlpha(surface, flag, key));
}

// Gamma
static mrb_value mrb_sdl_video_set_gamma (mrb_state *mrb, mrb_value self) {
  mrb_float red;
  mrb_float green;
  mrb_float blue;

  mrb_get_args(mrb, "|f", &red);
  mrb_get_args(mrb, "|f", &green);
  mrb_get_args(mrb, "|f", &blue);

  return mrb_fixnum_value(SDL_SetGamma(red, green, blue));
}
static mrb_value mrb_sdl_video_set_gamma_ramp (mrb_state *mrb, mrb_value self) {
  mrb_int r;
  mrb_int g;
  mrb_int b;

  mrb_get_args(mrb, "|i", &r);
  mrb_get_args(mrb, "|i", &g);
  mrb_get_args(mrb, "|i", &b);

  uint16_t red = (uint16_t) r;
  uint16_t green = (uint16_t) g;
  uint16_t blue = (uint16_t) b;

  return mrb_fixnum_value(SDL_SetGammaRamp(&red, &green, &blue));
}
static mrb_value mrb_sdl_video_get_gamma_ramp (mrb_state *mrb, mrb_value self) {
  mrb_int r;
  mrb_int g;
  mrb_int b;

  mrb_get_args(mrb, "|i", &r);
  mrb_get_args(mrb, "|i", &g);
  mrb_get_args(mrb, "|i", &b);

  uint16_t red = (uint16_t) r;
  uint16_t green = (uint16_t) g;
  uint16_t blue = (uint16_t) b;

  return mrb_fixnum_value(SDL_GetGammaRamp(&red, &green, &blue));
}

// Map colors to/from pixel format
static mrb_value mrb_sdl_video_map_rgb (mrb_state *mrb, mrb_value self) {
  mrb_value arg_format = mrb_nil_value();
  mrb_int red;
  mrb_int green;
  mrb_int blue;

  mrb_get_args(mrb, "|o", &arg_format);
  mrb_get_args(mrb, "|i", &red);
  mrb_get_args(mrb, "|i", &green);
  mrb_get_args(mrb, "|i", &blue);

  SDL_PixelFormat* format = mrb_value_to_sdl_pixel_format(mrb, arg_format);
  return mrb_fixnum_value(SDL_MapRGB(format, red, green, blue));
}
static mrb_value mrb_sdl_video_map_rgba (mrb_state *mrb, mrb_value self) {
  mrb_value arg_format = mrb_nil_value();
  mrb_int red;
  mrb_int green;
  mrb_int blue;
  mrb_int alpha;

  mrb_get_args(mrb, "|o", &arg_format);
  mrb_get_args(mrb, "|i", &red);
  mrb_get_args(mrb, "|i", &green);
  mrb_get_args(mrb, "|i", &blue);
  mrb_get_args(mrb, "|i", &alpha);

  SDL_PixelFormat* format = mrb_value_to_sdl_pixel_format(mrb, arg_format);
  return mrb_fixnum_value(SDL_MapRGBA(format, red, green, blue, alpha));
}
static mrb_value mrb_sdl_video_get_rgb (mrb_state *mrb, mrb_value self) {
  mrb_int pixel;
  mrb_value arg_format = mrb_nil_value();
  mrb_int r;
  mrb_int g;
  mrb_int b;

  mrb_get_args(mrb, "|i", &pixel);
  mrb_get_args(mrb, "|o", &arg_format);
  mrb_get_args(mrb, "|i", &r);
  mrb_get_args(mrb, "|i", &g);
  mrb_get_args(mrb, "|i", &b);

  uint8_t red = (uint8_t) r;
  uint8_t green = (uint8_t) g;
  uint8_t blue = (uint8_t) b;

  SDL_PixelFormat* format = mrb_value_to_sdl_pixel_format(mrb, arg_format);
  SDL_GetRGB(pixel, format, &red, &green, &blue);
  return mrb_nil_value();
}
static mrb_value mrb_sdl_video_get_rgba (mrb_state *mrb, mrb_value self) {
  mrb_int pixel;
  mrb_value arg_format = mrb_nil_value();
  mrb_int r;
  mrb_int g;
  mrb_int b;
  mrb_int a;

  mrb_get_args(mrb, "|i", &pixel);
  mrb_get_args(mrb, "|o", &arg_format);
  mrb_get_args(mrb, "|i", &r);
  mrb_get_args(mrb, "|i", &g);
  mrb_get_args(mrb, "|i", &b);
  mrb_get_args(mrb, "|i", &a);

  uint8_t red = (uint8_t) r;
  uint8_t green = (uint8_t) g;
  uint8_t blue = (uint8_t) b;
  uint8_t alpha = (uint8_t) a;

  SDL_PixelFormat* format = mrb_value_to_sdl_pixel_format(mrb, arg_format);
  SDL_GetRGBA(pixel, format, &red, &green, &blue, &alpha);
  return mrb_nil_value();
}

// Surfaces
static mrb_value mrb_sdl_video_create_rgb_surface (mrb_state *mrb, mrb_value self) {
  mrb_int flags;
  mrb_int width;
  mrb_int height;
  mrb_int depth;
  mrb_int r_mask;
  mrb_int g_mask;
  mrb_int b_mask;
  mrb_int a_mask;
  
  mrb_get_args(mrb, "|i", &flags);
  mrb_get_args(mrb, "|i", &width);
  mrb_get_args(mrb, "|i", &height);
  mrb_get_args(mrb, "|i", &depth);
  mrb_get_args(mrb, "|i", &r_mask);
  mrb_get_args(mrb, "|i", &g_mask);
  mrb_get_args(mrb, "|i", &b_mask);
  mrb_get_args(mrb, "|i", &a_mask);

  SDL_Surface* surface = SDL_CreateRGBSurface(flags, width, height, depth, r_mask, g_mask, b_mask, a_mask);
  return sdl_surface_to_mrb_value(mrb, self, surface);
}
// static mrb_value mrb_sdl_video_create_rgb_surface_from (mrb_state *mrb, mrb_value self) {
//   mrb_value pixels = mrb_nil_value();
//   mrb_int width;
//   mrb_int height;
//   mrb_int depth;
//   mrb_int pitch;
//   mrb_int r_mask;
//   mrb_int g_mask;
//   mrb_int b_mask;
//   mrb_int a_mask;
  
//   mrb_get_args(mrb, "|o", &pixels);
//   mrb_get_args(mrb, "|i", &width);
//   mrb_get_args(mrb, "|i", &height);
//   mrb_get_args(mrb, "|i", &depth);
//   mrb_get_args(mrb, "|i", &r_mask);
//   mrb_get_args(mrb, "|i", &g_mask);
//   mrb_get_args(mrb, "|i", &b_mask);
//   mrb_get_args(mrb, "|i", &a_mask);

//   SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels, width, height, depth, pitch, r_mask, g_mask, b_mask, a_mask);
//   return sdl_surface_to_mrb_value(mrb, self, surface);
// }
static mrb_value mrb_sdl_video_free_surface (mrb_state *mrb, mrb_value self) {
  mrb_value surface = mrb_nil_value();
  mrb_get_args(mrb, "|o", &surface);
  SDL_FreeSurface(mrb_value_to_sdl_surface(mrb, surface));
  return mrb_nil_value();
}
static mrb_value mrb_sdl_video_lock_surface (mrb_state *mrb, mrb_value self) {
  mrb_value surface = mrb_nil_value();
  mrb_get_args(mrb, "|o", &surface);
  return mrb_fixnum_value(SDL_LockSurface(mrb_value_to_sdl_surface(mrb, surface)));
}
static mrb_value mrb_sdl_video_unlock_surface (mrb_state *mrb, mrb_value self) {
  mrb_value surface = mrb_nil_value();
  mrb_get_args(mrb, "|o", &surface);
  SDL_UnlockSurface(mrb_value_to_sdl_surface(mrb, surface));
  return mrb_nil_value();
}
static mrb_value mrb_sdl_video_convert_surface (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_value arg_format = mrb_nil_value();
  mrb_int flags;

  mrb_get_args(mrb, "|o", &arg_surface);
  mrb_get_args(mrb, "|o", &arg_format);
  mrb_get_args(mrb, "|i", &flags);

  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  SDL_PixelFormat* format = mrb_value_to_sdl_pixel_format(mrb, arg_format);
  return sdl_surface_to_mrb_value(mrb, self, SDL_ConvertSurface(surface, format, flags));
}
static mrb_value mrb_sdl_video_display_format (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_get_args(mrb, "|o", &arg_surface);
  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  return sdl_surface_to_mrb_value(mrb, self, SDL_DisplayFormat(surface));
}
static mrb_value mrb_sdl_video_display_format_alpha (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_get_args(mrb, "|o", &arg_surface);
  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  return sdl_surface_to_mrb_value(mrb, self, SDL_DisplayFormatAlpha(surface));
}

// Bitmaps
static mrb_value mrb_sdl_video_load_bmp (mrb_state *mrb, mrb_value self) {
  const char *file;
  mrb_get_args(mrb, "|s", &file);
  return sdl_surface_to_mrb_value(mrb, self, SDL_LoadBMP(file));
}
static mrb_value mrb_sdl_video_save_bmp (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  const char *file;

  mrb_get_args(mrb, "|o", &arg_surface);
  mrb_get_args(mrb, "|s", &file);
  
  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  return mrb_fixnum_value(SDL_SaveBMP(surface, file));
}

// Clipping
static mrb_value mrb_sdl_video_set_clipping_rect (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_value arg_rect = mrb_nil_value();

  mrb_get_args(mrb, "|o", &arg_surface);
  mrb_get_args(mrb, "|o", &arg_rect);
  
  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  SDL_Rect* rect = mrb_value_to_sdl_rect(mrb, arg_rect);
  SDL_SetClipRect(surface, rect);
  return mrb_nil_value();
}
static mrb_value mrb_sdl_video_get_clipping_rect (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_value arg_rect = mrb_nil_value();

  mrb_get_args(mrb, "|o", &arg_surface);
  mrb_get_args(mrb, "|o", &arg_rect);
  
  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  SDL_Rect* rect = mrb_value_to_sdl_rect(mrb, arg_rect);
  SDL_GetClipRect(surface, rect);
  return mrb_nil_value();
}

// Blitting
static mrb_value mrb_sdl_video_blit_surface (mrb_state *mrb, mrb_value self) {
  mrb_value arg_src_surface = mrb_nil_value();
  mrb_value arg_src_rect = mrb_nil_value();
  mrb_value arg_dest_surface = mrb_nil_value();
  mrb_value arg_dest_rect = mrb_nil_value();

  mrb_get_args(mrb, "|o", &arg_src_surface);
  mrb_get_args(mrb, "|o", &arg_src_rect);
  mrb_get_args(mrb, "|o", &arg_dest_surface);
  mrb_get_args(mrb, "|o", &arg_dest_rect);

  SDL_Surface* src_surface = mrb_value_to_sdl_surface(mrb, arg_src_surface);
  SDL_Surface* dest_surface = mrb_value_to_sdl_surface(mrb, arg_dest_surface);
  SDL_Rect* src_rect = mrb_value_to_sdl_rect(mrb, arg_src_rect);
  SDL_Rect* dest_rect = mrb_value_to_sdl_rect(mrb, arg_dest_rect);
  return mrb_fixnum_value(SDL_BlitSurface(src_surface, src_rect, dest_surface, dest_rect));
}
static mrb_value mrb_sdl_video_fill_rect (mrb_state *mrb, mrb_value self) {
  mrb_value arg_surface = mrb_nil_value();
  mrb_value arg_rect = mrb_nil_value();
  mrb_int color;

  mrb_get_args(mrb, "|o", &arg_surface);
  mrb_get_args(mrb, "|o", &arg_rect);
  mrb_get_args(mrb, "|i", &color);

  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  SDL_Rect* rect = mrb_value_to_sdl_rect(mrb, arg_rect);
  return mrb_fixnum_value(SDL_FillRect(surface, rect, color));
}

// YUV Overlay
static mrb_value mrb_sdl_video_create_yuv_overlay (mrb_state *mrb, mrb_value self) {
  mrb_int width;
  mrb_int height;
  mrb_int format;
  mrb_value arg_surface = mrb_nil_value();
  
  mrb_get_args(mrb, "|i", &width);
  mrb_get_args(mrb, "|i", &height);
  mrb_get_args(mrb, "|i", &format);
  mrb_get_args(mrb, "|o", &arg_surface);

  SDL_Surface* surface = mrb_value_to_sdl_surface(mrb, arg_surface);
  SDL_Overlay* overlay = SDL_CreateYUVOverlay(width, height, format, surface);
  return sdl_overlay_to_mrb_value(mrb, self, overlay);
}
static mrb_value mrb_sdl_video_lock_yuv_overlay (mrb_state *mrb, mrb_value self) {
  mrb_value overlay = mrb_nil_value();
  mrb_get_args(mrb, "|o", &overlay);
  return mrb_fixnum_value(SDL_LockYUVOverlay(mrb_value_to_sdl_overlay(mrb, overlay)));
}
static mrb_value mrb_sdl_video_unlock_yuv_overlay (mrb_state *mrb, mrb_value self) {
  mrb_value overlay = mrb_nil_value();
  mrb_get_args(mrb, "|o", &overlay);
  SDL_UnlockYUVOverlay(mrb_value_to_sdl_overlay(mrb, overlay));
  return mrb_nil_value();
}
static mrb_value mrb_sdl_video_display_yuv_overlay (mrb_state *mrb, mrb_value self) {
  mrb_value arg_overlay = mrb_nil_value();
  mrb_value arg_rect = mrb_nil_value();

  mrb_get_args(mrb, "|o", &arg_overlay);
  mrb_get_args(mrb, "|o", &arg_rect);
  
  SDL_Overlay* overlay = mrb_value_to_sdl_overlay(mrb, arg_overlay);
  SDL_Rect* rect = mrb_value_to_sdl_rect(mrb, arg_rect);

  return mrb_fixnum_value(SDL_DisplayYUVOverlay(overlay, rect));
}
static mrb_value mrb_sdl_video_free_yuv_overlay (mrb_state *mrb, mrb_value self) {
  mrb_value overlay = mrb_nil_value();
  mrb_get_args(mrb, "|o", &overlay);
  SDL_FreeYUVOverlay(mrb_value_to_sdl_overlay(mrb, overlay));
  return mrb_nil_value();
}


/*******************************************************************************
 * GL module
 *
 * TODO:
 *  - Figure out SDL_GL_GetProcAddress, SDL_GL_GetAttribute, SDL_GL_SetAttribute
 ******************************************************************************/
static mrb_value mrb_sdl_gl_load_library (mrb_state *mrb, mrb_value self) {
  const char *path;
  mrb_get_args(mrb, "|s", &path);
  return mrb_fixnum_value(SDL_GL_LoadLibrary(path));
}
// static mrb_value mrb_sdl_gl_get_proc_address (mrb_state *mrb, mrb_value self) {
//   const char *proc;
//   mrb_get_args(mrb, "|s", &proc);
//   SDL_GL_GetProcAddress(proc);
//   return mrb_nil_value();
// }
// static mrb_value mrb_sdl_gl_get_attribute (mrb_state *mrb, mrb_value self) {
//   mrb_value arg_attr = mrb_nil_value();
//   mrb_int value;

//   mrb_get_args(mrb, "|o", &arg_attr);
//   mrb_get_args(mrb, "|i", &value);

//   SDL_GLattr* attr = mrb_value_to_sdl_gl_attr(mrb, arg_attr);
//   return mrb_fixnum_value(SDL_GL_GetAttribute(attr, value));
// }
// static mrb_value mrb_sdl_gl_set_attribute (mrb_state *mrb, mrb_value self) {
//   mrb_value arg_attr = mrb_nil_value();
//   mrb_int value;

//   mrb_get_args(mrb, "|o", &arg_attr);
//   mrb_get_args(mrb, "|i", &value);

//   SDL_GLattr* attr = mrb_value_to_sdl_gl_attr(mrb, arg_attr);
//   return mrb_fixnum_value(SDL_GL_SetAttribute(attr, value));
// }
static mrb_value mrb_sdl_gl_swap_buffers (mrb_state *mrb, mrb_value self) {
  SDL_GL_SwapBuffers();
  return mrb_nil_value();
}


/*******************************************************************************
 * Rect class
 ******************************************************************************/
static mrb_value mrb_sdl_rect_init (mrb_state *mrb, mrb_value self) {
  SDL_Rect rect;

  mrb_get_args(mrb, "|i", &rect.x);
  mrb_get_args(mrb, "|i", &rect.y);
  mrb_get_args(mrb, "|i", &rect.w);
  mrb_get_args(mrb, "|i", &rect.h);

  mrb_sdl_context* context = sdl_context_alloc(mrb);
  context->any.rect = &rect;

  context->instance = mrb_nil_value();
  mrb_iv_set(mrb, self, mrb_intern(mrb, "context"), mrb_obj_value(
    Data_Wrap_Struct(mrb, mrb->object_class, &sdl_context_type, (void*) context)
  ));

  return self;
}
static mrb_value mrb_sdl_rect_destroy (mrb_state *mrb, mrb_value self) {
  mrb_sdl_context* context = NULL;
  mrb_value value_context = mrb_iv_get(mrb, self, mrb_intern(mrb, "context"));
  Data_Get_Struct(mrb, value_context, &sdl_context_type, context);
  sdl_context_free(mrb, context);
  return self;
}


/*******************************************************************************
 * Color class
 ******************************************************************************/
static mrb_value mrb_sdl_color_init (mrb_state *mrb, mrb_value self) {
  SDL_Color color;

  mrb_get_args(mrb, "|i", &color.r);
  mrb_get_args(mrb, "|i", &color.g);
  mrb_get_args(mrb, "|i", &color.b);
  mrb_get_args(mrb, "|i", &color.unused);

  mrb_sdl_context* context = sdl_context_alloc(mrb);
  context->any.color = &color;

  context->instance = mrb_nil_value();
  mrb_iv_set(mrb, self, mrb_intern(mrb, "context"), mrb_obj_value(
    Data_Wrap_Struct(mrb, mrb->object_class, &sdl_context_type, (void*) context)
  ));

  return self;
}
static mrb_value mrb_sdl_color_destroy (mrb_state *mrb, mrb_value self) {
  mrb_sdl_context* context = NULL;
  mrb_value value_context = mrb_iv_get(mrb, self, mrb_intern(mrb, "context"));
  Data_Get_Struct(mrb, value_context, &sdl_context_type, context);
  sdl_context_free(mrb, context);
  return self;
}


/*******************************************************************************
 * Palette class
 ******************************************************************************/
static mrb_value mrb_sdl_palette_init (mrb_state *mrb, mrb_value self) {
  SDL_Palette palette;
  mrb_value arg_colors = mrb_nil_value();

  mrb_get_args(mrb, "|i", &palette.ncolors);
  mrb_get_args(mrb, "|o", &arg_colors);

  palette.colors = mrb_value_to_sdl_color(mrb, arg_colors);

  mrb_sdl_context* context = sdl_context_alloc(mrb);
  context->any.palette = &palette;

  context->instance = mrb_nil_value();
  mrb_iv_set(mrb, self, mrb_intern(mrb, "context"), mrb_obj_value(
    Data_Wrap_Struct(mrb, mrb->object_class, &sdl_context_type, (void*) context)
  ));

  return self;
}
static mrb_value mrb_sdl_palette_destroy (mrb_state *mrb, mrb_value self) {
  mrb_sdl_context* context = NULL;
  mrb_value value_context = mrb_iv_get(mrb, self, mrb_intern(mrb, "context"));
  Data_Get_Struct(mrb, value_context, &sdl_context_type, context);
  sdl_context_free(mrb, context);
  return self;
}


/*******************************************************************************
 * Register module
 ******************************************************************************/
void mrb_mruby_sdl_gem_init (mrb_state* mrb) {
  int ai = mrb_gc_arena_save(mrb);

  struct RClass* _class_sdl;
  struct RClass* _class_sdl_rect;
  struct RClass* _class_sdl_video;
  struct RClass* _class_sdl_gl;
  mrb_value sdl_gc_table;
  
  // Basic SDL setup
  _class_sdl = mrb_define_module(mrb, "SDL");
  mrb_define_module_function(mrb, _class_sdl, "init", mrb_sdl_init, ARGS_NONE());
  mrb_define_module_function(mrb, _class_sdl, "init_subsystem", mrb_sdl_init_sub_system, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl, "quit", mrb_sdl_quit, ARGS_NONE());
  mrb_define_module_function(mrb, _class_sdl, "quit_subsystem", mrb_sdl_quit_sub_system, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl, "was_init", mrb_sdl_was_init, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl, "get_error", mrb_sdl_get_error, ARGS_NONE());
  mrb_define_module_function(mrb, _class_sdl, "set_error_by_code", mrb_sdl_error, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl, "clear_error", mrb_sdl_clear_error, ARGS_NONE());
  mrb_gc_arena_restore(mrb, ai);

  _class_sdl_rect = mrb_define_class_under(mrb, _class_sdl, "Rect", mrb->object_class);
  mrb_define_method(mrb, _class_sdl_rect, "initialize", mrb_sdl_rect_init, ARGS_REQ(4));
  mrb_define_method(mrb, _class_sdl_rect, "destroy", mrb_sdl_rect_destroy, ARGS_NONE());
  mrb_gc_arena_restore(mrb, ai);

  _class_sdl_rect = mrb_define_class_under(mrb, _class_sdl, "Color", mrb->object_class);
  mrb_define_method(mrb, _class_sdl_rect, "initialize", mrb_sdl_color_init, ARGS_REQ(4));
  mrb_define_method(mrb, _class_sdl_rect, "destroy", mrb_sdl_color_destroy, ARGS_NONE());
  mrb_gc_arena_restore(mrb, ai);

  _class_sdl_rect = mrb_define_class_under(mrb, _class_sdl, "Palette", mrb->object_class);
  mrb_define_method(mrb, _class_sdl_rect, "initialize", mrb_sdl_palette_init, ARGS_REQ(2));
  mrb_define_method(mrb, _class_sdl_rect, "destroy", mrb_sdl_palette_destroy, ARGS_NONE());
  mrb_gc_arena_restore(mrb, ai);

  // Video setup
  _class_sdl_video = mrb_define_module_under(mrb, _class_sdl, "Video");
  mrb_define_module_function(mrb, _class_sdl_video, "surface", mrb_sdl_get_video_surface, ARGS_NONE());
  mrb_define_module_function(mrb, _class_sdl_video, "info", mrb_sdl_get_video_info, ARGS_NONE());
  mrb_define_module_function(mrb, _class_sdl_video, "driver_name", mrb_sdl_video_driver_name, ARGS_REQ(2));
  mrb_define_module_function(mrb, _class_sdl_video, "modes", mrb_sdl_video_list_modes, ARGS_NONE());
  mrb_define_module_function(mrb, _class_sdl_video, "mode_ok", mrb_sdl_video_mode_ok, ARGS_REQ(4));
  mrb_define_module_function(mrb, _class_sdl_video, "set_mode", mrb_sdl_video_set_mode, ARGS_REQ(4));
  mrb_define_module_function(mrb, _class_sdl_video, "update_rect", mrb_sdl_video_update_rect, ARGS_REQ(5));
  mrb_define_module_function(mrb, _class_sdl_video, "update_rects", mrb_sdl_video_update_rects, ARGS_REQ(3));
  mrb_define_module_function(mrb, _class_sdl_video, "flip", mrb_sdl_video_flip, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl_video, "set_colors", mrb_sdl_video_set_colors, ARGS_REQ(4));
  mrb_define_module_function(mrb, _class_sdl_video, "set_palette", mrb_sdl_video_set_palette, ARGS_REQ(5));
  mrb_define_module_function(mrb, _class_sdl_video, "set_gamma", mrb_sdl_video_set_gamma, ARGS_REQ(3));
  mrb_define_module_function(mrb, _class_sdl_video, "set_gamma_ramp", mrb_sdl_video_set_gamma_ramp, ARGS_REQ(3));
  mrb_define_module_function(mrb, _class_sdl_video, "gamma_ramp", mrb_sdl_video_get_gamma_ramp, ARGS_REQ(3));
  mrb_define_module_function(mrb, _class_sdl_video, "map_rgb", mrb_sdl_video_map_rgb, ARGS_REQ(4));
  mrb_define_module_function(mrb, _class_sdl_video, "map_rgba", mrb_sdl_video_map_rgba, ARGS_REQ(5));
  mrb_define_module_function(mrb, _class_sdl_video, "rgb", mrb_sdl_video_get_rgb, ARGS_REQ(5));
  mrb_define_module_function(mrb, _class_sdl_video, "rgba", mrb_sdl_video_get_rgba, ARGS_REQ(6));
  mrb_define_module_function(mrb, _class_sdl_video, "create_rgb_surface", mrb_sdl_video_create_rgb_surface, ARGS_REQ(8));
  mrb_define_module_function(mrb, _class_sdl_video, "free_surface", mrb_sdl_video_free_surface, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl_video, "lock_surface", mrb_sdl_video_lock_surface, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl_video, "unlock_surface", mrb_sdl_video_unlock_surface, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl_video, "convert_surface", mrb_sdl_video_convert_surface, ARGS_REQ(3));
  mrb_define_module_function(mrb, _class_sdl_video, "display_format", mrb_sdl_video_display_format, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl_video, "display_format_alpha", mrb_sdl_video_display_format_alpha, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl_video, "load_bmp", mrb_sdl_video_load_bmp, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl_video, "save_bmp", mrb_sdl_video_save_bmp, ARGS_REQ(2));
  mrb_define_module_function(mrb, _class_sdl_video, "set_color_key", mrb_sdl_video_set_color_key, ARGS_REQ(3));
  mrb_define_module_function(mrb, _class_sdl_video, "set_alpha", mrb_sdl_video_set_alpha, ARGS_REQ(3));
  mrb_define_module_function(mrb, _class_sdl_video, "set_clipping_rect", mrb_sdl_video_set_clipping_rect, ARGS_REQ(2));
  mrb_define_module_function(mrb, _class_sdl_video, "get_clipping_rect", mrb_sdl_video_get_clipping_rect, ARGS_REQ(2));
  mrb_define_module_function(mrb, _class_sdl_video, "blit_surface", mrb_sdl_video_blit_surface, ARGS_REQ(4));
  mrb_define_module_function(mrb, _class_sdl_video, "fill_rect", mrb_sdl_video_fill_rect, ARGS_REQ(3));
  mrb_define_module_function(mrb, _class_sdl_video, "create_yuv_overlay", mrb_sdl_video_create_yuv_overlay, ARGS_REQ(4));
  mrb_define_module_function(mrb, _class_sdl_video, "lock_yuv_overlay", mrb_sdl_video_lock_yuv_overlay, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl_video, "unlock_yuv_overlay", mrb_sdl_video_unlock_yuv_overlay, ARGS_REQ(1));
  mrb_define_module_function(mrb, _class_sdl_video, "display_yuv_overlay", mrb_sdl_video_display_yuv_overlay, ARGS_REQ(2));
  mrb_define_module_function(mrb, _class_sdl_video, "free_yuv_overlay", mrb_sdl_video_free_yuv_overlay, ARGS_REQ(1));
  mrb_gc_arena_restore(mrb, ai);

  _class_sdl_gl = mrb_define_module_under(mrb, _class_sdl, "GL");
  mrb_define_module_function(mrb, _class_sdl_gl, "load_library", mrb_sdl_gl_load_library, ARGS_REQ(1));
  // mrb_define_module_function(mrb, _class_sdl_gl, "proc_address", mrb_sdl_gl_get_proc_address, ARGS_REQ(1));
  // mrb_define_module_function(mrb, _class_sdl_gl, "attribute", mrb_sdl_gl_get_attribute, ARGS_REQ(2));
  // mrb_define_module_function(mrb, _class_sdl_gl, "set_attribute", mrb_sdl_gl_set_attribute, ARGS_REQ(2));
  mrb_define_module_function(mrb, _class_sdl_gl, "swap_buffers", mrb_sdl_gl_swap_buffers, ARGS_NONE());
  mrb_gc_arena_restore(mrb, ai);

  // Do I really need a GC table?
  sdl_gc_table = mrb_ary_new(mrb);
  mrb_define_const(mrb, _class_sdl, "$GC", sdl_gc_table);
}

void mrb_mruby_uv_gem_final (mrb_state* mrb) {}