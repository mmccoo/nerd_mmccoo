
#include <complex.h>
#include <fftw3.h>

#include <stdio.h>
#include <sndfile.h>

#include <iostream>
#include <math.h>
#include <algorithm>
#include <chrono>
#include <cassert>

// in this example, I write a sine wav to channel 1 for a period starting at 10% and lasting 10% of the total data.
// To the second channel, I write a similar sine wav, also for a duration or 10%. It is different from the
// first in that it begins at 20%

// I then FFT both, multiple the results FFT1 * complex_conjugate(FFT2) at each position.

// last, I inverse FFT that and save to channels 3/4 (real/imaginary)

int main(int argc, char** argv) {

  const int samplerate = 44100;

  // 2 seconds
  const int numsamples = samplerate*600;
  const double freq = 1000;
  
  if (argc < 2) {
    std::cerr << "need wav filename" << std::endl;
    exit(-1);
  }
  
  char *filename = argv[1];
  std::cout << "writing wav " << filename << std::endl;

  SF_INFO sfinfo;
  // http://www.mega-nerd.com/libsndfile/api.html#open
  sfinfo.format = SF_FORMAT_WAV|SF_FORMAT_PCM_16;
  sfinfo.samplerate = samplerate;
#ifdef SHOW_INTERMEDIATE
  sfinfo.channels = 8;
#else
  sfinfo.channels = 4;
#endif
  
  SNDFILE* sfile = sf_open(filename, SFM_WRITE, &sfinfo);
  if (sfile == NULL) {
    std::cout << "unable to open file " << filename << std::endl;
    std::cout << "error " << sf_strerror(sfile) << std::endl;
    exit(-1);
  }

  // http://www.fftw.org/fftw3_doc/Complex-One_002dDimensional-DFTs.html#Complex-One_002dDimensional-DFTs
  std::complex<double> *in   = (std::complex<double>*) fftw_malloc(sizeof(std::complex<double>) * numsamples);
  std::complex<double> *out1 = (std::complex<double>*) fftw_malloc(sizeof(std::complex<double>) * numsamples);
  std::complex<double> *out2 = (std::complex<double>*) fftw_malloc(sizeof(std::complex<double>) * numsamples);
  fftw_plan p_forward1  = fftw_plan_dft_1d(numsamples,
                                           reinterpret_cast<fftw_complex*>(in),
                                           reinterpret_cast<fftw_complex*>(out1),
                                           FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan p_forward2  = fftw_plan_dft_1d(numsamples,
                                           reinterpret_cast<fftw_complex*>(in),
                                           reinterpret_cast<fftw_complex*>(out2),
                                           FFTW_FORWARD, FFTW_ESTIMATE);
  fftw_plan p_backward = fftw_plan_dft_1d(numsamples,
                                          reinterpret_cast<fftw_complex*>(in),
                                          reinterpret_cast<fftw_complex*>(out1),
                                          FFTW_BACKWARD, FFTW_ESTIMATE);

  // use malloc instead of stack array (ie samples[numsamples*channel]) as this will likely
  // exceed stack size limit.
  double* samples = (double*) malloc(numsamples*sfinfo.channels*sizeof(double));

  const int baseline_length = numsamples/10;
  double* baseline = (double*) malloc(baseline_length*sizeof(double));
  for(int i=0; i<baseline_length; i++) {
    baseline[i] = sin(2*M_PI*freq*i/samplerate);
    //baseline[i] = double(i)/baseline_length;
  }

  
  int offsets[2] = { 1*numsamples/10, 2*numsamples/10 };
  fftw_plan plans[2] = { p_forward1, p_forward2};
  for (int chan=0; chan<2; chan++) {
    for(int i=0; i<numsamples; i++) {
      in[i] = samples[i*sfinfo.channels+chan] = 0;
    }
    assert(numsamples >= (baseline_length+offsets[chan]));
    for(int bi=0; bi<baseline_length; bi++) {
      int index = bi+offsets[chan];
      in[index] = samples[index*sfinfo.channels+chan] = baseline[bi];
    }

    auto begin = std::chrono::high_resolution_clock::now();
    fftw_execute(plans[chan]); /* repeat as needed */
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()/1000000000.0 << "sec" << std::endl;    
  }

  double maxval=0;
  for (int i=0; i<numsamples; i++) {
    maxval = std::max({
          maxval,
          std::abs(out1[i].real()),
          std::abs(out1[i].imag()),
          std::abs(out2[i].real()),
          std::abs(out2[i].imag())});
  }
  // piecewise multiple the FFT
  for (int i=0; i<numsamples; i++) {
    in[i] = std::conj(out1[i]) * out2[i];
#ifdef SHOW_INTERMEDIATE
    samples[i*sfinfo.channels+2] = out1[i].real()/maxval;
    samples[i*sfinfo.channels+3] = out1[i].imag()/maxval;
    samples[i*sfinfo.channels+4] = out1[i].real()/maxval;
    samples[i*sfinfo.channels+5] = out1[i].imag()/maxval;
#endif
  }
  fftw_execute(p_backward);

  maxval = 0;
  double maxcorrel=0;
  double maxcorrel_id=0;
  for (int i=0; i<numsamples; i++) {
    maxval = std::max({
          maxval,
          std::abs(out1[i].real()),
          std::abs(out1[i].imag())});
    double correl = std::abs(out1[i]);
    if (correl>maxcorrel) {
      maxcorrel = correl;
      maxcorrel_id = i;
    }
  }
  
  for(int i=0; i<numsamples; i++) {
    samples[i*sfinfo.channels+sfinfo.channels-2] = out1[i].real()/maxval;
    samples[i*sfinfo.channels+sfinfo.channels-1] = out1[i].imag()/maxval;
  }

  // freq corresponds to i*samplerate/numsamples. if numsamples=2*samplerate, f=i/2.
  // the max correlation id will be positive if the first input ahead of the second. ie if in1(t) ~= in2(t+maxcorrel_id)
  if (maxcorrel_id > numsamples/2) {
    // we want a negative number here.
    maxcorrel_id = maxcorrel_id-numsamples;
  }
  std::cout << "num samples " << numsamples << std::endl;
  std::cout << "correlation offset = " << maxcorrel_id << "(" << double(maxcorrel_id)/numsamples << ")" << std::endl;
  sf_writef_double(sfile, samples, numsamples);
  
  sf_close(sfile);

  fftw_destroy_plan(p_forward1);
  fftw_destroy_plan(p_forward2);
  fftw_destroy_plan(p_backward);
  fftw_free(in);
  fftw_free(out1);
  fftw_free(out2);

}
