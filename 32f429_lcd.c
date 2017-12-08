/* Includes ------------------------------------------------------------------*/
#include "32f429_lcd.h"
#include "fonts.c"



#define POLY_Y(Z)          ((int32_t)((Points + Z)->X))
#define POLY_X(Z)          ((int32_t)((Points + Z)->Y))   


#define ABS(X)  ((X) > 0 ? (X) : -(X))    

  

static sFONT *LCD_Currentfonts;
/* Global variables to set the written text color */
static uint16_t CurrentTextColor   = 0x0000;
static uint16_t CurrentBackColor   = 0xFFFF;

/* Default LCD configuration with LCD Layer 1 */
static uint32_t CurrentFrameBuffer = LCD_FRAME_BUFFER;
static uint32_t CurrentLayer = LCD_BACKGROUND_LAYER;

static void PutPixel(int16_t x, int16_t y);
static void LCD_PolyLineRelativeClosed(pPoint Points, uint16_t PointCount, uint16_t Closed);
static void LCD_AF_GPIOConfig(void);


void LCD_DeInit(void)
{ 

}


void LCD_Init(void)
{ 
  LTDC_InitTypeDef       LTDC_InitStruct;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_LTDC, ENABLE); /* Enable the LTDC Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2D, ENABLE); /* Enable the DMA2D Clock */
  
  LCD_AF_GPIOConfig();  /* Configure the LCD Control pins */ 
  
  //SDRAM_Init();
  LTDC_StructInit(&LTDC_InitStruct);
  
  /* LTDC Configuration *********************************************************/  
  LTDC_InitStruct.LTDC_HSPolarity = LTDC_HSPolarity_AL;//active low
  LTDC_InitStruct.LTDC_VSPolarity = LTDC_VSPolarity_AL;
  LTDC_InitStruct.LTDC_DEPolarity = LTDC_DEPolarity_AL;
  LTDC_InitStruct.LTDC_PCPolarity = LTDC_PCPolarity_IPC;
  
  /* Configure R,G,B component values for LCD background color */                   
  LTDC_InitStruct.LTDC_BackgroundRedValue       = 0;
  LTDC_InitStruct.LTDC_BackgroundGreenValue     = 0;
  LTDC_InitStruct.LTDC_BackgroundBlueValue      = 0;
  
  //PIXCLK = (HSE / PLL_M) * PLLSAI_N / PLLSAI_R / RCC_PLLSAIDivR
  //PIXCLK = (8Mhz / 4) * 201 / 4 / 4 = 25.125 Mhz
   //PIXCLK = (8Mhz / 8) * 402 / 4 / 4 = 25.125 Mhz
  //N, Q, R
  RCC_PLLSAIConfig(402, 7, 4);
  RCC_LTDCCLKDivConfig(RCC_PLLSAIDivR_Div4);
  
  RCC_PLLSAICmd(ENABLE);/* Enable PLLSAI Clock */
  /* Wait for PLLSAI activation */
  while(RCC_GetFlagStatus(RCC_FLAG_PLLSAIRDY) == RESET)
  {
  }
  
  /* Timing configuration */  
  // HSync = 96       VSync = 2
  // HBP   = 48       VBP   = 33
  // HFP   = 16       VFP   = 10
  // Landscape  : W=640, H=480
  //---------------------------------------
  LTDC_InitStruct.LTDC_HorizontalSync   = LCD_TIMING_HSYNC;  // HSync
  LTDC_InitStruct.LTDC_VerticalSync     = LCD_TIMING_VSYNC;  // VSync
  LTDC_InitStruct.LTDC_AccumulatedHBP   = (LCD_TIMING_HSYNC + LCD_TIMING_HBP); // HSync+HBP
  LTDC_InitStruct.LTDC_AccumulatedVBP   = (LCD_TIMING_VSYNC + LCD_TIMING_VBP);  // VSync+VBP
  LTDC_InitStruct.LTDC_AccumulatedActiveW = (LCD_TIMING_HSYNC + LCD_TIMING_HBP + LCD_PIXEL_WIDTH); // HSync+HBP+W
  LTDC_InitStruct.LTDC_AccumulatedActiveH = (LCD_TIMING_VSYNC + LCD_TIMING_VBP + LCD_PIXEL_HEIGHT); // VSync+VBP+H
  LTDC_InitStruct.LTDC_TotalWidth       = (LCD_TIMING_HSYNC + LCD_TIMING_HBP + LCD_PIXEL_WIDTH + LCD_TIMING_HFP);  // HSync+HBP+W+HFP
  LTDC_InitStruct.LTDC_TotalHeigh       = (LCD_TIMING_VSYNC + LCD_TIMING_VBP + LCD_PIXEL_HEIGHT + LCD_TIMING_VFP);  // VSync+VBP+H+VFP

  LTDC_Init(&LTDC_InitStruct);
}  

/**
  * @brief  Initializes the LCD Layers.
  * @param  None
  * @retval None
  */
