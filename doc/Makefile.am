TROFF = groff
TROFF_FLAGS = -t -ms

DOCS = Hacking.ms Player-Development.ms Protracker.ms

DOCS_PS = $(DOCS:.ms=.ps)
DOCS_TXT = $(DOCS:.ms=.txt)
DOCS_HTML = $(DOCS:.ms=.html)

EXTRA_DIST = $(DOCS) $(DOCS_PS) $(DOCS_TXT)

%.ps : %.ms
	$(TROFF) $(TROFF_FLAGS) -Tps $< > $@

%.txt : %.ms
	$(TROFF) $(TROFF_FLAGS) -Tascii $< > $@

%.html : %.ms
	$(TROFF) $(TROFF_FLAGS) -Thtml $< > $@

all-local: docs

clean-local:
	-rm -f $(DOCS_PS) $(DOCS_TXT) $(DOCS_HTML)

distclean-local:
	-rm -f *.png

dist-hook: docs

docs: docs_ps docs_txt

docs_ps: $(DOCS_PS)
docs_txt: $(DOCS_TXT)
docs_html: $(DOCS_HTML)

Hacking.ps: Hacking.ms
Player-Development.ps: Player-Development.ms
Protracker.ps: Protracker.ms

Hacking.txt: Hacking.ms
Player-Development.txt: Player-Development.ms
Protracker.txt: Protracker.ms

Hacking.html: Hacking.ms
Player-Development.html: Player-Development.ms
Protracker.html: Protracker.ms

.PHONY: docs docs_ps docs_txt docs_html
