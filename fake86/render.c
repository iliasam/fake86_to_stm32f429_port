/*
  Fake86: A portable, open-source 8086 PC emulator.
  Copyright (C)2010-2012 Mike Chambers

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* render.c: functions for SDL initialization, as well as video scaling/rendering.
   it is a bit messy. i plan to rework much of this in the future. i am also
   going to add hardware accelerated scaling soon. */


//#include <SDL/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "video.h"
#include "render.h"
#include "pseudoSDL.h"
#include "asciivga.h"
#include "text_render.h"

#include "32f429_lcd.h"


//SDL_Surface *screen = NULL;
uint32_t *scalemap = NULL;


//buffer used to hold LTDC picture
#pragma location = ".sdram"
uint16_t prestretch[LCD_PIXEL_HEIGHT][LCD_PIXEL_WIDTH];//uint32_t earlier

#pragma location = ".sdram"
uint16_t prestretch2[LCD_PIXEL_HEIGHT][LCD_PIXEL_WIDTH];//future dual bufferisation

extern uint8_t RAM[RAM_MEM_SIZE], portram[PORTS_MEM_SIZE];
extern uint8_t VRAM[VRAM_SIZE];
extern uint8_t vidmode, cgabg, blankattr, vidgfxmode, vidcolor, running;
extern uint16_t cursx, cursy, cols, rows, vgapage, cursorposition, cursorvisible;
extern uint8_t updatedscreen, clocksafe, port3da, port6, portout16;
extern uint16_t VGA_SC[0x100], VGA_CRTC[0x100], VGA_ATTR[0x100], VGA_GC[0x100];
extern uint32_t videobase, textbase, x, y;
extern uint32_t palettecga[16], palettevga[256];
extern uint32_t usefullscreen, usegrabmode;
extern uint16_t vtotal;

uint64_t totalframes = 0;
uint32_t framedelay = 20;
uint8_t scrmodechange = 0, noscale = 0, renderbenchmark = 0, doaudio = 0;
//uint8_t nosmooth = 0;
char windowtitle[64];
/*
#ifdef _WIN32
void VideoThread (void *dummy);
#else
void *VideoThread (void *dummy);
#endif
*/

void fake86_draw (void);


uint8_t initscreen (uint8_t *ver) 
{
  if (doaudio) 
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) ) return (0);
  else 
    if (SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER) ) return (0);
  
  SDL_SetVideoMode (640, 400, 32, SDL_HWSURFACE);
  initcga();
  /*        
#ifdef _WIN32
  InitializeCriticalSection (&screenmutex);
  _beginthread (VideoThread, 0, NULL);
#else
  pthread_create (&vidthread, NULL, (void *) VideoThread, NULL);
#endif
  */
  
  return (1);
}


uint32_t nw, nh; //native width and height, pre-stretching (i.e. 320x200 for mode 13h)


extern uint16_t oldw, oldh, constantw, constanth;

void VideoThread (void)
{
  //static uint32_t cursorprevtick, cursorcurtick, delaycalc;
  static uint32_t cursorprevtick, cursorcurtick;
  uint32_t time_now = SDL_GetTicks();
  static uint32_t last_display_update_time = 0;
  
  cursorprevtick = time_now;
  cursorvisible = 0;
  
  if (running)
  {
    cursorcurtick = SDL_GetTicks();
    if ( (cursorcurtick - cursorprevtick) >= 300) 
    {
      updatedscreen = 1;
      cursorvisible = ~cursorvisible & 1;
      cursorprevtick = cursorcurtick;
    }
    
    if ((updatedscreen || renderbenchmark) && ((time_now - last_display_update_time) > 300)) //limit FPS
    {
      updatedscreen = 0;
      fake86_draw();
      last_display_update_time = time_now;
      totalframes++;
    }
  }
}

