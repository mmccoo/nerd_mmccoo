
#ifndef WAV_UTILS_HH
#define WAV_UTILS_HH

#include <complex>
#include <tuple>
#include <list>
#include <vector>
#include <memory>
#include <cassert>
#include <chrono>

class mytimer {
public:
  mytimer() { reset(); }
  void reset() { begin = std::chrono::high_resolution_clock::now(); }

  double duration() {
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()/1000000000.0;
  }
  
private:
  std::chrono::time_point<std::chrono::system_clock> begin;
};



class Timeline;
class track_data;
struct wav_samples {
  wav_samples() : channels(0), frames(0), samplerate(0), samples(0x0), fft(0x0), timeline(0x0) { /* empty */ };
  int                        channels;
  long int                   frames;
  int                        samplerate;
  double*                    samples;     
  std::complex<double> *     fft;
  std::string                filename;
  std::shared_ptr<Timeline>  timeline;

  // stuff needed in kdenlive context.
  std::string producer;
  std::string resource; // this could be the same as filename except filename will be a downsampled version.
  track_data *td;

};

struct correlation_data {
  correlation_data() :
    wss1(0x0), wss2(0x0), offset(0),
    num_std(0), cross_correlation(0x0) { /* empty */ }
  correlation_data(wav_samples *wss1, wav_samples *wss2, int offset, double num_std) :
    wss1(wss1), wss2(wss2), offset(offset),
    num_std(num_std), cross_correlation(0) { /* empty */ }

  void write_wav(const std::string &filename);
  
  wav_samples *wss1;
  wav_samples *wss2;
  int offset;
  double num_std;

  std::complex<double> *cross_correlation;
  
};

class res_info;
class Timeline {
public:
  struct clip {
    wav_samples* wss;
    long int     offset;    
  };

  void addClip(wav_samples* wss,
               long int     offset) {
    assert(wss->timeline.get() != this);

    auto iter = clips.begin();
    for(/*empty*/; iter !=  clips.end(); iter++) {
      if ((*iter).offset > offset) {
        break;
      }
    }

    clips.insert(iter, {wss, offset});
  }

  long int wsOffset(wav_samples* wss) {
    for(auto iter = clips.begin(); iter !=  clips.end(); iter++) {
      if ((*iter).wss == wss) {
        return (*iter).offset;        
      }
    }
    // shouldn't be here. Why didn't we find wss?
    assert(0);
    return 0;
  }
  
  std::list<clip>& getClips() { return clips; }
  
private:
  std::list<clip> clips;
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
               int          samplerate,
               bool         savecrosscorrelation=false);


std::ostream&
operator<< (std::ostream& stream, Timeline& tl);

void
mergeTimelines(std::shared_ptr<Timeline> tl1,
               std::shared_ptr<Timeline> tl2,
               long int offset);

#endif
