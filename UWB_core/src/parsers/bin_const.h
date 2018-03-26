#ifndef _BIN_CONST
#define _BIN_CONST

#define FC_CARRY 0x01
#define FC_FU 0x02

//#define FC_SYNC_INIT 0x10
#define FC_SYNC_POLL 0x11
#define FC_SYNC_RESP (FC_SYNC_POLL + 1)
#define FC_SYNC_FIN (FC_SYNC_POLL + 2)

#define FC_VERSION_ASK 0x14
#define FC_VERSION_RESP (FC_VERSION_ASK + 1)
// empty
// empty
#define FC_STAT_ASK 0x18
#define FC_STAT_RESP (FC_STAT_ASK + 1)
#define FC_STAT_SET (FC_STAT_ASK + 2)
// empty
#define FC_TXSET_ASK 0x1C
#define FC_TXSET_RESP (FC_TXSET_ASK + 1)
#define FC_TXSET_SET (FC_TXSET_ASK + 2)
// empty

#endif
