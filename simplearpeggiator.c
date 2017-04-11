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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <sndfile.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"

#include "arpeggiator.h"

#define SIMPLEARPEGGIATOR_URI            \
    "https://github.com/johanberntsson/simple-arpeggiator-lv2"

/* has to correspond to port index numbers in simplearpeggiator.ttl */
enum {
	SIMPLEARPEGGIATOR_IN  = 0,
	SIMPLEARPEGGIATOR_OUT = 1,
	SIMPLEARPEGGIATOR_CHORD = 2,
	SIMPLEARPEGGIATOR_RANGE = 3,
	SIMPLEARPEGGIATOR_TIME = 4,
	SIMPLEARPEGGIATOR_GATE = 5
};

typedef struct {
    // Data types for communication with host
	LV2_URID atom_Blank;
	LV2_URID atom_Int;
	LV2_URID atom_Float;
	LV2_URID atom_Object;
	LV2_URID atom_Path;
	LV2_URID atom_Resource;
	LV2_URID atom_Sequence;
	LV2_URID atom_URID;
	LV2_URID atom_eventTransfer;
	// Midi parameters
	LV2_URID midi_Event;
	// Time parameters
    LV2_URID time_Position;
    LV2_URID time_beatsPerBar; // top number in a time signature, usually 4 (for 4/4)
    LV2_URID time_beatUnit; // bottom number in a time signature, usually 4 (for 4/4)
    LV2_URID time_barBeat; // The beat number within the bar, from 0 to beatsPerBar
    LV2_URID time_beatsPerMinute; // Tempo in beats per minute.
    LV2_URID time_speed; // fraction of normal speed. 0.0 is stopped, 1.0 is normal speed
} SimpleArpeggiatorURIs;

typedef struct {
    LV2_Atom_Event event;
    uint8_t        msg[3];
} MIDINoteEvent;

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

typedef struct {
	// Features
	LV2_URID_Map*            map;
    LV2_Log_Log*             log;

	// Ports
	const LV2_Atom_Sequence* in_port;
	LV2_Atom_Sequence*       out_port;
	float*                   chord_ptr;
	float*                   range_ptr; /* 1 - 9 octaves */
	float*                   time_ptr;
	float*                   gate_ptr; /* 0 - 100 % */

	// parameters (read from ports)
	enum chordtype           chord;
	int                      range;
	enum timetype            time;
	float                    gate;

    // Variables to keep track of the tempo information sent by the host
    double                   rate;   // Sample rate
    float                    bpm;    // Beats per minute (tempo)
    float                    speed;  // Transport speed (usually 0=stop, 1=play)
    uint32_t                 beat_unit;  // bottom number in a time signature
    uint32_t                 beats_per_bar;  // top number in a time signature
    uint32_t                 frames_per_beat; // number of frames in one beat
    uint32_t                 elapsed_frames;  // Frames since the start of the last click


    // arpeggio info
    uint8_t                  base_note; // base note of the current arpeggio
    uint32_t                 arpeggio_index; 
    uint32_t                 arpeggio_length; // number of arpeggio notes
    MIDINoteEvent            arpeggiator_note;
    uint32_t                 arpeggiator_note_last_frame;
    uint8_t                  arpeggio_notes[10*3];  // max octaves * max notes/octave

    // Logger convenience API
    LV2_Log_Logger           logger;

	// URIs
	SimpleArpeggiatorURIs    uris;
} SimpleArpeggiator;

static void connect_port(
        LV2_Handle instance,
        uint32_t   port,
        void*      data)
{
    FILE *f;
	SimpleArpeggiator* self = (SimpleArpeggiator*)instance;
	switch (port) {
	case SIMPLEARPEGGIATOR_IN:
		self->in_port = (const LV2_Atom_Sequence*)data;
		break;
	case SIMPLEARPEGGIATOR_OUT:
		self->out_port = (LV2_Atom_Sequence*)data;
		break;
	case SIMPLEARPEGGIATOR_CHORD:
		self->chord_ptr = (float *)data;
		break;
	case SIMPLEARPEGGIATOR_RANGE:
		self->range_ptr = (float *)data;
		break;
	case SIMPLEARPEGGIATOR_TIME:
		self->time_ptr = (float *) data;
		break;
	case SIMPLEARPEGGIATOR_GATE:
		self->gate_ptr = (float*)data;
		break;
	default:
		break;
	}
}

