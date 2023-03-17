#include "audio_ogg.h"
#include "oggz/oggz.h"
#include "my_log.h"

#define WRITE_CHUNK_SIZE 256

ogg_int64_t packetno = 0;
long serialno = -1;

OGGZ *oggz;

#define DISABLE_SLOWS_DOWN_ENCODING 0

struct __attribute__((packed)) OpusOggHeader {
  char signature[8] = {'O', 'p', 'u', 's', 'H', 'e', 'a', 'd'};
  uint8_t version = 1;
  uint8_t channelCount = 1;
  uint16_t preSkip = 0;
  uint32_t sampleRate = 48000;
  int16_t outputGain = 0;
  uint8_t channelMappingFamily = 0;
};

/// Simplified header w/o comments
struct __attribute__((packed)) OpusOggCommentHeader {
  char signature[8] = {'O', 'p', 'u', 's', 'T', 'a', 'g', 's'};
  uint32_t vendorStringLength = 3;
  char vendor[8] = "ABC";
  uint32_t userCommentListLength = 0;
};

void ogg_begin(OggOutputCb out_cb) {
  if (DISABLE_SLOWS_DOWN_ENCODING) Log.traceln("ogg_begin\n");

  oggz = oggz_new(OGGZ_WRITE | OGGZ_NONSTRICT);
  oggz_io_set_write(oggz, out_cb, NULL);

  serialno = oggz_serialno_new(oggz);
}

void ogg_writeHeaderPage(int recording_cannel_number, int recording_sampling_rate) {
  if (DISABLE_SLOWS_DOWN_ENCODING) Log.traceln("ogg_writeHeaderPage\n");

  ogg_packet op;

  // write header
  OpusOggHeader header;
  header.sampleRate = recording_sampling_rate;
  header.channelCount = recording_cannel_number;
  op.packet = (uint8_t *)&header;
  op.bytes = sizeof(header);
  op.granulepos = 0;
  op.packetno = packetno++;
  op.b_o_s = true;
  op.e_o_s = false;

  oggz_write_feed(oggz, &op, serialno, 0, NULL);
  while (int n = oggz_write(oggz, WRITE_CHUNK_SIZE)) {
    if (DISABLE_SLOWS_DOWN_ENCODING) Log.traceln("ogg_writeHeaderPage %d bytes written\n", n);
  };

  // write comment header
  OpusOggCommentHeader comment;
  op.packet = (uint8_t *)&comment;
  op.bytes = sizeof(comment);
  op.granulepos = 0;
  op.packetno = packetno++;
  op.b_o_s = false;
  op.e_o_s = false;

  oggz_write_feed(oggz, &op, serialno, OGGZ_FLUSH_AFTER, NULL);
  while (int n = oggz_write(oggz, WRITE_CHUNK_SIZE)) {
    if (DISABLE_SLOWS_DOWN_ENCODING) Log.traceln("ogg_writeHeaderPage %d bytes written\n", n);
  };
}

void ogg_writeAudioDataPage(uint8_t *data, size_t len, int pos) {
  if (DISABLE_SLOWS_DOWN_ENCODING) Log.traceln("ogg_writeAudioDataPage len %d granulepos %d \n", len, pos);
  ogg_packet op;

  // write audio data
  op.packet = data;
  op.bytes = len;
  op.granulepos = pos;
  op.packetno = packetno++;
  op.b_o_s = false;
  op.e_o_s = false;

  oggz_write_feed(oggz, &op, serialno, 0, NULL);
  while (int n = oggz_write(oggz, WRITE_CHUNK_SIZE)) {
    if (DISABLE_SLOWS_DOWN_ENCODING) Log.traceln("ogg_writeAudioDataPage %d bytes written\n", n);
  }
}

void ogg_writeFooterPage() {
  if (DISABLE_SLOWS_DOWN_ENCODING) Log.traceln("ogg_writeFooterPage\n");
  ogg_packet op;

  // write footer
  op.packet = (uint8_t *)nullptr;
  op.bytes = 0;
  op.granulepos = 0;
  op.packetno = packetno++;
  op.b_o_s = false;
  op.e_o_s = true;

  oggz_write_feed(oggz, &op, serialno, OGGZ_FLUSH_AFTER, NULL);
  while (int n = oggz_write(oggz, WRITE_CHUNK_SIZE)) {
    if (DISABLE_SLOWS_DOWN_ENCODING) Log.traceln("ogg_writeFooterPage %d bytes written\n", n);
  }
}

void ogg_end() {
  oggz_close(oggz);
}