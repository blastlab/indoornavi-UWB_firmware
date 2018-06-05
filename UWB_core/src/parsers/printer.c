#include "printer.h"

void PRINT_Version(const FC_VERSION_s *data, dev_addr_t did)
{
	const char* str_role;
	switch (data->role)
	{
	case RTLS_SINK:
		str_role = "SINK";
		break;
	case RTLS_ANCHOR:
		str_role = "ANCHOR";
		break;
	case RTLS_TAG:
		str_role = "TAG";
		break;
	case RTLS_LISTENER:
		str_role = "LISTENER";
		break;
	case RTLS_DEFAULT:
		str_role = "DEFAULT";
		break;
	default:
		str_role = "OTHER";
	}
	LOG_INF("version %X %s %d.%d %d.%d.%X", did, str_role, data->hMajor, data->hMinor, data->fMajor, data->fMinor, data->hash);
}

void PRINT_Stat(const FC_STAT_s *data, dev_addr_t did)
{
	LOG_INF("stat %x mV:%d Rx:%d Tx:%d Er:%d To:%d", did, data->battery_mV, data->rx_cnt, data->tx_cnt, data->err_cnt, data->to_cnt);
}