static void updateParameters(SimpleArpeggiator* self)
{
    bool updateArpeggiato = false;

    if(self->range != (int) *self->range_ptr) {
        self->range = (int) *self->range_ptr;
        updateArpeggiato = true;
        lv2_log_error(&self->logger, "new range %d\n", self->range);
    }
    if(self->chord != (enum chordtype) *self->chord_ptr) {
        self->chord = (enum chordtype) *self->chord_ptr;
        updateArpeggiato = true;
        lv2_log_error(&self->logger, "new chord %d\n", self->chord);
    }
    if(self->time != (enum timetype) *self->time_ptr) {
        self->time = (enum timetype) *self->time_ptr;
        updateArpeggiato = true;
        lv2_log_error(&self->logger, "new time %d\n", self->time);
    }
    if(self->gate != *self->gate_ptr) {
        self->gate = *self->gate_ptr;
        updateArpeggiato = true;
        lv2_log_error(&self->logger, "new gate %f\n", self->gate);
    }

    if(updateArpeggiato) {
        lv2_log_error(&self->logger, "updating arpeggio\n");
        int i;
        switch(self->chord) {
            case OCTAVE:
                for(i = 0; i < self->range; i++) {
                    self->arpeggio_notes[i] = 12 * i;
                }
                //lv2_log_error(&self->logger, "%d %d %d\n", i, self->arpeggio_notes[0], self->arpeggio_notes[1]);
                self->arpeggio_length = i;
                break;
            case MAJOR:
                for(i = 0; i < self->range; i++) {
                    self->arpeggio_notes[3 * i + 0] = 12 * i;
                    self->arpeggio_notes[3 * i + 1] = 12 * i + 4;
                    self->arpeggio_notes[3 * i + 2] = 12 * i + 3;
                }
                self->arpeggio_length = 3 * i;
                break;
            case MINOR:
                for(i = 0; i < self->range; i++) {
                    self->arpeggio_notes[3 * i + 0] = 12 * i;
                    self->arpeggio_notes[3 * i + 1] = 12 * i + 3;
                    self->arpeggio_notes[3 * i + 2] = 12 * i + 4;
                }
                self->arpeggio_length = 3 * i;
                break;
        }
    }
}
/**
 *    The activate() method resets the state completely
 *       */
static void activate(LV2_Handle instance)
{
    SimpleArpeggiator* self = (SimpleArpeggiator*)instance;
    //fprintf(stderr, "activate\n");
    self->elapsed_frames = 0;
    self->base_note = 128;

    updateParameters(self);
}

static LV2_Handle instantiate(
        const LV2_Descriptor*     descriptor,
        double                    rate,
        const char*               path,
        const LV2_Feature* const* features)
{
	// Allocate and initialise instance structure.
	SimpleArpeggiator* self = (SimpleArpeggiator*)malloc(sizeof(SimpleArpeggiator));
	if (!self) {
		return NULL;
	}
	memset(self, 0, sizeof(SimpleArpeggiator));

	// Get host features
	for (int i = 0; features[i]; ++i) {
		if (!strcmp(features[i]->URI, LV2_URID__map)) {
			self->map = (LV2_URID_Map*)features[i]->data;
        } else if (!strcmp(features[i]->URI, LV2_LOG__log)) {
            self->log = (LV2_Log_Log*)features[i]->data;
		}
	}
	if (!self->map) {
		fprintf(stderr, "Missing feature urid:map\n");
		free(self);
		return NULL;
	}

	// Map URIs
    SimpleArpeggiatorURIs* const uris = &self->uris;
    LV2_URID_Map* const map  = self->map;
	uris->atom_Blank         = map->map(map->handle, LV2_ATOM__Blank);
    uris->atom_Int           = map->map(map->handle, LV2_ATOM__Int);
    uris->atom_Float         = map->map(map->handle, LV2_ATOM__Float);
    uris->atom_Object        = map->map(map->handle, LV2_ATOM__Object);
	uris->atom_Path          = map->map(map->handle, LV2_ATOM__Path);
	uris->atom_Resource      = map->map(map->handle, LV2_ATOM__Resource);
	uris->atom_Sequence      = map->map(map->handle, LV2_ATOM__Sequence);
	uris->atom_URID          = map->map(map->handle, LV2_ATOM__URID);
	uris->atom_eventTransfer = map->map(map->handle, LV2_ATOM__eventTransfer);
	uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);
    uris->time_Position      = map->map(map->handle, LV2_TIME__Position);
    uris->time_beatsPerBar   = map->map(map->handle, LV2_TIME__beatsPerBar);
    uris->time_beatUnit      = map->map(map->handle, LV2_TIME__beatUnit);
    uris->time_barBeat       = map->map(map->handle, LV2_TIME__barBeat);
    uris->time_beatsPerMinute= map->map(map->handle, LV2_TIME__beatsPerMinute);
    uris->time_speed         = map->map(map->handle, LV2_TIME__speed);

	// initialise forge/logger
    lv2_log_logger_init(&self->logger, self->map, self->log);

    // Initialise instance fields
    self->rate       = rate;
    self->bpm        = 120.0f; // default (will be updated later)
    self->frames_per_beat = 60.0f / self->bpm * self->rate;

    // setting parameter defaults to trigger updates in activate later()
    self->time = NOTE_ERROR;
    self->chord = CHORD_ERROR;

	return (LV2_Handle)self;
}

