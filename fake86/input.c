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

/* input.c: functions for translation of SDL scancodes to BIOS scancodes,
   and handling of SDL events in general. */

#include <stdint.h>
#include "config.h"
#include "input.h"
//#include "sermouse.h"

//uint8_t keydown[0x100], keyboardwaitack = 0;

extern void doirq (uint8_t irqnum);
extern uint8_t running, portram[PORTS_MEM_SIZE];
extern uint8_t scrmodechange;
//extern struct sermouse_s sermouse;

uint8_t buttons = 0;
//extern void sermouseevent (uint8_t buttons, int8_t xrel, int8_t yrel);


uint8_t translatescancode (uint16_t keyval) 
{
	switch (keyval) 
        {
        case 4:
          return (0x1E);//A
          break;
        case 5:
          return (0x30);//B
          break;
        case 6:
          return (0x2E);//C
          break;
        case 7:
          return (0x20);//D
          break;
        case 8:
          return (0x12);//E
          break;
        case 9:
          return (0x21);//F
          break;
        case 10: //0x0a
          return (0x22);//G
          break;
          
        case 11:
          return (0x23);//H
          break;
        case 12:
          return (0x17);//I
          break;
        case 13:
          return (0x24);//J
          break;
        case 14:
          return (0x25);//K
          break;
        case 15:
          return (0x26);//L
          break;
        case 16: //0x10
          return (0x32);//M
          break;
        case 17:
          return (0x31);//N
          break;
          
        case 18://0X12
          return (0x18);//O
          break;
        case 19:
          return (0x19);//P
          break;
        case 20:
          return (0x10);//Q
          break;
        case 21:
          return (0x13);//R
          break;
        case 22:
          return (0x1F);//S
          break;
        case 23:
          return (0x14);//T
          break;
        case 24:
          return (0x16);//U
          break;

        case 0x19:
          return (0x2F);//V
          break;
        case 0x1a:
          return (0x11);//W
          break;
        case 0x1b:
          return (0x2D);//X
          break;
        case 0x1c:
          return (0x15);//Y
          break;
        case 0x1d:
          return (0x2C);//Z
          break;
          
        case 0x27://0
          return (0x0B);
          break;
          
        case 0x1E://1
          return (0x02);
          break;
          
        case 0x1f://2
          return (0x03);
          break;  
          
        case 0x20://3
          return (0x04);
          break;
          
        case 0x21://4
          return (0x05);
          break;
        case 0x22://5
          return (0x06);
          break;
        case 0x23://6
          return (0x07);
          break;          
        case 0x24://7
          return (0x08);
          break;
        case 0x25://8
          return (0x09);
          break;
        case 0x26://9
          return (0x0A);
          break;
          
        case 0x28:
          return (0x1C);
          break; //enter
          
        case 0x2C:
          return (0x39);
          break; //space
          
        case 0x29:
          return (0x01);
          break; //escape
          
        case 0x4c:
          return (0x53);//del
          break; 
          
        case 0x2A:
          return (0xE);
          break; //backspace
          
        case 0x2B:
          return (0x0F);
          break; //tab
          
          //******************************
          
        case 0x52://up
          return (0x48);
          break; 
          
        case 0x50://left
          return (0x4B);
          break;
          
        case 0x51://down
          return (0x50);
          break; 
          
        case 0x4f://right
          return (0x4d);
          break;
          
          //&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
        case 0x2d://minus
          return (0x0c);
          break; 
          
        case 0x2e://equal =
          return (0x0d);
          break;
          
        case 0x2f:// [
          return (0x1a);
          break; 
          
        case 0x30://]
          return (0x1b);
          break; 
          
        case 0x31:// "\"
          return (0x2b);
          break;
          
        case 0x33: //";"
          return (0x27);
          break; 
          
        case 0x34:// "'"
          return (0x28);
          break;
          
        case 0x35: //`
          return (0x29);
          break; 
          
        case 0x36: //comma
          return (0x33);
          break; 
          
        case 0x37://dot
          return (0x34);
          break;
          
       case 0x38://slash
          return (0x35);
          break; 
          
      case 0x60://my internal code
          return (0x2A);//left shift
          break;
          
        case 0x61://my internal code
          return (0x38);//left alt
          break; 
          
        case 0x62://my internal code
          return (0x1d);//left ctrl
          break; 
          
        case 0x3A: //F1
        case 0x3B:
        case 0x3C:
        case 0x3D:
        case 0x3E:
        case 0x3F:
        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43://F10
            return (keyval+1);
            break;
            
        case 0x44: //f11
          return (0x57);
          break; 
          
        case 0x45: //f12
          return (0x58);
          break;
         
          /*
        case 0x:
          return (0x);
          break; 
          
        case 0x:
          return (0x);
          break;
          */
        default:
          return (0);
          break; 
        }//end of switch
}

//Key kode - usb scan code
void emulate_key_down(uint8_t key_code)
{
  portram[0x60] = translatescancode (key_code);
  portram[0x64] |= 2;
  doirq(1);
}

//Key kode - usb scan code
void emulate_key_up(uint8_t key_code)
{
  portram[0x60] = translatescancode (key_code) | 0x80;
  portram[0x64] |= 2;
  doirq(1);
}

/*
void handleinput() 
{
	int mx = 0, my = 0;
	uint8_t tempbuttons;
	if (SDL_PollEvent (&event) ) 
        {
					case SDL_MOUSEBUTTONDOWN:
						if (SDL_WM_GrabInput (SDL_GRAB_QUERY) == SDL_GRAB_OFF) {
								mousegrabtoggle();
								break;
							}
						tempbuttons = SDL_GetMouseState (NULL, NULL);
						if (tempbuttons & 1) buttons = 2;
						else buttons = 0;
						if (tempbuttons & 4) buttons |= 1;
						sermouseevent (buttons, 0, 0);
						break;
					case SDL_MOUSEBUTTONUP:
						if (SDL_WM_GrabInput (SDL_GRAB_QUERY) == SDL_GRAB_OFF) break;
						tempbuttons = SDL_GetMouseState (NULL, NULL);
						if (tempbuttons & 1) buttons = 2;
						else buttons = 0;
						if (tempbuttons & 4) buttons |= 1;
						sermouseevent (buttons, 0, 0);
						break;
					case SDL_MOUSEMOTION:
						if (SDL_WM_GrabInput (SDL_GRAB_QUERY) == SDL_GRAB_OFF) break;
						SDL_GetRelativeMouseState (&mx, &my);
						sermouseevent (buttons, (int8_t) mx, (int8_t) my);
						SDL_WarpMouse (screen->w / 2, screen->h / 2);
						while (1) {
								SDL_PollEvent (&event);
								SDL_GetRelativeMouseState (&mx, &my);
								if ( (mx == 0) && (my == 0) ) break;
							}
						break;
					case SDL_QUIT:
						running = 0;
						break;
					default:
						break;
				}
		}
}
*/