void doscrmodechange(void) 
{
  //MutexLock (screenmutex);
  if (scrmodechange) 
  {
    if (constantw && constanth) SDL_SetVideoMode (constantw, constanth, 32, SDL_HWSURFACE | usefullscreen);
    else 
      SDL_SetVideoMode (nw, nh, 32, SDL_HWSURFACE | usefullscreen);
    
    //if (usefullscreen) SDL_WM_GrabInput (SDL_GRAB_ON); //always have mouse grab turned on for full screen mode
    //else SDL_WM_GrabInput (usegrabmode);
    //SDL_ShowCursor (SDL_DISABLE);
  }
  //MutexUnlock (screenmutex);
  scrmodechange = 0;
}

void fake86_draw (void)
{
	uint32_t planemode, vgapage, chary, charx, vidptr, divx, divy, curchar, curpixel, usepal, intensity, blockw, curheight, x1, y1;
        
        uint16_t color;//uint32_t earlier
        
	switch (vidmode) {
			case 0:
			case 1:
			case 2: //text modes
			case 3:
			case 7:
			case 0x82:
                        {
                          nw = 640;
                          nh = 400;
                          text_mode_render();
                          break;
                        }
                          
			case 4:
			case 5: //4 colors
                          {
                            nw = 320;
                            nh = 200;
                            usepal = (portram[0x3D9]>>5) & 1;
                            intensity = ( (portram[0x3D9]>>4) & 1) << 3;
                            for (y=0; y<200; y++) 
                            {
                              for (x=0; x<320; x++) 
                              {
                                charx = x;
                                chary = y;
                                vidptr = videobase + ( (chary>>1) * 80) + ( (chary & 1) * 8192) + (charx >> 2);
                                curpixel = RAM[vidptr];
                                switch (charx & 3)
                                {
                                case 3:
                                  curpixel = curpixel & 3;
                                  break;
                                case 2:
                                  curpixel = (curpixel>>2) & 3;
                                  break;
                                case 1:
                                  curpixel = (curpixel>>4) & 3;
                                  break;
                                case 0:
                                  curpixel = (curpixel>>6) & 3;
                                  break;
                                }//end of switch
                                if (vidmode==4) 
                                {
                                  curpixel = curpixel * 2 + usepal + intensity;
                                  if (curpixel == (usepal + intensity) )  curpixel = cgabg;
                                  color = palettecga[curpixel];
                                  prestretch[y][x] = color;
                                }
                                else 
                                {
                                  curpixel = curpixel * 63;
                                  color = palettecga[curpixel];
                                  prestretch[y][x] = color;
                                }
                              }//for x
                            }//for y
                            break;
                          }//mode 5
			case 6: //2 colors
                          {
                            nw = 640;
                            nh = 200;
                            for (y=0; y<200; y++) 
                            {
                              for (x=0; x<640; x++) 
                              {
                                charx = x;
                                chary = y;
                                vidptr = videobase + ( (chary>>1) * 80) + ( (chary&1) * 8192) + (charx>>3);
                                curpixel = (RAM[vidptr]>> (7- (charx&7) ) ) &1;
                                color = palettecga[curpixel*15];
                                //prestretch[y][x] = color;
                                prestretch[y*2][x] = color; //double pixels at Y
                                prestretch[y*2+1][x] = color;
                              }
                            }
                            break;
                          }
                                
			case 0x8: //160x200 16-color (PCjr)
				nw = 640; //fix this
				nh = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							vidptr = 0xB8000 + (y>>2) *80 + (x>>3) + ( (y>>1) &1) *8192;
							if ( ( (x>>1) &1) ==0) color = palettecga[RAM[vidptr] >> 4];
							else color = palettecga[RAM[vidptr] & 15];
							prestretch[y][x] = color;
						}
				break;
			case 0x9: //320x200 16-color (Tandy/PCjr)
				nw = 640; //fix this
				nh = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) {
							vidptr = 0xB8000 + (y>>3) *160 + (x>>2) + ( (y>>1) &3) *8192;
							if ( ( (x>>1) &1) ==0) color = palettecga[RAM[vidptr] >> 4];
							else color = palettecga[RAM[vidptr] & 15];
							prestretch[y][x] = color;
						}
				break;
			case 0xD:
			case 0xE:
				nw = 640; //fix this
				nh = 400; //part later
				for (y=0; y<400; y++)
					for (x=0; x<640; x++) 
                                        {
							divx = x>>1;
							divy = y>>1;
							vidptr = divy*40 + (divx>>3);
							x1 = 7 - (divx & 7);
							color = (VRAM[vidptr] >> x1) & 1;
							color += ( ( (VRAM[0x10000 + vidptr] >> x1) & 1) << 1);
							color += ( ( (VRAM[0x20000 + vidptr] >> x1) & 1) << 2);
							color += ( ( (VRAM[0x30000 + vidptr] >> x1) & 1) << 3);
							color = palettevga[color];
							prestretch[y][x] = color;
					}
				break;
			case 0x10:
                          nw = 640;
                          nh = 350;
                          for (y=0; y<nh; y++)
                            for (x=0; x<nw; x++) 
                            {
                              vidptr = y*80 + (x>>3);
                              x1 = 7 - (x & 7);
                              color = (VRAM[vidptr] >> x1) & 1;
                              color += ( ( (VRAM[0x10000 + vidptr] >> x1) & 1) << 1);
                              color += ( ( (VRAM[0x20000 + vidptr] >> x1) & 1) << 2);
                              color += ( ( (VRAM[0x30000 + vidptr] >> x1) & 1) << 3);
                              color = palettevga[color];
                              prestretch[y][x] = color;
                            }
                          break;
			case 0x12:
				nw = 640;
				nh = 480;
				vgapage = ( (uint32_t) VGA_CRTC[0xC]<<8) + (uint32_t) VGA_CRTC[0xD];
				for (y=0; y<nh; y++)
					for (x=0; x<nw; x++) {
							vidptr = y*80 + (x/8);
							color  = (VRAM[vidptr] >> (~x & 7) ) & 1;
							color |= ( (VRAM[vidptr+0x10000] >> (~x & 7) ) & 1) << 1;
							color |= ( (VRAM[vidptr+0x20000] >> (~x & 7) ) & 1) << 2;
							color |= ( (VRAM[vidptr+0x30000] >> (~x & 7) ) & 1) << 3;
							prestretch[y][x] = palettevga[color];
						}
				break;
			case 0x13:
                          {
                            if (vtotal == 11) { //ugly hack to show Flashback at the proper resolution
                              nw = 256;
                              nh = 224;
                            }
                            else {
                              nw = 320;
                              nh = 200;
                            }
                            if (VGA_SC[4] & 6) planemode = 1;
                            else planemode = 0;
                            vgapage = ( (uint32_t) VGA_CRTC[0xC]<<8) + (uint32_t) VGA_CRTC[0xD];
                            for (y=0; y<nh; y++)
                              for (x=0; x<nw; x++) {
                                if (!planemode) color = palettevga[RAM[videobase + y*nw + x]];
                                else {
                                  vidptr = y*nw + x;
                                  vidptr = vidptr/4 + (x & 3) *0x10000;
                                  vidptr = vidptr + vgapage - (VGA_ATTR[0x13] & 15);
                                  color = palettevga[VRAM[vidptr]];
                                }
                                prestretch[y][x] = color;
                              }
                            break;
                          }
		}

	if (vidgfxmode==0)
        {
          if (cursorvisible) 
          {
            curheight = 2;
            if (cols==80) blockw = 8;
            else blockw = 16;
            x1 = cursx * blockw;
            y1 = cursy * 8 + 8 - curheight;
            for (y=y1*2; y<=y1*2+curheight-1; y++)
              for (x=x1; x<=x1+blockw-1; x++) 
              {
                color = palettecga[RAM[videobase+cursy*cols*2+cursx*2+1]&15];
                prestretch[y&1023][x&1023] = color;
              }
          }
        }

}

