BUNDLE = simplearpeggiator.lv2
#INSTALL_DIR = /usr/lib/lv2
INSTALL_DIR = /home/johan/.lv2


$(BUNDLE): manifest.ttl simplearpeggiator.ttl simplearpeggiator.so
	rm -rf $(BUNDLE)
	mkdir $(BUNDLE)
	cp manifest.ttl simplearpeggiator.ttl simplearpeggiator.so $(BUNDLE)

simplearpeggiator.so: simplearpeggiator.c uris.h
	gcc -shared -fPIC -DPIC simplearpeggiator.c `pkg-config --cflags --libs lv2-plugin` -o simplearpeggiator.so


install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

clean:
	rm -rf $(BUNDLE) simplearpeggiator.so

