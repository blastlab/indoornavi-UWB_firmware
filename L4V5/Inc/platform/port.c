#include "platform/port.h"
#include "stm32l4xx_ll_crc.h"

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

void PORT_EnterStopMode() {
	__disable_irq();
	PORT_LedOff(LED_R1);
	PORT_LedOff(LED_G1);
	HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
}

extern USBD_HandleTypeDef hUsbDeviceFS;
extern USBD_DescriptorsTypeDef FS_Desc;
volatile uint32_t RCC_config[4];
void PORT_PrepareSleepMode() {
	PORT_LedOff(LED_R1);
	PORT_LedOff(LED_G1);
	USB_StopDevice(USB);
	USB_DevDisconnect(USB);
	RCC_config[0] = RCC->CR;
	RCC_config[1] = RCC->CFGR;
	RCC_config[2] = RCC->CSR;
	RCC_config[3] = RCC->CRRCR;
	HAL_PWREx_EnableLowPowerRunMode();
	MODIFY_REG(PWR->CR1, PWR_CR1_LPMS, PWR_CR1_LPMS_STOP0);
	SET_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
}

void PORT_ExitSleepMode() {
	CLEAR_BIT(SCB->SCR, ((uint32_t)SCB_SCR_SLEEPDEEP_Msk));
	HAL_PWREx_DisableLowPowerRunMode();
	RCC->CR = RCC_config[0];
	RCC->CFGR = RCC_config[1];
	RCC->CSR = RCC_config[2];
	RCC->CRRCR = RCC_config[3];
	USB_DevConnect(USB);
	USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
	USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
	USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
	USBD_Start(&hUsbDeviceFS);
}
