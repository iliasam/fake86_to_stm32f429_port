//This is the port of "Fake86" emulator for STM32F429 board (Core4X9I)
//This progecti is using Semihosting for printf!

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "32f429_sdram.h"
#include "32f429_lcd.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "sd_card_reader.h"
#include "fake86_main.h"
#include "render.h"
#include "config.h"

#include "usb_bsp.h"
#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_hid_core.h"


/* Private typedef -----------------------------------------------------------*/
#define LCD_RUN

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static __IO uint32_t uwTimingDelay;
RCC_ClocksTypeDef RCC_Clocks;

__ALIGN_BEGIN USB_OTG_CORE_HANDLE           USB_OTG_Core_dev __ALIGN_END ;
__ALIGN_BEGIN USBH_HOST USB_Host __ALIGN_END ;
char test_string[64];

/* Private function prototypes -----------------------------------------------*/
void init_gpio(void);

/* Private functions ---------------------------------------------------------*/

int main(void)
{ 
  //Clock is configured at SetSysClock()
  //SYSCLK is 192 MHz
  /* SysTick end of count event each 1ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
  
  init_gpio();
  SDRAM_Init();
  delay_ms(5);
 
#ifdef LCD_RUN
  LCD_Init();
  LCD_LayerInit();
  LTDC_Cmd(ENABLE);
  LCD_SetLayer(LCD_BACKGROUND_LAYER);
  LCD_Clear(LCD_COLOR_WHITE);
  LCD_SetFont(&Font16x24);
  LCD_DisplayStringLine(LINE(1), (uint8_t*)"Fake86 Port");
#endif
  
    /* Init Host Library */
  USBH_Init(&USB_OTG_Core_dev, 
#ifdef USE_USB_OTG_FS
            USB_OTG_FS_CORE_ID,
#else
            USB_OTG_HS_CORE_ID,
#endif
            &USB_Host,
            &HID_cb, 
            &USR_Callbacks);
  
  delay_ms(100);

  int8_t result = mount_file_reader();
  if (result < 0)
  {
#ifdef LCD_RUN
    LCD_DisplayStringLine(LINE(2), (uint8_t*)"Card Mount fail");
#endif
    while(1) {}
  }
  
  fake86_main_init();

  GPIO_SetBits(GPIOI, GPIO_Pin_7);//LED12
  
  while (1)
  {
    fake86_run_task();
    VideoThread();
    user_usb_process();
  }
}
  
void user_usb_process(void)
{
  USBH_Process(&USB_OTG_Core_dev , &USB_Host);
}

void init_gpio(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
  
  GPIO_StructInit(&GPIO_InitStructure);
  //LEDS
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOI, &GPIO_InitStructure);
}


void delay_ms(__IO uint32_t nTime)
{ 
  uwTimingDelay = nTime;
  while(uwTimingDelay != 0);
}


void TimingDelay_Decrement(void)
{
  if (uwTimingDelay != 0x00)
  { 
    uwTimingDelay--;
  }
}

