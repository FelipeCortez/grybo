#ifndef MIDI_H
#define MIDI_H

#include <iostream>
#include <midifile/MidiFile.h>
#include <vector>
#include <string>
#include <assert.h>

struct GameNote {
  float zPosition;
  int note;
};

struct TempoChange {
  float quarterNotesPerSecond;
  int ticks;
};

struct SecondsTempo {
  float seconds;
  float quarterNotesPerSecond;
  float pos;
};

std::vector<SecondsTempo> tempoTimeMap;

struct GameSong {
  int startDelay; // in ms
  std::vector<GameNote> gameNotes;
  std::vector<TempoChange> tempoChanges;
  unsigned int ticksPerQuarter;
};

void buildTimeMap(GameSong gameSong) {
  float currentSeconds = 0.0f;
  int i;

  if (gameSong.tempoChanges.size() == 1) {
    SecondsTempo secondsTempo;
    secondsTempo.seconds = 0.0f;
    secondsTempo.quarterNotesPerSecond = gameSong.tempoChanges[0].quarterNotesPerSecond;
    tempoTimeMap.push_back(secondsTempo);
  } else {
    for (i = 0; i < gameSong.tempoChanges.size() - 1; ++i) {
      int ticksBetweenChanges = (gameSong.tempoChanges[i + 1].ticks - gameSong.tempoChanges[i].ticks);
      float tempoQuarters =  ticksBetweenChanges / gameSong.ticksPerQuarter;
      float secondsWithTempo = tempoQuarters / gameSong.tempoChanges[i].quarterNotesPerSecond;

      SecondsTempo secondsTempo;
      secondsTempo.seconds = currentSeconds;
      secondsTempo.quarterNotesPerSecond = gameSong.tempoChanges[i].quarterNotesPerSecond;
      secondsTempo.pos = gameSong.tempoChanges[i].ticks / gameSong.ticksPerQuarter;
      tempoTimeMap.push_back(secondsTempo);

      currentSeconds += secondsWithTempo;
    }

    SecondsTempo secondsTempo;
    secondsTempo.seconds = currentSeconds;
    secondsTempo.quarterNotesPerSecond = gameSong.tempoChanges[i].quarterNotesPerSecond;
    secondsTempo.pos = gameSong.tempoChanges[i].ticks / gameSong.ticksPerQuarter;
    tempoTimeMap.push_back(secondsTempo);

  }
}

float msToPos(float ms, GameSong gameSong, bool debug = false) {
  float s = ms / 1000.0f;

  if (tempoTimeMap.size() == 0) {
    buildTimeMap(gameSong);
  }

  if (gameSong.tempoChanges.size() == 1) {
      return gameSong.tempoChanges[0].quarterNotesPerSecond * s;
  } else {
    int i;
    for (i = 0; i < tempoTimeMap.size() - 1; ++i) {
      if (s >= tempoTimeMap[i].seconds && s < tempoTimeMap[i + 1].seconds) {
        return tempoTimeMap[i].pos +
          ((s - tempoTimeMap[i].seconds) * tempoTimeMap[i].quarterNotesPerSecond);
      }
    }

    return tempoTimeMap[tempoTimeMap.size() - 1].pos +
      ((s - tempoTimeMap[tempoTimeMap.size() - 1].seconds) * tempoTimeMap[tempoTimeMap.size() - 1].quarterNotesPerSecond);
  }
}

GameSong getSongFromMidiFile(std::string midiFile) {
  // Reads events from a MIDI file and convert them to game notes and tempo changes

  MidiFile midifile(midiFile); // naming things is hard

  std::vector<GameNote> gameNotes;
  std::vector<TempoChange> tempoChanges;

  int startDelay = 0;
  int tracks = midifile.getTrackCount();
  int ticksPerQuarter = midifile.getTicksPerQuarterNote();
  cout << "TPQ" << ticksPerQuarter << endl;

  if (tracks > 1) {
    //cout << "TRACKS: " << tracks << endl;
  }

  for (int track = 0; track < tracks; ++track) {
    if (tracks > 1) {
      //cout << "\nTrack " << track << endl;
    }

    for (int event = 0; event < midifile[track].size(); ++event) {
      if (midifile[track][event].isTempo()) {
        TempoChange tempoChange;
        tempoChange.quarterNotesPerSecond = (float) midifile[track][event].getTempoBPM() / 60;
        tempoChange.ticks = midifile[track][event].tick;
        tempoChanges.push_back(tempoChange);
      } else if (midifile[track][event].isNoteOn()) {
        //cout << (64 - midifile[track][event][1]) << endl;
        //assert(64 - midifile[track][event][1] >= 0 &&
        //       64 - midifile[track][event][1] <= 4);

        // start: 65
        // G: 64
        // R: 63
        // Y: 62
        // B: 61
        // O: 60
        if (midifile[track][event][1] == 65) {
          cout << "start delay: ";
          cout << midifile[track][event].tick << endl;
          startDelay = midifile[track][event].tick;
        }

        GameNote note;
        note.zPosition = midifile[track][event].tick / (float) ticksPerQuarter;
        note.note = 64 - midifile[track][event][1];
        gameNotes.push_back(note);
      }
    }
  }

  GameSong gameSong;
  gameSong.gameNotes = gameNotes;
  gameSong.tempoChanges = tempoChanges;
  gameSong.ticksPerQuarter = ticksPerQuarter;
  gameSong.startDelay = startDelay;

  return gameSong;
}

#endif
