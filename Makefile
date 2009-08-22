# yes, handwritten makefile

all: rfk

rfk: rfk.c
	gcc -Wall -g rfk.c -o rfk `pkg-config --cflags --libs gtk+-2.0 hildon-1 dbus-glib-1 dbus-1`

clean:
	rm -f rfk

install: rfk rfk.png rfk-robot.png rfk-love.png rfk-kitten.png rfk.desktop non-kitten-items.rfk help.html
	install -d ${DESTDIR}/usr/bin
	install rfk ${DESTDIR}/usr/bin
	install -d ${DESTDIR}/usr/share/applications/hildon
	install rfk.desktop ${DESTDIR}/usr/share/applications/hildon
	install -d ${DESTDIR}/usr/share/dbus-1/services
	install org.robotfindskitten.service ${DESTDIR}/usr/share/dbus-1/services
	install -d ${DESTDIR}/usr/share/pixmaps
	install rfk.png ${DESTDIR}/usr/share/pixmaps
	install -d ${DESTDIR}/usr/share/rfk
	install rfk-robot.png ${DESTDIR}/usr/share/rfk
	install rfk-love.png ${DESTDIR}/usr/share/rfk
	install rfk-kitten.png ${DESTDIR}/usr/share/rfk
	install non-kitten-items.rfk ${DESTDIR}/usr/share/rfk
	install help.html ${DESTDIR}/usr/share/rfk
