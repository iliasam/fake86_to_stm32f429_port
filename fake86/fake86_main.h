/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FAK86_MAIN_H
#define __FAK86_MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

void fake86_inithardware(void);
int8_t fake86_main_init(void);

uint32_t loadbinary (uint32_t addr32, uint8_t *filename, uint8_t roflag);
uint32_t loadrom (uint32_t addr32, uint8_t *filename, uint8_t failure_fatal);

void fake86_run_task(void);


#endif /* __FAK86_MAIN_H */
