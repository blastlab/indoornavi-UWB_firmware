#include "carry.h"

carry_instance_t carry;

void CARRY_Init(bool isConnectedToServer)
{
  carry.isConnectedToServer = isConnectedToServer;
}

dev_addr_t CARRY_ParentAddres()
{
  return carry.toSinkId;
}

static inline int CARRY_GetVersion(const FC_CARRY_s* pcarry)
{
	int temp = pcarry->verHopsNum;
	temp = 0x0F & (temp >> 4);
	return temp;
}

static inline void CARRY_SetVersion(FC_CARRY_s* pcarry)
{
	pcarry->verHopsNum = (pcarry->verHopsNum & 0x0F) | (CARRY_VERSION<<4);
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


mac_buf_t *CARRY_PrepareBufTo(dev_addr_t target, FC_CARRY_s** out_pcarry)
{
  mac_buf_t *buf;
  uint8_t target_flags = 0;

  if(target == CARRY_ADDR_SINK) {
    target_flags = CARRY_FLAG_TARGET_SINK;
    if(carry.toSinkId == 0) {
      buf = MAC_BufferPrepare(ADDR_BROADCAST, true);
    } else {
      buf = MAC_BufferPrepare(carry.toSinkId, true);
    }
  } else if(target == CARRY_ADDR_SERVER) {
    target_flags = CARRY_FLAG_TARGET_SERVER;
    buf = MAC_Buffer();
    buf->isServerFrame = carry.isConnectedToServer;
  } else {
    target_flags = CARRY_FLAG_TARGET_DEV;
    buf = MAC_BufferPrepare(target, true);
  }

  if (buf != 0)
  {
    FC_CARRY_s *p_target = (FC_CARRY_s *)buf->dPtr;
    if(out_pcarry != 0) { //todo: ten if jest chyba niebezpieczny
      *out_pcarry = p_target;
    }
    FC_CARRY_s prot;
    prot.FC = FC_CARRY;
    prot.len = sizeof(FC_CARRY_s);
    prot.src_addr = settings.mac.addr;
    prot.flags = target_flags;
    prot.verHopsNum = 0; // zero hops number and verion
    CARRY_SetVersion(&prot);
    MAC_Write(buf, &prot, sizeof(FC_CARRY_s));
    int hops_cnt = CARRY_WriteTrace(buf, target);

    // do not overwrite trace to this target
    if (hops_cnt > 0)
    {
      CARRY_ASSERT(hops_cnt < CARRY_MAX_HOPS);
      buf->dPtr += hops_cnt * sizeof(dev_addr_t);
      p_target->verHopsNum = hops_cnt;
    }
  }
  return buf;
}


void CARRY_Send(mac_buf_t* buf, bool ack_req)
{
  if(buf->isServerFrame) {
	buf->isServerFrame = false;
    LOG_Bin(buf->buf, MAC_BufLen(buf));
    MAC_Free(buf);
  } else {
    MAC_Send(buf, ack_req);
  }
}


void CARRY_ParseMessage(const void *data, const prot_packet_info_t *info)
{
  const uint8_t* buf = (const uint8_t*)data;
  uint8_t len = buf[1];
  CARRY_ASSERT(buf[0] == FC_CARRY);
  prot_packet_info_t new_info;
  uint8_t *dataPointer;
  uint8_t dataSize;
  bool toSink, toMe, toServer, ackReq;

  // copy old info 
  memcpy(&new_info, info, sizeof(new_info));

  // broadcast without carry header
  // or standard data message with carry header
  FC_CARRY_s *pcarry = (FC_CARRY_s *)data;
  int version = CARRY_GetVersion(pcarry);
  uint8_t hops_num = pcarry->verHopsNum & CARRY_HOPS_NUM_MASK;
  uint8_t target = pcarry->flags & CARRY_FLAG_TARGET_MASK;
  dataPointer = (uint8_t *)&pcarry->hops[0];
  dataPointer += hops_num * sizeof(pcarry->hops[0]);
  // check target
  toSink = (target == CARRY_FLAG_TARGET_SERVER);
  toSink |= (target == CARRY_FLAG_TARGET_SINK);
  toMe = target == CARRY_FLAG_TARGET_DEV && hops_num == 0;
  toMe |=
      target == CARRY_FLAG_TARGET_DEV && pcarry->hops[0] == settings.mac.addr;
  toMe |= target == CARRY_FLAG_TARGET_SINK && settings.mac.role == RTLS_SINK;
  toMe |= (pcarry->flags & CARRY_FLAG_REROUTE) &&
          pcarry->hops[0] == settings.mac.addr;
  toServer = target == CARRY_FLAG_TARGET_SERVER;
  ackReq = pcarry->flags & CARRY_FLAG_ACK_REQ;
  // fill new info fields
  new_info.carry = (struct FC_CARRY_s*)pcarry;
  new_info.direct_src = pcarry->src_addr;
  dataSize = len - sizeof(FC_CARRY_s) - hops_num * sizeof(pcarry->hops[0]);

  if(version != CARRY_VERSION)
  {
	LOG_WRN("Rx carry with version %d (%d)", version, CARRY_VERSION);
  }

  if (toMe)
  {
    BIN_Parse(dataPointer, &new_info, dataSize);
  }
  else if (toServer)
  {
    if(carry.isConnectedToServer) {
      LOG_Bin(data, buf[1]);
    } else {
      // change header - source and destination address
      // and send frame
      FC_CARRY_s* tx_carry;
      mac_buf_t* tx_buf = CARRY_PrepareBufTo(CARRY_ADDR_SERVER, &tx_carry);
      CARRY_Write(tx_carry, tx_buf, dataPointer, dataSize);
      CARRY_Send(tx_buf, ackReq);
    }
  }
  else if (toSink)
  {
    // change header - source and destination address
    // and send frame
    FC_CARRY_s* tx_carry;
    mac_buf_t* tx_buf = CARRY_PrepareBufTo(CARRY_ADDR_SINK, &tx_carry);
    CARRY_Write(tx_carry, tx_buf, dataPointer, dataSize);
    CARRY_Send(tx_buf, ackReq);
  }
  else
  {
    //IASSERT(0);
	  LOG_WRN("Rx carry to nobody");
  }
}


unsigned char CARRY_Read8(mac_buf_t *frame)
{
  return MAC_Read8(frame);
}


void CARRY_Write8(FC_CARRY_s* carry, mac_buf_t *frame, unsigned char value)
{
  carry->len += 1;
  MAC_Write8(frame, value);
}


void CARRY_Read(mac_buf_t *frame, void *destination, unsigned int len)
{
  MAC_Read(frame, destination, len);
}


void CARRY_Write(FC_CARRY_s* carry, mac_buf_t *frame, const void *source, unsigned int len)
{
  carry->len += len;
  MAC_Write(frame, source, len);
}
