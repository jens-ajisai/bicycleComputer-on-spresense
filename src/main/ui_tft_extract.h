#include <arch/board/board.h>

#include "/Users/jens/work/mcu_prj/prj/spresense/references/spresense-sdk/spresense/nuttx/arch/arm/src/cxd56xx/cxd56_spi.h"
#include "SPI.h"
#include "TFT_Drivers/ST7789_Defines.h"

#define TFT_WIDTH 240
#define TFT_HEIGHT 240

#define TFT_CS 32
#define TFT_RST 5
#define TFT_DC 9

#define SPI_FREQUENCY 30000000
#define TFT_SPI_MODE SPI_MODE3

static void initDMA() {
#if defined(CONFIG_CXD56_DMAC)
  DMA_HANDLE hdl;
  dma_config_t conf;

  if (DEBUG_UITFT) Log.traceln("DISPLAY_SPI IS %d", DISPLAY_SPI);
  if (DEBUG_UITFT) Log.traceln("DISPLAY_DMA_TXCH IS %d", DISPLAY_DMA_TXCH);
  if (DEBUG_UITFT) Log.traceln("DISPLAY_DMA_TX_MAXSIZE IS %d", DISPLAY_DMA_TX_MAXSIZE);
  if (DEBUG_UITFT) Log.traceln("DISPLAY_DMA_TXCH_CFG IS %d", DISPLAY_DMA_TXCH_CFG);
  if (DEBUG_UITFT) Log.traceln("CXD56_DMAC_WIDTH8 IS %d", CXD56_DMAC_WIDTH8);
  if (DEBUG_UITFT) Log.traceln("CXD56_DMAC_WIDTH8 IS %d", CXD56_SPI_DMAC_CHTYPE_TX);

  hdl = cxd56_dmachannel(DISPLAY_DMA_TXCH, DISPLAY_DMA_TX_MAXSIZE);
  if (hdl) {
    conf.channel_cfg = DISPLAY_DMA_TXCH_CFG;
    conf.dest_width = CXD56_DMAC_WIDTH8;
    conf.src_width = CXD56_DMAC_WIDTH8;
    cxd56_spi_dmaconfig(DISPLAY_SPI, CXD56_SPI_DMAC_CHTYPE_TX, hdl, &conf);
  }

  hdl = cxd56_dmachannel(DISPLAY_DMA_RXCH, DISPLAY_DMA_RX_MAXSIZE);
  if (hdl) {
    conf.channel_cfg = DISPLAY_DMA_RXCH_CFG;
    conf.dest_width = CXD56_DMAC_WIDTH8;
    conf.src_width = CXD56_DMAC_WIDTH8;
    cxd56_spi_dmaconfig(DISPLAY_SPI, CXD56_SPI_DMAC_CHTYPE_RX, hdl, &conf);
  }
#endif
}

static void begin_tft_write() {
  SPI5.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, TFT_SPI_MODE));
  digitalWrite(TFT_CS, LOW);
}

static void end_tft_write() {
  digitalWrite(TFT_CS, HIGH);
  SPI5.endTransaction();
}

void writecommand(uint8_t c) {
  begin_tft_write();
  digitalWrite(TFT_DC, LOW);
  SPI5.transfer(c);
  digitalWrite(TFT_DC, HIGH);
  end_tft_write();
}

void writedata(uint8_t d) {
  begin_tft_write();
  digitalWrite(TFT_DC, HIGH);
  SPI5.transfer(d);
  digitalWrite(TFT_DC, LOW);
  end_tft_write();
}

static void setPinModes() {
  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);  // Chip select high (inactive)

  pinMode(TFT_DC, OUTPUT);
  digitalWrite(TFT_DC, HIGH);  // Data/Command high = data mode

  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST,
               HIGH);  // Set high, do not share pin with another SPI device
}

static void resetTft() {
  digitalWrite(TFT_RST, HIGH);
  delay(5);
  digitalWrite(TFT_RST, LOW);
  delay(20);
  digitalWrite(TFT_RST, HIGH);
}

int colstart = 0;
int rowstart = 0;
int _init_width = TFT_WIDTH;
int _init_height = TFT_HEIGHT;
int _width;
int _height;

static void setRotation(int m) {
  begin_tft_write();
  int rotation;
#include "TFT_Drivers/ST7789_Rotation.h"
  delayMicroseconds(10);
  end_tft_write();
}

static void initTftDriver() {
  setPinModes();

  SPI5.begin();  // This will set HMISO to input

  resetTft();
  delay(150);  // Wait for reset to complete

#include "TFT_Drivers/ST7789_Init.h"

  end_tft_write();

  setRotation(0);
}

void setAddrWindow(int32_t x0, int32_t y0, int32_t w, int32_t h) {
  begin_tft_write();
  int32_t x1 = x0 + w - 1;
  int32_t y1 = y0 + h - 1;

  x0 += colstart;
  x1 += colstart;
  y0 += rowstart;
  y1 += rowstart;
  printf("\n%d %d %d %d\n", x0, x1, y0, y1);

  uint8_t c;
  uint16_t d;

  digitalWrite(TFT_DC, LOW);
  c = TFT_CASET;
  SPI5.transfer(&c, 1);

  digitalWrite(TFT_DC, HIGH);
  d = (x0>>8) | (x0<<8);
  SPI5.transfer(&d, 2);
  d = (x1>>8) | (x1<<8);
  SPI5.transfer(&d, 2);
  digitalWrite(TFT_DC, LOW);

  c = TFT_PASET;
  SPI5.transfer(&c, 1);

  digitalWrite(TFT_DC, HIGH);
  d = (y0>>8) | (y0<<8);
  SPI5.transfer(&d, 2);
  d = (y1>>8) | (y1<<8);
  SPI5.transfer(&d, 2);
  digitalWrite(TFT_DC, LOW);

  c = TFT_RAMWR;
  SPI5.transfer(&c, 1);
  digitalWrite(TFT_DC, HIGH);

  end_tft_write();
}

void pushPixels(void* data_in, size_t len) {
  SPI5.transfer(data_in, len);
}
