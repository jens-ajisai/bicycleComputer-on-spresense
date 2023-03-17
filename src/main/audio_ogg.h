#pragma once

#include "oggz/oggz.h"
#include <sys/types.h>

typedef size_t (*OggOutputCb)(void *user_handle, void *buf, size_t n);


size_t ogg_writeFile_cb(void *user_handle, void *buf, size_t n);

void ogg_begin(OggOutputCb out_cb);
void ogg_writeHeaderPage(int recording_cannel_number, int recording_sampling_rate);
void ogg_writeAudioDataPage(uint8_t *data, size_t len, int pos);
void ogg_writeFooterPage();
void ogg_end();