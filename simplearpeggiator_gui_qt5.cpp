#include <QObject>
#include <QWidget>
#include <QDial>
#include <QLabel>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QString>

#include <stdio.h>

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include "simplearpeggiator.h"

#define SIMPLEARPEGGIATOR_UI_URI  \
        "https://github.com/johanberntsson/simple-arpeggiator-lv2#qt5"

class AmpGui : public QWidget {
	Q_OBJECT

public:
	AmpGui(QWidget* parent = 0);
	~AmpGui();
	QHBoxLayout* layout;

	QLabel* chord_label;
	QRadioButton* chord_octave;
	QRadioButton* chord_major;
	QRadioButton* chord_minor;
	QGroupBox* chord_group;
	QVBoxLayout* chord_layout;
	QSpacerItem *chord_spacer;

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

	float gate;

	LV2UI_Controller controller;
	LV2UI_Write_Function write_function;

public slots:
	void chordChanged(bool checked);
	void timeChanged(bool checked);
	void rangeChanged(int value);
	void gateChanged(int value);

};

AmpGui::AmpGui(QWidget* parent)
	: QWidget(parent) {
	    QRadioButton();


    chord_group = new QGroupBox();
	chord_label = new QLabel("Chord");
	chord_octave = new QRadioButton("Octave");
	chord_major = new QRadioButton("Major");
	chord_minor = new QRadioButton("Minor");
	chord_spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	chord_layout = new QVBoxLayout();
	chord_layout->addWidget(chord_label);
	chord_layout->addWidget(chord_octave);
	chord_layout->addWidget(chord_major);
	chord_layout->addWidget(chord_minor);
    chord_layout->addItem(chord_spacer);
    chord_group->setLayout(chord_layout);

    time_group = new QGroupBox();
	time_label = new QLabel("Time");
	time_1_1 = new QRadioButton("whole");
	time_1_2 = new QRadioButton("1/2");
	time_1_4 = new QRadioButton("1/4");
	time_1_8 = new QRadioButton("1/8");
	time_1_16 = new QRadioButton("1/16");
	time_1_32 = new QRadioButton("1/32");
	time_spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
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
	range_spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
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
	gate_spacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	gate_layout = new QVBoxLayout();
	gate_layout->addWidget(gate_label);
	gate_layout->addWidget(gate_dial);
    gate_layout->addItem(gate_spacer);
    gate_group->setLayout(gate_layout);

	layout = new QHBoxLayout();
	layout->addWidget(chord_group);
	layout->addWidget(time_group);
	layout->addWidget(range_group);
	layout->addWidget(gate_group);
	setLayout(layout);

#ifndef QT_NO_TOOLTIP
    chord_group->setToolTip("The chord defines what notes are played in each octave");
    time_group->setToolTip("The length of each arpeggio note");
    range_group->setToolTip("the arpeggio range in octaves");
    gate_group->setToolTip("The percent of a whole apreggio note that should be played. Seting it to less than 100% can create cool staccato effects.");
#endif

    chord_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
    time_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
    range_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
    gate_group->setStyleSheet("QGroupBox {  border: 1px solid gray;}");
}

AmpGui::~AmpGui() {
    // trying to delete the allocation in the constructor gives
    // segementation fault
}

void AmpGui::chordChanged(bool checked) {
    int chord = -1;
    if(!checked) return;
    if(chord_octave->isChecked()) chord = 0;
    if(chord_major->isChecked()) chord = 1;
    if(chord_minor->isChecked()) chord = 2;
    printf("chord %d\n", chord);
}

void AmpGui::timeChanged(bool checked) {
    int time = -1;
    if(!checked) return;
    if(time_1_1->isChecked()) time = 0;
    if(time_1_2->isChecked()) time = 1;
    if(time_1_4->isChecked()) time = 2;
    if(time_1_8->isChecked()) time = 3;
    if(time_1_16->isChecked()) time = 4;
    if(time_1_32->isChecked()) time = 5;
    printf("time %d\n", time);
}

void AmpGui::rangeChanged(int value) {
	int range = range_dial->value();
	range_label->setText(QString("Range: %1").arg(range));
}

void AmpGui::gateChanged(int value) {
	float gate = gate_dial->value();

	gate_label->setText(QString("Gate: %1 %").arg(gate));
	write_function(controller, AMP_GAIN, sizeof(gate), 0, &gate);
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

	AmpGui* pluginGui = new AmpGui();
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

	return (LV2UI_Handle)pluginGui;
}

void cleanup(LV2UI_Handle ui) {
	AmpGui* pluginGui = (AmpGui*) ui;

	delete pluginGui;
}

void port_event(LV2UI_Handle ui, uint32_t port_index, uint32_t buffer_size,
	uint32_t format, const void* buffer) {
	AmpGui* pluginGui = (AmpGui*) ui;
	float* pval = (float*) buffer;

	if ((format != 0) || (port_index < 0) || (port_index >= AMP_N_PORTS)) {
		return;
	}

	// Multiplication by 10 is to adjust for the dial step quantity
	// Addition by 0.5 is to round to int correctly
	pluginGui->gate_dial->setValue((int)((*pval * 10) + 0.5));
}

const void* extension_data(const char* uri) { return NULL; }

static LV2UI_Descriptor descriptor = {
	SIMPLEARPEGGIATOR_UI_URI, instantiate, cleanup, port_event, extension_data
};

const LV2UI_Descriptor* lv2ui_descriptor(uint32_t index) {
	switch (index) {
		case 0:  return &descriptor;
		default: return NULL;
	}
}

#include "simplearpeggiator_gui_qt5.moc.cpp"
