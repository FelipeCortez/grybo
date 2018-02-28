#include <iostream>
#include <midifile/MidiFile.h>
#include <vector>

struct GameNote {
  float measure;
  unsigned int note;
  float seconds;
};

std::vector<GameNote> notes;

int main() {
  MidiFile midifile("test.mid");

  int tracks = midifile.getTrackCount();
  cout << "TPQ: " << midifile.getTicksPerQuarterNote() << endl;
  int tpq = midifile.getTicksPerQuarterNote();
  int tpm = tpq * 4;

  if (tracks > 1) {
    cout << "TRACKS: " << tracks << endl;
  }
  for (int track=0; track < tracks; track++) {
    if (tracks > 1) {
      //cout << "\nTrack " << track << endl;
    }
    for (int event=0; event < midifile[track].size(); event++) {
      if (midifile[track][event].isTempo()) {
        //cout << "tempo:\t" << midifile[track][event].getTempoBPM() << endl;
      } else if (midifile[track][event].isNoteOn()) {
        /*
        cout << "noteon:\t" << midifile[track][event].getCommandByte() << "|\t"
             << (uint32_t)midifile[track][event][1] << "|\t"
             << midifile[track][event].tick << "|\t"
             << midifile[track][event].tick / (float)tpm << "|\t"
             << midifile.getTimeInSeconds(midifile[track][event].tick) << "|\t|"
             << endl;
        */
        GameNote newNote {
            midifile[track][event].tick / (float) tpm,
            (uint32_t) midifile[track][event][1],
            (float) midifile.getTimeInSeconds(midifile[track][event].tick)
        };
        cout << newNote.measure << "|" << newNote.note << endl;
        notes.push_back(newNote);
      }
    }
   }

  return 0;
}
