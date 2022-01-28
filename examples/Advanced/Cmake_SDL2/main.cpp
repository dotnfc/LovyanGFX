
#include <thread>

#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <LGFX_AUTODETECT.hpp>

void setup(void);
void loop(void);

void loopThread(void)
{
  setup();
  for (;;)
  {
    loop();
    std::this_thread::yield();
  }
}

int main(int argc, char* argv[])
{
  std::thread sub_thread(loopThread);
  for (;;)
  {
    std::this_thread::yield();
    lgfx::Panel_SDL2::imshowall();
  }
}
