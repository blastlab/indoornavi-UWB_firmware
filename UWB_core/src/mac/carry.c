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

void CARRY_SetYourParent(dev_addr_t did)
{
	carry.toSinkId = did;
}

// return pointer to target or zero
carry_target_t *CARRY_GetTarget(dev_addr_t target)
{
  for (int i = 0; i < settings.carry.targetCounter; ++i)
  {
    if (settings.carry.target[i].addr == target)
    {
      // target found
      return &settings.carry.target[i];
    }
  }

  // when you didn't find
  return 0;
}

carry_target_t* CARRY_NewTarget(dev_addr_t target)
{
  int cnt = settings.carry.targetCounter;
  carry_target_t* ptr;
  if(cnt >= CARRY_MAX_TARGETS) {
    return 0;
  } else {
    ptr = &settings.carry.target[cnt];
    memset(ptr, 0, sizeof(*ptr));
    ptr->addr = target;
    ptr->lastUpdateTime = PORT_TickMs();
    settings.carry.targetCounter += 1;
    return ptr;
  }
}

bool CARRY_ParentSet(dev_addr_t target, dev_addr_t parent)
{
  carry_target_t *ptarget;
  bool isParentKnown = parent == ADDR_BROADCAST || parent == settings.mac.addr;
  if(!isParentKnown) {
    isParentKnown = CARRY_GetTarget(parent) != 0;
  }
  if(!isParentKnown) {
    return false;
  }
  ptarget = CARRY_GetTarget(target);
  if(ptarget == 0){
	  ptarget = CARRY_NewTarget(target);
  }
  if(target != 0) {
    ptarget->parents[0] = parent;
    ptarget->parentsScore[0] = 0;
    ptarget->lastUpdateTime = PORT_TickMs();
    return true;
  }
  return false;
}

dev_addr_t CARRY_ParentGet(dev_addr_t target)
{
  carry_target_t* ptarget = CARRY_GetTarget(target);
  if(ptarget != 0) {
    return ptarget->parents[0];
  } else {
    return ADDR_BROADCAST;
  }
}

void CARRY_ParentDeleteAll()
{
	settings.carry.targetCounter = 0;
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

int CARRY_WriteTrace(uint8_t *buf, dev_addr_t target, dev_addr_t *nextDid)
{
  CARRY_ASSERT(buf != 0);
  int hopCnt = 0;
  dev_addr_t parent = CARRY_ParentGet(target);

  while(parent != ADDR_BROADCAST && parent != settings.mac.addr) {
    hopCnt += 1;
    memcpy(buf, &target, sizeof(dev_addr_t));
    buf += sizeof(dev_addr_t);
    target = parent;
    parent = CARRY_ParentGet(target);
  }

  *nextDid = target;
  return hopCnt;
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
    int hops_cnt;
    if(target == CARRY_ADDR_SINK) {
      hops_cnt = CARRY_WriteTrace(buf->dPtr + sizeof(FC_CARRY_s), carry.toSinkId == 0 ? ADDR_BROADCAST : carry.toSinkId, &buf->frame.dst);
    }
    else {
      hops_cnt = CARRY_WriteTrace(buf->dPtr + sizeof(FC_CARRY_s), target, &buf->frame.dst);
    }
    CARRY_ASSERT(hops_cnt < CARRY_MAX_HOPS);
    prot.verHopsNum += hops_cnt;
    prot.len += hops_cnt * sizeof(dev_addr_t); // hops data
    MAC_Write(buf, &prot, sizeof(FC_CARRY_s));
    buf->dPtr += hops_cnt * sizeof(dev_addr_t); // hops data
  }
  return buf;
}


void CARRY_Send(mac_buf_t* buf, bool ack_req)
{
  if(buf->isServerFrame) {
	buf->isServerFrame = false;
    LOG_Bin(buf->buf, MAC_BufLen(buf));
    MAC_Free(buf);
  } else if(buf->frame.dst == settings.mac.addr) {
    LOG_Bin(buf->frame.data, MAC_BufLen(buf)-MAC_HEAD_LENGTH);
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
  FC_CARRY_s* tx_carry;
  mac_buf_t* tx_buf;
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
      tx_buf = CARRY_PrepareBufTo(CARRY_ADDR_SERVER, &tx_carry);
      CARRY_Write(tx_carry, tx_buf, dataPointer, dataSize);
      CARRY_Send(tx_buf, ackReq);
    }
  } else if (toSink) {
    // change header - source and destination address
    // and send frame
    tx_buf = CARRY_PrepareBufTo(CARRY_ADDR_SINK, &tx_carry);
    CARRY_Write(tx_carry, tx_buf, dataPointer, dataSize);
    CARRY_Send(tx_buf, ackReq);
  } else if(hops_num > 0) {
    dev_addr_t nextDid = pcarry->hops[hops_num-1];
    int lenPre = (int)((uint8_t*)&pcarry->hops[hops_num-1] - (uint8_t*)data);
    int lenPost = len - lenPre - sizeof(dev_addr_t);
    mac_buf_t* tx_buf = MAC_BufferPrepare(nextDid, true);
    tx_carry = (FC_CARRY_s*)tx_buf->dPtr;
    MAC_Write(tx_buf, data, lenPre);
    MAC_Write(tx_buf, data + lenPre + sizeof(dev_addr_t), lenPost);
    tx_carry->len -= sizeof(dev_addr_t);
    tx_carry->verHopsNum -= 1;
    MAC_Send(tx_buf, (pcarry->flags & CARRY_FLAG_ACK_REQ) != 0);
  } else {
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
