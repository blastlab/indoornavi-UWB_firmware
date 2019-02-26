#include "bin_parser.h"
#include "../mac/carry.h"

uint8_t BIN_ParseSingle(const uint8_t* buf, const prot_packet_info_t* info) {
	IASSERT(buf != 0);
	extern const prot_cb_t prot_cb_tab[];
	uint8_t FC = buf[0];
	uint8_t len = buf[1];

	if (prot_cb_tab[FC].FC == FC && prot_cb_tab[FC].cb != 0) {
		prot_cb_tab[FC].cb(buf, info);
		return len;
	}
	return 0;
}

const uint8_t* BIN_Parse(const uint8_t data[], const prot_packet_info_t* info, int size) {
	uint8_t ret = 0;

	while (size > 0) {
		if (data[0] == FC_CARRY) {
			CARRY_ParseMessage(data, info);
			ret = data[1];
		}
		if (ret == 0) {
			ret = BIN_ParseSingle(data, info);
		}
		if (ret == 0) {
			LOG_ERR(ERR_BAD_OPCODE, data[0]);
			ret = data[1];  // frame len
			// when len is zero then break
			if (ret == 0) {
				break;
			}
		}
		size -= ret;
		data += ret;
	}
	return data;
}
