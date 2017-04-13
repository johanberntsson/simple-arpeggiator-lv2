#include <stdio.h>

#include <QDial>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QRadioButton>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "simplearpeggiator.h"

class SimpleArpeggiatorGUI : public QWidget {
    Q_OBJECT

    public:
        SimpleArpeggiatorGUI(QWidget* parent = 0);
        ~SimpleArpeggiatorGUI();
        QHBoxLayout* layout;
        QVBoxLayout* v1_layout;
        QVBoxLayout* v2_layout;
        QVBoxLayout* v3_layout;

        QLabel* chord_label;
        QRadioButton* chord_octave;
        QRadioButton* chord_major;
        QRadioButton* chord_minor;
        QGroupBox* chord_group;
        QVBoxLayout* chord_layout;
        QSpacerItem *chord_spacer;

        QLabel* dir_label;
        QRadioButton* dir_up;
        QRadioButton* dir_down;
        QRadioButton* dir_updown;
        QGroupBox* dir_group;
        QVBoxLayout* dir_layout;
        QSpacerItem *dir_spacer;

        QLabel* time_label;
        QRadioButton* time_1_1;
        QRadioButton* time_1_2;
        QRadioButton* time_1_4;
        QRadioButton* time_1_8;
        QRadioButton* time_1_16;
        QRadioButton* time_1_32;
        QGroupBox* time_group;
        QVBoxLayout* time_layout;
        QSpacerItem *time_spacer;

        QDial* range_dial;
        QLabel* range_label;
        QGroupBox* range_group;
        QVBoxLayout* range_layout;
        QSpacerItem *range_spacer;

        QDial* gate_dial;
        QLabel* gate_label;
        QGroupBox* gate_group;
        QVBoxLayout* gate_layout;
        QSpacerItem *gate_spacer;

        QDial* cycle_dial;
        QLabel* cycle_label;
        QGroupBox* cycle_group;
        QVBoxLayout* cycle_layout;
        QSpacerItem *cycle_spacer;

        QDial* skip_dial;
        QLabel* skip_label;
        QGroupBox* skip_group;
        QVBoxLayout* skip_layout;
        QSpacerItem *skip_spacer;

        float gate;

        LV2UI_Controller controller;
        LV2UI_Write_Function write_function;

    public slots:
        void chordChanged(bool checked);
        void timeChanged(bool checked);
        void rangeChanged(int value);
        void gateChanged(int value);
        void cycleChanged(int value);
        void skipChanged(int value);
        void dirChanged(bool checked);

};

