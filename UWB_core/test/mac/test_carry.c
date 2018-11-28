#include "mac/carry.h"
#include "unity.h"


#include "mock_port.h"
#include "mock_transceiver.h"
#include "mock_mac.h"

#include "logs.h"

TEST_FILE("iassert.c")
TEST_FILE("logs_common.c")

#ifndef TEST_ASSERT_M
#define TEST_ASSERT_M(EXPR) TEST_ASSERT_MESSAGE(EXPR, #EXPR);
#endif

mac_instance_t mac;
carry_instance_t carry;
settings_t settings = DEF_SETTINGS;

FAKE_VALUE_FUNC(int, LOG_Bin, const void*, int);
FAKE_VALUE_FUNC(const uint8_t*, BIN_Parse, const uint8_t *, const prot_packet_info_t*, int);

// FAKE_VALUE_FUNC(unsigned int, HAL_GetTick);
// FAKE_VALUE_FUNC(mac_buff_time_t, mac_port_buff_time);

mac_buf_t* MAC_BufferPrepare_custom_fake(dev_addr_t target, bool can_append) {
  static mac_buf_t buf;
  memset(&buf, 0, sizeof(buf));
  buf.frame.dst = target;
  buf.dPtr = &buf.frame.data[0];
  return &buf;
}

mac_buf_t* MAC_Buffer_custom_fake() {
  static mac_buf_t buf;
  memset(&buf, 0, sizeof(buf));
  buf.dPtr = &buf.buf[0];
  return &buf;
}

void MAC_Write_custom_fake(mac_buf_t* frame, const void* source, unsigned int len) {
  MAC_ASSERT(frame != 0);
	MAC_ASSERT(frame->dPtr >= (uint8_t* )frame);
	MAC_ASSERT(source != 0);
	MAC_ASSERT(0 <= len && len < MAC_BUF_LEN);
	const uint8_t* src = (uint8_t*)source;
	while (len > 0) {
		*frame->dPtr = *src;
		++frame->dPtr;
		++src;
		--len;
	}
}

void setUp(void) {
  RESET_FAKE(MAC_BufferPrepare);
  RESET_FAKE(MAC_Write);
  RESET_FAKE(MAC_Buffer);
  RESET_FAKE(LOG_Bin);
  RESET_FAKE(BIN_Parse);
  MAC_Write_fake.custom_fake = MAC_Write_custom_fake;
  MAC_Buffer_fake.custom_fake = MAC_Buffer_custom_fake;
  MAC_BufferPrepare_fake.custom_fake = MAC_BufferPrepare_custom_fake;
  
}

void tearDown(void) {
  CARRY_ParentDeleteAll();
}

void test_carry_PrepareBufTo_sink_from_anchor() {
  dev_addr_t parent = 0x8100;
  bool isConnectedToServer = false;
  settings.mac.role = RTLS_ANCHOR;

  CARRY_Init(isConnectedToServer);
  CARRY_SetYourParent(parent);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SINK, &carry);

  TEST_ASSERT_CALLED(MAC_BufferPrepare);
  TEST_ASSERT_M(buf->frame.dst == parent);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_SINK);

  CARRY_Send(buf, false);
  TEST_ASSERT_CALLED(MAC_Send);
}

void test_carry_PrepareBufTo_sink_from_tag() {
  dev_addr_t parent = 0x8101;
  bool isConnectedToServer = false;
  settings.mac.role = RTLS_TAG;

  CARRY_Init(isConnectedToServer);
  CARRY_SetYourParent(parent);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SINK, &carry);

  TEST_ASSERT_CALLED(MAC_BufferPrepare);
  TEST_ASSERT_M(buf->frame.dst == parent);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_SINK);

  CARRY_Send(buf, false);
  TEST_ASSERT_CALLED(MAC_Send);
}

void test_carry_PrepareBufTo_sink_from_sink() {
  bool isConnectedToServer = true;
  settings.mac.role = RTLS_SINK;
  settings.mac.addr = 0x8124;

  CARRY_Init(isConnectedToServer);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SINK, &carry);

  TEST_ASSERT_CALLED(MAC_BufferPrepare);
  TEST_ASSERT_M(buf->frame.dst == settings.mac.addr);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_SINK);

  CARRY_Send(buf, false);
  TEST_ASSERT_CALLED(BIN_Parse);
}

void test_carry_PrepareBufTo_server_from_anchor() {
  dev_addr_t parent = 0x8123;
  bool isConnectedToServer = false;
  settings.mac.role = RTLS_ANCHOR;

  CARRY_Init(isConnectedToServer);
  CARRY_SetYourParent(parent);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SERVER, &carry);

  TEST_ASSERT_CALLED(MAC_BufferPrepare);
  TEST_ASSERT_M(buf != 0);
  TEST_ASSERT_M(buf->frame.dst == parent);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_SERVER);

  CARRY_Send(buf, false);
  TEST_ASSERT_CALLED(MAC_Send);
}

void test_carry_PrepareBufTo_server_from_tag() {
  dev_addr_t parent = 0x8123;
  bool isConnectedToServer = false;
  settings.mac.role = RTLS_TAG;

  CARRY_Init(isConnectedToServer);
  CARRY_SetYourParent(parent);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SERVER, &carry);

  TEST_ASSERT_CALLED(MAC_BufferPrepare);
  TEST_ASSERT_M(buf != 0);
  TEST_ASSERT_M(buf->frame.dst == parent);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_SERVER);

  CARRY_Send(buf, false);
  TEST_ASSERT_CALLED(MAC_Send);
}

