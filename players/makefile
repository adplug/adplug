CXX             = gcc-3.0
CXXFLAGS        = -Wall
CPPFLAGS        = -Dstricmp=strcasecmp
PLAYERS		= protrack.o a2m.o amd.o d00.o dfm.o hsc.o hsp.o imf.o ksm.o \
			mid.o mkj.o mtk.o rad.o raw.o s3m.o sa2.o sng.o u6m.o bam.o

all: $(PLAYERS)

clean:
	rm -f $(PLAYERS)

# Replayer dependencies

$(PLAYERS): player.h
protrack.o: protrack.cpp protrack.h
a2m.o: a2m.cpp a2m.h protrack.h
amd.o: amd.cpp amd.h protrack.h
bam.o: bam.cpp bam.h
d00.o: d00.cpp d00.h
dfm.o: dfm.cpp dfm.h protrack.h
hsc.o: hsc.cpp hsc.h
hsp.o: hsp.cpp hsp.h hsc.h
imf.o: imf.cpp imf.h imfcrc.h
ksm.o: ksm.cpp ksm.h
#lds.o: lds.cpp lds.h protrack.h
mid.o: mid.cpp mid.h mididata.h
mkj.o: mkj.cpp mkj.h
mtk.o: mtk.cpp mtk.h hsc.h
rad.o: rad.cpp rad.h protrack.h
raw.o: raw.cpp raw.h
s3m.o: s3m.cpp s3m.h
sa2.o: sa2.cpp sa2.h protrack.h
sng.o: sng.cpp sng.h
u6m.o: u6m.cpp u6m.h