SimpleArpeggiatorGUI::SimpleArpeggiatorGUI(QWidget* parent)
    : QWidget(parent) {
        QRadioButton();


        chord_group = new QGroupBox();
        chord_label = new QLabel("Chord");
        chord_octave = new QRadioButton("Octave");
        chord_major = new QRadioButton("Major");
        chord_minor = new QRadioButton("Minor");
        chord_spacer = new QSpacerItem(20,40,QSizePolicy::Minimum, QSizePolicy::Expanding);
        chord_layout = new QVBoxLayout();
        chord_layout->addWidget(chord_label);
        chord_layout->addWidget(chord_octave);
        chord_layout->addWidget(chord_major);
        chord_layout->addWidget(chord_minor);
        chord_layout->addItem(chord_spacer);
        chord_group->setLayout(chord_layout);

        dir_group = new QGroupBox();
        dir_label = new QLabel("direction");
        dir_up = new QRadioButton("up");
        dir_down = new QRadioButton("down");
        dir_updown = new QRadioButton("up-down");
        dir_spacer = new QSpacerItem(20,40,QSizePolicy::Minimum, QSizePolicy::Expanding);
        dir_layout = new QVBoxLayout();
        dir_layout->addWidget(dir_label);
        dir_layout->addWidget(dir_up);
        dir_layout->addWidget(dir_down);
        dir_layout->addWidget(dir_updown);
        dir_layout->addItem(dir_spacer);
        dir_group->setLayout(dir_layout);

        time_group = new QGroupBox();
        time_label = new QLabel("Time");
        time_1_1 = new QRadioButton("whole");
        time_1_2 = new QRadioButton("1/2");
        time_1_4 = new QRadioButton("1/4");
        time_1_8 = new QRadioButton("1/8");
        time_1_16 = new QRadioButton("1/16");
        time_1_32 = new QRadioButton("1/32");
        time_spacer = new QSpacerItem(20,40,QSizePolicy::Minimum, QSizePolicy::Expanding);
        time_layout = new QVBoxLayout();
        time_layout->addWidget(time_label);
        time_layout->addWidget(time_1_1);
        time_layout->addWidget(time_1_2);
        time_layout->addWidget(time_1_4);
        time_layout->addWidget(time_1_8);
        time_layout->addWidget(time_1_16);
        time_layout->addWidget(time_1_32);
        time_layout->addItem(time_spacer);
        time_group->setLayout(time_layout);

        range_group = new QGroupBox();
        range_label = new QLabel("Range");
        range_dial = new QDial();
        range_dial->setRange(1, 9);
        range_dial->setValue(9);
        range_dial->setNotchesVisible(true);
        range_spacer = new QSpacerItem(20,40,QSizePolicy::Minimum, QSizePolicy::Expanding);
        range_layout = new QVBoxLayout();
        range_layout->addWidget(range_label);
        range_layout->addWidget(range_dial);
        range_layout->addItem(range_spacer);
        range_group->setLayout(range_layout);

        gate_group = new QGroupBox();
        gate_label = new QLabel("Gate");
        gate_dial = new QDial();
        gate_dial->setRange(0, 100);
        gate_dial->setNotchesVisible(true);
        gate_spacer = new QSpacerItem(20,40,QSizePolicy::Minimum, QSizePolicy::Expanding);
        gate_layout = new QVBoxLayout();
        gate_layout->addWidget(gate_label);
        gate_layout->addWidget(gate_dial);
        gate_layout->addItem(gate_spacer);
        gate_group->setLayout(gate_layout);

        skip_group = new QGroupBox();
        skip_label = new QLabel("skip");
        skip_dial = new QDial();
        skip_dial->setRange(0, 100);
        skip_dial->setNotchesVisible(true);
        skip_spacer = new QSpacerItem(20,40,QSizePolicy::Minimum, QSizePolicy::Expanding);
        skip_layout = new QVBoxLayout();
        skip_layout->addWidget(skip_label);
        skip_layout->addWidget(skip_dial);
        skip_layout->addItem(skip_spacer);
        skip_group->setLayout(skip_layout);

        cycle_group = new QGroupBox();
        cycle_label = new QLabel("cycle");
        cycle_dial = new QDial();
        cycle_dial->setRange(0, 6);
        cycle_dial->setNotchesVisible(true);
        cycle_spacer = new QSpacerItem(20,40,QSizePolicy::Minimum, QSizePolicy::Expanding);
        cycle_layout = new QVBoxLayout();
        cycle_layout->addWidget(cycle_label);
        cycle_layout->addWidget(cycle_dial);
        cycle_layout->addItem(cycle_spacer);
        cycle_group->setLayout(cycle_layout);

        layout = new QHBoxLayout();
        v1_layout = new QVBoxLayout();
        v2_layout = new QVBoxLayout();
        v3_layout = new QVBoxLayout();
        v1_layout->addWidget(chord_group);
        v1_layout->addWidget(dir_group);
        v2_layout->addWidget(range_group);
        v2_layout->addWidget(cycle_group);
        v3_layout->addWidget(gate_group);
        v3_layout->addWidget(skip_group);
        layout->addLayout(v1_layout);
        layout->addWidget(time_group);
        layout->addLayout(v2_layout);
        layout->addLayout(v3_layout);
        setLayout(layout);

#ifndef QT_NO_TOOLTIP
        chord_group->setToolTip("The chord defines what notes are played in each octave");
        dir_group->setToolTip("How the arpeggio is played");
        time_group->setToolTip("The length of each arpeggio note");
        range_group->setToolTip("The arpeggio range in octaves");
        gate_group->setToolTip("The percentiage of a whole apreggio note that should be played. Seting it to less than 100% can create cool staccato effects.");
        cycle_group->setToolTip("Cycle will jump over the 1-6th step in the arpeggio");
        skip_group->setToolTip("Skip will cause the arpeggio to pause randomly if set to more than 0%");
#endif

        chord_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
        dir_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
        time_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
        range_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
        gate_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
        cycle_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
        skip_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
    }

