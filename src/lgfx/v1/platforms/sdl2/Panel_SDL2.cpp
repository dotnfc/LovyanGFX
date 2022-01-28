/*----------------------------------------------------------------------------/
  Lovyan GFX - Graphics library for embedded devices.
  
Original Source:
 https://github.com/lovyan03/LovyanGFX/

Licence:
 [FreeBSD](https://github.com/lovyan03/LovyanGFX/blob/master/license.txt)

Author:
 [lovyan03](https://twitter.com/lovyan03)

Contributors:
 [ciniml](https://github.com/ciniml)
 [mongonta0716](https://github.com/mongonta0716)
 [tobozo](https://github.com/tobozo)
/----------------------------------------------------------------------------*/

#if defined (TARGET_SDL2)

#include "Panel_SDL2.hpp"

#include "../common.hpp"
#include "../../Bus.hpp"


#include <list>

uint16_t TFT_WIDTH = 320;
uint16_t TFT_HEIGHT = 480;
static uint8_t* sdl_gram = NULL;

namespace lgfx
{
 inline namespace v1
 {
  struct cvmat_info_t
  {
    Panel_SDL2* panel;
    const char* window_name;
  };
  
  static int _window_no;

//----------------------------------------------------------------------------

  static void memset_multi(uint8_t* buf, uint32_t c, size_t size, size_t length)
  {
    if (size == 1 || ((c & 0xFF) == ((c >> 8) & 0xFF) && (size == 2 || ((c & 0xFF) == ((c >> 16) & 0xFF)))))
    {
      memset(buf, c, size * length);
      return;
    }

    size_t l = length;
    if (l & ~0xF)
    {
      while ((l >>= 1) & ~0xF);
      ++l;
    }
    size_t len = l * size;
    length = (length * size) - len;
    uint8_t* dst = buf;
    if (size == 2) {
      do { // 2byte speed tweak
        *(uint16_t*)dst = c;
        dst += 2;
      } while (--l);
    } else {
      do {
        size_t i = 0;
        do {
          *dst++ = *(((uint8_t*)&c) + i);
        } while (++i != size);
      } while (--l);
    }
    if (!length) return;
    while (length > len) {
      memcpy(dst, buf, len);
      dst += len;
      length -= len;
      len <<= 1;
    }
    if (length) {
      memcpy(dst, buf, length);
    }
  }

  Panel_SDL2::~Panel_SDL2(void)
  {
    _img = nullptr;
  }

  Panel_SDL2::Panel_SDL2(void) : Panel_Device(), _window_name( "" )
  {
    _img = nullptr;
  }

  void Panel_SDL2::display(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h)
  {
//    cv::imshow(_window_name, _cv_mat);
//    cv::pollKey();
  }

  color_depth_t Panel_SDL2::setColorDepth(color_depth_t depth)
  {
    _write_bits = 24;
    _read_bits = 24;
    _write_depth = color_depth_t::rgb888_3Byte;
    _read_depth = color_depth_t::rgb888_3Byte;
    return color_depth_t::rgb888_3Byte;
  }

  void Panel_SDL2::beginTransaction(void) {}

  void Panel_SDL2::endTransaction(void) {}

  void Panel_SDL2::setRotation(uint_fast8_t r)
  {
    r &= 7;
    _rotation = r;
    _internal_rotation = ((r + _cfg.offset_rotation) & 3) | ((r & 4) ^ (_cfg.offset_rotation & 4));

    _width  = _cfg.panel_width;
    _height = _cfg.panel_height;
    if (_internal_rotation & 1) std::swap(_width, _height);
  }

  void Panel_SDL2::setWindow(uint_fast16_t xs, uint_fast16_t ys, uint_fast16_t xe, uint_fast16_t ye)
  {
    xs = std::max(0u, std::min<uint_fast16_t>(_width  - 1, xs));
    xe = std::max(0u, std::min<uint_fast16_t>(_width  - 1, xe));
    ys = std::max(0u, std::min<uint_fast16_t>(_height - 1, ys));
    ye = std::max(0u, std::min<uint_fast16_t>(_height - 1, ye));
    _xpos = xs;
    _xs = xs;
    _xe = xe;
    _ypos = ys;
    _ys = ys;
    _ye = ye;
  }

  void Panel_SDL2::drawPixelPreclipped(uint_fast16_t x, uint_fast16_t y, uint32_t rawcolor)
  {
    uint_fast8_t r = _internal_rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { y = _height - (y + 1); }
      if (r & 2)                  { x = _width  - (x + 1); }
      if (r & 1) { std::swap(x, y); }
    }

    size_t bw = _cfg.panel_width;
    size_t index = x + y * bw;
    {
      auto img = &((bgr888_t*)_img)[index];
      *img = rawcolor;
    }

    if (!getStartCount())
    {
      display(x, y, 1, 1);
    }
  }

