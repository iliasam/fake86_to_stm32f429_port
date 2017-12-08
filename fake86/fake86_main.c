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
/* main.c: functions to initialize the different components of Fake86,
   load ROM binaries, and kickstart the CPU emulator. */

#include "main.h"
#include "fake86_main.h"
#include "32f429_sdram.h"
#include "32f429_lcd.h"
#include "string.h"
#include "sd_card_reader.h"
#include "config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
     
#include "i8253.h" 
#include "i8259.h"
#include "video.h"
#include "timing.h"
#include "render.h" 
#include "cpu.h"
#include "disk.h"

#include "rombasic.h"
#include "videorom.h"
#include "pcxtbios.h" 

#define CPU_STEPS_COUNT 10000

const uint8_t *build = "Fake86 ILIASAM Port";

extern uint8_t RAM[RAM_MEM_SIZE], readonly[RO_MEM_SIZE];
extern uint8_t VRAM[VRAM_SIZE];

extern uint8_t running, renderbenchmark;
extern uint8_t scrmodechange, doaudio;
extern uint64_t totalexec, totalframes;
extern uint8_t bootdrive;

extern void reset86();

extern void *port_write_callback[PORTS_MEM_SIZE];
extern void *port_read_callback[PORTS_MEM_SIZE];
extern void *port_write_callback16[PORTS_MEM_SIZE];
extern void *port_read_callback16[PORTS_MEM_SIZE];

extern uint8_t insertdisk (uint8_t drivenum, char *filename);

uint8_t *biosfile = NULL, verbose = 1, cgaonly = 0, useconsole = 0;
uint32_t speed = 0;
uint8_t usessource = 0;
uint16_t constantw = 0;
uint16_t constanth = 0;
uint8_t speakerenabled = 0;

extern volatile uint32_t ms_tick;
uint32_t start_time_ms = 0;
int32_t test_duration = 0;
uint32_t cpu_ticks = 0;


int8_t fake86_main_init(void)
{
  memset(RAM, 0, sizeof(RAM));
  memset(readonly, 0, sizeof(readonly));
  memset(VRAM, 0, sizeof(VRAM));
  
  memset (readonly, 0, RO_MEM_SIZE);
  
  loadrom (0xFE000UL, "pcxtbios.bin", 1);
  loadrom (0xF6000UL, "rombasic.bin", 0);
  loadrom (0xC0000UL, "videorom.bin", 1);

  running = 1;
  reset86();
  printf ("OK!\n");

  fake86_inithardware();
  
  insertdisk(0, "floppy.img");
  insertdisk(128, "hdd.img");
  bootdrive = 0;

  //bootdrive = 255;//rom basic
  start_time_ms = ms_tick;
  return 0;
}


void fake86_inithardware(void)
{
  printf ("Init emulated hardware:\n");
  
  memset (port_write_callback, 0, sizeof (port_write_callback) );
  memset (port_read_callback, 0, sizeof (port_read_callback) );
  memset (port_write_callback16, 0, sizeof (port_write_callback16) );
  memset (port_read_callback16, 0, sizeof (port_read_callback16) );
  

  init8253();
  init8259();
  /*
  printf ("  - Intel 8237 DMA controller: ");
  init8237();
  printf ("OK\n");
  */
  
  initVideoPorts();
  inittiming();
  initscreen ( (uint8_t *) build);
  
}

void fake86_run_task(void)
{
  //while (running)
  if (running)
  {
    exec86(CPU_STEPS_COUNT);
    cpu_ticks+= CPU_STEPS_COUNT;
    if (scrmodechange) 
      doscrmodechange();
    if ((cpu_ticks > 10000000) && (test_duration != -1))
    {
      test_duration = ms_tick - start_time_ms;
      printf("Time: %d", test_duration);
      test_duration = -1;
    }
  }
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

uint32_t loadrom (uint32_t addr32, uint8_t *filename, uint8_t failure_fatal) 
{
  uint32_t readsize;
  readsize = loadbinary (addr32, filename, 1);
  //printf ("Loaded %s at 0x%05X (%lu KB)\n", filename, addr32, readsize >> 10);
  return (readsize);
}


uint32_t loadbinary (uint32_t addr32, uint8_t *filename, uint8_t roflag) 
{
  uint32_t readsize = 0;
  
  
  if (memcmp((char*)filename,"rombasic", 8) == 0)
  {
    readsize = sizeof(rombasic_data);
    memcpy(&RAM[addr32], rombasic_data, readsize);
  }
  else if (memcmp((char*)filename,"videorom", 8) == 0)
  {
    readsize = sizeof(videorom_data);
    memcpy(&RAM[addr32], videorom_data, readsize);
  }
  else if (memcmp((char*)filename,"pcxtbios", 8) == 0)
  {
    readsize = sizeof(pcxtbios_data);
    memcpy(&RAM[addr32], pcxtbios_data, readsize);
  }
  else
  {
    printf ("ROM load eror");
    while (1) {}
  }
  
  memset ( (void *) &readonly[addr32], roflag, readsize);
  return (readsize);
}
