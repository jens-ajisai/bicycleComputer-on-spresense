#pragma once

typedef void (*ProcessOutputCb)(uint8_t *buf, uint32_t size);

int initialize_recorder(uint32_t target_samplingrate, uint32_t target_channel_number,
                        uint32_t target_bit_length, const uint32_t default_mic_gain);
int deinit_recorder(void);

bool start_recorder(void);
void process_recorder(uint32_t rec_time, ProcessOutputCb out_cb);
bool stop_recorder(ProcessOutputCb out_cb);