static void cleanup(LV2_Handle instance)
{
	free(instance);
}


static void update_time(
        SimpleArpeggiator* self,
        const LV2_Atom_Object* obj,
		const LV2_Atom_Event* ev)
{
	const SimpleArpeggiatorURIs* uris = &self->uris;

	// Received new transport position/speed
	LV2_Atom *beat = NULL, *bpm = NULL, *speed = NULL;
	LV2_Atom  *beatsperbar = NULL, *beatunit = NULL;
	lv2_atom_object_get(obj,
			uris->time_barBeat, &beat,
			uris->time_beatsPerMinute, &bpm,
			uris->time_speed, &speed,
			uris->time_beatsPerBar, &beatsperbar,
			uris->time_beatUnit, &beatunit,
			NULL);
	if (bpm && bpm->type == uris->atom_Float) {
		if(self->bpm != ((LV2_Atom_Float*) bpm)->body) {
            // Tempo changed, update BPM
            self->bpm = ((LV2_Atom_Float*) bpm)->body;
            self->frames_per_beat = 60.0f / self->bpm * self->rate;
            //lv2_log_error(&self->logger, "bpm %f\n", self->bpm);
        }
	}
	if (speed && speed->type == uris->atom_Float) {
        if(self->speed != ((LV2_Atom_Float*) speed)->body) {
            // Speed changed, e.g. 0 (stop) to 1 (play)
            self->speed = ((LV2_Atom_Float*) speed)->body;
            //lv2_log_error(&self->logger, "speed %f\n", self->speed);
            if(self->speed > 0) {
                // restarted
                self->arpeggio_index = 0;
            }
        }
	}
	if (beatsperbar && beatsperbar->type == uris->atom_Float) {
		if(self->beats_per_bar != (int32_t) ((LV2_Atom_Float*) beatsperbar)->body) {
            // Number of beats in a bar changed
            self->beats_per_bar = (int32_t) ((LV2_Atom_Float*) beatsperbar)->body;
            //lv2_log_error(&self->logger, "beats_per_bar %d\n", self->beats_per_bar);
        }
	}
	if (beatunit && beatunit->type == uris->atom_Int) {
		if(self->beat_unit != (int32_t) ((LV2_Atom_Int*) beatunit)->body) {
            // Number of beats in a bar changed
            self->beat_unit = (int32_t) ((LV2_Atom_Int*) beatunit)->body;
            //lv2_log_error(&self->logger, "beat_unit %d\n", self->beat_unit);
        }
	}
	if (beat && beat->type == uris->atom_Float) {
		// Received a beat position, synchronise
        const float bar_beats  = ((LV2_Atom_Float*)beat)->body; // eg. 2.031
        const float beat_beats = bar_beats - floorf(bar_beats); // 0.031
        if(bar_beats < 1) {
            // new bar
            updateParameters(self);
            //self->elapsed_frames = beat_beats * self->frames_per_beat; // already processed frames
            //lv2_log_error(&self->logger, "beat %f %d/%d %d\n", beat_beats, self->beats_per_bar, self->beat_unit, self->elapsed_frames);
        }
	}

}

static float calculateArpeggiatorStep(enum timetype type, int beat_unit, int beats_per_bar)
{
    float note_length[] = { 1, 2, 4, 8, 16, 32 };
    return beats_per_bar / (note_length[type] * beat_unit);
}

