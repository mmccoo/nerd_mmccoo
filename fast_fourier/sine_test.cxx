#include <stdio.h>
#include <sndfile.h>
#include <complex.h>
#include <fftw3.h>

#include <iostream>
#include <math.h>
#include <algorithm>
#include <chrono>

int main(int argc, char** argv) {

  const int samplerate = 44100;

  // 2 seconds
  const int numsamples = samplerate*2;
  const double freq = 1000;

  char *filename = argv[1];
  std::cout << "writing wav " << filename << std::endl;

  SF_INFO sfinfo;
  // http://www.mega-nerd.com/libsndfile/api.html#open
  sfinfo.format = SF_FORMAT_WAV|SF_FORMAT_PCM_16;
  sfinfo.samplerate = samplerate;
  sfinfo.channels = 4;
  
  SNDFILE* sfile = sf_open(filename, SFM_WRITE, &sfinfo);
  if (sfile == NULL) {
    std::cout << "unable to open file " << filename << std::endl;
    std::cout << "error " << sf_strerror(sfile) << std::endl;
    exit(-1);
  }

  // http://www.fftw.org/fftw3_doc/Complex-One_002dDimensional-DFTs.html#Complex-One_002dDimensional-DFTs
  std::complex<double> *in  = (std::complex<double>*) fftw_malloc(sizeof(fftw_complex) * numsamples);
  std::complex<double> *out = (std::complex<double>*) fftw_malloc(sizeof(fftw_complex) * numsamples);
  fftw_plan p = fftw_plan_dft_1d(numsamples,
                                 reinterpret_cast<fftw_complex*>(in),
                                 reinterpret_cast<fftw_complex*>(out),
                                 FFTW_FORWARD,  FFTW_ESTIMATE);

  double samples[numsamples*sfinfo.channels];

  for(int i=0; i<numsamples; i++) {
    in[i] = sin(2*M_PI*freq*i/sfinfo.samplerate);
    samples[i*sfinfo.channels]   = in[i].real();
    samples[i*sfinfo.channels+1] = in[i].imag();
  }

  auto begin = std::chrono::high_resolution_clock::now();
  fftw_execute(p); /* repeat as needed */
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()/1000000000.0 << "ns" << std::endl;

  // freq corresponds to i*samplerate/numsamples. if numsamples=2*samplerate, f=i/2.
  for(int i=0; i<numsamples; i++) {
    samples[i*sfinfo.channels+2] = out[i].real();
    samples[i*sfinfo.channels+3] = out[i].imag();
    if (std::abs(out[i]) > 10) {
      std::cout << "at " << i << "(" << double(i)*samplerate/numsamples << "Hz) " << out[i] << std::endl;
    }
  }

  std::cout << "num samples " << numsamples << std::endl;
  sf_writef_double(sfile, samples, numsamples);
  
  sf_close(sfile);

  fftw_destroy_plan(p);
  fftw_free(in);
  fftw_free(out);

}
