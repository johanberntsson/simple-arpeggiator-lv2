BUNDLE = simplearpeggiator.lv2
#INSTALL_DIR = /usr/lib/lv2
INSTALL_DIR = /home/johan/.lv2

all: $(BUNDLE)

gui:  install
	jalv.qt5 https://github.com/johanberntsson/simple-arpeggiator-lv2

test-main: test.c
	gcc  test.c -lm -o test

test: test-main
	./test

$(BUNDLE): manifest.ttl simplearpeggiator.ttl simplearpeggiator.so simplearpeggiator_gui_qt5.so
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp manifest.ttl simplearpeggiator.ttl simplearpeggiator.so simplearpeggiator_gui_qt5.so $(BUNDLE)

simplearpeggiator.o: simplearpeggiator.c simplearpeggiator.h
	gcc -c -fPIC -DPIC simplearpeggiator.c 

arpeggiator.o: arpeggiator.c arpeggiator.h
	gcc -c -fPIC -DPIC arpeggiator.c 

simplearpeggiator.so: simplearpeggiator.o arpeggiator.o
	gcc -shared -fPIC -DPIC arpeggiator.o simplearpeggiator.o `pkg-config --cflags --libs lv2-plugin` -o simplearpeggiator.so

simplearpeggiator_gui_qt5.o: simplearpeggiator_gui_qt5.moc.cpp

simplearpeggiator_gui_qt5.moc.cpp: simplearpeggiator_gui_qt5.cpp 
	moc $(DEFINES) $(INCPATH) -i $< -o $@

simplearpeggiator_gui_qt5.so: simplearpeggiator_gui_qt5.cpp simplearpeggiator_gui_qt5.moc.cpp simplearpeggiator.h
	g++ $< -o $@ -shared -fPIC -Wl,--no-undefined `pkg-config --cflags --libs Qt5Core Qt5Gui Qt5Widgets`

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

clean:
	rm -rf $(BUNDLE) *.o *.so *.moc.cpp

