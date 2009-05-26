# yes, handwritten makefile

all: rfk

rfk: rfk.c
	gcc -Wall -g rfk.c -o rfk `pkg-config --cflags --libs gtk+-2.0` `pkg-config hildon-1 --cflags --libs`

clean:
	rm -f rfk

# some of these are not needed, or are possibly in the wrong place
install: rfk rfk.xpm rfk.desktop
	install -d ${DESTDIR}/usr/bin
	install rfk ${DESTDIR}/usr/bin
	install -d ${DESTDIR}/usr/share/applications/hildon
	install rfk.desktop ${DESTDIR}/usr/share/applications/hildon
	install -d ${DESTDIR}/usr/share/pixmaps
	install rfk.xpm ${DESTDIR}/usr/share/pixmaps
	install rfk-dimmed.xpm ${DESTDIR}/usr/share/pixmaps
	install rfk-robot.png ${DESTDIR}/usr/share/pixmaps
	install rfk-love.png ${DESTDIR}/usr/share/pixmaps
	install rfk-kitten.png ${DESTDIR}/usr/share/pixmaps
	install -d ${DESTDIR}/usr/share/icons/64x64/hildon/rfk.png
	install rfk.png ${DESTDIR}/usr/share/icons/64x64/hildon/rfk.png