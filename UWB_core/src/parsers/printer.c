#include "printer.h"


void PRINT_Version(const FC_VERSION_s *data, dev_addr_t did)
{
    LOG_INF("version ver:%d", data->version);
}

void PRINT_Stat(const FC_STAT_s *data, dev_addr_t did)
{
    LOG_INF("stat rx:%d", data->rx_cnt);
}