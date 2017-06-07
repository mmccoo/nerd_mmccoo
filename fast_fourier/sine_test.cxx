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
  fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * numsamples);
  fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * numsamples);
  fftw_plan p = fftw_plan_dft_1d(numsamples, in, out, FFTW_FORWARD, FFTW_ESTIMATE);

  double samples[numsamples*sfinfo.channels];

  for(int i=0; i<numsamples; i++) {
    samples[i*sfinfo.channels]   = in[i][0] = sin(2*M_PI*freq*i/sfinfo.samplerate);
    samples[i*sfinfo.channels+1] = in[i][1] = 0;
  }

  auto begin = std::chrono::high_resolution_clock::now();
  fftw_execute(p); /* repeat as needed */
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()/1000000000.0 << "ns" << std::endl;

  double minval = 0;
  int minid=0;
  double maxval = 0;
  int maxid=0;
  for(int i=0; i<numsamples; i++) {
    samples[i*sfinfo.channels+2] = out[i][0];
    samples[i*sfinfo.channels+3] = out[i][1];
    if (std::min(out[i][0], out[i][1]) < minval) {
      minval = std::min(out[i][0], out[i][1]);
      minid = i;
    }
    if (std::max(out[i][0], out[i][1]) > maxval) {
      maxval = std::max(out[i][0], out[i][1]);
      maxid = i;
    }
  }

  // freq corresponds to i*samplerate/numsamples. if numsamples=2*samplerate, f=i/2.
  std::cout << "min " << minval << "(" << double(minid)*samplerate/numsamples << ") max ";
  std::cout << maxval << "(" << double(maxid)*samplerate/numsamples << ")" << std::endl;
  std::cout << "num samples " << numsamples << std::endl;
  sf_writef_double(sfile, samples, numsamples);
  
  sf_close(sfile);

  fftw_destroy_plan(p);
  fftw_free(in);
  fftw_free(out);

}
