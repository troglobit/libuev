all: lint

lint: .linted

.linted: root/usr/bin/nmeamux
	LUA_EXTRA='../../agentframework/lua/?' /opt/toolchain/utils/scripts/bin/lualint $< && touch $@

pak:
	rm -rf amplight-solo-pak
	svn export amplight-solo amplight-solo-pak
	sed "s/-VVV-/$(shell pakversion gen amplight-solo )/"  amplight-solo/pak-info > amplight-solo-pak/pak-info
	pak create amplight-solo-pak

pak-clean:
	rm -fr *-pak *.pak

doc:
	amp-doxygen -W

doc-clean:
	rm -rf doxygen

clean:  pak-clean doc-clean
	rm -fr *.bxml
	rm -f ${OBJS} ${TARGETFILE} mapfile

.PHONY: all pak clean doc doc-clean

