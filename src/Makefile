debug:
	$(MAKE) -C common
	$(MAKE) -C channelserver
	$(MAKE) -C loginserver

release:
	$(MAKE) RELEASE=1 -C common
	$(MAKE) RELEASE=1 -C channelserver
	$(MAKE) RELEASE=1 -C loginserver

clean:
	$(MAKE) clean -C common
	$(MAKE) clean -C channelserver
	$(MAKE) clean -C loginserver

deps:
	$(MAKE) deps -C common

clean_deps:
	$(MAKE) clean_deps -C common

package:
	tar -zcvf tsr_server.$(shell date +%Y%m%d%H%M%S).tar.gz bin

