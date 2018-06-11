/*
 * FU.c
 *
 *  Created on: 7 cze 2017
 *      Author: Karol Trzci�ski
 */

//#include "main.h"
//#include "IAP.h"
#include "FU.h"

#if FU_BLOCK_SIZE % 8 != 0
#error 8 bytes are programmed to flash at once
#endif

// ===
// implementation
// ===

typedef struct {
  int sesionPacketCounter;
  uint16_t newCrc;
  uint32_t newVer;
  uint8_t newHash;
  uint32_t fileSize;
} FU_instance_t;

// information about new firmware from SOT frame
static FU_instance_t FU_instance;
static FU_prot FU_tx;

// ==================
// === bootloader ===
// ==================

// compare address of this function with FU_DESTINATION_x and
// return address of start a currently used flash program space
uint8_t *FU_GetCurrentFlashBase() {
  if ((void *)FU_GetCurrentFlashBase < FU_DESTINATION_2) {
    return FU_DESTINATION_1;
  } else {
    return FU_DESTINATION_2;
  }
}

// compare address of this function with FU_DESTINATION_x and
// return address of start flash program space to write new software
static inline uint8_t *FU_GetAddressToWrite() {
  if ((void *)FU_GetAddressToWrite < FU_DESTINATION_2) {
    return FU_DESTINATION_2;
  } else {
    return FU_DESTINATION_1;
  }
}

// check
int FU_AcceptFirmwareVersion(int Ver) {
  uint16_t fMinor = (Ver)&0xFFFF;
  if ((fMinor % 2) * FU_MAX_PROGRAM_SIZE + FU_DESTINATION_1 !=
      FU_GetAddressToWrite()) {
    return 0;
  } else {
    return 1;
  }
}

int FU_AcceptHardwareVersion(int Ver) {
  uint8_t hVer = Ver >> 24;
  if (H_MAJOR_CALC(hVer) != H_MAJOR_CALC(settings.version.h_version)) {
    return 0;
  } else {
    return 1;
  }
}

// prepare opcode with current protocol version
static inline uint8_t FU_MakeOpcode(uint8_t opcode) {
  return (FU_PROT_VERSION << 4) | (opcode & 0x0F);
}

// check if it is this opcode
static inline uint8_t FU_IsOpcode(uint8_t original, uint8_t toCkeck) {
  return (original & 0x0F) == toCkeck;
}

// return 1 if firmware version is inside this block of data
static inline uint8_t FU_IsVersionInside(const FU_prot *fup) {
  uint16_t dataLen = fup->frameLen - FU_PROT_HEAD_SIZE - 2; // -2 for CRC
  if (fup->extra * FU_BLOCK_SIZE <= FU_VERSION_LOC &&
      FU_VERSION_LOC < fup->extra * FU_BLOCK_SIZE + dataLen) {
    return 1;
  }
  return 0;
}

// check address of reset handler in new firmware --> correct version and
// location in flash
static uint8_t FU_IsNewFirmwareInBadPlace(uint32_t *new_reset_IRQ_addr) {
  if (*new_reset_IRQ_addr < (uint32_t)FU_DESTINATION_1) {
    return 1;
  } else if ((uint32_t)new_reset_IRQ_addr < *new_reset_IRQ_addr &&
             *new_reset_IRQ_addr <
                 (uint32_t)new_reset_IRQ_addr + FU_MAX_PROGRAM_SIZE) {
    return 0;
  } else {
    return 1;
  }
}

// return compressed firmware and hardware version from flash
static inline int FU_GetLocalHash() {
  return (uint8_t)(settings_otp->h_major + settings.version.f_hash);
}

// check if CRC is ok
static uint8_t FU_IsCRCError(const FU_prot *fup) {
  PORT_CrcReset();
  return PORT_CrcFeed(fup, fup->frameLen);
}

// return 1 data in flash and packet with new version is correct
static uint8_t FU_IsFlashCRCError(const FU_prot *fup) {
  // number of written bytes before this current frame
  int sizeP = fup->extra * FU_BLOCK_SIZE;
  // number of file data bytes in frame, -2 for CRC
  int sizeFup = fup->frameLen - FU_PROT_HEAD_SIZE - 2;
  void *destination = FU_GetAddressToWrite();
  PORT_CrcReset();
  PORT_CrcFeed(destination, sizeP);
  PORT_CrcFeed(fup->data, sizeFup);
  PORT_CrcFeed((uint8_t *)destination + sizeP + sizeFup,
                FU_instance.fileSize - sizeP - sizeFup);
  return PORT_CrcFeed(&FU_instance.newCrc, 2); // 0: ok, else error
}

