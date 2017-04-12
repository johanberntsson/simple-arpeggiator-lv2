#define SIMPLEARPEGGIATOR_URI            \
        "https://github.com/johanberntsson/simple-arpeggiator-lv2"

#define SIMPLEARPEGGIATOR_N_PORTS 6
/* has to correspond to port index numbers in simplearpeggiator.ttl */
enum {
    SIMPLEARPEGGIATOR_IN  = 0,
    SIMPLEARPEGGIATOR_OUT = 1,
    SIMPLEARPEGGIATOR_CHORD = 2,
    SIMPLEARPEGGIATOR_RANGE = 3,
    SIMPLEARPEGGIATOR_TIME = 4,
    SIMPLEARPEGGIATOR_GATE = 5
} PortIndex;

