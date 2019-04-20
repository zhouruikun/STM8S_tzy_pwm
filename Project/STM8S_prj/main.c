/**
******************************************************************************
* @file    Project/main.c 
* @author  MCD Application Team
* @version V2.2.0
* @date    30-September-2014
* @brief   Main program body
******************************************************************************
* @attention
*
* <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
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


/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"

/* Private defines -----------------------------------------------------------*/
#define LONG_PRESS_TIME 3000     //长按时间 单位ms
#define SHORT_PRESS_TIME_UP 2000 //短按时间上限 单位ms
#define SHORT_PRESS_TIME_LOW 100 //短按时间下限 单位ms
#define FREQ 500                 //pwm频率 单位HZ     

#define TIM_PERIOD 1000000/FREQ-1
#define IDLE 1
#define INIT 2
#define OUTPUT 3
#define END 4
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
extern int count;

void delayMS(int ms)
{
  count=ms;
  while(count>0);
}
int get_trig( BitStatus bit_status)
{
  int time =0;
  while(bit_status == GPIO_ReadInputPin(GPIOD, GPIO_PIN_3)){
    delayMS(10);
    time+=10;
  }
  return time;
}
int get_key(void)
{
  int time =0;
  while(RESET == GPIO_ReadInputPin(GPIOB, GPIO_PIN_5)){
    delayMS(10);
    time+=10;
  }
  return time;
}
int get_slope_add(int slope_period,int number,int cnt_period)
{
  return (long )number*cnt_period*0.99/(slope_period*1000.0)+cnt_period*0.01;
}
void main(void)
{
  int status=IDLE;
  int key_time = 0;
  int slope_time = 8;
  int slope_time_cnt = 0;
  /* Infinite loop */
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
  
  GPIO_Init(GPIOB, GPIO_PIN_5, GPIO_MODE_IN_PU_NO_IT);
  GPIO_Init(GPIOC, GPIO_PIN_3, GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(GPIOD, GPIO_PIN_3, GPIO_MODE_IN_FL_NO_IT);
  
  TIM1_TimeBaseInit(15, TIM1_COUNTERMODE_UP, TIM_PERIOD, 0x00);
  TIM1_OC3Init(TIM1_OCMODE_PWM1, TIM1_OUTPUTSTATE_ENABLE, TIM1_OUTPUTNSTATE_DISABLE, 0, TIM1_OCPOLARITY_HIGH, TIM1_OCNPOLARITY_HIGH, TIM1_OCIDLESTATE_SET, TIM1_OCNIDLESTATE_SET);
  TIM1_CtrlPWMOutputs(ENABLE);
  TIM1_Cmd(ENABLE);
  
  TIM2_TimeBaseInit(TIM2_PRESCALER_16, 999);
  TIM2_ITConfig(TIM2_IT_UPDATE, ENABLE);
  TIM2->IER =TIM2_IT_UPDATE;
  ITC_SetSoftwarePriority(ITC_IRQ_TIM2_OVF, ITC_PRIORITYLEVEL_0);
  TIM2_Cmd(ENABLE);
  
  
  __enable_interrupt();
  while (1)
  {
    BitStatus bit_status;
    bit_status = GPIO_ReadInputPin(GPIOB, GPIO_PIN_5);//读取按键
    if (bit_status == RESET)  //SET or RESET
    {
      key_time = get_key();//获取按键时间
      if(key_time>=LONG_PRESS_TIME)//如果大于LONG_PRESS_TIME
      {
        slope_time=1;//初始化时间1秒
        TIM1_CtrlPWMOutputs(DISABLE);//停止输出
        while(1)
        {
          key_time = get_key();//获取按键时间
          if(key_time>=LONG_PRESS_TIME)//如果大于LONG_PRESS_TIME
          {
            TIM1_CtrlPWMOutputs(ENABLE);//开启输出
            break;//退出设置
          }
          else if(key_time>=SHORT_PRESS_TIME_LOW&&key_time<=SHORT_PRESS_TIME_UP)//如果介于SHORT_PRESS_TIME_LOW和SHORT_PRESS_TIME_UP
          {
            slope_time++;//加1秒
            if(slope_time>21)//大于21秒回到1秒
            {
              slope_time=1;
            }
          }
        }
      }
    };

    delayMS(10);
    switch(status)
    {
    case IDLE:
      if(get_trig(SET) >=100)
      {
        status = INIT;
      };
      break;
    case INIT:
      slope_time_cnt=0;
      status = OUTPUT;
      break;
    case OUTPUT:
      if(get_trig(RESET) >=500)
      {
        status = END;
      };
      //正常输出缓慢上升
      slope_time_cnt+=10;
      TIM1_SetCompare3(get_slope_add(slope_time,slope_time_cnt,TIM_PERIOD));
      TIM1_CtrlPWMOutputs(ENABLE);//开启输出
      if(slope_time_cnt>=slope_time*1000)
      {
        status = END;
      }
      break;
    case END:
      TIM1_CtrlPWMOutputs(DISABLE);//停止输出+
      slope_time_cnt=0;
      status = IDLE;
      break;
    default:break;
    }
  }
  
}


#ifdef USE_FULL_ASSERT

/**
* @brief  Reports the name of the source file and the source line number
*   where the assert_param error has occurred.
* @param file: pointer to the source file name
* @param line: assert_param error line source number
* @retval : None
*/
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
