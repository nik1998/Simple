/***************************************************************************
*                      ZBOSS ZigBee Pro 2007 stack                         *
*                                                                          *
*          Copyright (c) 2012 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*          Copyright (c) 2011 ClarIDy Solutions, Inc., Taipei, Taiwan.     *
*                       http://www.claridy.com/                            *
*                                                                          *
*          Copyright (c) 2011 Uniband Electronic Corporation (UBEC),       *
*                             Hsinchu, Taiwan.                             *
*                       http://www.ubec.com.tw/                            *
*                                                                          *
*          Copyright (c) 2011 DSR Corporation Denver CO, USA.              *
*                       http://www.dsr-wireless.com                        *
*                                                                          *
*                            All rights reserved.                          *
*                                                                          *
*                                                                          *
* ZigBee Pro 2007 stack, also known as ZBOSS (R) ZB stack is available     *
* under either the terms of the Commercial License or the GNU General      *
* Public License version 2.0.  As a recipient of ZigBee Pro 2007 stack, you*
* may choose which license to receive this code under (except as noted in  *
* per-module LICENSE files).                                               *
*                                                                          *
* ZBOSS is a registered trademark of DSR Corporation AKA Data Storage      *
* Research LLC.                                                            *
*                                                                          *
* GNU General Public License Usage                                         *
* This file may be used under the terms of the GNU General Public License  *
* version 2.0 as published by the Free Software Foundation and appearing   *
* in the file LICENSE.GPL included in the packaging of this file.  Please  *
* review the following information to ensure the GNU General Public        *
* License version 2.0 requirements will be met:                            *
* http://www.gnu.org/licenses/old-licenses/gpl-2.0.html.                   *
*                                                                          *
* Commercial Usage                                                         *
* Licensees holding valid ClarIDy/UBEC/DSR Commercial licenses may use     *
* this file in accordance with the ClarIDy/UBEC/DSR Commercial License     *
* Agreement provided with the Software or, alternatively, in accordance    *
* with the terms contained in a written agreement between you and          *
* ClarIDy/UBEC/DSR.                                                        *
*                                                                          *
****************************************************************************
PURPOSE: Test for ZC application written using ZDO.
*/


#include "zb_common.h"
#include "zb_scheduler.h"
#include "zb_bufpool.h"
#include "zb_nwk.h"
#include "zb_aps.h"
#include "zb_zdo.h"

#ifndef ZB_ED_ROLE
#error define ZB_ED_ROLE to compile ze tests
#endif
/*! \addtogroup ZB_TESTS */
/*! @{ */

#define ZB_TEST_DUMMY_DATA_SIZE 10
#define dToggle 2
#define dStepUp 1
#define dChangeColor 3
#include "stm32.h"

int ind=1;
//static void send_data(zb_buf_t *buf);
void function_state(int ind);
void MyInit();
int arr[3];
void CommandParse(int com);
void ChangePulse(int delta);
void TimTim();
void Initleds(int Pins, int PinSource);
zb_ieee_addr_t g_ze_addr = {0xed, 0xed, 0xed, 0xed, 0xed, 0xed, 0xed, 0xed};

static void send_data(zb_uint8_t param) ZB_CALLBACK;
void data_indication(zb_uint8_t param) ZB_CALLBACK;



/*
  ZE joins to ZC(ZR), then sends APS packet.
*/


