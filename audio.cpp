#include "audio.h"
#include "stb_vorbis.h"
#include <climits>
#include <stdio.h>
#include <stdlib.h>

#define SAMPLE_RATE 44100

void callback(struct SoundIoOutStream *outstream, int frame_count_min, int frame_count_max) {
  const struct SoundIoChannelLayout* layout = &outstream->layout;
  struct SoundIoChannelArea* areas;
  int frames_left = frame_count_max;
  int err;
  while (frames_left > 0) {
    int frame_count = frames_left;

    if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count))) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(1);
    }

    if (!frame_count)
      break;

    AudioData* audioData;

    if (outstream->userdata != NULL) {
      audioData = (AudioData*) outstream->userdata;
    } else {
      audioData = nullptr;
    }

    float sample = 0;
    for (int frame = 0; frame < frame_count; frame += 1) {
      for (int channel = 0; channel < layout->channel_count; channel += 1) {
        float *ptr = (float*) (areas[channel].ptr + areas[channel].step * frame);

        if (audioData != nullptr && audioData->pos < audioData->len) {
          sample = (float) audioData->stream[audioData->pos + (channel % audioData->channels)] / SHRT_MAX;

        } else {
          sample = 0.0f;
        }

        *ptr = sample;
      }

      audioData->pos += audioData->channels;
    }

    if ((err = soundio_outstream_end_write(outstream))) {
      fprintf(stderr, "%s\n", soundio_strerror(err));
      exit(1);
    }

    frames_left -= frame_count;
  }
}

Audio::Audio() {
  int err;

  audioData = new AudioData();

  // soundio
  soundio = soundio_create();

  if (!soundio) {
    fprintf(stderr, "out of memory\n");
  }

  if ((err = soundio_connect(soundio))) {
    fprintf(stderr, "error connecting: %s", soundio_strerror(err));
  }

  soundio_flush_events(soundio);

  int default_out_device_index = soundio_default_output_device_index(soundio);
  if (default_out_device_index < 0) {
    fprintf(stderr, "no output device found");
  }

  // device
  device = soundio_get_output_device(soundio, default_out_device_index);
  if (!device) {
    fprintf(stderr, "out of memory");
  }

  // outstream
  outstream = soundio_outstream_create(device);
  outstream->format = SoundIoFormatFloat32NE;
  outstream->sample_rate = SAMPLE_RATE;
  outstream->write_callback = callback;
  outstream->userdata = audioData;

  if ((err = soundio_outstream_open(outstream))) {
    fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
  }

  if (outstream->layout_error)
    fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

  if ((err = soundio_outstream_start(outstream))) {
    fprintf(stderr, "unable to start device: %s", soundio_strerror(err));
  }

  fprintf(stderr, "Output device: %s\n", device->name);

  loadOgg();
}

bool Audio::loadOgg() {
  short *song;
  int channels, len, sampleRate;
  len = stb_vorbis_decode_filename("assets/ovo.ogg", &channels, &sampleRate, &song);

  if (len < 0) {
    return false;
  }

  audioData->pos = 0;
  audioData->len = len;
  audioData->stream = song;
  audioData->channels = channels;

  return true;
}

Audio::~Audio() {
  soundio_outstream_destroy(outstream);
  soundio_device_unref(device);
  soundio_destroy(soundio);
  free(audioData->stream);
}


AudioData::AudioData() {
  pos = 0;
  len = -1;
  channels = 0;
  stream = nullptr;
}
