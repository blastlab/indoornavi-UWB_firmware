#ifndef _MAC_CONST
#define _MAC_CONST

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
  FREE,  ///< packet is fre
  BUSY,  ///< packet under edition
  WAIT_FOR_TX,  ///< when packet wait on tx queue
  WAIT_FOR_TX_ACK,  ///< when packet wait on tx queue and need ack
  WAIT_FOR_ACK,  ///< wait for receive ack after transmission
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
#define ADDR_ANCHOR_FLAG	0x8000

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
  dev_addr_t direct_src;
  struct FC_CARRY_s *carry;
} prot_packet_info_t;

#endif
