/**
  ******************************************************************************
  * @file    stm32f429i_discovery_lcd.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    28-October-2013
  * @brief   This file contains all the functions prototypes for the 
  *          stm32f429i_discovery_lcd.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F429I_DISCOVERY_LCD_H
#define __STM32F429I_DISCOVERY_LCD_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
//#include "stm32f429i_discovery.h"
#include "32f429_sdram.h"
#include "fonts.h"


typedef struct 
{
  int16_t X;
  int16_t Y;
} Point, * pPoint;

#define LCD_TIMING_HSYNC        96
#define LCD_TIMING_HBP          48 //back
#define LCD_TIMING_HFP          16 //front

#define LCD_TIMING_VSYNC        2
#define LCD_TIMING_VBP          33
#define LCD_TIMING_VFP          10


/* LCD Size (Width and Height) */
#define  LCD_PIXEL_WIDTH    ((uint16_t)640)
#define  LCD_PIXEL_HEIGHT   ((uint16_t)480)
#define  LCD_PIXELS_CNT     (LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT)
#define  BUFFER_OFFSET   ((uint32_t)(LCD_PIXELS_CNT*2))

#define LCD_FRAME_BUFFER       ((uint32_t)0xD0000000)
//#define LCD_FRAME_BUFFER       ((uint32_t)0x20000000)
//#define LCD_FRAME_BUFFER       ((uint32_t)0x08000000)


/** 
  * @brief  LCD color  
  */ 
#define LCD_COLOR_WHITE          0xFFFF
#define LCD_COLOR_BLACK          0x0000
#define LCD_COLOR_GREY           0xF7DE
#define LCD_COLOR_BLUE           0x001F
#define LCD_COLOR_BLUE2          0x051F
#define LCD_COLOR_RED            0xF800
#define LCD_COLOR_MAGENTA        0xF81F
#define LCD_COLOR_GREEN          0x07E0
#define LCD_COLOR_CYAN           0x7FFF
#define LCD_COLOR_YELLOW         0xFFE0

/** 
  * @brief LCD default font 
  */ 
#define LCD_DEFAULT_FONT         Font16x24

/** 
  * @brief  LCD Direction  
  */ 
#define LCD_DIR_HORIZONTAL       0x0000
#define LCD_DIR_VERTICAL         0x0001

/**
  * @}
  */ 

/** 
  * @brief  LCD Layer  
  */ 
#define LCD_BACKGROUND_LAYER     0x0000
#define LCD_FOREGROUND_LAYER     0x0001

/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Exported_Macros
  * @{
  */ 
#define ASSEMBLE_RGB(R, G, B)    ((((R)& 0xF8) << 8) | (((G) & 0xFC) << 3) | (((B) & 0xF8) >> 3))  

/**
  * @}
  */ 

/** @defgroup STM32F429I_DISCOVERY_LCD_Exported_Functions
  * @{
  */ 
void     LCD_DeInit(void);   
void     LCD_Init(void);
void     LCD_LayerInit(void);
void     LCD_SetLayer(uint32_t Layerx);
void     LCD_SetColors(uint16_t _TextColor, uint16_t _BackColor); 
void     LCD_GetColors(uint16_t *_TextColor, uint16_t *_BackColor);
void     LCD_SetTextColor(uint16_t Color);
void     LCD_SetBackColor(uint16_t Color);
void     LCD_SetTransparency(uint8_t transparency);
void     LCD_ClearLine(uint16_t Line);
void     LCD_Clear(uint16_t Color);
uint32_t LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void     LCD_SetColorKeying(uint32_t RGBValue);
void     LCD_ReSetColorKeying(void);
void     LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c);
void     LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii);
void     LCD_SetFont(sFONT *fonts);
sFONT *  LCD_GetFont(void);
void     LCD_DisplayStringLine(uint16_t Line, uint8_t *ptr);
void     LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width);
void     LCD_WindowModeDisable(void);
void     LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction);
void     LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width);
void     LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void     LCD_DrawEllipse(int Xpos, int Ypos, int Radius, int Radius2);
void     LCD_DrawFullEllipse(int Xpos, int Ypos, int Radius, int Radius2);
void     LCD_DrawMonoPict(const uint32_t *Pict);
void     LCD_WriteBMP(uint32_t BmpAddress);
void     LCD_DrawUniLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void     LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);
void     LCD_PolyLine(pPoint Points, uint16_t PointCount);
void     LCD_PolyLineRelative(pPoint Points, uint16_t PointCount);
void     LCD_ClosedPolyLine(pPoint Points, uint16_t PointCount);
void     LCD_ClosedPolyLineRelative(pPoint Points, uint16_t PointCount);
void     LCD_FillPolyLine(pPoint Points, uint16_t PointCount);
void     LCD_Triangle(pPoint Points, uint16_t PointCount);
void     LCD_FillTriangle(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3);

/**
  * @}
  */    
  
#ifdef __cplusplus
}
#endif

#endif /* __STM32F429I_DISCOVERY_LCD_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
