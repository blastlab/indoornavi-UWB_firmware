#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "nrf_gpio.h"
#include "../logger/logs.h"
#include "parsers/base64.h"
#include "parsers/txt_parser.h"
#include "port.h"

#define LOG_BUF_LEN 1024
static char tx_buf[LOG_BUF_LEN + 1];

uint8_t PORT_UsbUartTransmit(uint8_t *buf, uint16_t len);

struct spiHandling {
		bool 		ifWaitingForAck;
		bool 		isAck;
		bool 		isNack;
		int 		txTick;
		uint8_t	rx_buf[LOG_BUF_LEN + 1];
} spiHandling = {
		.ifWaitingForAck = false,
		.isAck = false,
		.isNack = false,
		.txTick = 0,
		.rx_buf = { 0 }
};

#if LOG_SPI_EN
static void readFromEthSpiSlave(uint8_t *rx_buf, uint32_t len) {
	NRF_SPIM1->TXD.PTR = (uint32_t)NULL;
	NRF_SPIM1->TXD.MAXCNT = 0;
	NRF_SPIM1->RXD.PTR = (uint32_t)rx_buf;
	NRF_SPIM1->RXD.MAXCNT = len;
	NRF_SPIM1->EVENTS_END = 0x0UL;
	NRF_SPIM1->TXD.LIST = 0x0UL;
	NRF_SPIM1->RXD.LIST = 0x0UL;
	NRF_SPIM1->TASKS_START = 0x1UL;
	while(NRF_SPIM1->EVENTS_END == 0x0UL);
}
static const uint8_t ack_msg[4] = { LOG_PC_Ack, 4, 8, 216 };
static const uint8_t nack_msg[4] = { LOG_PC_Nack, 4, 145, 79 };
static const prot_packet_info_t bin_packet = {
		.original_src = CARRY_ADDR_SERVER,
		.last_src = CARRY_ADDR_SERVER,
		.carry = NULL
};

static void sendAckToEthSlave(bool isAck) {
#if ETH_SPI_SS_PIN
	if(isAck) {
		PORT_SpiTx((uint8_t *)ack_msg, sizeof(ack_msg), ETH_SPI_SS_PIN);
	} else {
		PORT_SpiTx((uint8_t *)nack_msg, sizeof(nack_msg), ETH_SPI_SS_PIN);
	}
#endif
}

void SpiSlaveRequest() {
#if ETH_SPI_SS_PIN
	// read first two bytes (opcode and len), then read the rest of message
	// only slave's irq can oall this method, so critical section is only needed for peripheral use
	CRITICAL(
	nrf_gpio_pin_clear(ETH_SPI_SS_PIN);
	readFromEthSpiSlave(spiHandling.rx_buf, FRAME_HEADER_SIZE);
	readFromEthSpiSlave(&spiHandling.rx_buf[FRAME_HEADER_SIZE], spiHandling.rx_buf[1] - FRAME_HEADER_SIZE);
	nrf_gpio_pin_set(ETH_SPI_SS_PIN);
	)
	int packet_len = spiHandling.rx_buf[1];
	int data_len = spiHandling.rx_buf[1] - FRAME_HEADER_SIZE - 2;
	switch(spiHandling.rx_buf[0]) {
		case LOG_PC_Bin:
			BIN_Parse(&spiHandling.rx_buf[FRAME_HEADER_SIZE], &bin_packet, data_len);
			break;

		case LOG_PC_Txt:
			PORT_CrcReset();
			if(PORT_CrcFeed(spiHandling.rx_buf, packet_len) == 0) {
				sendAckToEthSlave(true);
				TXT_Input((char *)&spiHandling.rx_buf[FRAME_HEADER_SIZE], data_len);
			} else {
				sendAckToEthSlave(false);
			}
			break;

		case LOG_PC_Ack:
			if(spiHandling.ifWaitingForAck) {
				spiHandling.ifWaitingForAck = false;
				LOG_BufPop();
			}
			break;

		case LOG_PC_Nack:
			if(spiHandling.ifWaitingForAck) {
				spiHandling.ifWaitingForAck = false;
			}
			break;
	}
#endif
}
#endif

void PORT_LogData(const void *bin, int size, LOG_PacketCodes_t pc, bool isSink) {
	#if LOG_SPI_EN
	if(isSink) {
		// when the device is not waiting for previous message ack or when waiting timed out
		if(spiHandling.ifWaitingForAck == false || (spiHandling.txTick + 50) < PORT_TickMs()) {
			spiHandling.ifWaitingForAck = true;
			spiHandling.txTick = PORT_TickMs();
			PORT_SpiTx((uint8_t *)bin, size, ETH_SPI_SS_PIN);
		} else {
			return;
		}
	}
	#endif
	#if LOG_USB_EN
	if(pc == LOG_PC_Bin) {
		strcpy(tx_buf, "B1000 ");
		int f = strlen(tx_buf);
		if (BASE64_TextSize(size - FRAME_HEADER_SIZE - 2) + f >= LOG_BUF_LEN) {
			LOG_ERR(ERR_BASE64_TOO_LONG_OUTPUT, ((uint8_t *)bin)[FRAME_HEADER_SIZE]);
		} else {
			f += BASE64_Encode((unsigned char*)(tx_buf + f), ((uint8_t *)bin + FRAME_HEADER_SIZE), size - FRAME_HEADER_SIZE - 2);
			tx_buf[f++] = '\n';
			PORT_UsbUartTransmit((uint8_t*)tx_buf, f);
		}
	} else if(pc == LOG_PC_Txt) {
		PORT_UsbUartTransmit(((uint8_t *)bin + FRAME_HEADER_SIZE), size - FRAME_HEADER_SIZE - 2);
	}
	#endif
#if !LOG_SPI_EN
	LOG_BufPop();
#endif
}