void LCD_LayerInit(void)
{
  LTDC_Layer_InitTypeDef LTDC_Layer_InitStruct; 
  LTDC_LayerStructInit(&LTDC_Layer_InitStruct);
  
  /* Windowing configuration */
  /* In this case all the active display area is used to display a picture then :
  Horizontal start = horizontal synchronization + Horizontal back porch
  Horizontal stop = Horizontal start + window width - 1
  Vertical start   = vertical synchronization + vertical back porch
  Vertical stop   = Vertical start + window height - 1
  */
  LTDC_Layer_InitStruct.LTDC_HorizontalStart = (LCD_TIMING_HSYNC + LCD_TIMING_HBP);
  LTDC_Layer_InitStruct.LTDC_HorizontalStop = (LCD_PIXEL_WIDTH + (LCD_TIMING_HSYNC + LCD_TIMING_HBP) - 1);
  LTDC_Layer_InitStruct.LTDC_VerticalStart = (LCD_TIMING_VSYNC + LCD_TIMING_VBP);
  LTDC_Layer_InitStruct.LTDC_VerticalStop = (LCD_PIXEL_HEIGHT + (LCD_TIMING_VSYNC + LCD_TIMING_VBP) - 1);

  /* the length of one line of pixels in bytes + 3 then :
  Line Lenth = Active high width x number of bytes per pixel + 3 
  Active high width         = LCD_PIXEL_WIDTH 
  number of bytes per pixel = 2    (pixel_format : RGB565) 
  */
  LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((LCD_PIXEL_WIDTH * 2) + 3);
  //LTDC_Layer_InitStruct.LTDC_CFBLineLength = ((LCD_PIXEL_WIDTH) + 3);//8bit

  /* the pitch is the increment from the start of one line of pixels to the 
  start of the next line in bytes, then :
  Pitch = Active high width x number of bytes per pixel */ 
  LTDC_Layer_InitStruct.LTDC_CFBPitch = (LCD_PIXEL_WIDTH * 2);
  //LTDC_Layer_InitStruct.LTDC_CFBPitch = (LCD_PIXEL_WIDTH);//8bit
  
  /* Configure the number of lines */  
  LTDC_Layer_InitStruct.LTDC_CFBLineNumber = LCD_PIXEL_HEIGHT;

  LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_RGB565;/* Pixel Format configuration*/
  //LTDC_Layer_InitStruct.LTDC_PixelFormat = LTDC_Pixelformat_L8;/* Pixel Format configuration*/
  LTDC_Layer_InitStruct.LTDC_ConstantAlpha = 255; /* Alpha constant (255 totally opaque) */
  /* Default Color configuration (configure A,R,G,B component values) */          
  LTDC_Layer_InitStruct.LTDC_DefaultColorBlue = 0;        
  LTDC_Layer_InitStruct.LTDC_DefaultColorGreen = 0;       
  LTDC_Layer_InitStruct.LTDC_DefaultColorRed = 0;         
  LTDC_Layer_InitStruct.LTDC_DefaultColorAlpha = 0;
  /* Configure blending factors */       
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_CA;    
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_CA;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM */    
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD_FRAME_BUFFER;
  LTDC_LayerInit(LTDC_Layer1, &LTDC_Layer_InitStruct);/* Initialize LTDC layer 1 */
  
  /* Configure Layer2 */
  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */     
  LTDC_Layer_InitStruct.LTDC_CFBStartAdress = LCD_FRAME_BUFFER + BUFFER_OFFSET; 
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_1 = LTDC_BlendingFactor1_PAxCA;    
  LTDC_Layer_InitStruct.LTDC_BlendingFactor_2 = LTDC_BlendingFactor2_PAxCA;
  //LTDC_LayerInit(LTDC_Layer2, &LTDC_Layer_InitStruct);
  LTDC_ReloadConfig(LTDC_IMReload);/* LTDC configuration reload */  
  
  /* Enable foreground & background Layers */
  LTDC_LayerCmd(LTDC_Layer1, ENABLE); 
  //LTDC_LayerCmd(LTDC_Layer2, ENABLE);
  LTDC_ReloadConfig(LTDC_IMReload);/* LTDC configuration reload */  
  
  LCD_SetFont(&LCD_DEFAULT_FONT); /* Set default font */  

  //LTDC_DitherCmd(ENABLE);/* dithering activation */
  LTDC_DitherCmd(DISABLE);/* dithering activation */
}

/**
  * @brief  Sets the LCD Layer.
  * @param  Layerx: specifies the Layer foreground or background.
  * @retval None
  */
void LCD_SetLayer(uint32_t Layerx)
{
  if (Layerx == LCD_BACKGROUND_LAYER)
  {
    CurrentFrameBuffer = LCD_FRAME_BUFFER; 
    CurrentLayer = LCD_BACKGROUND_LAYER;
  }
  else
  {
    CurrentFrameBuffer = LCD_FRAME_BUFFER + BUFFER_OFFSET;
    CurrentLayer = LCD_FOREGROUND_LAYER;
  }
}  

/**
  * @brief  Sets the LCD Text and Background colors.
  * @param  TextColor: specifies the Text Color.
  * @param  BackColor: specifies the Background Color.
  * @retval None
  */
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor)
{
  CurrentTextColor = TextColor; 
  CurrentBackColor = BackColor;
}

/**
  * @brief  Gets the LCD Text and Background colors.
  * @param  TextColor: pointer to the variable that will contain the Text 
            Color.
  * @param  BackColor: pointer to the variable that will contain the Background 
            Color.
  * @retval None
  */
void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor)
{
  *TextColor = CurrentTextColor;
  *BackColor = CurrentBackColor;
}

/**
  * @brief  Sets the Text color.
  * @param  Color: specifies the Text color code RGB(5-6-5).
  * @retval None
  */
void LCD_SetTextColor(uint16_t Color)
{
  CurrentTextColor = Color;
}

/**
  * @brief  Sets the Background color.
  * @param  Color: specifies the Background color code RGB(5-6-5).
  * @retval None
  */
void LCD_SetBackColor(uint16_t Color)
{
  CurrentBackColor = Color;
}

/**
  * @brief  Sets the Text Font.
  * @param  fonts: specifies the font to be used.
  * @retval None
  */
