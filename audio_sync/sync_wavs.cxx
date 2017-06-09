

#include <wav_utils.hxx>


#include <iostream>

///stats.hpp>
//#include <boost/accumulators/statistics/mean.hpp>
//#include <boost/accumulators/statistics/variance.hpp>

// this program is a lot like basic_convolution.cxx in ../fast_fourier.
// in this one, I read two wav files and find the point of maximum correlation.

// I then FFT both, multiple the results FFT1 * complex_conjugate(FFT2) at each position.

// last, I inverse FFT that and save to channels 3/4 (real/imaginary)


// ffmpeg -i sample_video/htc/VIDEO0635.mp4 -map 0:a -ac 1 tmp/htc.wav
// ffmpeg -i sample_video/s5/VID_20170608_152423.mp4 -map 0:a tmp/s5.wav
// find . | xargs -I file ffmpeg -i file -map 0:a -ac 1 -ar 10000 -y ../tmp10k/file

int main(int argc, char** argv) {

  const int samplerate = 44100;

  // 2 seconds
  const int numsamples = samplerate*600;
  const double freq = 1000;
  
  if (argc < 3) {
    std::cerr << "need two wav filenames" << std::endl;
    exit(-1);
  }
  
  char *filename1 = argv[1];
  char *filename2 = argv[2];
  std::cout << "reading wavs " << filename1 << " and " << filename2 << std::endl;

  wav_samples ws1 = read_wav(filename1);
  wav_samples ws2 = read_wav(filename2);
  
  int fftsize = std::max(ws1.frames, ws2.frames);
  std::cout << "need fft size of " << fftsize << std::endl;

  add_fft(ws1, fftsize);
  add_fft(ws2, fftsize);

  int offset = correlate_wavs(ws1, ws2, fftsize, ws1.samplerate);

  const int channels = 2;
  const int numaligned = fftsize+std::abs(offset);
  double* samples = (double*) malloc(numaligned*channels*sizeof(double));

  for(int i=0; i<numaligned; i++) {
    samples[2*i] = 0;
    samples[2*i+1] = 0;
  }

  // the correlation id will be positive if the first input ahead of the second.
  // seems to be reversed in reality.
  int offset1, offset2;
  if (offset < 0) {
    std::cout << filename1 << " is ahead of " << filename2 << " by " << -double(offset)/ws1.samplerate << std::endl;
    offset1 = 0;
    offset2 = -offset;
  } else {
    std::cout << filename2 << " is ahead of " << filename1 << " by " << double(offset)/ws1.samplerate << std::endl;
    offset1 = offset;
    offset2 = 0;
  }

  for(int i=0; i<ws1.frames; i++) {
    samples[channels*(i+offset1)] = ws1.samples[ws1.channels*i];      
  }
  for(int i=0; i<ws2.frames; i++) {
    samples[channels*(i+offset2)+1] = ws2.samples[ws2.channels*i];      
  }

  write_wav(samples, numaligned, ws1.samplerate, channels, "aligned.wav");
  
}