MAIN()
{
  ARGV_UNUSED;

#if !(defined KEIL || defined SDCC || defined ZB_IAR )
  if ( argc < 3 )
  {
    //printf("%s <read pipe path> <write pipe path>\n", argv[0]);
    return 0;
  }
#endif

  /* Init device, load IB values from nvram or set it to default */
#ifndef ZB8051
  ZB_INIT("zdo_ze", argv[1], argv[2]);
#else
  ZB_INIT((char*)"zdo_ze", (char*)"3", (char*)"3");
#endif
#ifdef ZB_SECURITY
  ZG->nwk.nib.security_level = 0;
#endif
  MyInit();
  ZB_IEEE_ADDR_COPY(ZB_PIB_EXTENDED_ADDRESS(), &g_ze_addr);
  ZB_PIB_RX_ON_WHEN_IDLE() = ZB_FALSE;
  ZB_AIB().aps_channel_mask = (1l << 22);
	
  if (zdo_dev_start() != RET_OK)
  {
    TRACE_MSG(TRACE_ERROR, "zdo_dev_start failed", (FMT__0));
  }
  else
  {
    zdo_main_loop();
  }

  TRACE_DEINIT();

  MAIN_RETURN(0);
}
void MyInit()
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
 // RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  //button interrupt
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG/*|RCC_APB2Periph_TIM1*/, ENABLE);
    //RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource0);
  SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource1);
  //Init LedGPIO
  GPIO_InitTypeDef  GPIO_InitStructure1;
  GPIO_InitStructure1.GPIO_Pin= GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure1.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure1.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure1);

 //button
  GPIO_InitTypeDef GPIO_In;
GPIO_In.GPIO_Pin= GPIO_Pin_1|GPIO_Pin_0;
  GPIO_In.GPIO_Mode = GPIO_Mode_IN;
  GPIO_In.GPIO_OType = GPIO_OType_PP;
  GPIO_In.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_In.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_In);
//interrupt
   EXTI_InitTypeDef eee;
   eee.EXTI_Line=EXTI_Line0;
   eee.EXTI_LineCmd=ENABLE;
   eee.EXTI_Mode=EXTI_Mode_Interrupt;
   eee.EXTI_Trigger=EXTI_Trigger_Falling;
   EXTI_Init(&eee);

  eee.EXTI_Line=EXTI_Line1;
  EXTI_Init(&eee);
   //vector
  NVIC_InitTypeDef nvec;
  nvec.NVIC_IRQChannel=EXTI0_IRQn;
  nvec.NVIC_IRQChannelPreemptionPriority=0x00;
  nvec.NVIC_IRQChannelSubPriority=0x01;
  nvec.NVIC_IRQChannelCmd=ENABLE;
  NVIC_Init(&nvec);

  nvec.NVIC_IRQChannel=EXTI1_IRQn;
  NVIC_Init(&nvec);

  Initleds(GPIO_Pin_8,GPIO_PinSource8);
  TimTim();
  arr[0]=0;
  arr[1]=0;
  arr[2]=0;
  GPIO_SetBits(GPIOD,GPIO_Pin_14); 
}
 //Init Leds
  void Initleds(int Pins,int PinSource)
  {
    GPIO_InitTypeDef  GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   // GPIO_PinAFConfig(GPIOA,PinSource,GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource8/*|GPIO_PinSource9|GPIO_PinSource10*/,GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9/*|GPIO_PinSource9|GPIO_PinSource10*/,GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10/*|GPIO_PinSource9|GPIO_PinSource10*/,GPIO_AF_TIM1);
 // GPIO_InitStructure.GPIO_Pin= Pins;
    GPIO_InitStructure.GPIO_Pin= GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
  }
   
  //Int Timer
  void TimTim()
  {
    TIM_TimeBaseInitTypeDef ttt;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);
    ttt.TIM_Period=100000/60-1;
    ttt.TIM_Prescaler=1680;
    ttt.TIM_ClockDivision=0;
    ttt.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1,&ttt);
    TIM_CtrlPWMOutputs(TIM1,ENABLE);
    TIM_Cmd(TIM1,ENABLE);
    TIM_OCInitTypeDef aaa;
    aaa.TIM_OCMode=TIM_OCMode_PWM1;
    aaa.TIM_Pulse=0;
    aaa.TIM_OCPolarity=TIM_OCPolarity_Low; 
    TIM_OC1Init(TIM1,&aaa);
    TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Enable);
    TIM_OC2Init(TIM1,&aaa);
    TIM_OC2PreloadConfig(TIM1,TIM_OCPreload_Enable);
    TIM_OC3Init(TIM1,&aaa);
    TIM_OC3PreloadConfig(TIM1,TIM_OCPreload_Enable);
    //Timer2
    TIM_TimeBaseInitTypeDef ttt2;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    ttt2.TIM_Period=1000000-1;
    ttt2.TIM_Prescaler=84-1;
    ttt2.TIM_ClockDivision=0;
    ttt2.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2,&ttt2);
    TIM_ITConfig(TIM2, TIM_IT_Update,ENABLE);
    TIM_Cmd(TIM2,ENABLE);
    //Timer vector
    NVIC_InitTypeDef nv;
    nv.NVIC_IRQChannel=TIM2_IRQn;
    nv.NVIC_IRQChannelPreemptionPriority=0x00;
    nv.NVIC_IRQChannelSubPriority=0x01;
    nv.NVIC_IRQChannelCmd=ENABLE;
    NVIC_Init(&nv);
  }