  void Panel_SDL2::writeFillRectPreclipped(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, uint32_t rawcolor)
  {
    uint_fast8_t r = _internal_rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { y = _height - (y + h); }
      if (r & 2)                  { x = _width  - (x + w); }
      if (r & 1) { std::swap(x, y);  std::swap(w, h); }
    }

    if (w > 1)
    {
      uint_fast8_t bytes = 3;
      uint_fast16_t bw = _cfg.panel_width;
      uint8_t* dst = &_img[(x + y * bw) * bytes];
      uint8_t* src = dst;
      uint_fast16_t add_dst = bw * bytes;
      uint_fast16_t len = w * bytes;

      if (w != bw)
      {
        dst += add_dst;
      }
      else
      {
        w *= h;
        h = 1;
      }
      memset_multi(src, rawcolor, bytes, w);
      while (--h)
      {
        memcpy(dst, src, len);
        dst += add_dst;
      }
    }
    else
    {
      size_t bw = _cfg.panel_width;
      size_t index = x + y * bw;
      {
        auto img = &((bgr888_t*)_img)[index];
        do { *img = rawcolor; img += bw; } while (--h);
      }
    }
  }

  void Panel_SDL2::writeBlock(uint32_t rawcolor, uint32_t length)
  {
    do
    {
      uint32_t h = 1;
      auto w = std::min<uint32_t>(length, _xe + 1 - _xpos);
      if (length >= (w << 1) && _xpos == _xs)
      {
        h = std::min<uint32_t>(length / w, _ye + 1 - _ypos);
      }
      writeFillRectPreclipped(_xpos, _ypos, w, h, rawcolor);
      if ((_xpos += w) <= _xe) return;
      _xpos = _xs;
      if (_ye < (_ypos += h)) { _ypos = _ys; }
      length -= w * h;
    } while (length);
  }

  void Panel_SDL2::_rotate_pixelcopy(uint_fast16_t& x, uint_fast16_t& y, uint_fast16_t& w, uint_fast16_t& h, pixelcopy_t* param, uint32_t& nextx, uint32_t& nexty)
  {
    uint32_t addx = param->src_x32_add;
    uint32_t addy = param->src_y32_add;
    uint_fast8_t r = _internal_rotation;
    uint_fast8_t bitr = 1u << r;
    if (bitr & 0b10010110) // case 1:2:4:7:
    {
      param->src_y32 += nexty * (h - 1);
      nexty = -(int32_t)nexty;
      y = _height - (y + h);
    }
    if (r & 2)
    {
      param->src_x32 += addx * (w - 1);
      param->src_y32 += addy * (w - 1);
      addx = -(int32_t)addx;
      addy = -(int32_t)addy;
      x = _width  - (x + w);
    }
    if (r & 1)
    {
      std::swap(x, y);
      std::swap(w, h);
      std::swap(nextx, addx);
      std::swap(nexty, addy);
    }
    param->src_x32_add = addx;
    param->src_y32_add = addy;
  }

  void Panel_SDL2::writePixels(pixelcopy_t* param, uint32_t length, bool use_dma)
  {
    uint_fast16_t xs = _xs;
    uint_fast16_t xe = _xe;
    uint_fast16_t ys = _ys;
    uint_fast16_t ye = _ye;
    uint_fast16_t x = _xpos;
    uint_fast16_t y = _ypos;
    const size_t bits = _write_bits;
    auto k = _cfg.panel_width * bits >> 3;

    uint_fast8_t r = _internal_rotation;
    if (!r)
    {
      uint_fast16_t linelength;
      do {
        linelength = std::min<uint_fast16_t>(xe - x + 1, length);
        param->fp_copy(&_img[y * k], x, x + linelength, param);
        if ((x += linelength) > xe)
        {
          x = xs;
          y = (y != ye) ? (y + 1) : ys;
        }
      } while (length -= linelength);
      _xpos = x;
      _ypos = y;
      return;
    }

    int_fast16_t ax = 1;
    int_fast16_t ay = 1;
    if ((1u << r) & 0b10010110) { y = _height - (y + 1); ys = _height - (ys + 1); ye = _height - (ye + 1); ay = -1; }
    if (r & 2)                  { x = _width  - (x + 1); xs = _width  - (xs + 1); xe = _width  - (xe + 1); ax = -1; }
    if (param->no_convert)
    {
      size_t bytes = bits >> 3;
      size_t xw = 1;
      size_t yw = _cfg.panel_width;
      if (r & 1) std::swap(xw, yw);
      size_t idx = y * yw + x * xw;
      auto data = (uint8_t*)param->src_data;
      do
      {
        auto dst = &_img[idx * bytes];
        size_t b = 0;
        do
        {
          dst[b] = *data++;
        } while (++b < bytes);
        if (x != xe)
        {
          idx += xw * ax;
          x += ax;
        }
        else
        {
          x = xs;
          y = (y != ye) ? (y + ay) : ys;
          idx = y * yw + x * xw;
        }
      } while (--length);
    }
    else
    {
      if (r & 1)
      {
        do
        {
          param->fp_copy(&_img[x * k], y, y + 1, param);
          if (x != xe)
          {
            x += ax;
          }
          else
          {
            x = xs;
            y = (y != ye) ? (y + ay) : ys;
          }
        } while (--length);
      }
      else
      {
        do
        {
          param->fp_copy(&_img[y * k], x, x + 1, param);
          if (x != xe)
          {
            x += ax;
          }
          else
          {
            x = xs;
            y = (y != ye) ? (y + ay) : ys;
          }
        } while (--length);
      }
    }
    if ((1u << r) & 0b10010110) { y = _height - (y + 1); }
    if (r & 2)                  { x = _width  - (x + 1); }
    _xpos = x;
    _ypos = y;
  }

  void Panel_SDL2::writeImage(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param, bool)
  {
    uint_fast8_t r = _internal_rotation;
    if (r == 0 && param->transp == pixelcopy_t::NON_TRANSP && param->no_convert)
    {
      auto sx = param->src_x;
      auto bits = param->src_bits;

      auto bw = _cfg.panel_width * bits >> 3;
      auto dst = &_img[bw * y];
      auto sw = param->src_bitwidth * bits >> 3;
      auto src = &((uint8_t*)param->src_data)[param->src_y * sw];
      if (sw == bw && this->_cfg.panel_width == w && sx == 0 && x == 0)
      {
        memcpy(dst, src, bw * h);
        return;
      }
      y = 0;
      dst +=  x * bits >> 3;
      src += sx * bits >> 3;
      w    =  w * bits >> 3;
      do
      {
        memcpy(&dst[y * bw], &src[y * sw], w);
      } while (++y != h);
      return;
    }

    uint32_t nextx = 0;
    uint32_t nexty = 1 << pixelcopy_t::FP_SCALE;
    if (r)
    {
      _rotate_pixelcopy(x, y, w, h, param, nextx, nexty);
    }
    uint32_t sx32 = param->src_x32;
    uint32_t sy32 = param->src_y32;

    y *= _cfg.panel_width;
    do
    {
      int32_t pos = x + y;
      int32_t end = pos + w;
      while (end != (pos = param->fp_copy(_img, pos, end, param))
         &&  end != (pos = param->fp_skip(      pos, end, param)));
      param->src_x32 = (sx32 += nextx);
      param->src_y32 = (sy32 += nexty);
      y += _cfg.panel_width;
    } while (--h);
  }

  void Panel_SDL2::writeImageARGB(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, pixelcopy_t* param)
  {
    uint32_t nextx = 0;
    uint32_t nexty = 1 << pixelcopy_t::FP_SCALE;
    if (_internal_rotation)
    {
      _rotate_pixelcopy(x, y, w, h, param, nextx, nexty);
    }
    uint32_t sx32 = param->src_x32;
    uint32_t sy32 = param->src_y32;

    uint32_t pos = x + y * _cfg.panel_width;
    uint32_t end = pos + w;
    param->fp_copy(_img, pos, end, param);
    while (--h)
    {
      pos += _cfg.panel_width;
      end = pos + w;
      param->src_x32 = (sx32 += nextx);
      param->src_y32 = (sy32 += nexty);
      param->fp_copy(_img, pos, end, param);
    }
  }

  void Panel_SDL2::readRect(uint_fast16_t x, uint_fast16_t y, uint_fast16_t w, uint_fast16_t h, void* dst, pixelcopy_t* param)
  {
    uint_fast8_t r = _internal_rotation;
    if (0 == r && param->no_convert)
    {
      h += y;
      auto bytes = _write_bits >> 3;
      auto bw = _cfg.panel_width;
      auto d = (uint8_t*)dst;
      w *= bytes;
      do {
        memcpy(d, &_img[(x + y * bw) * bytes], w);
        d += w;
      } while (++y != h);
    }
    else
    {
      param->src_bitwidth = _cfg.panel_width;
      param->src_data = _img;
      uint32_t nextx = 0;
      uint32_t nexty = 1 << pixelcopy_t::FP_SCALE;
      if (r)
      {
        uint32_t addx = param->src_x32_add;
        uint32_t addy = param->src_y32_add;
        uint_fast8_t rb = 1 << r;
        if (rb & 0b10010110) // case 1:2:4:7:
        {
          nexty = -(int32_t)nexty;
          y = _height - (y + 1);
        }
        if (r & 2)
        {
          addx = -(int32_t)addx;
          x = _width - (x + 1);
        }
        if ((r+1) & 2)
        {
          addy  = -(int32_t)addy;
        }
        if (r & 1)
        {
          std::swap(x, y);
          std::swap(addx, addy);
          std::swap(nextx, nexty);
        }
        param->src_x32_add = addx;
        param->src_y32_add = addy;
      }
      size_t dstindex = 0;
      uint32_t x32 = x << pixelcopy_t::FP_SCALE;
      uint32_t y32 = y << pixelcopy_t::FP_SCALE;
      param->src_x32 = x32;
      param->src_y32 = y32;
      do
      {
        param->src_x32 = x32;
        x32 += nextx;
        param->src_y32 = y32;
        y32 += nexty;
        dstindex = param->fp_copy(dst, dstindex, dstindex + w, param);
      } while (--h);
    }
  }

  void Panel_SDL2::copyRect(uint_fast16_t dst_x, uint_fast16_t dst_y, uint_fast16_t w, uint_fast16_t h, uint_fast16_t src_x, uint_fast16_t src_y)
  {
    uint_fast8_t r = _internal_rotation;
    if (r)
    {
      if ((1u << r) & 0b10010110) { src_y = _height - (src_y + h); dst_y = _height - (dst_y + h); }
      if (r & 2)                  { src_x = _width  - (src_x + w); dst_x = _width  - (dst_x + w); }
      if (r & 1) { std::swap(src_x, src_y);  std::swap(dst_x, dst_y);  std::swap(w, h); }
    }

    size_t bytes = _write_bits >> 3;
    size_t len = w * bytes;
    int32_t add = _cfg.panel_width * bytes;
    if (src_y < dst_y) add = -add;
    int32_t pos = (src_y < dst_y) ? h - 1 : 0;
    uint8_t* src = &_img[(src_x + (src_y + pos) * _cfg.panel_width) * bytes];
    uint8_t* dst = &_img[(dst_x + (dst_y + pos) * _cfg.panel_width) * bytes];
    do
    {
      memmove(dst, src, len);
      src += add;
      dst += add;
    } while (--h);
  }

  uint_fast8_t Panel_SDL2::getTouchRaw(touch_point_t* tp, uint_fast8_t count)
  {
     memcpy(tp, &_touch_point, sizeof(touch_point_t));
    return _touch_point.size ? 1 : 0;
  }

