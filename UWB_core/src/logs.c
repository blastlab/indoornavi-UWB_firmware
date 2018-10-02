#include "logs.h"


typedef struct{
  int code;
  const char* frm;
} LOG_CODE_t;


LOG_CODE_t LOG_CodeInf[] = {
  {INF_CLEAR_HELP, "clear [-m,-p,-mp]"},
  {INF_CLEARED_PARENTS, "cleared parents"},
  {INF_CLEARED_MEASURES, "cleared measures"},
  {INF_CLEARED_PARENTS_AND_MEASURES, "cleared parents and measures"},
  {INF_ROUTE, "route auto:%d"},
  {INF_MEASURE_CMD_CNT, "measure cnt:%d"},
  {INF_MEASURE_CMD_SET, "measure set %X with %d anchors"},
  {INF_PARENT_DESCRIPTION, "parent of %X is %X (%d)"},
  {INF_PARENT_SET, "parent %X for %d devices, failed for %d"},
  {INF_PARENT_CNT, "parent cnt:%d"},
  {INF_SETANCHORS_SET, "setanchors set %d anchors"},
  {INF_SETTAGS_SET, "settags set %d tags with %d anchors"},
};

LOG_CODE_t LOG_CodeErr[] = {
  {ERR_FLASH_ERASING, ""},
  {ERR_MAC_NO_MORE_BUFFERS, "No more buffers"},
  {ERR_BAD_OPCODE_LEN,  "%s bad len %d!=%d"},
  {ERR_RF_BAD_CHANNEL, "rfset ch 1..7 (without 6)"},
  {ERR_RF_BAD_BAUDRATE, "rfset br 110/850/6800"},
  {ERR_RF_BAD_PREAMBLE_LEN, "rfset plen 64/128/256/512/1024/1536/2048/4096"},
  {ERR_RF_BAD_PRF, "rfset prf 16/64"},
  {ERR_RF_BAD_PAC, "rfset pac 8/16/32/64"},
  {ERR_RF_BAD_CODE, "rfset code 1..24"},
  {ERR_RF_BAD_NSSFD, "rfset nssfd 0/1"},
  {ERR_PARENT_FOR_SINK, "parent can't be set for sink"},
  {ERR_PARENT_NEED_ANCHOR, "parent must be an anchor (%X)"},
  {ERR_MEASURE_FAILED_DID, "measure failed after %X"},
  {ERR_MEASURE_FAILED_ANC_CNT, "measure failed ancCnt:%d"},
  {ERR_BLE_INACTIVE, "BLE is disabled"},
};

LOG_CODE_t LOG_CodeWrn[] = {
  {WRN_CARRY_INCOMPATIBLE_VERSION, "CARRY incompatible version %d (%d)"},
  {WRN_CARRY_TARGET_NOBODY, "CARRY target nobody"},
  {WRN_MAC_FRAME_BAD_OPCODE, "MAC frame with bad opcode %X"},
  {WRN_MAC_UNSUPPORTED_MAC_FRAME, "MAC unsupported frame type %X"},
  {WRN_MAC_UNSUPPORTED_ACK_FRAME, "MAC ack frame is not supported yet"},
  {WRN_MAC_TOO_BIG_FRAME, "Frame with size %d can't be send within %dus slot"},
  {WRN_MAC_TX_ERROR, "Tx err"},
  {WRN_RANGING_TOO_SMALL_PERIOD, "Too small period! Now N:%d T:%d"},
};

const int LOG_CodeErrN = sizeof(LOG_CodeErr) / sizeof(*LOG_CodeErr);
const int LOG_CodeInfN = sizeof(LOG_CodeInf) / sizeof(*LOG_CodeInf);
const int LOG_CodeWrnN = sizeof(LOG_CodeWrn) / sizeof(*LOG_CodeWrn);

const char* LOG_GetFormat(int number, LOG_CODE_t array[], int len)
{
  LOG_CODE_t* end = &array[len];
  LOG_CODE_t* ptr = &array[0];

  while(ptr != end)
  {
    if(ptr->code == number)
    {
      return ptr->frm;
    }
    ++ptr;
  }
  
  return 0;
}


void LOG_CRIT(ERR_codes code, const char *frm, ...) {
	va_list arg;
	va_start(arg, frm);
	LOG_Text('C', (int)code, frm, arg);
	va_end(arg);
}

void LOG_ERR(ERR_codes code, ...) {
  const char* frm = LOG_GetFormat(code, LOG_CodeErr, LOG_CodeErrN);
	va_list arg;
	va_start(arg, frm);
	LOG_Text('E', (int)code, frm, arg);
	va_end(arg);
}

void LOG_WRN(WRN_codes code, ...) {
  const char* frm = LOG_GetFormat(code, LOG_CodeWrn, LOG_CodeWrnN);
	va_list arg;
	va_start(arg, frm);
	LOG_Text('W', (int)code, frm, arg);
	va_end(arg);
}

void LOG_INF(INF_codes code, ...) {
  const char* frm = LOG_GetFormat(code, LOG_CodeInf, LOG_CodeInfN);
	va_list arg;
	va_start(arg, frm);
	LOG_Text('I', (int)code, frm, arg);
	va_end(arg);
}

void LOG_DBG(const char *frm, ...) {
	va_list arg;
	va_start(arg, frm);
	LOG_Text('D', 0, frm, arg);
	va_end(arg);
}

void LOG_TEST(TEST_codes code, const char *frm, ...) {
	va_list arg;
	va_start(arg, frm);
	LOG_Text('T', (int)code, frm, arg);
	va_end(arg);
}