void LCD_SetFont(sFONT *fonts)
{
  LCD_Currentfonts = fonts;
}

/**
  * @brief  Configure the transparency.
  * @param  transparency: specifies the transparency, 
  *         This parameter must range from 0x00 to 0xFF.
  * @retval None
  */
void LCD_SetTransparency(uint8_t transparency)
{
  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  {
    LTDC_LayerAlpha(LTDC_Layer1, transparency);
  }
  else
  {     
    LTDC_LayerAlpha(LTDC_Layer2, transparency);
  }
  LTDC_ReloadConfig(LTDC_IMReload);
}

/**
  * @brief  Gets the Text Font.
  * @param  None.
  * @retval the used font.
  */
sFONT *LCD_GetFont(void)
{
  return LCD_Currentfonts;
}

/**
  * @brief  Clears the selected line.
  * @param  Line: the Line to be cleared.
  *   This parameter can be one of the following values:
  *     @arg LCD_LINE_x: where x can be: 0..13 if LCD_Currentfonts is Font16x24
  *                                      0..26 if LCD_Currentfonts is Font12x12 or Font8x12
  *                                      0..39 if LCD_Currentfonts is Font8x8
  * @retval None
  */
void LCD_ClearLine(uint16_t Line)
{
  uint16_t refcolumn = 0;
  /* Send the string character by character on lCD */
  while ((refcolumn < LCD_PIXEL_WIDTH) && (((refcolumn + LCD_Currentfonts->Width)& 0xFFFF) >= LCD_Currentfonts->Width))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, ' ');
    /* Decrement the column position by 16 */
    refcolumn += LCD_Currentfonts->Width;
  }
}

/**
  * @brief  Clears the hole LCD.
  * @param  Color: the color of the background.
  * @retval None
  */
void LCD_Clear(uint16_t Color)
{
  uint32_t index = 0;
  /* erase memory */
  for (index = 0x00; index < BUFFER_OFFSET; index++)
  {
    *(__IO uint16_t*)(CurrentFrameBuffer + (2*index)) = Color;
  } 
}

/**
  * @brief  Sets the cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position. 
  * @retval Display Address
  */
uint32_t LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{  
  return CurrentFrameBuffer + 2*(Xpos + (LCD_PIXEL_WIDTH*Ypos));
}

/**
  * @brief  Config and Sets the color Keying.
  * @param  RGBValue: Specifies the Color reference. 
  * @retval None
  */
