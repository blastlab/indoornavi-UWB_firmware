#include "platform/port.h"
#include "stm32l4xx_ll_crc.h"
#include "logger/logs.h" // trace

void PORT_AdcInit();
void PORT_SpiInit();
void PORT_CrcInit();
void PORT_TimeInit();

void PORT_Init() {
	PORT_SpiInit();
	PORT_AdcInit();
	PORT_CrcInit();
	PORT_TimeInit();
	PORT_ImuInit(PORT_GetHwRole() == RTLS_TAG);
#if !DBG
	PORT_WatchdogInit();
#endif
	HAL_NVIC_EnableIRQ(DW_EXTI_IRQn);
}

void PORT_LowPowerRun() {
}

void PORT_WatchdogInit() {
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_WWDG);
	LL_WWDG_SetCounter(WWDG, 127);
	LL_WWDG_Enable(WWDG);
	LL_WWDG_SetPrescaler(WWDG, LL_WWDG_PRESCALER_8);
	LL_WWDG_SetWindow(WWDG, 127);
}

void PORT_WatchdogRefresh() {
	if (LL_WWDG_GetCounter(WWDG) < LL_WWDG_GetWindow(WWDG)) {
		LL_WWDG_SetCounter(WWDG, 127);
	}
}

// turn led on
void PORT_LedOn(int LED_x) {
	switch (LED_x) {
		case LED_G1:
			LL_GPIO_SetOutputPin(LED_G1_GPIO_Port, LED_G1_Pin);
			break;
		case LED_R1:
			LL_GPIO_SetOutputPin(LED_R1_GPIO_Port, LED_R1_Pin);
			break;
		default:
			IASSERT(0);
			break;
	}
}

// turrn led off
void PORT_LedOff(int LED_x) {
	switch (LED_x) {
		case LED_G1:
			LL_GPIO_ResetOutputPin(LED_G1_GPIO_Port, LED_G1_Pin);
			break;
		case LED_R1:
			LL_GPIO_ResetOutputPin(LED_R1_GPIO_Port, LED_R1_Pin);
			break;
		default:
			IASSERT(0);
			break;
	}
}

// reset dw 1000 device by polling RST pin down for a few ms
void PORT_ResetTransceiver() {
	// May be pulled low by external open drain driver to reset the DW1000.
	// Must not be pulled high by external source.
	// from DW1000 datasheet table 1
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable GPIO used for DW1000 reset

	GPIO_InitStructure.Pin = DW_RST_Pin;
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	HAL_GPIO_Init(DW_RST_GPIO_Port, &GPIO_InitStructure);

	// drive the RSTn pin low
	HAL_GPIO_WritePin(DW_RST_GPIO_Port, DW_RST_Pin, GPIO_PIN_RESET);
	PORT_SleepMs(2);

	// put the pin back to tri-state ... as input
	GPIO_InitStructure.Pin = DW_RST_Pin;
	GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
	HAL_GPIO_Init(DW_RST_GPIO_Port, &GPIO_InitStructure);

	HAL_Delay(2);
}

void PORT_Reboot() {
	// turn off USB, to reconnect after reset
	// it help from usb timeout error from the host side
	USB_StopDevice(USB);
	USB_DevDisconnect(USB);
	for (volatile int i = 99999; i > 0; --i)
		;                // disabled irq safe delay
	PORT_SleepMs(10);  // to be sure
	NVIC_SystemReset();
}

extern USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_DescriptorsTypeDef FS_Desc;
volatile uint32_t RCC_config[4];
uint32_t temp_RCC_APB11, temp_RCC_APB12, temp_RCC_APB2, temp_NVIC_ICER[sizeof(NVIC->ICER)];

static int sleep_cnt = 0;
void LOG_Trace(TRACE_t);

