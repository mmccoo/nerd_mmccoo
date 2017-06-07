
#include <stdio.h>
#include <sndfile.h>

#include <iostream>


int main(int argc, char** argv) {

  char *filename = argv[1];
  std::cout << "getting props for " << filename << std::endl;

  SF_INFO sfinfo;
  // http://www.mega-nerd.com/libsndfile/api.html#open
  // When opening a file for read, the format field should be set to zero before calling sf_open().
  sfinfo.format = 0x0;
  
  SNDFILE* sfile = sf_open(filename, SFM_READ, &sfinfo);
  if (sfile == NULL) {
    std::cout << "unable to open file " << filename << std::endl;
    std::cout << "error " << sf_strerror(sfile) << std::endl;
    exit(-1);
  }
 
  if (sfinfo.format & SF_FORMAT_WAV) {
    std::cout << "file is a wav file" << std::endl;
    if (sfinfo.format & SF_FORMAT_PCM_S8) {
      std::cout << "  8 bit signed" << std::endl;
    } else     if (sfinfo.format & SF_FORMAT_PCM_16) {
      std::cout << "  16 bit" << std::endl;
    } else     if (sfinfo.format & SF_FORMAT_PCM_24) {
      std::cout << "  24 bit" << std::endl;
    } else     if (sfinfo.format & SF_FORMAT_PCM_32) {
      std::cout << "  32 bit" << std::endl;
    } else {
      std::cout << "  unknown wav type" << std::endl;
    }
  } else {
    std::cout << "file is some other format" << std::endl;
  }
  std::cout << "num channels    " << sfinfo.channels << std::endl;
  std::cout << "frames          " << sfinfo.frames   << std::endl;
  std::cout << "sample rate     " << sfinfo.samplerate << std::endl;
      
  sf_close(sfile);
}