void LCD_SetColorKeying(uint32_t RGBValue)
{  
  LTDC_ColorKeying_InitTypeDef   LTDC_colorkeying_InitStruct;
  
  /* configure the color Keying */
  LTDC_colorkeying_InitStruct.LTDC_ColorKeyBlue = 0x0000FF & RGBValue;
  LTDC_colorkeying_InitStruct.LTDC_ColorKeyGreen = (0x00FF00 & RGBValue) >> 8;
  LTDC_colorkeying_InitStruct.LTDC_ColorKeyRed = (0xFF0000 & RGBValue) >> 16;  

  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  {   
    /* Enable the color Keying for Layer1 */
    LTDC_ColorKeyingConfig(LTDC_Layer1, &LTDC_colorkeying_InitStruct, ENABLE);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
  else
  {
    /* Enable the color Keying for Layer2 */
    LTDC_ColorKeyingConfig(LTDC_Layer2, &LTDC_colorkeying_InitStruct, ENABLE);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
}

/**
  * @brief  Disable the color Keying.
  * @param  RGBValue: Specifies the Color reference. 
  * @retval None
  */
void LCD_ReSetColorKeying(void)
{
  LTDC_ColorKeying_InitTypeDef   LTDC_colorkeying_InitStruct;
  
  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  {   
    /* Disable the color Keying for Layer1 */
    LTDC_ColorKeyingConfig(LTDC_Layer1, &LTDC_colorkeying_InitStruct, DISABLE);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
  else
  {
    /* Disable the color Keying for Layer2 */
    LTDC_ColorKeyingConfig(LTDC_Layer2, &LTDC_colorkeying_InitStruct, DISABLE);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
} 

/**
  * @brief  Draws a character on LCD.
  * @param  Xpos: the Line where to display the character shape.
  * @param  Ypos: start column address.
  * @param  c: pointer to the character data.
  * @retval None
  */
void LCD_DrawChar(uint16_t Xpos, uint16_t Ypos, const uint16_t *c)
{
  uint32_t index = 0, counter = 0, xpos =0;
  uint32_t  Xaddress = 0;
  
  xpos = Xpos*LCD_PIXEL_WIDTH*2;
  Xaddress += Ypos;
  
  for(index = 0; index < LCD_Currentfonts->Height; index++)
  {
    
    for(counter = 0; counter < LCD_Currentfonts->Width; counter++)
    {
          
      if((((c[index] & ((0x80 << ((LCD_Currentfonts->Width / 12 ) * 8 ) ) >> counter)) == 0x00) &&(LCD_Currentfonts->Width <= 12))||
        (((c[index] & (0x1 << counter)) == 0x00)&&(LCD_Currentfonts->Width > 12 )))
      {
          /* Write data value to all SDRAM memory */
         *(__IO uint16_t*) (CurrentFrameBuffer + (2*Xaddress) + xpos) = CurrentBackColor;
      }
      else
      {
          /* Write data value to all SDRAM memory */
         *(__IO uint16_t*) (CurrentFrameBuffer + (2*Xaddress) + xpos) = CurrentTextColor;         
      }
      Xaddress++;
    }
      Xaddress += (LCD_PIXEL_WIDTH - LCD_Currentfonts->Width);
  }
}

/**
  * @brief  Displays one character (16dots width, 24dots height).
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..29
  * @param  Column: start column address.
  * @param  Ascii: character ascii code, must be between 0x20 and 0x7E.
  * @retval None
  */
void LCD_DisplayChar(uint16_t Line, uint16_t Column, uint8_t Ascii)
{
  Ascii -= 32;

  LCD_DrawChar(Line, Column, &LCD_Currentfonts->table[Ascii * LCD_Currentfonts->Height]);
}

/**
  * @brief  Displays a maximum of 20 char on the LCD.
  * @param  Line: the Line where to display the character shape .
  *   This parameter can be one of the following values:
  *     @arg Linex: where x can be 0..9
  * @param  *ptr: pointer to string to display on LCD.
  * @retval None
  */
void LCD_DisplayStringLine(uint16_t Line, uint8_t *ptr)
{  
  uint16_t refcolumn = 0;
  /* Send the string character by character on lCD */
  while ((refcolumn < LCD_PIXEL_WIDTH) && ((*ptr != 0) & (((refcolumn + LCD_Currentfonts->Width) & 0xFFFF) >= LCD_Currentfonts->Width)))
  {
    /* Display one character on LCD */
    LCD_DisplayChar(Line, refcolumn, *ptr);
    /* Decrement the column position by width */
    refcolumn += LCD_Currentfonts->Width;
    /* Point on the next character */
    ptr++;
  }
}

/**
  * @brief  Sets a display window
  * @param  Xpos: specifies the X bottom left position from 0 to 240.
  * @param  Ypos: specifies the Y bottom left position from 0 to 320.
  * @param  Height: display window height, can be a value from 0 to 320.
  * @param  Width: display window width, can be a value from 0 to 240.
  * @retval None
  */
void LCD_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{

  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  { 
    /* reconfigure the layer1 position */
    LTDC_LayerPosition(LTDC_Layer1, Xpos, Ypos);
    LTDC_ReloadConfig(LTDC_IMReload);
    
    /* reconfigure the layer1 size */
    LTDC_LayerSize(LTDC_Layer1, Width, Height);
    LTDC_ReloadConfig(LTDC_IMReload);
 }
 else
 {   
    /* reconfigure the layer2 position */
    LTDC_LayerPosition(LTDC_Layer2, Xpos, Ypos);
    LTDC_ReloadConfig(LTDC_IMReload); 
   
   /* reconfigure the layer2 size */
    LTDC_LayerSize(LTDC_Layer2, Width, Height);
    LTDC_ReloadConfig(LTDC_IMReload);
  }
}

/**
  * @brief  Disables LCD Window mode.
  * @param  None
  * @retval None
  */
void LCD_WindowModeDisable(void)
{
  LCD_SetDisplayWindow(0, 0, LCD_PIXEL_HEIGHT, LCD_PIXEL_WIDTH); 
}

/**
  * @brief  Displays a line.
  * @param Xpos: specifies the X position, can be a value from 0 to 480.
  * @param Ypos: specifies the Y position, can be a value from 0 to 272.
  * @param Length: line length.
  * @param Direction: line direction.
  *   This parameter can be one of the following values: LCD_DIR_HORIZONTAL or LCD_DIR_VERTICAL.
  * @retval None
  */
void LCD_DrawLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length, uint8_t Direction)
{
  DMA2D_InitTypeDef      DMA2D_InitStruct;
  
  uint32_t  Xaddress = 0;
  uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0;
  
  Xaddress = CurrentFrameBuffer + 2*(LCD_PIXEL_WIDTH*Ypos + Xpos);
 
  Red_Value = (0xF800 & CurrentTextColor) >> 11;
  Blue_Value = 0x001F & CurrentTextColor;
  Green_Value = (0x07E0 & CurrentTextColor) >> 5;

  /* Configure DMA2D */    
  DMA2D_DeInit();  
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;       
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;      
  DMA2D_InitStruct.DMA2D_OutputGreen = Green_Value;      
  DMA2D_InitStruct.DMA2D_OutputBlue = Blue_Value;     
  DMA2D_InitStruct.DMA2D_OutputRed = Red_Value;                
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;                  
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;                  
  
  if(Direction == LCD_DIR_HORIZONTAL)
  {                                                      
    DMA2D_InitStruct.DMA2D_OutputOffset = 0;                
    DMA2D_InitStruct.DMA2D_NumberOfLine = 1;            
    DMA2D_InitStruct.DMA2D_PixelPerLine = Length; 
  }
  else
  {                                                            
    DMA2D_InitStruct.DMA2D_OutputOffset = LCD_PIXEL_WIDTH - 1;                
    DMA2D_InitStruct.DMA2D_NumberOfLine = Length;            
    DMA2D_InitStruct.DMA2D_PixelPerLine = 1;  
  }
  
  DMA2D_Init(&DMA2D_InitStruct);  
  /* Start Transfer */ 
  DMA2D_StartTransfer();  
  /* Wait for CTC Flag activation */
  while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
  {
  }
  
}

/**
  * @brief  Displays a rectangle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Height: display rectangle height, can be a value from 0 to 320.
  * @param  Width: display rectangle width, can be a value from 0 to 240.
  * @retval None
  */
void LCD_DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Height, uint16_t Width)
{
  /* draw horizontal lines */
  LCD_DrawLine(Xpos, Ypos, Width, LCD_DIR_HORIZONTAL);
  LCD_DrawLine(Xpos, (Ypos+ Height), Width, LCD_DIR_HORIZONTAL);
  
  /* draw vertical lines */
  LCD_DrawLine(Xpos, Ypos, Height, LCD_DIR_VERTICAL);
  LCD_DrawLine((Xpos + Width), Ypos, Height, LCD_DIR_VERTICAL);
}

/**
  * @brief  Draw a circle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius: radius of the circle.
  * @retval None
  */
void LCD_DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
    int x = -Radius, y = 0, err = 2-2*Radius, e2;
    do {
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor; 
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor;
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;
        *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;
      
        e2 = err;
        if (e2 <= y) {
            err += ++y*2+1;
            if (-x == y && e2 <= x) e2 = 0;
        }
        if (e2 > x) err += ++x*2+1;
    }
    while (x <= 0);
}

/**
  * @brief  Draw a full ellipse.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius: minor radius of ellipse.
  * @param  Radius2: major radius of ellipse.  
  * @retval None
  */
void LCD_DrawFullEllipse(int Xpos, int Ypos, int Radius, int Radius2)
{
  int x = -Radius, y = 0, err = 2-2*Radius, e2;
  float K = 0, rad1 = 0, rad2 = 0;
  
  rad1 = Radius;
  rad2 = Radius2;
  
  if (Radius > Radius2)
  { 
    do 
    {
      K = (float)(rad1/rad2);
      LCD_DrawLine((Xpos+x), (Ypos-(uint16_t)(y/K)), (2*(uint16_t)(y/K) + 1), LCD_DIR_VERTICAL);
      LCD_DrawLine((Xpos-x), (Ypos-(uint16_t)(y/K)), (2*(uint16_t)(y/K) + 1), LCD_DIR_VERTICAL);
      
      e2 = err;
      if (e2 <= y) 
      {
        err += ++y*2+1;
        if (-x == y && e2 <= x) e2 = 0;
      }
      if (e2 > x) err += ++x*2+1;
      
    }
    while (x <= 0);
  }
  else
  {
    y = -Radius2; 
    x = 0;
    do 
    { 
      K = (float)(rad2/rad1);       
      LCD_DrawLine((Xpos-(uint16_t)(x/K)), (Ypos+y), (2*(uint16_t)(x/K) + 1), LCD_DIR_HORIZONTAL);
      LCD_DrawLine((Xpos-(uint16_t)(x/K)), (Ypos-y), (2*(uint16_t)(x/K) + 1), LCD_DIR_HORIZONTAL);
      
      e2 = err;
      if (e2 <= x) 
      {
        err += ++x*2+1;
        if (-y == x && e2 <= y) e2 = 0;
      }
      if (e2 > y) err += ++y*2+1;
    }
    while (y <= 0);
  }
}

/**
  * @brief  Displays an Ellipse.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius: specifies Radius.
  * @param  Radius2: specifies Radius2.
  * @retval None
  */
void LCD_DrawEllipse(int Xpos, int Ypos, int Radius, int Radius2)
{
  int x = -Radius, y = 0, err = 2-2*Radius, e2;
  float K = 0, rad1 = 0, rad2 = 0;
   
  rad1 = Radius;
  rad2 = Radius2;
  
  if (Radius > Radius2)
  { 
    do {
      K = (float)(rad1/rad2);
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos+(uint16_t)(y/K))))) = CurrentTextColor; 
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos+(uint16_t)(y/K))))) = CurrentTextColor;
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+x) + LCD_PIXEL_WIDTH*(Ypos-(uint16_t)(y/K))))) = CurrentTextColor;
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-x) + LCD_PIXEL_WIDTH*(Ypos-(uint16_t)(y/K))))) = CurrentTextColor;
            
      e2 = err;
      if (e2 <= y) {
        err += ++y*2+1;
        if (-x == y && e2 <= x) e2 = 0;
      }
      if (e2 > x) err += ++x*2+1;
    }
    while (x <= 0);
  }
  else
  {
    y = -Radius2; 
    x = 0;
    do { 
      K = (float)(rad2/rad1);
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-(uint16_t)(x/K)) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor; 
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+(uint16_t)(x/K)) + LCD_PIXEL_WIDTH*(Ypos+y)))) = CurrentTextColor;
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos+(uint16_t)(x/K)) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;
      *(__IO uint16_t*) (CurrentFrameBuffer + (2*((Xpos-(uint16_t)(x/K)) + LCD_PIXEL_WIDTH*(Ypos-y)))) = CurrentTextColor;
      
      e2 = err;
      if (e2 <= x) {
        err += ++x*2+1;
        if (-y == x && e2 <= y) e2 = 0;
      }
      if (e2 > y) err += ++y*2+1;     
    }
    while (y <= 0);
  }
}

