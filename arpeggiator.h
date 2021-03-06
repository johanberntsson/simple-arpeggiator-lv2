/*
   SimpleArpeggiator LV2 Plugin
   Copyright 2017 Johan Berntsson

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
   */

enum chordtype {
    OCTAVE = 0,
    MAJOR = 1,
    MINOR = 2,
    CHORD_ERROR
};

enum timetype {
    NOTE_1_1 = 0,
    NOTE_1_2 = 1,
    NOTE_1_4 = 2,
    NOTE_1_8 = 3,
    NOTE_1_16 = 4,
    NOTE_1_32 = 5,
    NOTE_ERROR
};

enum dirtype {
    DIR_UP = 0,
    DIR_DOWN = 1,
    DIR_UPDOWN = 2,
    DIR_ERROR
};

float getGate();

int setChord(enum chordtype chord);
int setRange(int range);
int setTime(enum timetype time);
int setGate(float gate);
int setCycle(int range);
int setSkip(float gate);
int setDir(enum dirtype time);


void resetArpeggio();
void updateArpeggioNotes();
uint8_t nextNote(uint8_t base_note);

float note_as_fraction_of_bar(int beat_unit, int beats_per_bar);


