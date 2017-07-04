include ../nall/Makefile
include ../phoenix/Makefile

application := beat
flags := $(flags) -std=gnu++11 -O3 -fomit-frame-pointer -I..
link := $(flags) -s
objects := obj/phoenix.o obj/$(application).o

ifeq ($(platform),win)
  objects += obj/resource.o
endif

all: $(objects)
ifeq ($(platform),win)
endif
	$(compiler) -o $(application) $(objects) $(link) $(phoenixlink)

obj/resource.o:
	windres --target=pe-i386 ../phoenix/windows/phoenix.rc obj/resource.o

obj/phoenix.o:
	$(compiler) $(phoenixflags) -o obj/phoenix.o -c ../phoenix/phoenix.cpp

obj/$(application).o: $(application).cpp
	$(compiler) $(cppflags) $(flags) -o obj/$(application).o -c $(application).cpp

resource: force
	sourcery resource/resource.xml resource/resource.cpp resource/resource.hpp

install:
	sudo cp $(application) /usr/local/bin/$(application)

uninstall:
	sudo rm /usr/local/bin/$(application)

clean:
	-@$(call delete,obj/*.o)

force:
