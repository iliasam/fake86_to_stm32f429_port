#include "sd_card_reader.h"
#include "sdcard.h"
#include "ff.h"
#include "diskio.h"
#include "string.h"

 /* Private functions ---------------------------------------------------------*/
FATFS fs;            // Work area (file system object) for logical drive
FIL fsrc, fdst;      // file objects

FIL image_file;

FRESULT res;         // FatFs function common result code
UINT br, bw;         // File R/W count

#define SECTOR_SIZE     512
#define DTA_FILE_OFFSET 16 //in bytes

uint8_t image_data[SECTOR_SIZE];
uint32_t bytesread = 0;

#if _USE_LFN
char Fs_LongFileName[_MAX_LFN];
#endif

void NVIC_Configuration(void);
void read_file(void);

void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

  NVIC_InitStructure.NVIC_IRQChannel = SDIO_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

int8_t mount_file_reader(void)
{
  FRESULT res;

  printf("disk_initialize:%d\r\n", disk_initialize(0));
  res = f_mount(&fs, "0", 1);
  printf("f_mount:%d\r\n", (uint8_t)res);
  if (res != FR_OK)
    return -1;//mount fail
  return 1;
}

//ram_pointer - data will be placed here
/*
int8_t read_dta_file_from_card(char* image_path, uint8_t* ram_pointer)
{
  uint32_t bytes_read = 0;
  static uint32_t bmp_addr;
  uint32_t ind = 0;
  
  uint32_t image_pos = 0;
  uint32_t file_pos = DTA_FILE_OFFSET;
  
  uint32_t bytes_to_read_cnt = 0;
  uint32_t image_size = 0;//file size
  
  
  res = f_open(&image_file, image_path, FA_OPEN_EXISTING | FA_READ);
  if (res != FR_OK)
    return -2;//can not open file
  
  
  f_open (&image_file, image_path, FA_OPEN_EXISTING | FA_READ);
  image_size = 640*480*2;
  
  do
  {
    if (image_size < SECTOR_SIZE)
    {
      bytes_to_read_cnt = image_size;
    }
    else
    {
      bytes_to_read_cnt = SECTOR_SIZE;
    }
    image_size -= bytes_to_read_cnt;
    
    f_lseek(&image_file, file_pos);
    f_read (&image_file, image_data, bytes_to_read_cnt, (UINT *)&bytes_read);
    memcpy((void*)(ram_pointer+image_pos), (void*)image_data, bytes_to_read_cnt);
    image_pos+=bytes_to_read_cnt;
    file_pos+=bytes_to_read_cnt;
  }
  while (image_size > 0);
  f_close (&image_file);
      
  return 1;
}
*/