/**
  * @brief  Displays a mono-color picture.
  * @param  Pict: pointer to the picture array.
  * @retval None
  */
void LCD_DrawMonoPict(const uint32_t *Pict)
{
  uint32_t index = 0, counter = 0;
  
   
  for(index = 0; index < 2400; index++)
  {
    for(counter = 0; counter < 32; counter++)
    {
      if((Pict[index] & (1 << counter)) == 0x00)
      {
        *(__IO uint16_t*)(CurrentFrameBuffer) = CurrentBackColor;
      }
      else
      {
        *(__IO uint16_t*)(CurrentFrameBuffer) = CurrentTextColor;
      }
    }
  }
}

/**
  * @brief  Displays a bitmap picture loaded in the internal Flash.
  * @param  BmpAddress: Bmp picture address in the internal Flash.
  * @retval None
  */
void LCD_WriteBMP(uint32_t BmpAddress)
{
  uint32_t index = 0, size = 0, width = 0, height = 0, bit_pixel = 0;
  uint32_t Address;
  uint32_t currentline = 0, linenumber = 0;
 
  Address = CurrentFrameBuffer;

  /* Read bitmap size */
  size = *(__IO uint16_t *) (BmpAddress + 2);
  size |= (*(__IO uint16_t *) (BmpAddress + 4)) << 16;
  
  /* Get bitmap data address offset */
  index = *(__IO uint16_t *) (BmpAddress + 10);
  index |= (*(__IO uint16_t *) (BmpAddress + 12)) << 16;

  /* Read bitmap width */
  width = *(uint16_t *) (BmpAddress + 18);
  width |= (*(uint16_t *) (BmpAddress + 20)) << 16;

  /* Read bitmap height */
  height = *(uint16_t *) (BmpAddress + 22);
  height |= (*(uint16_t *) (BmpAddress + 24)) << 16;

  /* Read bit/pixel */
  bit_pixel = *(uint16_t *) (BmpAddress + 28);  
 
  if (CurrentLayer == LCD_BACKGROUND_LAYER)
  {
    /* reconfigure layer size in accordance with the picture */
    LTDC_LayerSize(LTDC_Layer1, width, height);
    LTDC_ReloadConfig(LTDC_VBReload);

    /* Reconfigure the Layer pixel format in accordance with the picture */    
    if ((bit_pixel/8) == 4)
    {
      LTDC_LayerPixelFormat(LTDC_Layer1, LTDC_Pixelformat_ARGB8888);
      LTDC_ReloadConfig(LTDC_VBReload);
    }
    else if ((bit_pixel/8) == 2)
    {
      LTDC_LayerPixelFormat(LTDC_Layer1, LTDC_Pixelformat_RGB565);
      LTDC_ReloadConfig(LTDC_VBReload);
    }
    else 
    {
      LTDC_LayerPixelFormat(LTDC_Layer1, LTDC_Pixelformat_RGB888);
      LTDC_ReloadConfig(LTDC_VBReload);
    } 
  }
  else
  {
    /* reconfigure layer size in accordance with the picture */
    LTDC_LayerSize(LTDC_Layer2, width, height);
    LTDC_ReloadConfig(LTDC_VBReload); 
    
    /* Reconfigure the Layer pixel format in accordance with the picture */
    if ((bit_pixel/8) == 4)
    {
      LTDC_LayerPixelFormat(LTDC_Layer2, LTDC_Pixelformat_ARGB8888);
      LTDC_ReloadConfig(LTDC_VBReload);
    }
    else if ((bit_pixel/8) == 2)
    {
      LTDC_LayerPixelFormat(LTDC_Layer2, LTDC_Pixelformat_RGB565);
      LTDC_ReloadConfig(LTDC_VBReload);
    }
    else
    {
      LTDC_LayerPixelFormat(LTDC_Layer2, LTDC_Pixelformat_RGB888);
      LTDC_ReloadConfig(LTDC_VBReload);  
    }
  }
 
  /* compute the real size of the picture (without the header)) */  
  size = (size - index); 

  /* bypass the bitmap header */
  BmpAddress += index;

  /* start copie image from the bottom */
  Address += width*(height-1)*(bit_pixel/8);
  
  for(index = 0; index < size; index++)
  {
    *(__IO uint8_t*) (Address) = *(__IO uint8_t *)BmpAddress;
    
    /*jump on next byte */   
    BmpAddress++;
    Address++;
    currentline++;
    
    if((currentline/(bit_pixel/8)) == width)
    {
      if(linenumber < height)
      {
        linenumber++;
        Address -=(2*width*(bit_pixel/8));
        currentline = 0;
      }
    }
  }
}

