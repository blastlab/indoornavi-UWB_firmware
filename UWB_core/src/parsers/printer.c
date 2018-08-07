#include "printer.h"

void PRINT_Version(const FC_VERSION_s *data, dev_addr_t did)
{
    const char *str_role;
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

void PRINT_TurnOn(const FC_TURN_ON_s *data, dev_addr_t did)
{
    LOG_INF("Device turn on %X", did);
}

void PRINT_TurnOff(const FC_TURN_OFF_s *data, dev_addr_t did)
{
    LOG_INF("Device turn off %X", did);
}

void PRINT_Beacon(const FC_BEACON_s *data, dev_addr_t did)
{
    LOG_INF("Beacon from %X", did);
}

void PRINT_DeviceAccepted(const FC_DEV_ACCEPTED_s *data, dev_addr_t did)
{
    LOG_INF("Device accepted from %X", did);
}

void PRINT_RFSet(const FC_RF_SET_s *data, dev_addr_t did)
{
    const int _f[8] = {0, 3494, 3994, 4493, 3994, 6490, 0, 6490};
    const int _bw[8] = {0, 499, 499, 499, 1331, 499, 0, 1082};
    const int _br[3] = {110, 850, 6800};
    const int _pac[4] = {8, 16, 32, 64};
    int br = _br[data->br];
    int prf = (data->prf == DWT_PRF_16M) ? 16 : 64;
    int pac = _pac[data->pac];
    int plen;
    switch (data->plen)
    {
    case DWT_PLEN_64:
        plen = 64;
        break;
    case DWT_PLEN_128:
        plen = 128;
        break;
    case DWT_PLEN_256:
        plen = 256;
        break;
    case DWT_PLEN_512:
        plen = 512;
        break;
    case DWT_PLEN_1024:
        plen = 1024;
        break;
    case DWT_PLEN_1536:
        plen = 1536;
        break;
    case DWT_PLEN_2048:
        plen = 2048;
        break;
    case DWT_PLEN_4096:
        plen = 4096;
        break;
    default:
        plen = 0;
        break;
    }
    LOG_INF("ch:%d-%d/%d br:%d plen:%d prf:%d pac:%d code:%d nsSfd:%d sfdTo:%d",
            data->chan, _f[data->chan], _bw[data->chan],
            br, plen, prf, pac, data->code, data->ns_sfd, data->sfd_to);
}

void PRINT_Measure(const measure_t *data)
{
  LOG_INF("a %X>%X %d %d %d %d", data->did1, data->did2, data->dist_cm,
          data->rssi_cdbm, data->fpp_cdbm, data->snr_cdbm);
}