static void update_arp(
		SimpleArpeggiator*    self,
		uint32_t              begin,
		uint32_t              end,
		const uint32_t        out_capacity)
{
    if(self->speed < 1.0) return;

    float step_ratio = calculateArpeggiatorStep((enum timetype) *self->time_ptr, self->beat_unit, self->beats_per_bar);
    uint32_t step_in_frames = (self->frames_per_beat * self->beats_per_bar) * step_ratio;

    for (uint32_t i = begin; i < end; ++i) {
        if(self->elapsed_frames == self->arpeggiator_note_last_frame) {
            self->arpeggiator_note.msg[0] = 0x80;
            lv2_atom_sequence_append_event(
                    self->out_port, out_capacity, &self->arpeggiator_note.event);
        }
        if(self->elapsed_frames % step_in_frames == 0) {
            if(self->base_note < 128) {
                if(self->arpeggio_index >= self->arpeggio_length) {
                    self->arpeggio_index = 0;
                }


				self->arpeggiator_note.event.time.frames = 0;
				self->arpeggiator_note.event.body.type   = self->uris.midi_Event;
				self->arpeggiator_note.event.body.size   = 3;
				self->arpeggiator_note.msg[0] = 0x90;
				self->arpeggiator_note.msg[1] = self->base_note + 
				    self->arpeggio_notes[self->arpeggio_index % self->arpeggio_length];
				self->arpeggiator_note.msg[2] = 127;

				if(self->arpeggiator_note.msg[1] < 128) {
				    // calculate note off time
                    self->arpeggiator_note_last_frame = self->elapsed_frames +
                        (self->gate * step_in_frames) / 100;
				    // send the note to the midi bus
                    lv2_atom_sequence_append_event(
                            self->out_port, out_capacity, &self->arpeggiator_note.event);
                }


                ++self->arpeggio_index;
            }

        }
        ++self->elapsed_frames;
    }
}

static int update_midi(
		SimpleArpeggiator*    self,
		const uint8_t* const  msg,
        const uint32_t out_capacity
		)
{
    // return 0 if consumed by this filter
    //lv2_log_error(&self->logger, "midi command %x %d %d\n", msg[0], msg[1], msg[2]);

	switch (lv2_midi_message_type(msg)) {
		case LV2_MIDI_MSG_NOTE_ON:
		    // only allow one note at a time
		    if(self->base_note == 128) {
                //lv2_log_error(&self->logger, "note on %d\n", msg[1]);
		        self->base_note = msg[1];
            }
            return 0;
		case LV2_MIDI_MSG_NOTE_OFF:
		    if(self->base_note == msg[1]) {
                //lv2_log_error(&self->logger, "note off %d\n", msg[1]);
		        self->base_note = 128;
            }
            return 0;
		default:
			// Forward all other MIDI events directly
			return 1;
	}
	return 1;
}

static void run(LV2_Handle instance, uint32_t   sample_count)
{
	SimpleArpeggiator*     self = (SimpleArpeggiator*)instance;
	SimpleArpeggiatorURIs* uris = &self->uris;

	// Initially self->out_port contains a Chunk with size set to capacity
	// Get the capacity
	const uint32_t out_capacity = self->out_port->atom.size;
	// Write an empty Sequence header to the output
	lv2_atom_sequence_clear(self->out_port);
	self->out_port->atom.type = self->in_port->atom.type;

    uint32_t last_t = 0; // range [0,sample_count]

	// Read incoming events
    LV2_ATOM_SEQUENCE_FOREACH(self->in_port, ev) {
        //lv2_log_error(&self->logger, "event %d\n", ev->body.type);
        if (ev->body.type == uris->atom_Object ||
                ev->body.type == uris->atom_Blank) {
            const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
            if (obj->body.otype == uris->time_Position) {
                // Received position information (bar/beat/bpm changes)
                update_time(self, obj, ev);
            }
        } else if (ev->body.type == uris->midi_Event) {
			const uint8_t* const msg = (const uint8_t*)(ev + 1);
			if(update_midi(self, msg, out_capacity)) {
			    // check if midi note_on or note_off
                lv2_atom_sequence_append_event(
                        self->out_port, out_capacity, ev);
            }
		}

        // update for this iteration
		update_arp(self, last_t, ev->time.frames, out_capacity);
		last_t = ev->time.frames;
    }

    // update for the remainder of the cycle
    update_arp(self, last_t, sample_count, out_capacity);
}

static const void* extension_data(const char* uri)
{
	return NULL;
}

static const LV2_Descriptor descriptor = {
	SIMPLEARPEGGIATOR_URI,
	instantiate,
	connect_port,
	activate,
	run,
	NULL,  // deactivate,
	cleanup,
	extension_data
};

LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}

