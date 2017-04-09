/*
  LV2 SimpleArpeggiator Example Plugin
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

#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include "./uris.h"

/* has to correspond to index port numbers in *.ttl */
enum {
	SIMPLEARPEGGIATOR_IN  = 0,
	SIMPLEARPEGGIATOR_OUT = 1,
	SIMPLEARPEGGIATOR_GAIN = 2
};

typedef struct {
	// Features
	LV2_URID_Map* map;

	// Ports
	const LV2_Atom_Sequence* in_port;
	LV2_Atom_Sequence*       out_port;
	float *                  gain;

	// URIs
	SimpleArpeggiatorURIs uris;
} SimpleArpeggiator;

static void
connect_port(LV2_Handle instance,
             uint32_t   port,
             void*      data)
{
	SimpleArpeggiator* self = (SimpleArpeggiator*)instance;
	switch (port) {
	case SIMPLEARPEGGIATOR_IN:
		self->in_port = (const LV2_Atom_Sequence*)data;
		break;
	case SIMPLEARPEGGIATOR_OUT:
		self->out_port = (LV2_Atom_Sequence*)data;
		break;
	case SIMPLEARPEGGIATOR_GAIN:
		self->gain = (float*)data;
		break;
	default:
		break;
	}
}

static LV2_Handle
instantiate(const LV2_Descriptor*     descriptor,
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
		}
	}
	if (!self->map) {
		fprintf(stderr, "Missing feature urid:map\n");
		free(self);
		return NULL;
	}

	// Map URIs and initialise forge/logger
	map_simplearpeggiator_uris(self->map, &self->uris);
 
	return (LV2_Handle)self;
}

static void
cleanup(LV2_Handle instance)
{
	free(instance);
}

static void
run(LV2_Handle instance,
    uint32_t   sample_count)
{
	SimpleArpeggiator*     self = (SimpleArpeggiator*)instance;
	SimpleArpeggiatorURIs* uris = &self->uris;

	// Struct for a 3 byte MIDI event, used for writing notes
	typedef struct {
		LV2_Atom_Event event;
		uint8_t        msg[3];
	} MIDINoteEvent;

	// Initially self->out_port contains a Chunk with size set to capacity

	// Get the capacity
	const uint32_t out_capacity = self->out_port->atom.size;

	// Write an empty Sequence header to the output
	lv2_atom_sequence_clear(self->out_port);
	self->out_port->atom.type = self->in_port->atom.type;

	// Read incoming events
	LV2_ATOM_SEQUENCE_FOREACH(self->in_port, ev) {
		if (ev->body.type == uris->midi_Event) {
			const uint8_t* const msg = (const uint8_t*)(ev + 1);
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
					newnote.msg[1] = msg[1] + *self->gain;  // Pitch up 7 semitones
					//newnote.msg[1] = msg[1] + 7;  // Pitch up 7 semitones
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
	}
}

static const void*
extension_data(const char* uri)
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

LV2_SYMBOL_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
	switch (index) {
	case 0:
		return &descriptor;
	default:
		return NULL;
	}
}
