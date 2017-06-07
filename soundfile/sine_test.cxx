#include <stdio.h>
#include <sndfile.h>
#include <complex.h>

#include <iostream>
#include <math.h>   
int main(int argc, char** argv) {

  char *filename = argv[1];
  std::cout << "writing wav " << filename << std::endl;

  SF_INFO sfinfo;
  // http://www.mega-nerd.com/libsndfile/api.html#open
  sfinfo.format = SF_FORMAT_WAV|SF_FORMAT_PCM_16;
  sfinfo.samplerate = 44100;
  sfinfo.channels = 1;
  
  SNDFILE* sfile = sf_open(filename, SFM_WRITE, &sfinfo);
  if (sfile == NULL) {
    std::cout << "unable to open file " << filename << std::endl;
    std::cout << "error " << sf_strerror(sfile) << std::endl;
    exit(-1);
  }

  const int numsamples = sfinfo.samplerate*2;
  double samples[numsamples*sfinfo.channels];
  const double freq = 1000;
  for(int i=0; i<numsamples; i++) {
    for(int channel = 0; channel<sfinfo.channels; channel++) {
      samples[i*sfinfo.channels+channel] = sin(2*M_PI*freq*i/sfinfo.samplerate);
    }
  }
  sf_write_double(sfile, samples, numsamples);
  
  sf_close(sfile);
}