/**
  * @brief  Displays a full rectangle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Height: rectangle height.
  * @param  Width: rectangle width.
  * @retval None
  */
void LCD_DrawFullRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  DMA2D_InitTypeDef      DMA2D_InitStruct;
  
  uint32_t  Xaddress = 0; 
  uint16_t Red_Value = 0, Green_Value = 0, Blue_Value = 0;
 
  Red_Value = (0xF800 & CurrentTextColor) >> 11;
  Blue_Value = 0x001F & CurrentTextColor;
  Green_Value = (0x07E0 & CurrentTextColor) >> 5;
  
  Xaddress = CurrentFrameBuffer + 2*(LCD_PIXEL_WIDTH*Ypos + Xpos);
  
  /* configure DMA2D */
  DMA2D_DeInit();
  DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;       
  DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;      
  DMA2D_InitStruct.DMA2D_OutputGreen = Green_Value;      
  DMA2D_InitStruct.DMA2D_OutputBlue = Blue_Value;     
  DMA2D_InitStruct.DMA2D_OutputRed = Red_Value;                
  DMA2D_InitStruct.DMA2D_OutputAlpha = 0x0F;                  
  DMA2D_InitStruct.DMA2D_OutputMemoryAdd = Xaddress;                
  DMA2D_InitStruct.DMA2D_OutputOffset = (LCD_PIXEL_WIDTH - Width);                
  DMA2D_InitStruct.DMA2D_NumberOfLine = Height;            
  DMA2D_InitStruct.DMA2D_PixelPerLine = Width;
  DMA2D_Init(&DMA2D_InitStruct); 
  
  /* Start Transfer */ 
  DMA2D_StartTransfer();
  
  /* Wait for CTC Flag activation */
  while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
  {
  } 

  LCD_SetTextColor(CurrentTextColor);
}

