BUNDLE = simplearpeggiator.lv2
#INSTALL_DIR = /usr/lib/lv2
INSTALL_DIR = /home/johan/.lv2


$(BUNDLE): manifest.ttl simplearpeggiator.ttl simplearpeggiator.so
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp manifest.ttl simplearpeggiator.ttl simplearpeggiator.so $(BUNDLE)

simplearpeggiator.o: simplearpeggiator.c
	gcc -c -fPIC -DPIC simplearpeggiator.c 

arpeggiator.o: arpeggiator.c arpeggiator.h
	gcc -c -fPIC -DPIC arpeggiator.c 

simplearpeggiator.so: simplearpeggiator.o arpeggiator.o
	gcc -shared -fPIC -DPIC arpeggiator.o simplearpeggiator.o `pkg-config --cflags --libs lv2-plugin` -o simplearpeggiator.so


install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

clean:
	rm -rf $(BUNDLE) simplearpeggiator.so simplearpeggiator.o arpeggiator.o

