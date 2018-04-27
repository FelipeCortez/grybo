#include <soundio/soundio.h>
#include <string>

struct AudioData {
  int startDelay;
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
  std::string songId;

  Audio(std::string songId);
  ~Audio();
  bool loadOgg();
};
