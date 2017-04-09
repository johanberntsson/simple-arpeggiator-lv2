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
    LV2_URID time_barBeat;
    LV2_URID time_beatsPerMinute;
    LV2_URID time_speed;
} SimpleArpeggiatorURIs;


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
    BEAT = 0,
    BEAT_1_2 = 1,
    BEAT_1_4 = 2,
    BEAT_1_8 = 3,
    BEAT_1_16 = 4,
    BEAT_1_32 = 5
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
    uris->time_barBeat       = map->map(map->handle, LV2_TIME__barBeat);
    uris->time_beatsPerMinute= map->map(map->handle, LV2_TIME__beatsPerMinute);
    uris->time_speed         = map->map(map->handle, LV2_TIME__speed);

	// initialise forge/logger
    lv2_log_logger_init(&self->logger, self->map, self->log);

	return (LV2_Handle)self;
}

static void cleanup(LV2_Handle instance)
{
	free(instance);
}

static void update_position(SimpleArpeggiator* self, const LV2_Atom_Object* obj)
{
	const SimpleArpeggiatorURIs* uris = &self->uris;

	// Received new transport position/speed
	LV2_Atom *beat = NULL, *bpm = NULL, *speed = NULL;
	lv2_atom_object_get(obj,
			uris->time_barBeat, &beat,
			uris->time_beatsPerMinute, &bpm,
			uris->time_speed, &speed,
			NULL);
	if (bpm && bpm->type == uris->atom_Float) {
		// Tempo changed, update BPM
		self->bpm = ((LV2_Atom_Float*)bpm)->body;
	}
	if (speed && speed->type == uris->atom_Float) {
		// Speed changed, e.g. 0 (stop) to 1 (play)
		self->speed = ((LV2_Atom_Float*)speed)->body;
		//lv2_log_error(&self->logger, "speed %f\n", self->speed);
	}
	if (beat && beat->type == uris->atom_Float) {
		// Received a beat position, synchronise
		// This hard sync may cause clicks, a real plugin would be more graceful
		lv2_log_error(&self->logger, "beat!\n");
	}

}

static void update_midi(
		SimpleArpeggiator*    self,
		const uint8_t* const  msg,
		const uint32_t        out_capacity,
		const LV2_Atom_Event* ev)
{
	enum chordtype chord = (int) *self->chord;
	int range = (int) *self->range;
	enum timetype time = (int) *self->chord;
	float gate = *self->gate;

	// Struct for a 3 byte MIDI event, used for writing notes
	typedef struct {
		LV2_Atom_Event event;
		uint8_t        msg[3];
	} MIDINoteEvent;

lv2_log_error(&self->logger, "midi start %x\n", msg[0]);

	switch (lv2_midi_message_type(msg)) {
		case LV2_MIDI_MSG_NOTE_ON:
		case LV2_MIDI_MSG_NOTE_OFF:
			// Forward note to output
			lv2_atom_sequence_append_event(
					self->out_port, out_capacity, ev);


			const uint8_t note = msg[1];
			if (note <= 127 - 7) {
				// Make a note one 5th (7 semitones) higher than input
				MIDINoteEvent newnote;

				// Could simply do newnote.event = *ev here instead...
				newnote.event.time.frames = ev->time.frames;  // Same time
				newnote.event.body.type   = ev->body.type;    // Same type
				newnote.event.body.size   = ev->body.size;    // Same size

				newnote.msg[0] = msg[0];      // Same status
				//newnote.msg[1] = msg[1] + 7;  // Pitch up 7 semitones
				newnote.msg[1] = msg[1] + range;  // Pitch up 7 semitones
				newnote.msg[2] = msg[2];      // Same velocity

				// Write 5th event
				lv2_atom_sequence_append_event(
						self->out_port, out_capacity, &newnote.event);
			}
			break;
		default:
			// Forward all other MIDI events directly
			lv2_atom_sequence_append_event(
					self->out_port, out_capacity, ev);
			break;
	}
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
    LV2_ATOM_SEQUENCE_FOREACH(self->in_port, ev) {
        lv2_log_error(&self->logger, "event %d\n", ev->body.type);
        if (ev->body.type == uris->atom_Object ||
                ev->body.type == uris->atom_Blank) {
            const LV2_Atom_Object* obj = (const LV2_Atom_Object*)&ev->body;
            if (obj->body.otype == uris->time_Position) {
                // Received position information, update
                update_position(self, obj);
            }
        } else if (ev->body.type == uris->midi_Event) {
			const uint8_t* const msg = (const uint8_t*)(ev + 1);
			update_midi(self, msg, out_capacity, ev);
		}
	}
}

static const void* extension_data(const char* uri)
{
	return NULL;
}

static const LV2_Descriptor descriptor = {
	SIMPLEARPEGGIATOR_URI,
	instantiate,
	connect_port,
	NULL,  // activate,
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

