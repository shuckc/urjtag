WEB=shell.urjtag.sourceforge.net:/home/groups/u/ur/urjtag
JTAG=../jtag
BOOK=./htdocs/book

RSYNC=rsync
RSYNCOPTS=-av --rsh=ssh --exclude=.svn

DIRS="cgi-bin" "htdocs"

sync-to-sf: book
	$(RSYNC) $(RSYNCOPTS) $(DIRS) "$(WEB)/"

sync-from-sf:
	$(RSYNC) $(RSYNCOPTS) "$(WEB)/*" .

.PHONY: book
book: $(BOOK)/UrJTAG.dbk
	docbook2html -o $(BOOK) $(BOOK)/UrJTAG.dbk

$(BOOK)/UrJTAG.dbk: $(JTAG)/doc/*.txt
	mkdir -p $(BOOK)
	asciidoc -b docbook -d book -o $(BOOK)/UrJTAG.dbk $(JTAG)/doc/UrJTAG.txt

