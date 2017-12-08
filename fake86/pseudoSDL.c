#include "pseudoSDL.h"
#include "32f429_lcd.h"

extern volatile uint32_t ms_tick;

int SDL_Init(uint32_t flags)
{
  return 0;
}

void SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags)
{
  //LCD_Clear(LCD_COLOR_BLUE2);
  //LCD_SetTextColor(LCD_COLOR_WHITE);
  //LCD_DrawRect(0, 0, height-1, width-1);
}

uint32_t SDL_GetTicks(void)
{
  return ms_tick;
}