int ok1=0;
int ok2=0;
int co=0;
void TIM2_IRQHandler(void)
{
  if(ok1|ok2)
  {
    co++;
  }
  if(co==2)
  {
    co=0;
    CommandParse(ok1*10+ok2);
    function_state(ok1+2*ok2);
    ok1=0;
    ok2=0;
  }
  if(TIM_GetITStatus(TIM2,TIM_IT_Update)!=RESET)
  {
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}

void EXTI1_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line1)!=RESET)
  {
    ok2=1;
    EXTI_ClearITPendingBit(EXTI_Line1);
  }
}
void EXTI0_IRQHandler(void)
{
  if(EXTI_GetITStatus(EXTI_Line0)!=RESET)
  {
    ok1=1;
    EXTI_ClearITPendingBit(EXTI_Line0);
  }
}
void CommandParse(int com)
{
  switch(com)
  {
    case 10:
    {
      arr[ind-1]+=100;
      if(arr[ind-1]>1600)
      {
        arr[ind-1]=0;
      }
      if(ind==1)
      TIM_SetCompare1(TIM1,arr[ind-1]);
      else
      if(ind==2)
      TIM_SetCompare2(TIM1,arr[ind-1]); 
      else
      TIM_SetCompare3(TIM1,arr[ind-1]);
      break;
    }
    case 1:
    {
      if(arr[ind-1]==0)
      {
        arr[ind-1]=800;
      }
      else
      {
        arr[ind-1]=0;
      }
      if(ind==1)
        TIM_SetCompare1(TIM1,arr[ind-1]);
      else
      if(ind==2)
        TIM_SetCompare2(TIM1,arr[ind-1]); 
      else
      TIM_SetCompare3(TIM1,arr[ind-1]); 
      break;
    }
    case 11:
    {
       GPIO_ResetBits(GPIOD,GPIO_Pin_12|GPIO_Pin_14|GPIO_Pin_15); 
      if(ind==1)
      {
        GPIO_SetBits(GPIOD,GPIO_Pin_12); 
        ind=2;
      }
      else
            if(ind==2)
            {
              GPIO_SetBits(GPIOD,GPIO_Pin_15); 
              ind=3;
            }
            else
            if(ind==3)
            {
              GPIO_SetBits(GPIOD,GPIO_Pin_14); 
              ind=1;
            }
      break;
    }
 }
}
void zb_zdo_startup_complete(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  if (buf->u.hdr.status == 0)
  {
    TRACE_MSG(TRACE_APS1, "Device STARTED OK", (FMT__0));
    //zb_af_set_data_indication(data_indication);
    
    //ZB_SCHEDULE_ALARM(send_data, param, 195);
  }
  else
  {
    TRACE_MSG(TRACE_ERROR, "Device started FAILED status %d", (FMT__D, (int)buf->u.hdr.status));
    zb_free_buf(buf);
  }
}
/*static void send_data(zb_buf_t *buf)
{
  zb_apsde_data_req_t *req;
 // zb_uint8_t *ptr = NULL;
 // zb_short_t i;

 // ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
  req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
  req->dst_addr.addr_short = 0; // send to ZC 
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 10;
  req->dst_endpoint = 10;

  buf->u.hdr.handle = 0x11;

  //for (i = 0 ; i < ZB_TEST_DATA_SIZE ; ++i)
 // {
   // ptr[i] = i % 32 + '0';
 // }
  TRACE_MSG(TRACE_APS3, "Sending apsde_data.request", (FMT__0));

  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}*/


