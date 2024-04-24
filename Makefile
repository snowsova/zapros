PREFIX = /usr/local
PROG   = zapros

all:
	$(CC) ${PROG}.c -o ${PROG}

debug:
	clang ${PROG}.c -Wall -Werror -fsanitize=undefined,address -o ${PROG}

install: all
	install -Dm755 ./${PROG}   ${PREFIX}/bin/${PROG}
	install -Dm644 ./${PROG}.1 ${PREFIX}/share/man/man1/${PROG}.1


uninstall:
	rm -f ${PREFIX}/${PROG}
	rm -f ${PREFIX}/share/man/man1/${PROG}.1


clean:
	rm -f ${PROG} *.o *.out