SimpleArpeggiatorGUI::~SimpleArpeggiatorGUI() {
    // trying to delete the allocation in the constructor gives
    // segementation fault
}

void SimpleArpeggiatorGUI::chordChanged(bool checked) {
    float chord = 0;
    if(!checked) return;
    if(chord_octave->isChecked()) chord = 0;
    if(chord_major->isChecked()) chord = 1;
    if(chord_minor->isChecked()) chord = 2;
    write_function(controller, SIMPLEARPEGGIATOR_CHORD, sizeof(gate), 0, &chord);
}

void SimpleArpeggiatorGUI::timeChanged(bool checked) {
    float time = 0;
    if(!checked) return;
    if(time_1_1->isChecked()) time = 0;
    if(time_1_2->isChecked()) time = 1;
    if(time_1_4->isChecked()) time = 2;
    if(time_1_8->isChecked()) time = 3;
    if(time_1_16->isChecked()) time = 4;
    if(time_1_32->isChecked()) time = 5;
    write_function(controller, SIMPLEARPEGGIATOR_TIME, sizeof(gate), 0, &time);
}

void SimpleArpeggiatorGUI::rangeChanged(int value) {
    float range = range_dial->value();
    range_label->setText(QString("Range: %1").arg(range));
    write_function(controller, SIMPLEARPEGGIATOR_RANGE, sizeof(gate), 0, &range);
}

void SimpleArpeggiatorGUI::gateChanged(int value) {
    float gate = gate_dial->value();
    gate_label->setText(QString("Gate: %1 %").arg(gate));
    write_function(controller, SIMPLEARPEGGIATOR_GATE, sizeof(gate), 0, &gate);
}

void SimpleArpeggiatorGUI::cycleChanged(int value) {
    float cycle = cycle_dial->value();
    cycle_label->setText(QString("Cycle: %1").arg(cycle));
    write_function(controller, SIMPLEARPEGGIATOR_CYCLE, sizeof(cycle), 0, &cycle);
}

void SimpleArpeggiatorGUI::skipChanged(int value) {
    float skip = skip_dial->value();
    skip_label->setText(QString("Skip: %1 %").arg(skip));
    write_function(controller, SIMPLEARPEGGIATOR_SKIP, sizeof(skip), 0, &skip);
}

void SimpleArpeggiatorGUI::dirChanged(bool checked) {
    float dir = 0;
    if(!checked) return;
    if(dir_up->isChecked()) dir = 0;
    if(dir_down->isChecked()) dir = 1;
    if(dir_updown->isChecked()) dir = 2;
    write_function(controller, SIMPLEARPEGGIATOR_DIR, sizeof(gate), 0, &dir);
}

LV2UI_Handle instantiate(const struct _LV2UI_Descriptor* descriptor,
        const char* plugin_uri, const char* bundle_path,
        LV2UI_Write_Function write_function,
        LV2UI_Controller controller, LV2UI_Widget* widget,
        const LV2_Feature* const* features) {

    if (strcmp(plugin_uri, SIMPLEARPEGGIATOR_URI) != 0) {
        fprintf(stderr, "AMP_UI error: this GUI does not support plugin with URI %s\n", plugin_uri);
        return NULL;
    }

    SimpleArpeggiatorGUI* pluginGui = new SimpleArpeggiatorGUI();
    *widget = pluginGui;

    if (pluginGui == NULL) return NULL;

    pluginGui->controller = controller;
    pluginGui->write_function = write_function;

    QObject::connect(pluginGui->chord_octave, SIGNAL(toggled(bool)),
            pluginGui, SLOT(chordChanged(bool)));
    QObject::connect(pluginGui->chord_major, SIGNAL(toggled(bool)),
            pluginGui, SLOT(chordChanged(bool)));
    QObject::connect(pluginGui->chord_minor, SIGNAL(toggled(bool)),
            pluginGui, SLOT(chordChanged(bool)));
    QObject::connect(pluginGui->time_1_1, SIGNAL(toggled(bool)),
            pluginGui, SLOT(timeChanged(bool)));
    QObject::connect(pluginGui->time_1_2, SIGNAL(toggled(bool)),
            pluginGui, SLOT(timeChanged(bool)));
    QObject::connect(pluginGui->time_1_4, SIGNAL(toggled(bool)),
            pluginGui, SLOT(timeChanged(bool)));
    QObject::connect(pluginGui->time_1_8, SIGNAL(toggled(bool)),
            pluginGui, SLOT(timeChanged(bool)));
    QObject::connect(pluginGui->time_1_16, SIGNAL(toggled(bool)),
            pluginGui, SLOT(timeChanged(bool)));
    QObject::connect(pluginGui->time_1_32, SIGNAL(toggled(bool)),
            pluginGui, SLOT(timeChanged(bool)));
    QObject::connect(pluginGui->range_dial, SIGNAL(valueChanged(int)),
            pluginGui, SLOT(rangeChanged(int)));
    QObject::connect(pluginGui->gate_dial, SIGNAL(valueChanged(int)),
            pluginGui, SLOT(gateChanged(int)));
    QObject::connect(pluginGui->cycle_dial, SIGNAL(valueChanged(int)),
            pluginGui, SLOT(cycleChanged(int)));
    QObject::connect(pluginGui->skip_dial, SIGNAL(valueChanged(int)),
            pluginGui, SLOT(skipChanged(int)));
    QObject::connect(pluginGui->dir_up, SIGNAL(toggled(bool)),
            pluginGui, SLOT(dirChanged(bool)));
    QObject::connect(pluginGui->dir_down, SIGNAL(toggled(bool)),
            pluginGui, SLOT(dirChanged(bool)));
    QObject::connect(pluginGui->dir_updown, SIGNAL(toggled(bool)),
            pluginGui, SLOT(dirChanged(bool)));

    return (LV2UI_Handle)pluginGui;
}