// calculate CRC and add it at position fup[frameLen-2] and fup[frameLen-1]
static void FU_FillCRC(const FU_prot *fup) {
  PORT_CrcReset();
  uint16_t txCrc = PORT_CrcFeed(fup, fup->frameLen - 2);
  ((uint8_t *)(fup))[fup->frameLen - 2] =
      (uint8_t)(txCrc >> 8); // add 2 CRC bytes
  ((uint8_t *)(fup))[fup->frameLen - 1] = (uint8_t)(txCrc);
}

// add 2 to packet length, write current version, calculate CRC, send packet via
// resp
static void FU_SendResponse(FU_prot *fup, const prot_packet_info_t *info) {
#ifdef TEST_FU
  DW_ASSERT(fup->frameLen >= FU_PROT_HEAD_SIZE);
  DW_ASSERT(fup->frameLen < FU_PROT_HEAD_SIZE + FU_MAX_DATA_SIZE);
  DW_ASSERT((fup->opcode >> 4) == FU_PROT_VERSION);
  if (FU_IsOpcode(fup->opcode, FU_OPCODE_ABORT) ||
      FU_IsOpcode(fup->opcode, FU_OPCODE_ACK) ||
      FU_IsOpcode(fup->opcode, FU_OPCODE_EOT)) {
    DW_ASSERT(fup->frameLen == FU_PROT_HEAD_SIZE);
  } else if (FU_IsOpcode(fup->opcode, FU_OPCODE_ASK_VER)) {
    DW_ASSERT(fup->frameLen == FU_PROT_HEAD_SIZE + 1);
  } else if (FU_IsOpcode(fup->opcode, FU_OPCODE_SOT)) {
    DW_ASSERT(fup->frameLen == FU_PROT_HEAD_SIZE + 7);
  } else {
    DW_ASSERT(fup->frameLen >= FU_PROT_HEAD_SIZE);
  }
#endif
  fup->FC = FC_FU;
  // podaj swoja aktualna wersje firmwaru
  fup->hash = (uint8_t)FU_GetLocalHash();
  fup->frameLen += 2; // correct value for CRC calculation
  FU_FillCRC(fup);
  mac_buf_t *buf = carry_prepare_response(info);
  MAC_Write(buf, fup, fup->frameLen);
  MAC_Send(buf, true);
}

// set opcode, error code, package length then version and CRC and send message
// via resp
static void FU_SendError(const prot_packet_info_t *info, uint16_t err) {
  FU_tx.opcode = FU_MakeOpcode(FU_OPCODE_ABORT); // zglos blad
  FU_tx.extra = err;
  FU_tx.frameLen = FU_PROT_HEAD_SIZE;
  FU_SendResponse(&FU_tx, info);
}

/* Public functions ---------------------------------------------------------*/

// zapisz odebrane CRC dla pliku,
// wyczysc caly bank pamieci - zajmuje duzo czasu
static void FU_SOT(const FU_prot *fup_d, const prot_packet_info_t *info) {
  FU_SOT_prot *fup = (FU_SOT_prot *)fup_d;
  if (fup->frameLen != sizeof(FU_SOT_prot)) { // 2 for CRC, 4 for size
    FU_SendError(info, FU_ERR_BAD_FRAME_LEN);
    return;
  } // sprawdz czy ta wersja jest nowsza - wiekszy numerek 'version'
  else if (FU_AcceptFirmwareVersion(fup->fversion) == 0) {
    FU_SendError(info, FU_ERR_BAD_F_VERSION);
    return;
  } else if (FU_AcceptHardwareVersion(fup->fversion) == 0) {
    FU_SendError(info, FU_ERR_BAD_H_VERSION);
    return;
  } else if (fup->fileSize > FU_MAX_PROGRAM_SIZE || fup->fileSize == 0) {
    FU_SendError(info, FU_ERR_BAD_FILE_SIZE);
    return;
  } else if (settings.version.boot_reserved != BOOTLOADER_MAGIC_NUMBER) {
    FU_SendError(info, FU_ERR_FIR_NOT_ACCEPTED_YET);
    return;
  }

  // overwrite new firmware info
  FU_instance.newCrc = fup->firmwareCRC; // zapis CRC calego pliku .bin
  FU_instance.newVer = fup->fversion;    // zapis numeru nowej wersji programu
  FU_instance.fileSize = fup->fileSize;
  FU_instance.newHash = fup->hash;
  FU_instance.sesionPacketCounter = 0;

  // check result
  if (PORT_FlashErase(FU_GetAddressToWrite(), FU_instance.fileSize) !=
      PORT_Success) {
    FU_SendError(info, FU_ERR_FLASH_ERASING);
    return;
  }

  // check erased memory
  uint8_t *ptr = FU_GetAddressToWrite();
  while (fup->fileSize > 0) {
    --fup->fileSize;
    if (*ptr != 0xFF) {
      FU_SendError(info, FU_ERR_FLASH_ERASING);
      return;
    }
  }

  // send ack
  FU_tx.opcode = FU_MakeOpcode(FU_OPCODE_ACK);
  FU_tx.frameLen = FU_PROT_HEAD_SIZE;
  FU_SendResponse(&FU_tx, info);
  LOG_DBG("FU_SOT ok");
}

