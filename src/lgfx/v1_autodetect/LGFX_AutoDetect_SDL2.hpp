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
#pragma once

#include "../v1_init.hpp"
#include "common.hpp"
#include "../v1/platforms/sdl2/Panel_SDL2.hpp"

namespace lgfx
{
 inline namespace v1
 {
//----------------------------------------------------------------------------

  class LGFX : public LGFX_Device
  {
    lgfx::Panel_SDL2 _panel_instance;

    bool init_impl(bool use_reset, bool use_clear)
    {
      return LGFX_Device::init_impl(false, use_clear);
    }

  public:

    LGFX(int width = 320, int height = 480)
    {
      auto cfg = _panel_instance.config();
      cfg.memory_width = width;
      cfg.panel_width = width;
      cfg.memory_height = height;
      cfg.panel_height = height;
      _panel_instance.config(cfg);
      setPanel(&_panel_instance);
      _board = board_t::board_SDL2;
    }
  };

//----------------------------------------------------------------------------
 }
}

using LGFX = lgfx::LGFX;