void cleanup(LV2UI_Handle ui) {
    SimpleArpeggiatorGUI* pluginGui = (SimpleArpeggiatorGUI*) ui;

    delete pluginGui;
}

void port_event(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
        uint32_t format, const void* buffer) {
    SimpleArpeggiatorGUI* pluginGui = (SimpleArpeggiatorGUI*) ui;
    float* pval = (float*) buffer;
    int n;

    if ((format != 0) || (port_index < 0) || (port_index >= SIMPLEARPEGGIATOR_N_PORTS)) {
        return;
    }

    // Addition by 0.5 is to round to int correctly
    switch(port_index) {
        case SIMPLEARPEGGIATOR_CHORD:
            n = (int) (*pval  + 0.5);
            if(n == 0) pluginGui->chord_octave->setChecked(true);
            if(n == 1) pluginGui->chord_major->setChecked(true);
            if(n == 2) pluginGui->chord_minor->setChecked(true);
            break;
        case SIMPLEARPEGGIATOR_TIME:
            n = (int) (*pval  + 0.5);
            if(n == 0) pluginGui->time_1_1->setChecked(true);
            if(n == 1) pluginGui->time_1_2->setChecked(true);
            if(n == 2) pluginGui->time_1_4->setChecked(true);
            if(n == 3) pluginGui->time_1_8->setChecked(true);
            if(n == 4) pluginGui->time_1_16->setChecked(true);
            if(n == 5) pluginGui->time_1_32->setChecked(true);
            break;
        case SIMPLEARPEGGIATOR_RANGE:
            pluginGui->range_dial->setValue((int)(*pval  + 0.5));
            break;
        case SIMPLEARPEGGIATOR_GATE:
            pluginGui->gate_dial->setValue((int)(*pval  + 0.5));
            break;
        case SIMPLEARPEGGIATOR_CYCLE:
            pluginGui->cycle_dial->setValue((int)(*pval  + 0.5));
            break;
        case SIMPLEARPEGGIATOR_SKIP:
            pluginGui->skip_dial->setValue((int)(*pval  + 0.5));
            break;
        case SIMPLEARPEGGIATOR_DIR:
            n = (int) (*pval  + 0.5);
            if(n == 0) pluginGui->dir_up->setChecked(true);
            if(n == 1) pluginGui->dir_down->setChecked(true);
            if(n == 2) pluginGui->dir_updown->setChecked(true);
            break;
    }
}

const void* extension_data(const char* uri) { return NULL; }

static LV2UI_Descriptor descriptor = {
    SIMPLEARPEGGIATOR_URI "#qt5", instantiate, cleanup, port_event, extension_data
};

const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index) {
    switch (index) {
        case 0:  return &descriptor;
        default: return NULL;
    }
}

#include "simplearpeggiator_gui_qt5.moc.cpp"
