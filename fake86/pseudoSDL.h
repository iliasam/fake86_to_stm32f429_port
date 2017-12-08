#ifndef _PSEUDO_SDL_H
#define _PSEUDO_SDL_H

#define SDL_INIT_TIMER          0x00000001u
#define SDL_INIT_AUDIO          0x00000010u
#define SDL_INIT_VIDEO          0x00000020u  /**< SDL_INIT_VIDEO implies SDL_INIT_EVENTS */

#define SDL_HWSURFACE	0x00000001	/**< Surface is in video memory */

#include <stdint.h>

uint32_t SDL_GetTicks(void);
int SDL_Init(uint32_t flags);
void SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags);

#endif

