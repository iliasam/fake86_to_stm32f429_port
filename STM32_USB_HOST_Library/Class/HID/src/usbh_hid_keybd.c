/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_keybd.h"
#include "input.h"

#define USB_KEYBOARD_BUF_SIZE   8

#define KEY_MOD_LCTRL  0x01
#define KEY_MOD_LSHIFT 0x02
#define KEY_MOD_RSHIFT 0x20
#define KEY_MOD_LALT   0x04

static void  KEYBRD_Init (void);
static void  KEYBRD_Decode(uint8_t *data);

void process_functional_keys(uint8_t cur_state);


#ifdef USB_OTG_HS_INTERNAL_DMA_ENABLED
 #if defined   (__CC_ARM) /*!< ARM Compiler */
  __align(4) 
 #elif defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4
 #elif defined (__GNUC__) /*!< GNU Compiler */
 #pragma pack(4) 
 #elif defined  (__TASKING__) /*!< TASKING Compiler */                           
  __align(4) 
 #endif /* __CC_ARM */
#endif
 
/** @defgroup USBH_HID_KEYBD_Private_Variables
* @{
*/
HID_cb_TypeDef HID_KEYBRD_cb= 
{
  KEYBRD_Init,
  KEYBRD_Decode
};



/**
* @brief  KEYBRD_Init.
*         Initialize the keyboard function.
* @param  None
* @retval None
*/
static void  KEYBRD_Init (void)
{
  /* Call User Init*/
  USR_KEYBRD_Init();
}

/**
* @brief  KEYBRD_ProcessData.
*         The function is to decode the pressed keys.
* @param  pbuf : Pointer to the HID IN report data buffer
* @retval None
*/
//iliasam modufication of function
static void KEYBRD_Decode(uint8_t *pbuf)
{
  static  uint8_t   buf_prev_state[USB_KEYBOARD_BUF_SIZE];
  uint8_t i = 0;
  
  process_functional_keys(pbuf[i]);
  
  for (i=1; i< USB_KEYBOARD_BUF_SIZE; i++)
  {
    if (pbuf[i] != buf_prev_state[i])
    {
      //current button state changed
      
      if (pbuf[i] == 0)
      {
        //key up
        if (i > 0) //not functional
          emulate_key_up(buf_prev_state[i]);//send previous state          
      }
      else
      {
        //key down
        if (i > 0) //not functional
          emulate_key_down(pbuf[i]);
      }
    }
      
    buf_prev_state[i] = pbuf[i];
  }
  
  
  
  
}

void process_functional_keys(uint8_t cur_state)
{
  static uint8_t prev_state = 0;
  
  if (prev_state != cur_state)//somthing changed
  {
    uint8_t changed_mask = prev_state ^ cur_state;
    
    if ((changed_mask & KEY_MOD_LSHIFT) != 0)
    {
      if ((cur_state & KEY_MOD_LSHIFT) != 0)
        emulate_key_down(0x60);//INTERNAL
      else
        emulate_key_up(0x60);
    }
    
    if ((changed_mask & KEY_MOD_RSHIFT) != 0)
    {
      if ((cur_state & KEY_MOD_RSHIFT) != 0)
        emulate_key_down(0x60);//INTERNAL
      else
        emulate_key_up(0x60);
    }
    
    if ((changed_mask & KEY_MOD_LALT) != 0)
    {
      if ((cur_state & KEY_MOD_LALT) != 0)
        emulate_key_down(0x61);//INTERNAL
      else
        emulate_key_up(0x61);
    }
    
    if ((changed_mask & KEY_MOD_LCTRL) != 0)
    {
      if ((cur_state & KEY_MOD_LCTRL) != 0)
        emulate_key_down(0x62);//INTERNAL
      else
        emulate_key_up(0x62);
    }
  }
  
  prev_state = cur_state;
}

/*
static void KEYBRD_Decode(uint8_t *pbuf)
{
  static  uint8_t   shift;
  static  uint8_t   keys[KBR_MAX_NBR_PRESSED];
  static  uint8_t   keys_new[KBR_MAX_NBR_PRESSED];
  static  uint8_t   keys_last[KBR_MAX_NBR_PRESSED];
  static  uint8_t   key_newest;
  static  uint8_t   nbr_keys;
  static  uint8_t   nbr_keys_new;
  static  uint8_t   nbr_keys_last;
  uint8_t   ix;
  uint8_t   jx;
  uint8_t   error;
  uint8_t   output;            
  
  nbr_keys      = 0;
  nbr_keys_new  = 0;
  nbr_keys_last = 0;
  key_newest    = 0x00;
  
  
  // Check if Shift key is pressed                                                                       
  if ((pbuf[0] == KBD_LEFT_SHIFT) || (pbuf[0] == KBD_RIGHT_SHIFT)) {
    shift = TRUE;
  } else {
    shift = FALSE;
  }
  
  error = FALSE;
  
  // Check for the value of pressed key
  for (ix = 2; ix < 2 + KBR_MAX_NBR_PRESSED; ix++) {                       
    if ((pbuf[ix] == 0x01) ||
        (pbuf[ix] == 0x02) ||
          (pbuf[ix] == 0x03)) {
            error = TRUE;
          }
  }
  
  if (error == TRUE) {
    return;
  }
  
  nbr_keys     = 0;
  nbr_keys_new = 0;
  for (ix = 2; ix < 2 + KBR_MAX_NBR_PRESSED; ix++) {
    if (pbuf[ix] != 0) {
      keys[nbr_keys] = pbuf[ix];                                       
      nbr_keys++;
      for (jx = 0; jx < nbr_keys_last; jx++) {                         
        if (pbuf[ix] == keys_last[jx]) {
          break;
        }
      }
      
      if (jx == nbr_keys_last) {
        keys_new[nbr_keys_new] = pbuf[ix];
        nbr_keys_new++;
      }
    }
  }
  
  if (nbr_keys_new == 1) {
    key_newest = keys_new[0];
    
    if (shift == TRUE) {
      output =  HID_KEYBRD_ShiftKey[HID_KEYBRD_Codes[key_newest]];
    } else {
      output =  HID_KEYBRD_Key[HID_KEYBRD_Codes[key_newest]];
    }
    
    // call user process handle
    USR_KEYBRD_ProcessData(output);
  } else {
    key_newest = 0x00;
  }
  
  
  nbr_keys_last  = nbr_keys;
  for (ix = 0; ix < KBR_MAX_NBR_PRESSED; ix++) {
    keys_last[ix] = keys[ix];
  }
}
*/
