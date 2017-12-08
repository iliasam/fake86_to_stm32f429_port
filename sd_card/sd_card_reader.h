#ifndef __SD_CARD_READER_H
#define __SD_CARD_READER_H

#include "stdint.h"

int8_t mount_file_reader(void);
int8_t read_dta_file_from_card(char* image_path, uint8_t* ram_pointer);


#endif

