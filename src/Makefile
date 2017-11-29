debug:
	$(MAKE) -C common
	$(MAKE) -C channelserver

release:
	$(MAKE) RELEASE=1 -C common
	$(MAKE) RELEASE=1 -C channelserver

clean:
	$(MAKE) clean -C common
	$(MAKE) clean -C channelserver

deps:
	$(MAKE) deps -C common

clean_deps:
	$(MAKE) clean_deps -C common
