
#ifndef WAV_UTILS_HH
#define WAV_UTILS_HH

#include <complex>

struct wav_samples {
  int                   channels;
  long int              frames;
  int                   samplerate;
  double*               samples;     
  std::complex<double> *fft;
};

wav_samples read_wav(const char *filename);
void add_fft(wav_samples& ws, long int fftsize);
void write_wav(double* samples,
               long int length,
               int samplerate,
               int channels,
               const char* filename);

void write_wav_for_fft(std::complex<double> *fft,
                       long int fftsize,
                       int samplerate,
                       const char* filename);

int correlate_wavs(wav_samples& ws1,
                   wav_samples& ws2,
                   long int     fftsize,
                   int          samplerate);


#endif