// sprawdz czy to jest ramka z wersja nowego programu,
// gdy tak jest to sprawdz CRC dla calego programu
static void FU_Data(const FU_prot *fup, const prot_packet_info_t *info) {
  if (FU_instance.fileSize == 0) {
    FU_SendError(info, FU_ERR_BAD_FILE_SIZE);
    return;
  } else if (fup->hash != (uint8_t)FU_instance.newHash) {
    // sprawdz czy zgadza sie wersja z ta z ramki SOT
    FU_SendError(info, FU_ERR_BAD_FRAME_HASH);
    return;
  } else if (fup->extra * FU_BLOCK_SIZE >= FU_instance.fileSize) {
    FU_SendError(info, FU_ERR_BAD_OFFSET);
    return;
  } else if (FU_IsVersionInside(fup) &&
             !FU_IsOpcode(fup->opcode,
                          FU_OPCODE_EOT)) { // gdy ta paczka zawiera version a
    // nie jest typu EOT, to zg�o� b��d
    FU_SendError(info, FU_ERR_VERSION_IN_PACKAGE);
  }
  // gdy nie ma wersji firmwaru w tej paczce lub jest, ale zgadza sie CRC
  // to zaladuj program do flash
  uint16_t dataSize = fup->frameLen - FU_PROT_HEAD_SIZE - 2; // 2 for CRC
  unsigned char *address = FU_GetAddressToWrite() + FU_BLOCK_SIZE * fup->extra;
  PORT_WatchdogRefresh();
  int ret = PORT_FlashSave(address, fup->data, dataSize);
  if (ret != 0) {
    FU_SendError(info, FU_ERR_FLASH_WRITING);
    return;
  } else { // sukces
    FU_instance.sesionPacketCounter += 1;
    FU_tx.opcode = FU_MakeOpcode(FU_OPCODE_ACK);
    FU_tx.frameLen = FU_PROT_HEAD_SIZE;
    FU_SendResponse(&FU_tx, info);
  }
}

// process last message with data as a device
// so you have to
static void FU_EOT(const FU_prot *fup, const prot_packet_info_t *info) {
  if (FU_IsFlashCRCError(fup)) {
    FU_SendError(info, FU_ERR_BAD_FLASH_CRC);
  } else if (FU_IsNewFirmwareInBadPlace(((uint32_t *)FU_GetAddressToWrite()) +
                                        1)) {
    FU_SendError(info, FU_ERR_BAD_F_VERSION);
  } else { // dostalismy wersje z poprawna wersja firmwaru, konczymy transmisje
    FU_Data(fup, info); // zapisz ostatnio porcje danych we flashu
    FU_instance.fileSize = 0;
    FU_instance.newHash = 0;
    LOG_INF("FU successfully firmware uploaded");
    PORT_WatchdogRefresh();
    PORT_SleepMs(5); // to send messages
    PORT_WatchdogRefresh();
    PORT_Reboot();
  }
}