void test_carry_ParseMessage_to_server_from_tag() {
  bool isConnectedToServer = false;
  settings.mac.role = RTLS_TAG;
  carry.toSinkId = 0x8010;

  CARRY_Init(isConnectedToServer);

  FC_CARRY_s* pcarry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SERVER, &pcarry);

  TEST_ASSERT_CALLED_TIMES(1, MAC_BufferPrepare);

  prot_packet_info_t info;
  memset(&info, 0, sizeof(info));
  info.last_src = info.original_src = settings.mac.addr;
  CARRY_ParseMessage(pcarry, &info);

  TEST_ASSERT_CALLED_TIMES(2, MAC_BufferPrepare);
  TEST_ASSERT(MAC_BufferPrepare_fake.arg0_history[0] == carry.toSinkId);
}

void test_carry_PrepareBufTo_server_from_sink() {
  bool isConnectedToServer = true;
  settings.mac.role = RTLS_SINK;

  CARRY_Init(isConnectedToServer);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SERVER, &carry);

  TEST_ASSERT_M(buf != 0);
  TEST_ASSERT_M(buf->isServerFrame == true);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_SERVER);

  CARRY_Send(buf, false);
  TEST_ASSERT_CALLED(LOG_Bin);
}

void test_carry_ParseMessage_to_server_from_sink() {
  bool isConnectedToServer = true;
  settings.mac.role = RTLS_SINK;

  CARRY_Init(isConnectedToServer);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SERVER, &carry);


  prot_packet_info_t info;
  memset(&info, 0, sizeof(info));
  info.last_src = info.original_src = settings.mac.addr;
  CARRY_ParseMessage(carry, &info);

  TEST_ASSERT_CALLED(LOG_Bin);
}

void test_carry_PrepareBufTo_anchor_without_hops() {
  bool isConnectedToServer = true;
  settings.mac.role = RTLS_SINK;
  dev_addr_t target = 0x8520;

  CARRY_Init(isConnectedToServer);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(target, &carry);

  TEST_ASSERT_CALLED(MAC_BufferPrepare);
  TEST_ASSERT_M(buf != 0);
  TEST_ASSERT_M(buf->frame.dst == target);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_DEV);
}

void test_carry_ParentSet() {
  bool isConnectedToServer = true;
  settings.mac.role = RTLS_SINK;
  dev_addr_t target = 0x8000;
  dev_addr_t hop1 = 0x8001;

  CARRY_Init(isConnectedToServer);
  CARRY_ParentSet(hop1, settings.mac.addr);
  int r = CARRY_ParentSet(target, hop1);

  TEST_ASSERT_M(CARRY_ParentGet(hop1) == settings.mac.addr);
  TEST_ASSERT_M(CARRY_ParentGet(target) == hop1);
  TEST_ASSERT_M(CARRY_ParentGet(settings.mac.addr));
}

void test_carry_PrepareBufTo_anchor_with_one_hop() {
  bool isConnectedToServer = true;
  settings.mac.role = RTLS_SINK;
  dev_addr_t target = 0x8020;
  dev_addr_t hop1 = 0x8021;

  CARRY_Init(isConnectedToServer);
  CARRY_ParentSet(hop1, settings.mac.addr);
  CARRY_ParentSet(target, hop1);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(target, &carry);

  TEST_ASSERT_CALLED(MAC_BufferPrepare);
  TEST_ASSERT_M(buf != 0);
  TEST_ASSERT_M(buf->frame.dst == hop1);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_DEV);
  TEST_ASSERT_M((carry->verHopsNum & CARRY_HOPS_NUM_MASK) == 1);
  TEST_ASSERT_M(carry->hops[0]  == target);
}

void test_carry_PrepareBufTo_anchor_with_two_hop() {
  bool isConnectedToServer = true;
  settings.mac.role = RTLS_SINK;
  dev_addr_t target = 0x8030;
  dev_addr_t hop1 = 0x8031;
  dev_addr_t hop2 = 0x8032;

  CARRY_Init(isConnectedToServer);
  CARRY_ParentSet(hop1, settings.mac.addr);
  CARRY_ParentSet(hop2, hop1);
  CARRY_ParentSet(target, hop2);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(target, &carry);

  TEST_ASSERT_CALLED(MAC_BufferPrepare);
  TEST_ASSERT_M(buf != 0);
  TEST_ASSERT_M(buf->frame.dst == hop1);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_DEV);
  TEST_ASSERT_M((carry->verHopsNum & CARRY_HOPS_NUM_MASK) == 2);
  TEST_ASSERT_M(carry->hops[0]  == target);
  TEST_ASSERT_M(carry->hops[1]  == hop2);
}

void test_carry_read_write() {
  uint8_t data[4] = {1, 2, 3, 4};
  uint8_t rd_buf[4];

  bool isConnectedToServer = true;
  settings.mac.role = RTLS_SINK;

  CARRY_Init(isConnectedToServer);

  FC_CARRY_s* carry;
  mac_buf_t* buf = CARRY_PrepareBufTo(CARRY_ADDR_SERVER, &carry);

  TEST_ASSERT_M(buf != 0);
  TEST_ASSERT_M(buf->isServerFrame == true);
  TEST_ASSERT_M(carry != 0);
  TEST_ASSERT_M((carry->flags & CARRY_FLAG_TARGET_MASK) == CARRY_FLAG_TARGET_SERVER);

  int old_len = carry->len;
  uint8_t* old_dPtr = buf->dPtr;
  CARRY_Write(carry, buf, data, 3);
  TEST_ASSERT(carry->len == old_len + 3);

  CARRY_Write8(carry, buf, 4);
  TEST_ASSERT(carry->len == old_len + 4);

  CARRY_Read(buf, rd_buf, 3);
  TEST_ASSERT_CALLED(MAC_Read);
  CARRY_Read8(buf);
  TEST_ASSERT_CALLED(MAC_Read8);
}