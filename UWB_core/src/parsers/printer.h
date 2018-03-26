#ifndef _PRINTER_H
#define _PRINTER_H
#include "bin_struct.h"
#include "../logs.h"

void PRINT_Version(const FC_VERSION_s *data, dev_addr_t did);

void PRINT_Stat(const FC_STAT_s *data, dev_addr_t did);

#endif
