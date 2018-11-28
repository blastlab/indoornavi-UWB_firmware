#include "mac.h"
#include "unity.h"

#include "logs.h"

#include "mock_port.h"
#include "mock_transceiver.h"
#include "mock_sync.h"
#include "mock_toa.h"
#include "mock_toa_routine.h"

// include this files to test
TEST_FILE("iassert.c")
TEST_FILE("logs_common.c")
TEST_FILE("mac.c")

settings_t settings = DEF_SETTINGS;
settings_otp_t _settings_otp;
settings_otp_t const *settings_otp = &_settings_otp;
extern mac_instance_t mac;
#define TEST_ASSERT_M(expr) TEST_ASSERT_MESSAGE(expr, #expr)

//FAKE_VALUE_FUNC(int, SYNC_RxToCb);
//FAKE_VALUE_FUNC(int, SYNC_RxCb, const void *, const prot_packet_info_t *);
//FAKE_VALUE_FUNC(int, SYNC_TxCb, int64_t);
FAKE_VOID_FUNC(CARRY_ParseMessage, mac_buf_t *);
FAKE_VOID_FUNC(dwt_forcetrxoff);
FAKE_VOID_FUNC(dwt_rxreset);
FAKE_VOID_FUNC(dwt_setrxtimeout, uint16_t);
FAKE_VALUE_FUNC(int, dwt_rxenable, int);
FAKE_VOID_FUNC(FU_AcceptFirmware);
FAKE_VOID_FUNC(listener_isr, const dwt_cb_data_t *)

// private function
mac_buf_t *_MAC_BufGetOldestToTx();

mac_buff_time_t time_mock_value() {
  static mac_buff_time_t i = 0;
  return i++;
}

void setUp(void) {
  PORT_TickHr_fake.custom_fake = time_mock_value;
  for (int i = 0; i < MAC_BUF_CNT; ++i) {
    mac.buf[i].state = FREE;
  }
}

void tearDown(void) {}

void test_MAC_Read_write_zero_len() {
  const char data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  unsigned char rbuf[100];
  mac_buf_t *buf = MAC_Buffer();
  // write zero len
  uint8_t idata = *buf->dPtr;
  uint8_t *idPtr = buf->dPtr;
  MAC_Write(buf, data + 1, 0);
  MAC_Write(buf, data + 1, 0);
  TEST_ASSERT_M(buf->dPtr == idPtr);
  TEST_ASSERT_M(*buf->dPtr == idata);
  memset(rbuf, 0xAA, 100);
  TEST_ASSERT_M(*rbuf == 0xAA);
  MAC_Read(buf, rbuf, 0);
  TEST_ASSERT_M(buf->dPtr == idPtr);
  TEST_ASSERT_M(rbuf[0] == 0xAA);
}

void test_MAC_Read_write_positive_len() {
  const uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  uint8_t rbuf[100];
  mac_buf_t *buf;
  for (int i = 0; i < 10; ++i) {
    buf = MAC_Buffer();
    TEST_ASSERT_M(buf != 0);
    MAC_Write(buf, data, i);
    buf->dPtr = buf->buf; // reset read dPtr
    MAC_Read(buf, rbuf, i);
    TEST_ASSERT_M(memcmp(rbuf, data, i) == 0);
    MAC_Free(buf);
  }
}

/*void test_MAC_Buffer_overflow() {
  mac_buf_t *buf;
  // reserve each free buffer
  for (int i = 0; i < MAC_BUF_CNT; ++i) {
    buf = MAC_Buffer();
    TEST_ASSERT(buf != 0);
  }
  // overload
  for (int i = 0; i < 10 * MAC_BUF_CNT; ++i) {
    buf = MAC_Buffer();
    TEST_ASSERT(buf == 0);
  }
}*/

void test_MAC_BufGetOldestToTx_return_buf() {
  uint8_t data[] = {1, 2, 3, 4, 5, 6};
  data[1] = sizeof(data);

  mac_buf_t *buf = MAC_BufferPrepare(ADDR_BROADCAST, false);
  TEST_ASSERT(buf != 0);
  MAC_Write(buf, data, data[1]);
  MAC_Send(buf, false);

  /*TEST_ASSERT(buf->state == WAIT_FOR_TX);
  TEST_ASSERT_NOT_EQUAL(0, _MAC_BufGetOldestToTx());

  buf->state = FREE;
  TEST_ASSERT(0 == _MAC_BufGetOldestToTx());*/
}