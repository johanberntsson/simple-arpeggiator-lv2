A simple arpeggiator LV2 module for Linux DAWs
===

INTRODUCTION
------------

This arpeggiator is inspired by the LMMS standard arpeggiator effect. It can be added to effect chains in hosts such as QTractor to provide interesting bass lines and similar effects.

USAGE
-----

These parameters are supported:

* **chord** three types are supported: octave, major, and minor
* **range** the arpeggio range in octaves
* **time** set the length of each arpeggio note, for instance 1/8ths.
* **gate** the percent of a whole apreggio note that should be played. Seting it to less than 100% can create cool staccato effects.

CODE
----

The code is divided into these logical parts:

**Plugin interface**:
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

**Graphical User Interface**:
The optional GUI is designed with the FLUID IDE. The definition file is gui.fld, and the implementation files are ...

RESOURCES
---------
Useful information for LV2 development:

* LV2 spec: http://lv2plug.in/ns/lv2core/
* Good examples: http://lv2plug.in/book/
* Outdated but useful info: http://ll-plugins.nongnu.org/lv2pftci/
* Midi clock: https://en.wikipedia.org/wiki/MIDI_beat_clock
* GUI examples: https://github.com/badosu/BadAmp
* GUI with fluid: http://mountainbikesandtrombones.blogspot.com.au/2014/08/making-lv2-plugin-gui-yes-in-inkscape.html
* Fluid: http://www.fltk.org/doc-1.1/fluid.html
