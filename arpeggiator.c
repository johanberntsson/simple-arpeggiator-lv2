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
#include <stdio.h>
#include <stdint.h>
#include "arpeggiator.h"

typedef struct {
    enum chordtype   chord;
    int              range;
    enum timetype    time;
    float            gate;
    int              cycle;
    float            skip;
    enum dirtype     dir;

    uint32_t         note_index; 
    uint32_t         arpeggio_length; // number of arpeggio notes
    uint8_t          arpeggio_notes[10*3];  // max octaves * max notes/octave
} Arpeggiator;

Arpeggiator arp_state;

float getGate() {
    return arp_state.gate;
}

/* Setters: return 0 if no change, -1 if new value set */
int setChord(enum chordtype chord) {
    if(arp_state.chord != chord) {
        arp_state.chord = chord;
        return -1;
    }
    return 0;
}

int setRange(int range) {
    if(arp_state.range != range) {
        arp_state.range = range;
        return -1;
    }
    return 0;
}

int setTime(enum timetype time) {
    if(arp_state.time != time) {
        arp_state.time = time;
        return -1;
    }
    return 0;
}

int setGate(float gate) {
    if(arp_state.gate != gate) {
        arp_state.gate = gate;
        return -1;
    }
    return 0;
}

int setCycle(int cycle) {
    if(arp_state.cycle != cycle) {
        arp_state.cycle = cycle;
        return -1;
    }
    return 0;
}

int setSkip(float skip) {
    if(arp_state.skip != skip) {
        arp_state.skip = skip;
        return -1;
    }
    return 0;
}

int setDir(enum dirtype dir) {
    if(arp_state.dir != dir) {
        arp_state.dir = dir;
        return -1;
    }
    return 0;
}

void updateArpeggioNotes() {
    int i;
    switch(arp_state.chord) {
        case OCTAVE:
            for(i = 0; i < arp_state.range; i++) {
                arp_state.arpeggio_notes[i] = 12 * i;
            }
            //lv2_log_error(&self.logger, "%d %d %d\n", i, self.arpeggio_notes[0], self.arpeggio_notes[1]);
            arp_state.arpeggio_length = i;
            break;
        case MAJOR:
            for(i = 0; i < arp_state.range; i++) {
                arp_state.arpeggio_notes[3 * i + 0] = 12 * i;
                arp_state.arpeggio_notes[3 * i + 1] = 12 * i + 4;
                arp_state.arpeggio_notes[3 * i + 2] = 12 * i + 3;
            }
            arp_state.arpeggio_length = 3 * i;
            break;
        case MINOR:
            for(i = 0; i < arp_state.range; i++) {
                arp_state.arpeggio_notes[3 * i + 0] = 12 * i;
                arp_state.arpeggio_notes[3 * i + 1] = 12 * i + 3;
                arp_state.arpeggio_notes[3 * i + 2] = 12 * i + 4;
            }
            arp_state.arpeggio_length = 3 * i;
            break;
    }
    resetArpeggio();
}

void resetArpeggio() {
    arp_state.note_index = 0;
}

uint8_t nextNote(uint8_t base_note) {
    /*
    if(arp_state.note_index >= arp_state.arpeggio_length) {
        arp_state.note_index = 0;
    }
    */

    uint8_t note =  base_note + arp_state.arpeggio_notes[
        arp_state.note_index % arp_state.arpeggio_length];
    
    ++arp_state.note_index;
    return note;
}

float note_as_fraction_of_bar(int beat_unit, int beats_per_bar) {
    // return the arpeggiator step as a fraction of a bar
    float note_length[] = { 1, 2, 4, 8, 16, 32 };
    return beats_per_bar / (note_length[arp_state.time] * beat_unit);
}


