all: .symbolic
	wpp386 timer.cpp
	wpp386 irq.cpp
	wpp386 pindos.cpp
	wpp386 dpmi.cpp
	wlib -n -b timer.lib +timer +irq +pindos +dpmi

clean: .symbolic
	del *.obj

distclean: clean .symbolic
	del timer.lib
