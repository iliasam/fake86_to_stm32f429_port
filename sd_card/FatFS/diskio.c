/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "sdcard.h"

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */

#define BlockSize       4096 /* Block Size in Bytes */
#define SD_Mode         0   //0 dma,1 interrupt

static SD_CardInfo SDCardInfo;

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (BYTE drv) /* Physical drive nmuber (0..) */
{
  SD_Error Status;
  
  if(drv == 0)
  {
    Status = SD_Init();
    
    if(Status != SD_OK)
    {
      return STA_NOINIT;
    }
    else
    {
      Status = SD_GetCardInfo(&SDCardInfo);
      
      if (Status != SD_OK)
      {
        return  STA_NOINIT;//RES_NOTRDY;  //NOT READY
      }
      // Select Card 
      Status = SD_SelectDeselect((u32) (SDCardInfo.RCA << 16));
      
      if (Status != SD_OK)
      {
        return  STA_NOINIT;
      }
      
      switch(SD_Mode)
      {
        case 0:  //dma
        Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
        if (Status != SD_OK)
        {  
          return RES_NOTRDY;
        }
        
        Status = SD_SetDeviceMode(SD_DMA_MODE);
        if (Status != SD_OK)
        {
          return RES_NOTRDY;
        }
        break;
        
        case 1://interrupt
        Status = SD_EnableWideBusOperation(SDIO_BusWide_4b);
        if (Status != SD_OK)
        {  
          return RES_NOTRDY;
        }
        
        Status = SD_SetDeviceMode(SD_INTERRUPT_MODE);  
        if (Status != SD_OK)
        {
          return RES_NOTRDY;
        }
        break;
        
        default :
        return RES_NOTRDY;
      }
      
      return 0;
    }
  }
  else
  {
    return STA_NOINIT;  
  }
}

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (BYTE drv)		/* Physical drive nmuber (0..) */
{
  if(drv == 0)
  {
    return 0;
  }
  else
  {
    return STA_NOINIT;  
  }
  
} 

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
DRESULT disk_read (
                   BYTE drv,       /* Physical drive nmuber (0..) */
                   BYTE *buff,     /* Data buffer to store read data */
                   DWORD sector,   /* Sector address (LBA) */
                   BYTE count      /* Number of sectors to read (1..255) */
)
{
  SD_Error Status; 
  
  if (!count) return RES_PARERR;
  
  if(drv == 0)
  {		
    switch(SD_Mode)
    {
      case 0:  //dma
      if(count == 1)
      {      
        Status = SD_ReadBlock(sector << 9,(u32 *)(&buff[0]),BlockSize);                                
      }                                                
      else  
      {    
        Status = SD_ReadMultiBlocks(sector << 9,(u32 *)(&buff[0]),BlockSize,count);                                      
      }
      break;
      
      case 1: //int
      if(count == 1)   
      {      
        Status = SD_ReadBlock(sector<<9,(u32 *)(&buff[0]),BlockSize);                                              
      }                                                
      else   
      {    
        Status = SD_ReadMultiBlocks(sector<<9 ,(u32 *)(&buff[0]),BlockSize,count);                                     
      }  	  
      break;
      
      default:
      Status=SD_ERROR;
    }
    
    if(Status == SD_OK)
      return RES_OK;
    else
      return RES_ERROR;
  }
  else
  {
    return RES_ERROR;  
  }
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
#if _READONLY == 0
DRESULT disk_write (
                    BYTE drv,			/* Physical drive nmuber (0..) */
                    const BYTE *buff,	        /* Data to be written */
                    DWORD sector,		/* Sector address (LBA) */
                    BYTE count			/* Number of sectors to write (1..255) */
)
{
  SD_Error Status;
  
  if (!count)  return RES_PARERR;
  
  if(drv == 0)
  {
    switch(SD_Mode)
    {
      case 0:  //dma
      if(count == 1)   
      {      
        Status = SD_WriteBlock(sector << 9,(u32 *)(&buff[0]),BlockSize);                                           
      }                                                
      else    
      {    
        Status =SD_WriteMultiBlocks(sector << 9,(u32 *)(&buff[0]),BlockSize,count);                                          
      }  	  
      break;
      
      case 1:
      if(count == 1)
      {      
        Status = SD_WriteBlock(sector << 9 ,(u32 *)(&buff[0]),BlockSize);                                            
      }                                                
      else 
      {    
        Status = SD_WriteMultiBlocks(sector << 9 ,(u32 *)(&buff[0]),BlockSize,count);                                     
      }  
      break;
      
      default :
      Status=SD_ERROR;
    }
    
    if(Status == SD_OK)
      return RES_OK;
    else
      return RES_ERROR;
  }
  else
  {
    return RES_ERROR;  
  }
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
DRESULT disk_ioctl (
                    BYTE drv,		/* Physical drive nmuber (0..) */
                    BYTE ctrl,		/* Control code */
                    void *buff		/* Buffer to send/receive control data */
)
{
  u32 x, y, z;
  DRESULT res;
  
  if (drv==0)
  {
    switch(ctrl)
    {
      case CTRL_SYNC:
      if(SD_GetTransferState()==SD_NO_TRANSFER)
      {
        res = RES_OK;
      }
      else
      {
        res = RES_ERROR;
      }
      break;
      
      case GET_BLOCK_SIZE:
      *(WORD*)buff = BlockSize;
      res = RES_OK;
      break;
      
      case GET_SECTOR_COUNT:
      ////formula of the capacity///////////////
      //
      //  memory capacity = BLOCKNR * BLOCK_LEN
      // 
      // BLOCKNR = (C_SIZE + 1)* MULT
      //
      //           C_SIZE_MULT+2
      // MULT = 2
      //
      //               READ_BL_LEN
      // BLOCK_LEN = 2
      //////////////////////////////////////////
      if (SD_GetCardInfo(&SDCardInfo)==SD_OK)
      {
        x=SDCardInfo.SD_csd.DeviceSize+1; //C_SIZE + 1
        y=SDCardInfo.SD_csd.DeviceSizeMul+2; //C_SIZE_MULT+2
        z=SDCardInfo.SD_csd.RdBlockLen+y;
        *(DWORD*)buff =x<<z; 
        res = RES_OK;
      }
      else
      {
        res = RES_ERROR ;
      }
      break;
      
    default:
      res = RES_PARERR;
    }
    return res;
  }
  else
  {
    return RES_ERROR;  
  }
}


