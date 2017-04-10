A simple arpeggiator LV2 module for Linux DAWs
===

INTRODUCTION
------------

This arpeggiator is inspired by the LMMS standard arpeggiator effect. It can be added to effect chains in hosts such as QTractor to provide interesting bass lines and similar effects.

CODE
----

The code is divided into three parts

**User interface**:
manifest.ttl and simplearpeggiator.ttl define the input, output
and control ports. A host like QTractor will create a user interface
from the control port information which allows the user to adjust
the arpeggiator parameters.

**Plugin**:
simplearpeggiator.c handles the lifecycle of the plugin, 
including instantiation, updating of control parameter information
and providing access of the state information to the host application
for permanent storage and recovery.

Events are processed in the run() function, and routed to
update_midi() for midi on/off messages, and update_time() for
time synchronization messages.

**Arpeggiator**:
The actual arpeggiator functionality is all in arpeggiator.c, which
is called from simplearpeggiator.c. This allows the apreggiator to
be easily reused in future applications, such as other plugin formats
or stand-alone applications.

RESOURCES
---------
Useful information for LV2 development:
* LV2 spec: http://lv2plug.in/ns/lv2core/
* Good examples: http://lv2plug.in/book/
* Outdated but useful info: http://ll-plugins.nongnu.org/lv2pftci/
* Midi clock: https://en.wikipedia.org/wiki/MIDI_beat_clock
