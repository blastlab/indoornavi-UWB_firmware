#ifndef _CARRY_CONST
#define _CARRY_CONST

/// 4 bit value
#define CARRY_VERSION (1)

#define CARRY_HOPS_NUM_MASK (0x0F)
#define CARRY_MAX_HOPS 16

// bit 7, 6
#define CARRY_FLAG_TARGET_DEV (0 << 6)
#define CARRY_FLAG_TARGET_SINK (1 << 6)
#define CARRY_FLAG_TARGET_SERVER (2 << 6)
#define CARRY_FLAG_TARGET_MASK (3 << 6)
// bit 5, 4
#define CARRY_FLAG_ACK_REQ (1 << 4)
#define CARRY_FLAG_REROUTE (2 << 4)
// bit 3

#endif
