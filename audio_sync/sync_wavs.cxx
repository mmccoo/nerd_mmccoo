

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

  bool include_cc = false;
  if (argc > 3) {
    if (std::string(argv[3]) == "true") {
      include_cc = true;
    } else {
      printf("unknown third option. expecting true, got %s\n", argv[3]);
      exit(-1);
    }
  }
  
  wav_samples ws1;
  ws1.filename = argv[1];
  read_wav(ws1);
  wav_samples ws2;
  ws2.filename = argv[2];
  read_wav(ws2);

  // I want the fftsize to be twice the sample length. Why?
  // If ws2 preceeds ws1, then the peak will be in the second
  // half of the cross-correlation.
  // What if ws1 preceeds ws2 by more than half of the sample
  // length? In that case, the peak will also be in the second
  // half.
  // By using twice the length, that second case will have a peak
  // in the first half. A peak in the second half will only be
  // possible with case 1.
  int fftsize = std::max(ws1.frames, ws2.frames)*2;
  std::cout << "need fft size of " << fftsize << std::endl;

  add_fft(ws1, fftsize);
  add_fft(ws2, fftsize);

  correlation_data cd = correlate_wavs(ws1, ws2, fftsize, ws1.samplerate, include_cc);

  std::cout << "offset is " << double(cd.offset)/double(ws1.samplerate) << std::endl;
  
  const int channels = (include_cc ? 4 : 2);

  // have to divide by two since we multiplied above.
  const int numaligned = fftsize/2+std::abs(cd.offset);
  
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

  if (include_cc) {
    double maxval = 0;
    
    for(int i=0; i<fftsize/2; i++) {
      maxval = std::max(maxval, std::abs(cd.cross_correlation[i]));
    }    
    for(int i=0; i<fftsize/2; i++) {
      samples[channels*i+2] =           cd.cross_correlation[i].real()/maxval;
      samples[channels*i+3] =           cd.cross_correlation[i].imag()/maxval;
    }
  }
  
  write_wav(samples, numaligned, cd.wss1->samplerate, channels, std::string("aligned.wav"));
  
}
