# AdPlug Makefile for WATCOM v11, (c) 2001 Simon Peter <dn.tlp@gmx.net>

CXX = wpp386
CXXFLAGS = -oneatx -oh -oi+ -ei -zp8 -5 -fpi87 -fp5 -zq
CPPFLAGS = -dstd= -dstring=String

PLAYERS = protrack.obj a2m.obj amd.obj d00.obj dfm.obj hsc.obj hsp.obj imf.obj ksm.obj mid.obj mkj.obj mtk.obj rad.obj raw.obj s3m.obj sa2.obj sng.obj bam.obj

.cpp.obj:
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $[.

all: players.lib

players.lib: $(PLAYERS)
        wlib -n -b players.lib +protrack +a2m +amd +d00 +dfm +hsc +hsp +imf +ksm +mid +mkj +mtk +rad +raw +s3m +sa2 +sng +bam

clean: .symbolic
	del *.obj
	del players.lib

$(PLAYERS): player.h
protrack.obj: protrack.cpp protrack.h
a2m.obj: a2m.cpp a2m.h protrack.h
amd.obj: amd.cpp amd.h protrack.h
bam.obj: bam.cpp bam.h
d00.obj: d00.cpp d00.h
dfm.obj: dfm.cpp dfm.h protrack.h
hsc.obj: hsc.cpp hsc.h
hsp.obj: hsp.cpp hsp.h hsc.h
imf.obj: imf.cpp imf.h imfcrc.h
ksm.obj: ksm.cpp ksm.h
#lds.obj: lds.cpp lds.h protrack.h
mid.obj: mid.cpp mid.h mididata.h
mkj.obj: mkj.cpp mkj.h
mtk.obj: mtk.cpp mtk.h hsc.h
rad.obj: rad.cpp rad.h protrack.h
raw.obj: raw.cpp raw.h
s3m.obj: s3m.cpp s3m.h
sa2.obj: sa2.cpp sa2.h protrack.h
sng.obj: sng.cpp sng.h
