csources := $(wildcard src/*.c)
cppsources := $(wildcard src/*.cpp)
cobjects := $(csources:src/%.c=cobj/%.o)
cppobjects := $(cppsources:src/%.cpp=obj/%.o)
cdeps := $(cobjects:.o=.d)
cppdeps := $(cppobjects:.o=.d)

CC := x86_64-w64-mingw32-gcc
CXX := x86_64-w64-mingw32-g++

CFLAGS := -m64 -fopenmp
CPPFLAGS := -m64 -MMD -MP -fopenmp

LDLIBS := -Iinclude -Llib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf -lSDL2_image \
-lcomdlg32 -lnfd -lole32 -loleaut32 -luuid -lhpdf -lz -lKiWi -Wno-narrowing

bin/hanjiestudio.exe: $(cppobjects) $(cobjects)
	$(CXX) $(CPPFLAGS) $^ $(LDLIBS) -o $@

obj/%.o: src/%.cpp
	$(CXX) $(CPPFLAGS) -c -O3 $(OUTPUT_OPTION) $<

cobj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -O3 $(OUTPUT_OPTION) $<

clean:
	-del $(subst /,\,$(cppobjects) $(cobjects) $(deps)) bin\hanjiestudio.exe

run:
	bin/hanjiestudio.exe

-include $(deps)
