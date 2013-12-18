/* Includes ------------------------------------------------------------------*/

//#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"

#include "main.h"
#include "stm32f4xx_conf.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define TESTRESULT_ADDRESS         0x080FFFFC
#define ALLTEST_PASS               0x00000000
#define ALLTEST_FAIL               0x55555555

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

uint16_t PrescalerValue = 0;

__IO uint32_t TimingDelay;
__IO uint8_t UserButtonPressed = 0x00;


/* Private function prototypes -----------------------------------------------*/
static void LCD_task(void *pvParameters);

/**
 * @brief  This function handles EXTI0_IRQ Handler.
 * @param  None
 * @retval None
 */
void EXTI0_IRQHandler(void)
{
	/* Clear the EXTI line pending bit */
	EXTI_ClearITPendingBit(USER_BUTTON_EXTI_LINE);
	if(UserButtonPressed){
		UserButtonPressed=0x00;
	}else
		UserButtonPressed=0x01;

}

#include "LCD/LCDConfig.h"

int main(void)
{
	RCC_ClocksTypeDef RCC_Clocks;

	/* Configure SysTick */
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);

	FLASH_ProgramWord(TESTRESULT_ADDRESS, ALLTEST_PASS);
	SystemInit();
	//while(TimingDelay);


	/* Initialize LEDs and User_Button on STM32F4-Discovery --------------------*/
	STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);

	/* Initialize LEDs to be managed by GPIO */
	STM_EVAL_LEDInit(LED4);
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED5);
	STM_EVAL_LEDInit(LED6);

	/* Turn OFF all LEDs */
	STM_EVAL_LEDOff(LED4);
	STM_EVAL_LEDOff(LED3);
	STM_EVAL_LEDOff(LED5);
	STM_EVAL_LEDOff(LED6);

	/* Reset UserButton_Pressed variable */
	UserButtonPressed = 0x00;

	/* Create a task to button check. */
	xTaskCreate(LCD_task,
			(signed portCHAR *) "LCD Task",
			512 /* stack size */, NULL,
			tskIDLE_PRIORITY + 5, NULL);

	/* Start running the tasks. */
	vTaskStartScheduler();

	return 0;
}

void GPIO_Output_Config(void){
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_PinAFConfig(GPIOE, GPIO_PinSource3|GPIO_PinSource4|GPIO_PinSource5|GPIO_PinSource6|GPIO_PinSource7|GPIO_PinSource8|GPIO_PinSource9|GPIO_PinSource10|GPIO_PinSource11|GPIO_PinSource12|GPIO_PinSource13, GPIO_AF_TIM3);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;            // Alt Function - Push Pull
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init( GPIOE, &GPIO_InitStructure );
}

static void LCD_task(void *pvParameters)
{
	LCD_InitTypeDef lcd = {.GPIO = GPIOE, .RS_Pin = GPIO_Pin_11, .RW_Pin = GPIO_Pin_12, .E_Pin = GPIO_Pin_13, .DB_Pins = {GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7, GPIO_Pin_8, GPIO_Pin_9, GPIO_Pin_10}, .max_col = 20, .max_row = 4, .col = 0, .row = 0};

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_Output_Config();
	LCD_Init(&lcd);

	vTaskDelay(5);
	LCD_printf(&lcd, "LCD Library %d", 123);
	while (1) {
		/* Toggle LED4 */
		STM_EVAL_LEDOn(LED4);
		vTaskDelay(100);
		STM_EVAL_LEDOff(LED4);
		/* Toggle LED3 */
		STM_EVAL_LEDOn(LED3);
		vTaskDelay(100);
		STM_EVAL_LEDOff(LED3);
	}
}


/**
 * @brief  This function handles the test program fail.
 * @param  None
 * @retval None
 */
void Fail_Handler(void)
{
	/* Erase last sector */
	FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
	/* Write FAIL code at last word in the flash memory */
	FLASH_ProgramWord(TESTRESULT_ADDRESS, ALLTEST_FAIL);

	while(1)
	{
		/* Toggle Red LED */
		STM_EVAL_LEDToggle(LED5);
		vTaskDelay(5);
	}
}

void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
}

void vApplicationTickHook()
{
}
