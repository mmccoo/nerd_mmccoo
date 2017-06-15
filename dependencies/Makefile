
$(info in depdencies makefile)

all: libsndfile-1.0.28/install/lib/libsndfile.a fftw-3.3.6-pl2/install/lib/libfftw3.a tinyxml2/libtinyxml2.a


libsndfile-1.0.28/install/lib/libsndfile.a:
	cd ../dependencies && wget http://www.mega-nerd.com/libsndfile/files/libsndfile-1.0.28.tar.gz
	cd ../dependencies && tar -xf libsndfile-1.0.28.tar.gz
	cd ../dependencies && mkdir libsndfile-1.0.28/install
	cd ../dependencies/libsndfile-1.0.28 && ./configure prefix=`realpath install`
	cd ../dependencies/libsndfile-1.0.28 && make install

fftw3: fftw-3.3.6-pl2/install/lib/libfftw3.a

fftw-3.3.6-pl2/install/lib/libfftw3.a:
	wget ftp://ftp.fftw.org/pub/fftw/fftw-3.3.6-pl2.tar.gz
	tar -xf fftw-3.3.6-pl2.tar.gz
	mkdir -p fftw-3.3.6-pl2
	cd fftw-3.3.6-pl2 && ./configure prefix=`realpath install` && make install

tinyxml2/libtinyxml2.a:
	mkdir -p ../dependencies
	git clone https://github.com/leethomason/tinyxml2.git
	cd tinyxml2 && make
	cd tinyxml2 && doxygen dox

clean:
	rm -rf libsndfile-1.0.28*
	rm -rf fftw-3.3.6-pl2*
	rm -rf tinyxml2
