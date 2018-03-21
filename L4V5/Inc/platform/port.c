#include "platform/port.h"

void port_sleep_ms(unsigned int time_ms)
{
	HAL_Delay(time_ms);
}

unsigned int port_tick_ms()
{
	HAL_GetTick();
}

// get high resosolution clock tick
unsigned int port_tick_hr()
{
    return 0;
}

// reset dw 1000 device by polling RST pin down for a few ms
void reset_DW1000()
{
}

// get deca spi mutex
decaIrqStatus_t decamutexon(void)
{
    return 0;
}

// release deca spi mutex
void decamutexoff(decaIrqStatus_t s)
{
}

// set SPI speed below 3MHz when param is false or below 20MHz when true
void spi_speed_slow(bool fast)
{
}

// returns DWT_SUCCESS for success or DWT_ERROR for error
int readfromspi(uint16_t headerLength, const uint8_t *headerBuffer, uint32_t readlength, uint8_t *readBuffer)
{
    return 0;
}

// returns DWT_SUCCESS for success or DWT_ERROR for error
int writetospi(uint16_t headerLength, const uint8_t *headerBuffer, uint32_t bodylength, const uint8_t *bodyBuffer)
{
    return 0;
}