/**
  * @brief  Displays a full circle.
  * @param  Xpos: specifies the X position, can be a value from 0 to 240.
  * @param  Ypos: specifies the Y position, can be a value from 0 to 320.
  * @param  Radius
  * @retval None
  */
void LCD_DrawFullCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius)
{
  int32_t  D;    /* Decision Variable */ 
  uint32_t  CurX;/* Current X Value */
  uint32_t  CurY;/* Current Y Value */ 
  
  D = 3 - (Radius << 1);
  
  CurX = 0;
  CurY = Radius;
  
  while (CurX <= CurY)
  {
    if(CurY > 0) 
    {
      LCD_DrawLine(Xpos - CurX, Ypos - CurY, 2*CurY, LCD_DIR_VERTICAL);
      LCD_DrawLine(Xpos + CurX, Ypos - CurY, 2*CurY, LCD_DIR_VERTICAL);
    }
    
    if(CurX > 0) 
    {
      LCD_DrawLine(Xpos - CurY, Ypos - CurX, 2*CurX, LCD_DIR_VERTICAL);
      LCD_DrawLine(Xpos + CurY, Ypos - CurX, 2*CurX, LCD_DIR_VERTICAL);
    }
    if (D < 0)
    { 
      D += (CurX << 2) + 6;
    }
    else
    {
      D += ((CurX - CurY) << 2) + 10;
      CurY--;
    }
    CurX++;
  }
  
  LCD_DrawCircle(Xpos, Ypos, Radius);  
}

/**
  * @brief  Displays an uni-line (between two points).
  * @param  x1: specifies the point 1 x position.
  * @param  y1: specifies the point 1 y position.
  * @param  x2: specifies the point 2 x position.
  * @param  y2: specifies the point 2 y position.
  * @retval None
  */
void LCD_DrawUniLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    PutPixel(x, y);             /* Draw the current pixel */
    num += numadd;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  }
}

/**
  * @brief  Displays an triangle.
  * @param  Points: pointer to the points array.
  * @retval None
  */
void LCD_Triangle(pPoint Points, uint16_t PointCount)
{
  int16_t X = 0, Y = 0;
  pPoint First = Points;

  if(PointCount != 3)
  {
    return;
  }

  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    LCD_DrawUniLine(X, Y, Points->X, Points->Y);
  }
  LCD_DrawUniLine(First->X, First->Y, Points->X, Points->Y);
}

/**
  * @brief  Fill an triangle (between 3 points).
  * @param  x1..3: x position of triangle point 1..3.
  * @param  y1..3: y position of triangle point 1..3.
  * @retval None
  */
void LCD_FillTriangle(uint16_t x1, uint16_t x2, uint16_t x3, uint16_t y1, uint16_t y2, uint16_t y3)
{ 
  
  int16_t deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
  yinc1 = 0, yinc2 = 0, den = 0, num = 0, numadd = 0, numpixels = 0, 
  curpixel = 0;
  
  deltax = ABS(x2 - x1);        /* The difference between the x's */
  deltay = ABS(y2 - y1);        /* The difference between the y's */
  x = x1;                       /* Start x off at the first pixel */
  y = y1;                       /* Start y off at the first pixel */
  
  if (x2 >= x1)                 /* The x-values are increasing */
  {
    xinc1 = 1;
    xinc2 = 1;
  }
  else                          /* The x-values are decreasing */
  {
    xinc1 = -1;
    xinc2 = -1;
  }
  
  if (y2 >= y1)                 /* The y-values are increasing */
  {
    yinc1 = 1;
    yinc2 = 1;
  }
  else                          /* The y-values are decreasing */
  {
    yinc1 = -1;
    yinc2 = -1;
  }
  
  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
  {
    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
    yinc2 = 0;                  /* Don't change the y for every iteration */
    den = deltax;
    num = deltax / 2;
    numadd = deltay;
    numpixels = deltax;         /* There are more x-values than y-values */
  }
  else                          /* There is at least one y-value for every x-value */
  {
    xinc2 = 0;                  /* Don't change the x for every iteration */
    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
    den = deltay;
    num = deltay / 2;
    numadd = deltax;
    numpixels = deltay;         /* There are more y-values than x-values */
  }
  
  for (curpixel = 0; curpixel <= numpixels; curpixel++)
  {
    LCD_DrawUniLine(x, y, x3, y3);
    
    num += numadd;              /* Increase the numerator by the top of the fraction */
    if (num >= den)             /* Check if numerator >= denominator */
    {
      num -= den;               /* Calculate the new numerator value */
      x += xinc1;               /* Change the x as appropriate */
      y += yinc1;               /* Change the y as appropriate */
    }
    x += xinc2;                 /* Change the x as appropriate */
    y += yinc2;                 /* Change the y as appropriate */
  }  
  
  
}
/**
  * @brief  Displays an poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_PolyLine(pPoint Points, uint16_t PointCount)
{
  int16_t X = 0, Y = 0;

  if(PointCount < 2)
  {
    return;
  }

  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    LCD_DrawUniLine(X, Y, Points->X, Points->Y);
  }
}

/**
  * @brief  Displays an relative poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @param  Closed: specifies if the draw is closed or not.
  *           1: closed, 0 : not closed.
  * @retval None
  */
static void LCD_PolyLineRelativeClosed(pPoint Points, uint16_t PointCount, uint16_t Closed)
{
  int16_t X = 0, Y = 0;
  pPoint First = Points;

  if(PointCount < 2)
  {
    return;
  }  
  X = Points->X;
  Y = Points->Y;
  while(--PointCount)
  {
    Points++;
    LCD_DrawUniLine(X, Y, X + Points->X, Y + Points->Y);
    X = X + Points->X;
    Y = Y + Points->Y;
  }
  if(Closed)
  {
    LCD_DrawUniLine(First->X, First->Y, X, Y);
  }  
}

