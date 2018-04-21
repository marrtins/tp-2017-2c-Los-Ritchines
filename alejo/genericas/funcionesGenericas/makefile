RM=rm -rf
CC=gcc

C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')
OBJS=$(C_SRCS:./%.c=build/%.o)

# Clean and compile .so
all: build/libfuncionesGenericas.so

create-dirs:
	mkdir -p build/funcionesGenericas

build/libfuncionesGenericas.so: create-dirs $(OBJS)
	$(CC)  -shared -Wextra -Wall -o "build/libfuncionesGenericas.so" $(OBJS)

build/funcionesGenericas/%.o: funcionesGenericas/%.c
	$(CC)  -c -Wextra -Wall -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

# Clean build files
clean:
	$(RM) build

install: all
	cp -u build/libfuncionesGenericas.so /usr/lib
	cp --parents -u $(H_SRCS) /usr/include

uninstall:
	rm -f /usr/lib/libfuncionesGenericas.so
	rm -rf /usr/include/funcionesGenericas

.PHONY: all create-dirs clean install uninstall
