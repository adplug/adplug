all: txtgfx.obj window.obj .symbolic

txtgfx.obj: txtgfx.c
	wcc386 txtgfx.c

window.obj: window.cpp
	wpp386 window.cpp

clean: .symbolic
	del *.obj
