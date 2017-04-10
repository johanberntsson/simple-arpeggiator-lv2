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
#    include <stdbool.h>
#endif

#include <sndfile.h>
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/log/logger.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/ext/log/log.h"
#include "arpeggiator.h"

#define SIMPLEARPEGGIATOR_URI          "https://github.com/johanberntsson/simple-arpeggiator-lv2"
#define EG_SIMPLEARPEGGIATOR_sample      SIMPLEARPEGGIATOR_URI "#sample"
#define EG_SIMPLEARPEGGIATOR_applySample SIMPLEARPEGGIATOR_URI "#applySample"
#define EG_SIMPLEARPEGGIATOR_freeSample  SIMPLEARPEGGIATOR_URI "#freeSample"

typedef struct {
	LV2_URID atom_Blank;
	LV2_URID atom_Int;
	LV2_URID atom_Float;
	LV2_URID atom_Object;
	LV2_URID atom_Path;
	LV2_URID atom_Resource;
	LV2_URID atom_Sequence;
	LV2_URID atom_URID;
	LV2_URID atom_eventTransfer;
	LV2_URID eg_applySample;
	LV2_URID eg_sample;
	LV2_URID eg_freeSample;
	LV2_URID midi_Event;
	LV2_URID patch_Set;
	LV2_URID patch_property;
	LV2_URID patch_value;
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

/* has to correspond to index port numbers in *.ttl */
enum {
	SIMPLEARPEGGIATOR_IN  = 0,
	SIMPLEARPEGGIATOR_OUT = 1,
	SIMPLEARPEGGIATOR_CHORD = 2,
	SIMPLEARPEGGIATOR_RANGE = 3,
	SIMPLEARPEGGIATOR_TIME = 4,
	SIMPLEARPEGGIATOR_GATE = 5
};

enum chordtype {
    OCTAVE = 0,
    MAJOR = 1,
    MINOR = 2
};

enum timetype {
    NOTE_WHOLE = 0,
    NOTE_1_2 = 1,
    NOTE_1_4 = 2,
    NOTE_1_8 = 3,
    NOTE_1_16 = 4,
    NOTE_1_32 = 5
};

typedef struct {
	// Features
	LV2_URID_Map*            map;
    LV2_Log_Log*             log;

	// Ports
	const LV2_Atom_Sequence* in_port;
	LV2_Atom_Sequence*       out_port;
	float *                  chord;
	float *                  range; /* 1 - 9 octaves */
	float *                  time;
	float *                  gate; /* 0 - 200 % */

    // Variables to keep track of the tempo information sent by the host
    double rate;   // Sample rate
    float  bpm;    // Beats per minute (tempo)
    float  speed;  // Transport speed (usually 0=stop, 1=play)
    float  beat_unit;  // the note value that counts as one beat
    float  beats_per_bar;  // 
    uint32_t frames_per_beat;  
    uint32_t beat_start_pos; // frame number when the last beat detected
    uint32_t elapsed_len;  // Frames since the start of the last click


    uint8_t base_note; // the note the current arpeggio is based on
    uint8_t arp_index; // current note in the arpeggio
    uint32_t current_start; // start time for current note
    uint32_t current_stop; // stop time for current note

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
		self->chord = (float *)data;
		break;
	case SIMPLEARPEGGIATOR_RANGE:
		self->range = (float *)data;
		break;
	case SIMPLEARPEGGIATOR_TIME:
		self->time = (float *) data;
		break;
	case SIMPLEARPEGGIATOR_GATE:
		self->gate = (float*)data;
		break;
	default:
		break;
	}
}

/**
 *    The activate() method resets the state completely, so the wave offset is
 *       zero and the envelope is off.
 *       */