// obluga paczki przychodzacej
void FU_HandleAsDevice(const FU_prot *fup, const prot_packet_info_t *info) {
  if (FU_IsCRCError(fup)) {
    // check CRC
    FU_SendError(info, FU_ERR_BAD_FRAME_CRC);
    return;
  } else if ((fup->opcode >> 4) != FU_PROT_VERSION) {
    // check protocol version
    FU_SendError(info, FU_ERR_BAD_PROT_VER);
    return;
  }
  if (FU_IsOpcode(fup->opcode, FU_OPCODE_DATA)) {
    // przeslanie paczki z danymi
    FU_Data(fup, info);
  } else if (FU_IsOpcode(fup->opcode, FU_OPCODE_SOT)) {
    // rozpoczyna transmisje nowej paczki
    FU_SOT(fup, info);
  } else if (FU_IsOpcode(fup->opcode, FU_OPCODE_EOT)) {
    FU_EOT(fup, info);
  } else {
    FU_SendError(info, FU_ERR_BAD_OPCODE_SET);
  }
}


void FU_AcceptFirmware()
{
	if(PORT_BkpRegisterRead(BOOTLOADER_MAGIC_REG) != BOOTLOADER_MAGIC_NUMBER)
	{
		PORT_BkpRegisterWrite(BOOTLOADER_MAGIC_REG, BOOTLOADER_MAGIC_NUMBER);
	}
}

// funkcja wywo�ywana z main
void FU_Init(bool forceNoFirmwareCheck) {
  FU_ASSERT(FU_MAX_PROGRAM_SIZE % FLASH_PAGE_SIZE == 0);
  FU_ASSERT(FU_DESTINATION_2+FU_MAX_PROGRAM_SIZE <= (void*)(FLASH_BASE + FLASH_BANK_SIZE));
  FU_instance.newVer = FU_GetLocalHash();
  if(forceNoFirmwareCheck) {
  	FU_AcceptFirmware();
  }
}

// ===========
// TEST module
// ===========
// - redefine FU_VERSION_LOC TO 0x12 (for example)
#if defined(TEST_FU)
uint8_t test_fup_buf[512];
static FU_prot *test_fup = (FU_prot *)test_fup_buf;
static FU_SOT_prot *test_sot = (FU_SOT_prot *)test_fup_buf;

#ifndef DW_ASSERT
void DW_ASSERT(uint8_t cond) {
  while (cond == 0) {
  }
}
#endif

static void FU_Test_Resp(const FU_prot *fup) { *test_fup = *fup; }

