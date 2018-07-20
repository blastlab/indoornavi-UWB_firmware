#include "carry.h"

carry_instance_t carry;

void CARRY_Init(bool isConnectedToServer)
{
  carry.isConnectedToServer = isConnectedToServer;
}

// return pointer to target or zero
carry_target_t *_CARRY_FindTarget(dev_addr_t target)
{
  for (int i = 0; i < CARRY_MAX_TARGETS; ++i)
  {
    if (carry.target[i].addr == target)
    {
      // target found
      return &carry.target[i];
    }
  }

  // when you didn't find
  return 0;
}

// return pointer to trace or zero
carry_trace_t *_CARRY_FindTrace(carry_target_t *ptarget)
{
  if (ptarget == 0)
  {
    return 0;
  }

  for (int i = 0; i < CARRY_MAX_TRACE; ++i)
  {
    if (ptarget->trace[i].fail_cnt < settings.carry.trace_max_fail_cnt)
    {
      mac_buff_time_t delta = mac_port_buff_time();
      delta -= ptarget->trace[i].last_update_time;
      if (delta < settings.carry.trace_max_fail_cnt)
      {
        return &ptarget->trace[i];
      }
    }
  }

  // when you didn't found valid trace
  return 0;
}

int CARRY_WriteTrace(mac_buf_t *buf, dev_addr_t target)
{
  CARRY_ASSERT(buf != 0);
  carry_target_t *ptarget = _CARRY_FindTarget(target);
  carry_trace_t *ptrace = _CARRY_FindTrace(ptarget);

  if (ptrace != 0)
  {
    int len = ptrace->path_len;
    MAC_Write(buf, ptrace->path, len * sizeof(dev_addr_t));
    return len;
  }

  return 0;
}

mac_buf_t *CARRY_PrepareBufTo(dev_addr_t target)
{
  mac_buf_t *buf = MAC_BufferPrepare(target, true);

  if (buf != 0)
  {
    FC_CARRY_s *p_target = (FC_CARRY_s *)buf->dPtr;
    FC_CARRY_s prot;
    prot.src_addr = settings.mac.addr;
    prot.flag_hops = CARRY_FLAG_TARGET_DEV;

    MAC_Write(buf, &prot, sizeof(FC_CARRY_s));
    int hops_cnt = CARRY_WriteTrace(buf, target);

    // do not overwrite trace to this target
    if (hops_cnt > 0)
    {
      CARRY_ASSERT(hops_cnt < CARRY_MAX_HOPS);
      buf->dPtr += hops_cnt * sizeof(dev_addr_t);
      p_target->flag_hops = (p_target->flag_hops & ~CARRY_HOPS_NUM_MASK) | hops_cnt;
    }
  }
  return buf;
}

void CARRY_ParseMessage(mac_buf_t *buf)
{
  prot_packet_info_t info;
  uint8_t *dataPointer;
  uint8_t dataSize;
  bool toSink, toMe, toServer, ackReq;
  // broadcast without carry header
  if (buf->frame.dst == ADDR_BROADCAST)
  {
    dataPointer = &buf->frame.data[0];
    ackReq = false;
    toSink = toServer = false;
    toMe = true;
    info.direct_src = buf->frame.src;
  }
  else
  {
    // or standard data message with carry header
    FC_CARRY_s *pcarry = (FC_CARRY_s *)&buf->frame.data[0];
    uint8_t hops_num = pcarry->flag_hops & CARRY_HOPS_NUM_MASK;
    uint8_t target = pcarry->flag_hops & CARRY_FLAG_TARGET_MASK;
    dataPointer = (uint8_t *)&pcarry->hops[0];
    dataPointer += hops_num * sizeof(pcarry->hops[0]);
    toSink = (target == CARRY_FLAG_TARGET_SERVER);
    toSink |= (target == CARRY_FLAG_TARGET_SINK);
    toMe = target == CARRY_FLAG_TARGET_DEV && hops_num == 0;
    toMe |=
        target == CARRY_FLAG_TARGET_DEV && pcarry->hops[0] == settings.mac.addr;
    toMe |= target == CARRY_FLAG_TARGET_SINK && settings.mac.role == RTLS_SINK;
    toMe |= (pcarry->flag_hops & CARRY_FLAG_REROUTE) &&
            pcarry->hops[0] == settings.mac.addr;
    toServer = target == CARRY_FLAG_TARGET_SERVER;
    ackReq = pcarry->flag_hops & CARRY_FLAG_ACK_REQ;
    info.carry = (struct FC_CARRY_s *)pcarry;
    info.direct_src = pcarry->src_addr;
    buf->dPtr = dataPointer;
  }
  dataSize = buf->buf + buf->rx_len - dataPointer;

  if (toMe)
  {
    BIN_Parse(buf, &info, dataSize);
  }
  else if (toServer && carry.isConnectedToServer)
  {
    LOG_Bin(&buf->frame.data[0], buf->rx_len - MAC_HEAD_LENGTH);
  }
  else if (toSink)
  {
    MAC_FillFrameTo(buf, carry.toSinkId);
    buf->dPtr = dataPointer + dataSize;
    MAC_Send(buf, ackReq);
  }
  else
  {
    IASSERT(0);
  }
}
