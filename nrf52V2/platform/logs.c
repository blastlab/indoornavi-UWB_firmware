#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "../logger/logs.h"
#include "parsers/base64.h"
#include "parsers/txt_parser.h"
#include "port.h"

#define LOG_BUF_LEN 1024
static char tx_buf[LOG_BUF_LEN + 1];
extern circular_buff_s logs_buf;

uint8_t PORT_UsbUartTransmit(uint8_t *buf, uint16_t len);

struct spiHandling {
		volatile bool ifWaitingForAck;
		volatile int 	txTick;
		volatile int	nackCount;
		uint8_t				rx_buf[256];
} spiHandling = {
		.ifWaitingForAck = false,
		.txTick = 0,
		.nackCount = 0
};

#if LOG_SPI_EN
static uint8_t ack_msg[4] = { LOG_PC_Ack, 4, 8, 216 };
static uint8_t nack_msg[4] = { LOG_PC_Nack, 4, 145, 79 };
static prot_packet_info_t bin_packet = {
		.original_src = CARRY_ADDR_SERVER,
		.last_src = CARRY_ADDR_SERVER,
		.carry = NULL
};

static inline void sendAckToEthSlave(bool isAck) {
#if ETH_SPI_SS_PIN
	CRITICAL(
	PORT_SpiTx((uint8_t *)((isAck) ? ack_msg : nack_msg), sizeof(ack_msg), ETH_SPI_SS_PIN);
	)
#endif
}

void SpiSlaveRequest() {
#if ETH_SPI_SS_PIN
	// read first two bytes (opcode and len), then read the rest of message
	// only slave's irq can oall this method, so critical section is only needed for peripheral use
	CRITICAL(
	nrf_gpio_pin_clear(ETH_SPI_SS_PIN);
	PORT_SpiRx(spiHandling.rx_buf, FRAME_HEADER_SIZE + 2, 0);
	if(2 < (int)(spiHandling.rx_buf[1] - FRAME_HEADER_SIZE - 2) && (LOG_PC_Bin <= spiHandling.rx_buf[0] && spiHandling.rx_buf[0] <= LOG_PC_Txt)) {
		PORT_SpiRx(&spiHandling.rx_buf[FRAME_HEADER_SIZE + 2], spiHandling.rx_buf[1] - FRAME_HEADER_SIZE - 2, 0);
	}
	nrf_gpio_pin_set(ETH_SPI_SS_PIN);
	)
	int packet_len = spiHandling.rx_buf[1];
	int data_len = packet_len - FRAME_HEADER_SIZE - 2;
	uint16_t crc = 0xffff;
	switch(spiHandling.rx_buf[0]) {
		case LOG_PC_Bin:
			BIN_Parse(&spiHandling.rx_buf[FRAME_HEADER_SIZE], &bin_packet, data_len);
			break;

		case LOG_PC_Txt:
			crc = 0xffff;
			if(PORT_CrcFeed(&crc, spiHandling.rx_buf, packet_len) == 0) {
				sendAckToEthSlave(true);
				TXT_Input((char *)&spiHandling.rx_buf[FRAME_HEADER_SIZE], data_len);
			} else {
				sendAckToEthSlave(false);
			}
			break;

		case LOG_PC_Ack:
			if(spiHandling.ifWaitingForAck) {
				CRITICAL(
				BUF_Pop(&logs_buf);
				)
				spiHandling.nackCount = 0;
				spiHandling.ifWaitingForAck = false;
			}
			break;

		case LOG_PC_Nack:
			if(spiHandling.ifWaitingForAck) {
				if(5 <= ++spiHandling.nackCount) {
					CRITICAL(
					BUF_Pop(&logs_buf);
					)
					spiHandling.nackCount = 0;
				}
				spiHandling.ifWaitingForAck = false;
			}
			break;

		default:
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
		if((spiHandling.ifWaitingForAck == false || (spiHandling.txTick + 10) < PORT_TickMs())
																							&& nrf_gpio_pin_read(ETH_SPI_SLAVE_IRQ) == 1) {
			spiHandling.ifWaitingForAck = true;
			spiHandling.txTick = PORT_TickMs();
			/* PORT_LogData is already called in critical section */
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
	CRITICAL(
	BUF_Pop(&logs_buf);
	)
#endif
}
