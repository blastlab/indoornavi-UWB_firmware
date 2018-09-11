#include "txt_parser.h"

char _txt_buf_raw[256];
static char *txt_buf_wptr = _txt_buf_raw;
static txt_buf_t txt_buf = {.cmd = _txt_buf_raw,
                     .start = _txt_buf_raw,
                     .end = _txt_buf_raw + sizeof(_txt_buf_raw),
										 .cnt = 0};

// zwroc wskaznik za num spacjami albo 0
cchar *TXT_PointParamNumber(const txt_buf_t *buf, cchar *cmd, int num) {
  cchar *ptr = cmd;
  while (num > 0) {
    while (*ptr != ' ' && *ptr != 0) {
      INCREMENT_CYCLE(ptr, buf->start, buf->end);
    }
    while (*ptr == ' ') {
      INCREMENT_CYCLE(ptr, buf->start, buf->end);
    }
    if (*ptr == 0) {
      return 0;
    }
    --num;
  }
  return ptr;
}

// return
int TXT_AtoI(const txt_buf_t *buf, cchar *ptr, int base) {
  int result = 0;
  int minus = 1;
  if (ptr == 0) {
    return -1;
  }
  while(*ptr == ' ' && *ptr != 0) {
	  INCREMENT_CYCLE(ptr, buf->start, buf->end);
  }
  if(*ptr == '-') {
	  minus = -1;
	  INCREMENT_CYCLE(ptr, buf->start, buf->end);
  }
  while (('0' <= *ptr && *ptr <= '9') ||
         (base == 16 && 'a' <= tolower(*ptr) && tolower(*ptr) <= 'f')) {
	  if(('0' <= *ptr && *ptr <= '9') || (base > 10 && 'a' <= tolower(*ptr) && tolower(*ptr) <= 'f'))
	  {
		  result *= base;
		  result += *ptr <= '9' ? *ptr - '0' : tolower(*ptr) - 'a' + 10;
	  } else
	  {
		  return result;
	  }
	  INCREMENT_CYCLE(ptr, buf->start, buf->end);
  }
  return result*minus;
}

// return pointer to
int TXT_GetParam(const txt_buf_t *buf, cchar *cmd, int base) {
  int i;
  cchar *ptr = buf->cmd;

  while (*ptr != 0) {
    for (i = 0; *ptr == cmd[i]; ++i) {
      INCREMENT_CYCLE(ptr, buf->start, buf->end);
    }
    if (cmd[i] == 0) {
      return TXT_AtoI(buf, ptr, base);
    }
    INCREMENT_CYCLE(ptr, buf->start, buf->end);
  }
  return -1;
}

int TXT_GetParamNum(const txt_buf_t *buf, int num, int base) {
  cchar *ptr = TXT_PointParamNumber(buf, buf->cmd, num);
  if(ptr != 0) {
    return TXT_AtoI(buf, ptr, base);
  } else {
    return -1;
  }
}

bool TXT_StartsWith(const txt_buf_t* buf, cchar* cmd) {
	volatile cchar* ptr = buf->cmd;
	while (*cmd != 0) {
		if (*cmd != *ptr) {
			return false;
		}
    INCREMENT_CYCLE(ptr, buf->start, buf->end);
		++cmd;
	}
	return true;
}


void TXT_Parse(const txt_buf_t *buf) {
  // in txt_parser_cb.c
  extern const txt_cb_t txt_cb_tab[];
  extern const int txt_cb_len;

  prot_packet_info_t info;
  memset(&info, 0, sizeof(info));
  int did = TXT_GetParam(buf, "did:", 16);
  info.direct_src = did > 0 ? did : CARRY_ADDR_SERVER;

  for (int i = 0; i < txt_cb_len; ++i) {
    if (TXT_StartsWith(buf, txt_cb_tab[i].cmd)) {
      IASSERT(txt_cb_tab[i].cb != 0);
      txt_cb_tab[i].cb(buf, &info);
      return;
    }
  }
  LOG_ERR("Bad command");
}

// take input to data parser, ignore \r and split by \n
void TXT_Input(const char *str, int len) {
	while(len-- > 0) {
		txt_buf_wptr[0] = str[0];
		// new command
		if(str[0] == '\n') {
			txt_buf_wptr[0] = 0;
			++txt_buf.cnt;
		}
		// not ignored char
		if(str[0] != '\r'){
			++txt_buf_wptr;
			if(txt_buf_wptr >= txt_buf.end) {
				txt_buf_wptr = (char*)txt_buf.start;
			}
		}
		++str;
	}
}

// look for any new text message to parse
void TXT_Control() {
	if(txt_buf.cnt > 0){
		TXT_Parse(&txt_buf);
		--txt_buf.cnt;
		while(txt_buf.cmd[0] != 0) {
			INCREMENT_CYCLE(txt_buf.cmd, txt_buf.start, txt_buf.end);
		}
		INCREMENT_CYCLE(txt_buf.cmd, txt_buf.start, txt_buf.end);
	}
}
