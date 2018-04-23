#include <soundio/soundio.h>

struct AudioData {
  int pos;
  int len;
  int channels;
  short* stream;

  AudioData();
};

struct Audio {
  SoundIo* soundio;
  SoundIoDevice* device;
  SoundIoOutStream* outstream;
  AudioData* audioData;

  Audio();
  ~Audio();
  bool loadOgg();
};