void PORT_PrepareSleepMode() {
	LOG_Trace(TRACE_PREPARE_SLEEP);
	USB_StopDevice(USB);
	USB_DevDisconnect(USB);
	HAL_SYSCFG_DisableVREFBUF();
	CLEAR_BIT(ADC1->CR, ADC_CR_ADEN);
	CLEAR_BIT(ADC1_COMMON->CCR, ADC_CCR_VBATEN);
	CLEAR_BIT(ADC1_COMMON->CCR, ADC_CCR_VREFEN);

	LL_ADC_EnableDeepPowerDown(ADC1);
	LOG_Trace(TRACE_GO_SLEEP);
	PORT_LedOff(LED_R1);
	PORT_LedOff(LED_G1);
	++sleep_cnt;

	/*LL_AHB1_GRP1_EnableClockStopSleep(
	 LL_AHB1_GRP1_PERIPH_DMA1 |
	 LL_AHB1_GRP1_PERIPH_DMA2 |
	 LL_AHB1_GRP1_PERIPH_FLASH |
	 LL_AHB1_GRP1_PERIPH_CRC);

	 LL_AHB2_GRP1_EnableClockStopSleep(
	 LL_AHB2_GRP1_PERIPH_RNG |
	 LL_AHB2_GRP1_PERIPH_AES |
	 LL_AHB2_GRP1_PERIPH_ADC |
	 LL_AHB2_GRP1_PERIPH_GPIOA |
	 LL_AHB2_GRP1_PERIPH_GPIOB |
	 LL_AHB2_GRP1_PERIPH_GPIOC |
	 LL_AHB2_GRP1_PERIPH_GPIOH);

	 LL_APB1_GRP2_EnableClockStopSleep(
	 LL_APB1_GRP1_PERIPH_LPTIM1 |
	 LL_APB1_GRP1_PERIPH_USB |
	 LL_APB1_GRP1_PERIPH_SPI3 |
	 LL_APB1_GRP1_PERIPH_SPI2 |
	 LL_APB1_GRP1_PERIPH_WWDG |
	 LL_APB1_GRP1_PERIPH_TIM2);

	 LL_APB1_GRP1_EnableClockStopSleep(
	 LL_APB2_GRP1_PERIPH_SYSCFG |
	 LL_APB2_GRP1_PERIPH_SPI1 |
	 LL_APB2_GRP1_PERIPH_USART1);
	 */
}

bool PORT_EnterSleepMode() {
	if (sleep_cnt > 0) {
		HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
		//HAL_PWREx_EnterSTOP0Mode(PWR_STOPENTRY_WFI);
		//HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	}
	return sleep_cnt > 0;
}

/**
 * @brief Configures system clock after wake-up from STOP: enable MSI, PLL, ADC1
 *  and select PLL as system clock source.
 * @param None
 * @retval None
 */
static void SYSCLKConfig_AfterSTOP(void) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	uint32_t pFLatency = 0;
	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE()
	;
	/* Get the Oscillators configuration according to the internal RCC registers */
	HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
	/* Enable PLL */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/* Get the Clocks configuration according to the internal RCC registers */
	HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);
	/* Select PLL as system clock source and keep HCLK, PCLK1 and PCLK2 clocks dividers as before */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, pFLatency) != HAL_OK) {
		Error_Handler();
	}
	/* ENABLE ADC1 source clock */
	RCC_PeriphCLKInitTypeDef PeriphClkInit;
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USART1
	    | RCC_PERIPHCLK_LPTIM1 | RCC_PERIPHCLK_USB | RCC_PERIPHCLK_ADC;
	PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
	PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSI;
	PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
	PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_MSI;
	PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
	PeriphClkInit.PLLSAI1.PLLSAI1N = 16;
	PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
	PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
	PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

void PORT_ExitSleepMode() {
	LOG_Trace(TRACE_WAKEUP);
//	CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
//	HAL_PWREx_DisableLowPowerRunMode();
//	RCC->CR = RCC_config[0];
//	RCC->CFGR = RCC_config[1];
//	RCC->CSR = RCC_config[2];
//	RCC->CRRCR = RCC_config[3];
//	USB_DevConnect(USB);
//	USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
//	USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
//	USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
//	USBD_Start(&hUsbDeviceFS);
	if (sleep_cnt == 1) {
		SYSCLKConfig_AfterSTOP();
	}

	if (sleep_cnt > 0) {
		//HAL_PWREx_DisableLowPowerRunMode();
		--sleep_cnt;
	}
}
