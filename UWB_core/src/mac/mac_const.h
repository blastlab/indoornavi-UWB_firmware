#ifndef _MAC_CONST
#define _MAC_CONST
#include <stdint.h>

#define FR_CR_BEACON 0x00
#define FR_CR_DATA 0x01
#define FR_CR_ACK 0x02
#define FR_CR_MAC 0x03
#define FR_CR_TYPE_MASK 0x07
#define FR_CR_SECURE 0x08
#define FR_CR_PENDING 0x10
#define FR_CR_RACK 0x20
#define FR_CR_PAN 0x40
#define FR_CRH_DA_NONE 0x00
#define FR_CRH_DA_SHORT 0x08
#define FR_CRH_DA_LONG 0x0C
#define FR_CRH_FVER0 0x00
#define FR_CRH_FVER1 0x01
#define FR_CRH_SA_NONE 0x00
#define FR_CRH_SA_SHORT 0x80
#define FR_CRH_SA_LONG 0xC0

typedef enum {
	FREE,             ///< packet is fre
	BUSY,             ///< packet under edition
	WAIT_FOR_TX,      ///< when packet wait on tx queue
	WAIT_FOR_TX_ACK,  ///< when packet wait on tx queue and need ack
	WAIT_FOR_ACK,     ///< wait for receive ack after transmission
} mac_buf_state;

typedef enum {
	RTLS_TAG = 'T',
	RTLS_ANCHOR = 'A',
	RTLS_SINK = 'S',
	RTLS_LISTENER = 'L',
	RTLS_DEFAULT = 'D'
} rtls_role;

typedef unsigned short dev_addr_t;
typedef unsigned short pan_dev_addr_t;
#define ADDR_BROADCAST 0xffff
#define ADDR_ANCHOR_FLAG 0x8000

#define ADDR_TAG(X) ((X&ADDR_ANCHOR_FLAG)==0)
#define ADDR_ANCHOR(X) ((X&ADDR_ANCHOR_FLAG)==ADDR_ANCHOR_FLAG)

#ifndef CORTEX_M
#ifndef __packed
#define __packed
#endif
#endif

struct FC_CARRY_s;

/**
 * @brief packet extra information struct
 *
 */
typedef struct {
	dev_addr_t original_src, last_src;
	struct FC_CARRY_s* carry;
} prot_packet_info_t;

/**
 * @brief see #FC_t description
 *
 */
typedef struct {
	uint8_t FC, len;
	uint8_t hop_cnt_batt; ///< number of did in hops[] array (upper nibble) voltage msb (lower nibble)
	uint8_t voltage; /// voltage lsb, mV-2000
	uint32_t serial_hi; ///< device serial number from settings.version.serial
	uint32_t serial_lo; ///< device serial number from settings.version.serial
	dev_addr_t src_did; ///< device id of beacon sender
	dev_addr_t hops[0];  ///< packet route sink_neighbour..src_neighbour
}__packed FC_BEACON_s;

#endif
