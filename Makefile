
CC 	:= gcc
CFLAGS := -I. -O2 -Wall -funroll-loops -ffast-math -fPIC -DPIC
LD := gcc
LDFLAGS := -O2 -Wall -shared -lasound 

SND_PCM_OBJECTS = ext_plugin_sample.o dummy_algo.o
SND_PCM_LIBS =
SND_PCM_BIN = libasound_module_pcm_myplug.so

.PHONY: all clean install uninstall

all: $(SND_PCM_BIN)

$(SND_PCM_BIN): $(SND_PCM_OBJECTS)
	@echo LD $@
	$(LD) $(LDFLAGS) $(SND_PCM_LIBS) $(SND_PCM_OBJECTS) -o $(SND_PCM_BIN)

%.o: %.c
	@echo GCC $<
	$(CC) -c $(CFLAGS) $<

clean:
	@echo Cleaning...
	$(Q)rm -vf *.o *.so

install: all
	@echo Installing...
	install -m 644 $(SND_PCM_BIN) ${DESTDIR}/usr/lib/x86_64-linux-gnu/alsa-lib/

uninstall:
	@echo Un-installing...
	rm ${DESTDIR}/usr/lib/x86_64-linux-gnu/alsa-lib/$(SND_PCM_BIN)