uint8_t FU_Test() {
  // sprawdz obliczanie CRC
  static const uint8_t aDataBuffer[] = {
      0x21, 0x10, 0x00, 0x00, 0x63, 0x30, 0x42, 0x20, 0xa5, 0x50,
      0x84, 0x40, 0xe7, 0x70, 0xc6, 0x60, 0x4a, 0xa1, 0x29, 0x91,
      0x8c, 0xc1, 0x6b, 0xb1, 0xce, 0xe1, 0xad, 0xd1, 0x31, 0x12,
      0xef, 0xf1, 0x52, 0x22, 0x73, 0x32, 0xa1, 0xb2, 0xc3};
  static const uint32_t BUFFER_SIZE =
      sizeof(aDataBuffer) / sizeof(*aDataBuffer);
#define firstPartSize FU_BLOCK_SIZE // size of first data frame
#undef FU_VERSION_LOC
#define FU_VERSION_LOC (firstPartSize + 2)
  DW_ASSERT(BUFFER_SIZE == 39);

  // make test
  FU_PrepareCRC();
  FU_FeedCRC(aDataBuffer, 2);
  FU_FeedCRC(aDataBuffer + 2, 5);
  volatile uint32_t r1 = LL_CRC_ReadData32(CRC);
  FU_PrepareCRC();
  FU_FeedCRC(aDataBuffer, 7);
  volatile uint32_t r2 = LL_CRC_ReadData32(CRC);
  UNUSED(r1);
  UNUSED(r2);
  DW_ASSERT(r1 == r2);

  FU_PrepareCRC();
  FU_FeedCRC(aDataBuffer, BUFFER_SIZE);
  r1 = LL_CRC_ReadData32(CRC);
  DW_ASSERT(r1 == 0xB4B0);

  // sprawdz wysylanie bledy
  // SOT
  test_fup->FC = MAC_FC_FIRMWARE_UPGRADE;
  test_fup->opcode = FU_MakeOpcode(FU_OPCODE_ACK); // bad CRC
  test_fup->frameLen = FU_PROT_HEAD_SIZE;
  FU_SendResponse(test_fup, FU_Test_Resp);
  test_fup->opcode = 0xff; // bad CRC
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(test_fup->extra == FU_ERR_BAD_FRAME_CRC);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ABORT));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2); // 2 for CRC
  DW_ASSERT(test_fup->hash == FU_GetLocalHash());

  // bad protocol version
  test_fup->opcode = FU_OPCODE_ACK;
  test_fup->frameLen =
      FU_PROT_HEAD_SIZE +
      2; // +2 for CRC, because we doeasn't use FU_SendResponse;
  FU_FillCRC(test_fup);
  FU_Test_Resp(test_fup);
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(test_fup->extra == FU_ERR_BAD_PROT_VER);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ABORT));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2); // 2 for CRC
  DW_ASSERT(test_fup->hash == FU_GetLocalHash());

  uint32_t test_data;
  /*test_sot->opcode = FU_MakeOpcode(FU_OPCODE_SOT); // bad version of firmware
  test_sot->frameLen = FU_PROT_HEAD_SIZE+7+2; // +2 for CRC, because we doeasn't
  use FU_SendResponse
  test_sot->fversion = FU_GetLocalHash()-1;
  test_sot->fileSize = 10;
  FU_FillCRC((FU_prot*)test_sot);
  FU_Test_Resp((FU_prot*)test_sot);
  FU_HandleAsDevice((FU_prot*)test_sot, FU_Test_Resp);
  DW_ASSERT(test_fup->extra == FU_ERR_BAD_VERSION);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ABORT));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE+2);  // 2 for CRC
  DW_ASSERT(test_fup->version == FU_GetLocalHash());*/

  test_sot->opcode = FU_MakeOpcode(FU_OPCODE_SOT); // too big file
  test_sot->frameLen = FU_PROT_HEAD_SIZE + 7 +
                       2; // +2 for CRC, because we doeasn't use FU_SendResponse
  test_sot->fversion = FU_GetLocalHash() + 1;
  test_sot->fileSize = FLASH_BANK_SIZE + 1;
  FU_FillCRC((FU_prot *)test_sot);
  FU_Test_Resp((FU_prot *)test_sot);
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(test_fup->extra == FU_ERR_BAD_FILE_SIZE);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ABORT));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2); // 2 for CRC
  DW_ASSERT(test_fup->hash == FU_GetLocalHash());

  test_sot->opcode = FU_MakeOpcode(FU_OPCODE_SOT); // empty file
  test_sot->frameLen = FU_PROT_HEAD_SIZE + 7 +
                       2; // +2 for CRC, because we doeasn't use FU_SendResponse
  test_sot->fversion = FU_GetLocalHash() + 1;
  test_sot->fileSize = 0;
  FU_FillCRC((FU_prot *)test_sot);
  FU_Test_Resp((FU_prot *)test_sot);
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(test_fup->extra == FU_ERR_BAD_FILE_SIZE);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ABORT));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2); // 2 for CRC
  DW_ASSERT(test_fup->hash == FU_GetLocalHash());

  // success transmission
  FU_PrepareCRC();
  FU_FeedCRC(aDataBuffer, BUFFER_SIZE);
  test_sot->opcode = FU_MakeOpcode(FU_OPCODE_SOT); // ok
  test_sot->frameLen = FU_PROT_HEAD_SIZE + 7 +
                       2; // +2 for CRC, because we doeasn't use FU_SendResponse
  test_sot->fversion = FU_GetLocalHash() + 1;
  test_sot->firmwareCRC = LL_CRC_ReadData16(CRC);
  test_sot->fileSize = BUFFER_SIZE;
  FU_FillCRC((FU_prot *)test_sot);
  FU_Test_Resp((FU_prot *)test_sot);
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ACK));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2); // 2 for CRC
  DW_ASSERT(test_fup->hash == FU_GetLocalHash());

  // check file CRC
  FU_PrepareCRC();
  FU_FeedCRC(aDataBuffer, BUFFER_SIZE);
  DW_ASSERT(FU_NEW_CRC == LL_CRC_ReadData16(CRC));

  test_fup->opcode = FU_MakeOpcode(FU_OPCODE_DATA); // bad version
  test_fup->frameLen = FU_PROT_HEAD_SIZE + 4 +
                       2; // +2 for CRC, because we doeasn't use FU_SendResponse
  test_fup->hash = FU_GetLocalHash() + 2;
  test_fup->extra = 0;
  test_data = 0x12345678;
  memcpy(test_fup->data, &test_data, 4);
  FU_FillCRC(test_fup);
  FU_Test_Resp(test_fup);
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(test_fup->extra == FU_ERR_BAD_VERSION);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ABORT));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2); // 2 for CRC
  DW_ASSERT(test_fup->hash == FU_GetLocalHash());

  test_fup->opcode = FU_MakeOpcode(FU_OPCODE_DATA); // bad CRC
  test_fup->frameLen = FU_PROT_HEAD_SIZE + 4 +
                       2; // +2 for CRC, because we doeasn't use FU_SendResponse
  test_fup->hash = FU_GetLocalHash() + 1;
  test_fup->extra = 0;
  memcpy(test_fup->data, &test_data, 4);
  test_data = 0;
  memcpy(test_fup->data + 4, &test_data, 4);
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(test_fup->extra == FU_ERR_BAD_FRAME_CRC);
  DW_ASSERT(FU_MakeOpcode(FU_OPCODE_ABORT));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2); // 2 for CRC
  DW_ASSERT(test_fup->hash == FU_GetLocalHash());

  // first data
  test_fup->opcode = FU_MakeOpcode(FU_OPCODE_DATA);
  test_fup->frameLen = FU_PROT_HEAD_SIZE + firstPartSize +
                       2; // +2 for CRC, because we doeasn't use FU_SendResponse
  test_fup->hash = FU_GetLocalHash() + 1;
  test_fup->extra = 0;
  memcpy(test_fup->data, aDataBuffer, firstPartSize);
  FU_FillCRC(test_fup);
  FU_Test_Resp(test_fup);
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ACK));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2); // 2 for CRC
  DW_ASSERT(test_fup->hash == FU_GetLocalHash());
  DW_ASSERT(
      memcmp((void *)FU_GetAddressToWrite(), aDataBuffer, firstPartSize) == 0);

  test_fup->opcode = FU_MakeOpcode(FU_OPCODE_DATA); // bad version
  test_fup->frameLen = FU_PROT_HEAD_SIZE + 4 +
                       2; // +2 for CRC, because we doeasn't use FU_SendResponse
  test_fup->hash = FU_GetLocalHash() + 2;
  test_fup->extra = 0;
  test_data = 0x12345678;
  memcpy(test_fup->data, &test_data, 4);
  memcpy(test_fup->data + 4, &test_data, 4);
  FU_FillCRC(test_fup);
  FU_Test_Resp(test_fup);
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(test_fup->extra == FU_ERR_BAD_VERSION);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ABORT));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2); // 2 for CRC
  DW_ASSERT(test_fup->hash == FU_GetLocalHash());

  // second - last data
  test_fup->opcode = FU_MakeOpcode(FU_OPCODE_DATA);
  test_fup->frameLen = FU_PROT_HEAD_SIZE + BUFFER_SIZE - firstPartSize +
                       2; // +2 for CRC, because we doeasn't use FU_SendResponse
  test_fup->hash = FU_GetLocalHash() + 1;
  test_fup->extra = firstPartSize / FU_BLOCK_SIZE;
  memcpy(test_fup->data, aDataBuffer + firstPartSize,
         BUFFER_SIZE - firstPartSize);
  FU_FillCRC(test_fup);
  FU_Test_Resp(test_fup);
  FU_HandleAsDevice(test_fup, FU_Test_Resp);
  DW_ASSERT(FU_IsOpcode(test_fup->opcode, FU_OPCODE_ACK));
  DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE + 2);
  // DW_ASSERT(test_fup->version == FU_GetLocalFVersion()+1);

  // check written memory
  DW_ASSERT(memcmp((void *)FU_GetAddressToWrite(), aDataBuffer, BUFFER_SIZE) ==
            0);
  // full data are now save

  // host test

  //	test_fup->opcode = -1; // bad flag
  //	FU_HandleAsHost(test_fup, FU_Test_Resp);
  //	DW_ASSERT(test_fup->extra == FU_ERR_BAD_OPCODE_SET);
  //	DW_ASSERT(test_fup->opcode == FU_OPCODE_ABORT);
  //	DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE+2);
  //	DW_ASSERT(test_fup->version == FU_GetLocalFVersion());
  //
  //	test_fup->opcode = FU_OPCODE_DATA; // bad flag
  //	FU_HandleAsHost(test_fup, FU_Test_Resp);
  //	DW_ASSERT(test_fup->extra == FU_ERR_BAD_OPCODE_SET);
  //	DW_ASSERT(test_fup->opcode == FU_OPCODE_ABORT);
  //	DW_ASSERT(test_fup->frameLen == FU_PROT_HEAD_SIZE+2);
  //	DW_ASSERT(test_fup->version == FU_GetLocalFVersion());
  return 1;
}
#endif