void send_data(zb_uint8_t param) ZB_CALLBACK
{
  zb_buf_t *buf = ZB_BUF_FROM_REF(param);
  zb_apsde_data_req_t *req = ZB_GET_BUF_TAIL(buf, sizeof(zb_apsde_data_req_t));
 // zb_uint8_t *ptr = NULL;
  //zb_short_t i;

  req->dst_addr.addr_short = 0; /* send to ZC */
  req->addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
  req->tx_options = ZB_APSDE_TX_OPT_ACK_TX;
  req->radius = 1;
  req->profileid = 2;
  req->src_endpoint = 10;
  req->dst_endpoint = 10;

  buf->u.hdr.handle = 0x11;
  //ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);

 // for (i = 0 ; i < ZB_TEST_DATA_SIZE ; ++i)
 // {
   // ptr[i] = i % 32 + '0';
 // }
  TRACE_MSG(TRACE_APS2, "Sending apsde_data.request", (FMT__0));

  ZB_SCHEDULE_CALLBACK(zb_apsde_data_request, ZB_REF_FROM_BUF(buf));
}
void  Toggle (zb_uint8_t param)
{
    zb_uint8_t *ptr = NULL;
   zb_buf_t *buf = ZB_BUF_FROM_REF(param);
   ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
   ptr[0]=dToggle;
   send_data(param);
}
void  StepUp (zb_uint8_t param)
{
    zb_uint8_t *ptr = NULL;
   zb_buf_t *buf = ZB_BUF_FROM_REF(param);
   ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
   ptr[0]= dStepUp ;
   send_data(param);
}
void  ChangeColor (zb_uint8_t param)
{
    zb_uint8_t *ptr = NULL;
   zb_buf_t *buf = ZB_BUF_FROM_REF(param);
   ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
   ptr[0]= dChangeColor ;
   send_data(param);
}
void function_state(int ind)
{
    zb_uint8_t *ptr = NULL;
    int i;
    zb_buf_t *buf =zb_get_out_buf();// ZB_BUF_FROM_REF(param);
   zb_uint8_t param=ZB_REF_FROM_BUF(buf);
    ZB_BUF_INITIAL_ALLOC(buf, ZB_TEST_DATA_SIZE, ptr);
    for (i = 1 ; i < ZB_TEST_DATA_SIZE ; ++i)
    {
      ptr[i] =0 ;
    }

       switch(ind)
       {
         case dToggle:
         {
           Toggle(param);
           break;
         }
         case dStepUp:
         {
           StepUp(param);
           break;
         }
         case dChangeColor:
         {
           ChangeColor(param);
           break;
         }
         default:break;
        }
         TRACE_MSG(TRACE_APS1, "Recall fuction", (FMT__0)); 

      // ZB_SCHEDULE_ALARM(zc_send_data,0,5*ZB_TIME_ONE_SECOND);
}



void data_indication(zb_uint8_t param)
{
  zb_uint8_t *ptr;
  zb_buf_t *asdu = (zb_buf_t *)ZB_BUF_FROM_REF(param);

  /* Remove APS header from the packet */
  ZB_APS_HDR_CUT_P(asdu, ptr);

  TRACE_MSG(TRACE_APS2, "data_indication: packet %p len %d handle 0x%x", (FMT__P_D_D,
                         asdu, (int)ZB_BUF_LEN(asdu), asdu->u.hdr.status));
  
  ZB_SCHEDULE_ALARM(send_data, param, 195);
}




/*! @} */
