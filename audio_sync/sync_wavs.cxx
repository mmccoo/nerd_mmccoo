

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

  if (argc < 3) {
    std::cerr << "need two wav filenames" << std::endl;
    exit(-1);
  }
  
  wav_samples ws1;
  ws1.filename = argv[1];
  read_wav(ws1);
  wav_samples ws2;
  ws2.filename = argv[2];
  read_wav(ws2);
  
  int fftsize = std::max(ws1.frames, ws2.frames);
  std::cout << "need fft size of " << fftsize << std::endl;

  add_fft(ws1, fftsize);
  add_fft(ws2, fftsize);

  correlation_data cd = correlate_wavs(ws1, ws2, fftsize, ws1.samplerate);

  const int channels = 2;
  const int numaligned = fftsize+std::abs(cd.offset);
  std::vector<double> samples(numaligned*channels,0);

  for(int i=0; i<numaligned; i++) {
    samples[channels*i] = 0;
    samples[channels*i+1] = 0;
  }

  // in correlation_data struct, wss1 will be earlier in the timeline.
  std::cout << cd.wss1->filename << " is ahead of " << cd.wss2->filename
            << " by " << double(cd.offset)/cd.wss1->samplerate << std::endl;

  for(int i=0; i<cd.wss1->frames; i++) {
    samples[channels*i] =               cd.wss1->samples[cd.wss1->channels*i];
  }
  for(int i=0; i<cd.wss2->frames; i++) {
    // the +1 is because we want the second channel.
    samples[channels*(i+cd.offset)+1] = cd.wss2->samples[cd.wss2->channels*i];
  }

  write_wav(samples, numaligned, cd.wss1->samplerate, channels, std::string("aligned.wav"));
  
}
