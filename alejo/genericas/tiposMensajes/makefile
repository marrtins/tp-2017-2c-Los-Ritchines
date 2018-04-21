H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')

install: all
	cp --parents -u $(H_SRCS) /usr/include

uninstall:
	rm -rf /usr/include/tiposMensajes

.PHONY: all create-dirs clean install uninstall