static void activate(LV2_Handle instance)
{
    SimpleArpeggiator* self = (SimpleArpeggiator*)instance;

    self->elapsed_len = 0;
    self->beat_start_pos = 0;
    self->base_note = 128;
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
	uris->eg_applySample     = map->map(map->handle, EG_SIMPLEARPEGGIATOR_applySample);
	uris->eg_freeSample      = map->map(map->handle, EG_SIMPLEARPEGGIATOR_freeSample);
	uris->eg_sample          = map->map(map->handle, EG_SIMPLEARPEGGIATOR_sample);
	uris->midi_Event         = map->map(map->handle, LV2_MIDI__MidiEvent);
	uris->patch_Set          = map->map(map->handle, LV2_PATCH__Set);
	uris->patch_property     = map->map(map->handle, LV2_PATCH__property);
	uris->patch_value        = map->map(map->handle, LV2_PATCH__value);
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
		// Tempo changed, update BPM
		self->bpm = ((LV2_Atom_Float*) bpm)->body;
        self->frames_per_beat = 60.0f / self->bpm * self->rate;
		lv2_log_error(&self->logger, "bpm %f\n", self->bpm);
	}
	if (speed && speed->type == uris->atom_Float) {
		// Speed changed, e.g. 0 (stop) to 1 (play)
		self->speed = ((LV2_Atom_Float*) speed)->body;
		lv2_log_error(&self->logger, "speed %f\n", self->speed);
	}
	if (beatsperbar && beatsperbar->type == uris->atom_Float) {
		// Number of beats in a bar
		self->beats_per_bar = ((LV2_Atom_Float*) beatsperbar)->body;
		lv2_log_error(&self->logger, "beats_per_bar %f\n", self->beats_per_bar);
	}
	if (beatunit && beatunit->type == uris->atom_Int) {
		// Number of beats in a bar
		self->beat_unit = ((LV2_Atom_Int*) beatunit)->body;
		lv2_log_error(&self->logger, "beat_unit %f\n", self->beat_unit);
	}
	if (beat && beat->type == uris->atom_Float) {
		// Received a beat position, synchronise
        const float frames_per_beat = 60.0f / self->bpm * self->rate;
        const float bar_beats       = ((LV2_Atom_Float*)beat)->body;
        const float beat_beats      = bar_beats - floorf(bar_beats);
        self->elapsed_len           = beat_beats * frames_per_beat;
        self->beat_start_pos        = beat_beats * frames_per_beat;
        self->arp_index             = 0;
		lv2_log_error(&self->logger, "beat %f/%f %f\n", 
		        self->beats_per_bar, self->beat_unit, bar_beats);
	}

}

static void update_arp(
		SimpleArpeggiator*    self,
		uint32_t              begin,
		uint32_t              end,
		const uint32_t        out_capacity)
{
    if(self->speed < 1.0) return;

    for (uint32_t i = begin; i < end; ++i) {
        uint32_t elapsed_frames = self->elapsed_len - self->beat_start_pos;
        if(elapsed_frames % (self->frames_per_beat/2) == 0) {
            lv2_log_error(&self->logger, "1/2 %d\n", self->arp_index);

            if(self->base_note < 128) {
                MIDINoteEvent newnote;

				newnote.event.time.frames = 0;
				newnote.event.body.type   = self->uris.midi_Event;
				newnote.event.body.size   = 3;
				newnote.msg[0] = 0x90;
				newnote.msg[1] = self->base_note;
				if(self->arp_index % 2) newnote.msg[1] += 12;
				newnote.msg[2] = 127;
                
				lv2_atom_sequence_append_event(
						self->out_port, out_capacity, &newnote.event);
            }

            ++self->arp_index;
        }
        ++self->elapsed_len;
    }
}

static int update_midi(
		SimpleArpeggiator*    self,
		const uint8_t* const  msg,
        const uint32_t out_capacity
		)
{
    // return 0 if consumed by this filter
    lv2_log_error(&self->logger, "midi command %x %d %d\n", msg[0], msg[1], msg[2]);

	switch (lv2_midi_message_type(msg)) {
		case LV2_MIDI_MSG_NOTE_ON:
		    // only allow one note at a time
		    if(self->base_note == 128) {
                lv2_log_error(&self->logger, "note on %d\n", msg[1]);
		        self->base_note = msg[1];
            }
            return 0;
		case LV2_MIDI_MSG_NOTE_OFF:
		    if(self->base_note == msg[1]) {
                lv2_log_error(&self->logger, "note off %d\n", msg[1]);
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


	// Read incoming events
    uint32_t                 last_t = 0;
    LV2_ATOM_SEQUENCE_FOREACH(self->in_port, ev) {
        //lv2_log_error(&self->logger, "event %d\n", ev->body.type);
        if (ev->body.type == uris->atom_Object ||
                ev->body.type == uris->atom_Blank) {
            const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
            if (obj->body.otype == uris->time_Position) {
                // Received position information, update
                update_time(self, obj, ev);
            }
        } else if (ev->body.type == uris->midi_Event) {
			const uint8_t* const msg = (const uint8_t*)(ev + 1);
			if(update_midi(self, msg, out_capacity)) {
                lv2_atom_sequence_append_event(
                        self->out_port, out_capacity, ev);
            }
		}

        // update for this iteration
		update_arp(self, last_t, ev->time.frames, out_capacity);
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