/**
  * @brief  Displays a closed poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_ClosedPolyLine(pPoint Points, uint16_t PointCount)
{
  LCD_PolyLine(Points, PointCount);
  LCD_DrawUniLine(Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y);
}

/**
  * @brief  Displays a relative poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_PolyLineRelative(pPoint Points, uint16_t PointCount)
{
  LCD_PolyLineRelativeClosed(Points, PointCount, 0);
}

/**
  * @brief  Displays a closed relative poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_ClosedPolyLineRelative(pPoint Points, uint16_t PointCount)
{
  LCD_PolyLineRelativeClosed(Points, PointCount, 1);
}

/**
  * @brief  Displays a  full poly-line (between many points).
  * @param  Points: pointer to the points array.
  * @param  PointCount: Number of points.
  * @retval None
  */
void LCD_FillPolyLine(pPoint Points, uint16_t PointCount)
{
 
  int16_t X = 0, Y = 0, X2 = 0, Y2 = 0, X_center = 0, Y_center = 0, X_first = 0, Y_first = 0, pixelX = 0, pixelY = 0, counter = 0;
  uint16_t  IMAGE_LEFT = 0, IMAGE_RIGHT = 0, IMAGE_TOP = 0, IMAGE_BOTTOM = 0;  

  IMAGE_LEFT = IMAGE_RIGHT = Points->X;
  IMAGE_TOP= IMAGE_BOTTOM = Points->Y;

  for(counter = 1; counter < PointCount; counter++)
  {
    pixelX = POLY_X(counter);
    if(pixelX < IMAGE_LEFT)
    {
      IMAGE_LEFT = pixelX;
    }
    if(pixelX > IMAGE_RIGHT)
    {
      IMAGE_RIGHT = pixelX;
    }
    
    pixelY = POLY_Y(counter);
    if(pixelY < IMAGE_TOP)
    { 
      IMAGE_TOP = pixelY;
    }
    if(pixelY > IMAGE_BOTTOM)
    {
      IMAGE_BOTTOM = pixelY;
    }
  }  
  
  if(PointCount < 2)
  {
    return;
  }
  
  X_center = (IMAGE_LEFT + IMAGE_RIGHT)/2;
  Y_center = (IMAGE_BOTTOM + IMAGE_TOP)/2;
 
  X_first = Points->X;
  Y_first = Points->Y;
  
  while(--PointCount)
  {
    X = Points->X;
    Y = Points->Y;
    Points++;
    X2 = Points->X;
    Y2 = Points->Y;    
  
    LCD_FillTriangle(X, X2, X_center, Y, Y2, Y_center);
    LCD_FillTriangle(X, X_center, X2, Y, Y_center, Y2);
    LCD_FillTriangle(X_center, X2, X, Y_center, Y2, Y);   
  }
  
  LCD_FillTriangle(X_first, X2, X_center, Y_first, Y2, Y_center);
  LCD_FillTriangle(X_first, X_center, X2, Y_first, Y_center, Y2);
  LCD_FillTriangle(X_center, X2, X_first, Y_center, Y2, Y_first); 
}



/**
  * @brief  GPIO config for LTDC.
  * @param  None
  * @retval None
  */
static void LCD_AF_GPIOConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);
  
  
  /* GPIOs Configuration */
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  
  /* GPIOA configuration */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* GPIOB configuration */
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF9_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF9_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_LTDC);
  
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
  GPIO_Init(GPIOB, &GPIO_InitStruct);
  
  /* GPIOÑ configuration */
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_LTDC);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
  
  /* GPIOF configuration */
  GPIO_PinAFConfig(GPIOF, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOF, &GPIO_InitStruct);
  
  /* GPIOG configuration */
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource6, GPIO_AF_LTDC);
  //GPIO_PinAFConfig(GPIOG, GPIO_PinSource7, GPIO_AF_LTDC);//pix clock
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource11, GPIO_AF_LTDC);
  
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource12, GPIO_AF9_LTDC);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_11 | GPIO_Pin_12;
  
  //GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_11 ;//test
  
  GPIO_Init(GPIOG, &GPIO_InitStruct);
  
  /* GPIOH configuration */
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource11, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource13, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOH, GPIO_PinSource14, GPIO_AF_LTDC);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 | GPIO_Pin_14;	
  GPIO_Init(GPIOH, &GPIO_InitStruct);
  
  /* GPIOI configuration */
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource2, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource9, GPIO_AF_LTDC);
  GPIO_PinAFConfig(GPIOI, GPIO_PinSource10, GPIO_AF_LTDC);
  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_9 | GPIO_Pin_10;
  
  //GPIO_PinAFConfig(GPIOI, GPIO_PinSource4, GPIO_AF_LTDC); //BLUE4
  //GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_4| GPIO_Pin_9 | GPIO_Pin_10;  
  GPIO_Init(GPIOI, &GPIO_InitStruct);
}

/**
  * @brief  Displays a pixel.
  * @param  x: pixel x.
  * @param  y: pixel y.  
  * @retval None
  */
static void PutPixel(int16_t x, int16_t y)
{ 
  if(x < 0 || x >= LCD_PIXEL_WIDTH || y < 0 || y >= LCD_PIXEL_HEIGHT)
    return;
  LCD_DrawLine(x, y, 1, LCD_DIR_HORIZONTAL);
}