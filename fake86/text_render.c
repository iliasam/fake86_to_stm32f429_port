#include "text_render.h"
#include "config.h"
#include <stdint.h>
#include "32f429_lcd.h"

#define SYMBOL_W	8
#define SYMBOL_H	16

extern uint8_t RAM[RAM_MEM_SIZE], portram[PORTS_MEM_SIZE];
extern uint8_t VRAM[VRAM_SIZE], vidmode, cgabg, blankattr, vidgfxmode, vidcolor, running;
extern uint16_t VGA_SC[0x100], VGA_CRTC[0x100], VGA_ATTR[0x100], VGA_GC[0x100];
extern uint8_t fontcga[32768];
extern uint32_t palettecga[16], palettevga[256];
extern uint32_t videobase, textbase, x, y;
extern uint16_t cursx, cursy, cols, rows, vgapage, cursorposition, cursorvisible;
#pragma location = ".sdram"
extern uint16_t prestretch[LCD_PIXEL_HEIGHT][LCD_PIXEL_WIDTH];//uint32_t earlier
extern volatile uint32_t ms_tick;
volatile uint32_t render_duration = 0;

void old_text_mode_render(void);

void draw_symbol(uint8_t symb_code, uint8_t xpos, uint8_t ypos, uint32_t back_color, uint32_t pixel_color,  uint8_t divx, uint8_t divy);


void text_mode_render(void)
{
  uint32_t start_time = ms_tick;
  
  //all text  modes are N x 25 symbols
  uint8_t char_x, char_y;
  uint8_t divx, divy;
  uint32_t pointer_offset = 0;
  uint32_t vidptr, back_color, pixel_color, vgapage;
  uint8_t curchar;
  
  vgapage = ( (uint32_t) VGA_CRTC[0xC]<<8) + (uint32_t) VGA_CRTC[0xD];
  if ((portram[0x3D8]==9) && (portram[0x3D4]==9))
  {
    pointer_offset = vgapage + videobase;
    divy = 4;
  }
  else
  {
    pointer_offset = videobase;
    divy = 16;
  }
  if (cols==80)
    divx = 1;
  else
    divx = 2;
  
  
  for (char_y = 0; char_y < rows; char_y++ )
  {
    for (char_x = 0; char_x < cols; char_x++)
    {
      vidptr = pointer_offset + char_y*cols*2 + char_x*2;
      curchar = RAM[vidptr];
      //curchar = 65; //test
      
      //find color of the symbol
      if (vidcolor)
      {
        if (portram[0x3D8] & 128) 
          back_color = palettecga[ (RAM[vidptr+1]/16) & 7];
        else 
          back_color = palettecga[RAM[vidptr+1]/16]; //high intensity background
        
        pixel_color = palettecga[RAM[vidptr+1]&15];
      }
      else
      {
        if ( (RAM[vidptr+1] & 0x70) ) 
        {
          back_color = palettecga[7];
          pixel_color = palettecga[0];
        }
        else 
        {
          back_color = palettecga[0];
          pixel_color = palettecga[7];
        }
      }
      draw_symbol(curchar, char_x, char_y, back_color, pixel_color,  divx, divy);
    }
  }
  render_duration = ms_tick - start_time;
}

void draw_symbol(uint8_t symb_code, uint8_t xpos, uint8_t ypos, uint32_t back_color, uint32_t pixel_color,  uint8_t divx, uint8_t divy)
{
  uint16_t x, y;
  uint16_t x_start = xpos*SYMBOL_W;
  uint16_t x_stop  = x_start + SYMBOL_W;
  
  uint16_t y_start = ypos*SYMBOL_H;
  uint16_t y_stop  = y_start + SYMBOL_H;
  uint8_t pixel_state;
  uint16_t font_pos = 0;
  
  if((divy == 16) && (divx == 1))
  {
    font_pos = symb_code*128;
    for (y=y_start; y < y_stop; y++)
      for (x=x_start; x < x_stop; x++)
      {
        pixel_state = fontcga[font_pos];
        if (!pixel_state)//off
          prestretch[y][x] = back_color;
        else
          prestretch[y][x] = pixel_color;
        font_pos++;
      }
  }
  else
  {
    
    for (y=y_start; y < y_stop; y++)
      for (x=x_start; x < x_stop; x++)
      {
        pixel_state = fontcga[symb_code*128 + (y %  divy) * 8 + ( (x / divx) % 8) ];
        if (!pixel_state)//off
          prestretch[y][x] = back_color;
        else
          prestretch[y][x] = pixel_color;
      }
  }
  
}