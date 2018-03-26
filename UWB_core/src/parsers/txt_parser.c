#include "txt_parser.h"

char _txt_buf_raw[256];
txt_buf_t txt_buf = {.cmd = _txt_buf_raw,
                     .start = _txt_buf_raw,
                     .end = _txt_buf_raw + sizeof(_txt_buf_raw)};

// zwroc wskaznik za num spacjami albo 0
cchar *TXT_PointParamNumber(const txt_buf_t *buf, cchar *cmd, int num) {
  cchar *ptr = cmd;
  while (num > 0) {
    while (*ptr != ' ' && *ptr != 0) {
      ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
    }
    while (*ptr == ' ') {
      ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
    }
    if (*ptr == 0) {
      return 0;
    }
  }
  return ptr;
}

// return
int TXT_AtoI(const txt_buf_t *buf, cchar *ptr, int base) {
  int result = 0;
  if (ptr == 0) {
    return -1;
  }
  while (('0' <= *ptr && *ptr <= '9') ||
         (base == 16 && 'a' <= tolower(*ptr) && tolower(*ptr) <= 'f')) {
    result *= base;
    result += *ptr <= '9' ? *ptr - '0' : tolower(*ptr) - 'a';
    ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
  }
  return result;
}

// return pointer to
int TXT_GetParam(const txt_buf_t *buf, cchar *cmd, int base) {
  int i;
  cchar *ptr = buf->cmd;

  while (*ptr != 0) {
    for (i = 0; *ptr == cmd[i]; ++i) {
      ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
    }
    if (cmd[i] == 0) {
      return TXT_AtoI(buf, ptr, base);
    }
    ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
  }
  return -1;
}

bool TXT_StartsWith(const txt_buf_t* buf, cchar* cmd) {
	cchar* ptr = buf->cmd;
	while (*cmd != 0) {
		if (*cmd != *ptr) {
			return false;
		}
		ptr = ptr + 1 < buf->end ? ptr + 1 : buf->start;
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
  info.direct_src = did > 0 ? did : ADDR_BROADCAST;

  for (int i = 0; i < txt_cb_len; ++i) {
    if (TXT_StartsWith(buf, txt_cb_tab[i].cmd)) {
      IASSERT(txt_cb_tab[i].cb != 0);
      txt_cb_tab[i].cb(buf, &info);
      return;
    }
  }
  LOG_ERR("Bad command (version, stat)");
}
