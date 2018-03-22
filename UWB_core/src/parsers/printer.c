#include "printer.h"


void print_version(const FC_VERSION_s *data, dev_addr_t did)
{
    LOG_INF("version ver:%d", data->version);
}

void print_stat(const FC_STAT_s *data, dev_addr_t did)
{
    LOG_INF("stat rx:%d", data->rx_cnt);
}