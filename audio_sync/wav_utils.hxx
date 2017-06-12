
#ifndef WAV_UTILS_HH
#define WAV_UTILS_HH

#include <complex>
#include <tuple>
#include <list>
#include <vector>
#include <memory>

class Timeline;
struct wav_samples {
  wav_samples() : channels(0), frames(0), samplerate(0), samples(0x0), fft(0x0), timeline(0x0) { /* empty */ };
  int                        channels;
  long int                   frames;
  int                        samplerate;
  double*                    samples;     
  std::complex<double> *     fft;
  std::string                filename;
  std::shared_ptr<Timeline>  timeline;
};

struct correlation_data {
  correlation_data(wav_samples *wss1, wav_samples *wss2, int offset, double num_std) :
    wss1(wss1), wss2(wss2), offset(offset), num_std(num_std) { /* empty */ }

  wav_samples *wss1;
  wav_samples *wss2;
  int offset;
  double num_std;
  
};




bool read_wav(wav_samples& ws);
void add_fft(wav_samples& ws, long int fftsize);
void write_wav(std::vector<double>& samples,
               long int length,
               int samplerate,
               int channels,
               const std::string &filename);

void write_wav_for_fft(std::complex<double> *fft,
                       long int fftsize,
                       int samplerate,
                       const char* filename);

correlation_data
correlate_wavs(wav_samples& ws1,
               wav_samples& ws2,
               long int     fftsize,
               int          samplerate);


#endif
