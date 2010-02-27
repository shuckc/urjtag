WEB=arniml,urjtag@web.sourceforge.net:/home/groups/u/ur/urjtag
JTAG=../urjtag
BOOK=./htdocs/book

RSYNC=rsync
RSYNCOPTS=-av --rsh=ssh --exclude=.svn

JWOPTS=-d UrJTAG.dsl\#html

DIRS="cgi-bin" "htdocs"

sync-to-sf: book
	$(RSYNC) $(RSYNCOPTS) $(DIRS) "$(WEB)/"

sync-from-sf:
	$(RSYNC) $(RSYNCOPTS) "$(WEB)/*" .

.PHONY: book
# book: $(BOOK)/UrJTAG.html
book: $(BOOK)/UrJTAG.dbk $(BOOK)/UrJTAG.html
	docbook2html $(JWOPTS) -o $(BOOK) $(BOOK)/UrJTAG.dbk

.PHONY: $(BOOK)/UrJTAG.html
$(BOOK)/UrJTAG.html: $(BOOK)/UrJTAG.dbk
	docbook2html $(JWOPTS) -o $(BOOK) -u $(BOOK)/UrJTAG.dbk

$(BOOK)/UrJTAG.dbk: $(JTAG)/doc/*.txt
	mkdir -p $(BOOK)
	asciidoc -b docbook -d book -o $(BOOK)/UrJTAG.dbk $(JTAG)/doc/UrJTAG.txt