#ifdef WIN32
#include <Windows.h>
#endif // Win32
  //#define SDL_MAIN_HANDLED 1
  #include <SDL.h>
  #pragma comment(lib, "SDL2.lib")

  static SDL_Renderer* renderer = NULL;
  static SDL_Texture* texture = NULL;
  static HANDLE g_hThreadWindow = INVALID_HANDLE_VALUE;
  static int g_sdl2_run_flag;

  int sdl2Poll(void)
  {
	  SDL_Event event;

	  if (SDL_PollEvent(&event)) {
		  if (event.type == SDL_QUIT) {
			  exit(1);  // exit sdl window
			  return 1;
		  }
	  }

	  return 0;
  }

  DWORD WINAPI ThreadProc(LPVOID lpParam)
  {
	  char title[128];
	  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		  printf("Failed to initialize SDL: %s\n", SDL_GetError());
		  return -1;
	  }
	  atexit(SDL_Quit);

	  sprintf(title, "TFT Simulator");

	  SDL_Window* window = SDL_CreateWindow(title,
		  SDL_WINDOWPOS_UNDEFINED,
		  SDL_WINDOWPOS_UNDEFINED,
          TFT_WIDTH,
		  TFT_HEIGHT,
		  0);

	  if (window == NULL) {
		  printf("Failed to create window: %s\n", SDL_GetError());
		  return -2;
	  }

	  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	  if (!renderer) {
		  printf("Failed to create renderer: %s\n", SDL_GetError());
		  return -3;
	  }

	  /* Use unscaled coordinate system */
	  SDL_RenderSetLogicalSize(renderer, TFT_WIDTH, TFT_HEIGHT);

	  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, 
						SDL_TEXTUREACCESS_STREAMING, TFT_WIDTH, TFT_HEIGHT);
	  g_sdl2_run_flag = 1;

	  while (g_sdl2_run_flag) {
		  if (sdl2Poll() == 0) {
			  continue;
		  }
		  else {
			  g_sdl2_run_flag = 0;
		  }
	  }

	  return 0;
  }

  bool Panel_SDL2::init(bool use_reset)
  {
	DWORD threadID;

	TFT_WIDTH = _cfg.panel_width;
	TFT_HEIGHT = _cfg.panel_height;

    if (sdl_gram == NULL) {
        // sdl_gram will not be freed!!!
        sdl_gram = (uint8_t*)malloc(TFT_WIDTH * TFT_HEIGHT * 3 /* RGB array */);
    }
    _img = sdl_gram;

	if (g_hThreadWindow == INVALID_HANDLE_VALUE) {
		g_hThreadWindow = CreateThread(NULL, 0, ThreadProc, NULL, 0, &threadID);
	}
	while (g_sdl2_run_flag == 0)
		;

    return Panel_Device::init(use_reset);
  }

  void Panel_SDL2::imshowall(void)
  {
	  SDL_UpdateTexture(texture, NULL, sdl_gram, TFT_WIDTH * 3);
	  SDL_RenderCopy(renderer, texture, NULL, NULL);
	  SDL_RenderPresent(renderer);
  }

  //----------------------------------------------------------------------------
  void yield(void)
  {
#ifdef WIN32
	  Sleep(0);
#endif // WIN32
  }

  void delay(unsigned long milliseconds)
  {
#ifdef WIN32
	  Sleep(milliseconds);
#endif // WIN32
  }

  void delayMicroseconds(unsigned int us)
  {
#ifdef WIN32
	  int32_t ms = us / 1000;
	  if (ms == 0)
		  return;

	  Sleep(ms);
#endif // WIN32
  }

  uint64_t _os_start = 0;
  unsigned long millis()
  {
#ifdef WIN32
	  if (_os_start == 0) {
		  _os_start = GetTickCount64();
		  return 0;
	  }
	  else {
		  return (GetTickCount64() - _os_start);
	  }
#endif // WIN32
  }

  unsigned long micros(void)
  {
#ifdef WIN32
	  LARGE_INTEGER t1;
	  //QueryPerformanceFrequency(&tc);
	  QueryPerformanceCounter(&t1);
	  return t1.LowPart;
#endif // WIN32
  }

//----------------------------------------------------------------------------
 }
}

#endif
