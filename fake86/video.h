#ifndef _FAKE86_VIDEO_H
#define _FAKE86_VIDEO_H

uint16_t rgb(uint8_t r, uint8_t g, uint8_t b);
void initVideoPorts(void);
uint8_t readVGA (uint32_t addr32);
void writeVGA (uint32_t addr32, uint8_t value);
uint8_t inVGA (uint16_t portnum);
void vidinterrupt(void);
void initcga(void);

#endif

