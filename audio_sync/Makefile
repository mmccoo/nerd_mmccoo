
# for sync_kdenlive to work, you have to have ffmpeg installed

# for this makefile to work, you'll need these:
#
# sudo apt-get install libboost-all-dev
# sudo apt-get install libcgal-dev
#
# OR
#
# in ../dependencies of this repository, do
# make and it'll get boost for you

# Add .d to Make's recognized suffixes.
SUFFIXES += .d

CXX = g++


# I'm using isystem for boost to avoid warnings that originate from there.
CFLAGS = -Wall -g -I. -std=c++14 -I../dependencies/tinyxml2 -isystem../dependencies/boost_1_63_0

# to get the dependencies, cd to ../dependencies and do make all
SNDFILE_PATH=../dependencies/libsndfile-1.0.28/install/lib/pkgconfig
CFLAGS += $(shell export PKG_CONFIG_PATH=$(SNDFILE_PATH) && pkg-config --cflags sndfile)
LFLAGS =  $(shell export PKG_CONFIG_PATH=$(SNDFILE_PATH) && pkg-config --libs sndfile)

FFTW_PATH=../dependencies/fftw-3.3.6-pl2/install/lib/pkgconfig
CFLAGS += $(shell export PKG_CONFIG_PATH=$(FFTW_PATH) && pkg-config --cflags fftw3)
LFLAGS += $(shell export PKG_CONFIG_PATH=$(FFTW_PATH) && pkg-config --libs fftw3)


TARGETS = xmltest sync_wavs varianceexample sync_audio sync_kdenlive

all: $(TARGETS)

sync_wavs: wav_utils.o
sync_audio: wav_utils.o
xmltest: kdenlive_utils.o
sync_kdenlive: kdenlive_utils.o wav_utils.o

$(TARGETS): %: %.o ../dependencies/libsndfile-1.0.28/install/lib/libsndfile.a ../dependencies/fftw-3.3.6-pl2/install/lib/libfftw3.a ../dependencies/tinyxml2/libtinyxml2.a
	$(CXX) -o $@ $^ $(LFLAGS)


%.o: %.cxx
	$(CXX) -M $(CFLAGS) $< > $*.d
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(TARGETS) *.d

test: $(TARGETS)
	./xmltest sample_video/sample.kdenlive


# pull in dependency info for *existing* .o files
# got this hint here: http://scottmcpeak.com/autodepend/autodepend.html
# except I'm using wildcard instead of a hard list of obj/d files.
# it's important for this to be after the first rule. Otherwise,
# make with no arguments will use one of these.
-include $(wildcard *.d)
