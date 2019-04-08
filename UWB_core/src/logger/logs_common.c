/**
 * @file logs_common.c
 * @author Karol Trzcinski
 * @brief logger engine implementation
 * @date 2018-10-02
 *
 * @copyright Copyright (c) 2018
 *
 */
#include "logs.h"


typedef struct{
  int code;
  const char* frm;
} LOG_CODE_t;


#undef ADD_ITEM
#undef ADD_ITEM_M
#undef COMMENT
#undef ARG

#define ADD_ITEM(CODE,ENUM_VALUE,MESSAGE) [ENUM_VALUE]={CODE,#ENUM_VALUE},
#define ADD_ITEM_M(CODE,ENUM_VALUE,MESSAGE) [ENUM_VALUE]={CODE,MESSAGE},
#define COMMENT(X)
#define ARG(NAME,DESCRIPTION)

static LOG_CODE_t LOG_CodeCrit[ERR_codes_N] = {
#include "logger/logs_crit.h"
    };
static LOG_CODE_t LOG_CodeErr[ERR_codes_N] = {
#include "logger/logs_err.h"
    };

static LOG_CODE_t LOG_CodeWrn[WRN_codes_N] = {
#include "logger/logs_wrn.h"
    };

static LOG_CODE_t LOG_CodeInf[INF_codes_N] = {
#include "logger/logs_inf.h"
    };

static LOG_CODE_t LOG_CodeTest[TEST_codes_N] = {
#include "logger/logs_test.h"
    };

#undef ADD_ITEM
#undef ADD_ITEM_M
#undef COMMENT
#undef ARG

#define LOG_BUF_LEN 1024
static char buf[LOG_BUF_LEN + 1];
circular_buff_s logs_buf = {
		.head = 0,
		.tail = 1,
		.overflow = false
};

int LOG_CheckUniqInArray(LOG_CODE_t target[], int len)
{
	int repeats = 0;
  for(int i = 1; i < len; ++i)
  {
    for(int j = 0; j < i; ++j) {
      if(target[i].code == target[j].code) {
				LOG_CRIT(CRIT_LOG_CODES_ARE_NOT_UNIQ, target[i].code);
				++repeats;
      }
    }
  }
	return repeats;
}

int LOG_CheckMonotonousInArray(LOG_CODE_t target[], int len) {
	int unmonotonous = 0;
	for (int i = 1; i < len; ++i) {
		if (target[i].code < target[i - 1].code) {
			LOG_CRIT(CRIT_LOG_CODES_ARE_NOT_MONOTONOUS, target[i].code);
			++unmonotonous;
		}
	}
	return unmonotonous;
}

int LOG_CheckArray(LOG_CODE_t target[], int len) {
	int i = 0;
	i += LOG_CheckUniqInArray(target, len);
	i += LOG_CheckMonotonousInArray(target, len);
	return i;
}

void LOG_SelfTest() {
	LOG_CheckArray(LOG_CodeCrit, CRIT_codes_N);
	LOG_CheckArray(LOG_CodeErr, ERR_codes_N);
	LOG_CheckArray(LOG_CodeWrn, WRN_codes_N);
	LOG_CheckArray(LOG_CodeInf, INF_codes_N);
	LOG_CheckArray(LOG_CodeTest, TEST_codes_N);
}

#if DBG
TRACE_t trace_history[TRACE_CNT];
#endif

void LOG_Trace(TRACE_t t) {
#if DBG
	if (t == TRACE_SYSTICK && t == trace_history[0])
	return;

	CRITICAL(
			for (int i = TRACE_CNT-1; i > 0; --i) {
				trace_history[i] = trace_history[i - 1];
			}
			trace_history[0] = t;
	)
#endif
}

void LOG_CRIT(ERR_codes code, ...) {
	const char* frm = LOG_CodeCrit[code].frm;
	const int code_num = LOG_CodeCrit[code].code;
	va_list arg;
	va_start(arg, code);
	LOG_Text('C', (int)code_num, frm, arg);
	va_end(arg);
}

void LOG_ERR(ERR_codes code, ...) {
	const char* frm = LOG_CodeErr[code].frm;
	const int code_num = LOG_CodeErr[code].code;
	va_list arg;
	va_start(arg, code);
	LOG_Text('E', code_num, frm, arg);
	va_end(arg);
}

void LOG_WRN(WRN_codes code, ...) {
	const char* frm = LOG_CodeWrn[code].frm;
	const int code_num = LOG_CodeWrn[code].code;
	va_list arg;
	va_start(arg, code);
	LOG_Text('W', (int)code_num, frm, arg);
	va_end(arg);
}

void LOG_INF(INF_codes code, ...) {
	const char* frm = LOG_CodeInf[code].frm;
	const int code_num = LOG_CodeInf[code].code;
	va_list arg;
	va_start(arg, code);
	LOG_Text('I', (int)code_num, frm, arg);
	va_end(arg);
}

void LOG_DBG(const char *frm, ...) {
	va_list arg;
	va_start(arg, frm);
	LOG_Text('D', 0, frm, arg);
	va_end(arg);
}

void LOG_TEST(TEST_codes code, ...) {
	const char* frm = LOG_CodeTest[code].frm;
	const int code_num = LOG_CodeTest[code].code;
	va_list arg;
	va_start(arg, code);
	LOG_Text('T', (int)code_num, frm, arg);
	va_end(arg);
}

int LOG_Text(char type, int num, const char* frm, va_list arg) {
	int n, f;
	CRITICAL(
  // prefix np. "E101 "
  snprintf(buf, LOG_BUF_LEN, "%c%d ", type, num);
  f = strlen(buf);
  // zawartosc
  n = vsnprintf(buf + f, LOG_BUF_LEN - f, frm, arg) + f;
  if (n > 0 && n < LOG_BUF_LEN) {
  	buf[n++] = '\r';
    buf[n++] = '\n';
    buf[n] = 0;
    BUF_WritePacket(&logs_buf, (uint8_t*)buf, LOG_PC_Txt, n);
  }
)
  return n;
}

int LOG_Bin(const void* bin, int size) {
  int n;
	CRITICAL(
	n = BUF_WritePacket(&logs_buf, (uint8_t*)bin, LOG_PC_Bin, size);
	)
  return n;
}

void LOG_Control(bool isSink) {
	CRITICAL(
		int packet_len = BUF_GetHeadPacketLen(&logs_buf);
		// when buffer is empty
		if(packet_len == 0) {
			goto exit;
		} else {
			uint8_t *data_ptr = BUF_GetHeadPacketPtr(&logs_buf);
			LOG_PacketCodes_t code = BUF_GetHeadPacketOpcode(&logs_buf);
			PORT_LogData(data_ptr, packet_len, code, isSink);
		}
exit:
	)
	return;
}
