

#include <wav_utils.hxx>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>

#include <iostream>
#include <algorithm>
#include <chrono>
#include <set>

///stats.hpp>
//#include <boost/accumulators/statistics/mean.hpp>
//#include <boost/accumulators/statistics/variance.hpp>

// this program is a lot like sync_wavs.cxx
// in this one, I read a bunch of wav files and find the points of maximum correlation.

// ffmpeg -i sample_video/htc/VIDEO0635.mp4 -map 0:a -ac 1 tmp/htc.wav
// ffmpeg -i sample_video/s5/VID_20170608_152423.mp4 -map 0:a tmp/s5.wav
// find . | xargs -I file ffmpeg -i file -map 0:a -ac 1 -ar 10000 -y ../tmp10k/file

// https://en.wikipedia.org/wiki/Cross-correlation
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

std::ostream&
operator<< (std::ostream& stream,
            Timeline& tl)
{
  stream << "timeline \n";
  BOOST_FOREACH(auto cl, tl.getClips()) {
    stream << "  " << cl.wss->filename << ":" << cl.offset/cl.wss->samplerate << std::endl;
  }

  return stream;
}

void
mergeTimelines(std::shared_ptr<Timeline> tl1,
               std::shared_ptr<Timeline> tl2,
               long int offset) {
  
  std::cout << "merging " << *tl1 << " with " << *tl2 << "    offset " << offset/double(tl1->getClips().begin()->wss->samplerate) << std::endl;

  BOOST_FOREACH(Timeline::clip c, tl2->getClips()) {
    tl1->addClip(c.wss, c.offset+offset);
    c.wss->timeline = tl1;
  }

  tl2->getClips().clear();
}

int main(int argc, char** argv) {

  if (argc < 3) {
    std::cerr << "need at least two wav filenames" << std::endl;
    exit(-1);
  }

  boost::accumulators::accumulator_set<long int,
                                       boost::accumulators::features<boost::accumulators::tag::variance,
                                                                     boost::accumulators::tag::density> >
    acc(boost::accumulators::tag::density::num_bins = 5,
        boost::accumulators::tag::density::cache_size = 5);
  typedef boost::iterator_range<std::vector<std::pair<double, double> >::iterator > histogram_type; 
 

  std::vector<wav_samples> wss;
  for(int i=1; i<argc; i++) {
    wss.push_back(wav_samples());
    wss[i-1].filename = argv[i];
    read_wav(wss[i-1]);
    acc(wss[i-1].frames);
  }

  std::cout << "mean " << boost::accumulators::mean(acc)
            << " variance " << sqrt(boost::accumulators::variance(acc))
            << " max " << boost::accumulators::max(acc)
            << " min " << boost::accumulators::min(acc)
            << std::endl;

#if 0
  histogram_type hist = boost::accumulators::density(acc);
  for( int i = 0; i < hist.size(); i++ ) 
  {
    std::cout << "Bin lower bound: " << hist[i].first << ", Value: " << hist[i].second << std::endl; 
  }
#endif
  
  std::sort(wss.begin(), wss.end(),
        [] (const wav_samples& struct1, const wav_samples& struct2)
        {
            return (struct1.frames > struct2.frames);
        }
    );

  long int framebin = boost::accumulators::max(acc)-sqrt(boost::accumulators::variance(acc));
  BOOST_FOREACH(wav_samples& ws, wss) {
    if (ws.frames < framebin) {
      framebin -=  sqrt(boost::accumulators::variance(acc));
      std::cout << "new bin " << framebin << std::endl;
    }
    std::cout << ws.filename << " " << ws.frames << std::endl;
  }

  // I multiple by two so I can tell the different between a shifted after b and
  // b shifted after a.
  int fftsize = boost::accumulators::max(acc)*2;
  std::cout << "need fft size of " << fftsize << std::endl;

  mytimer mt;
  BOOST_FOREACH(wav_samples& ws, wss) {
    add_fft(ws, fftsize);
  }
  std::cout << "time for all ffts " << mt.duration() << std::endl;

  std::vector<correlation_data> correlations;
  mt.reset();
  for(int i=0; i<wss.size(); i++) {
    for (int j=i+1; j<wss.size(); j++) {
      correlation_data cd = correlate_wavs(wss[i], wss[j], fftsize, wss[i].samplerate);
      //std:: cout << wss[i].filename << " " << wss[j].filename << " offset " << offset << " num std " << num_std << std::endl;
      correlations.push_back(cd);
    }
  }
  std::cout << "time for all pairs correlations " << mt.duration() << std::endl;

  std::sort(correlations.begin(), correlations.end(), [](auto const &t1, auto const &t2) {
      return t1.num_std > t2.num_std;
    });

  
  for(int i=0; i<correlations.size(); i++) {
    std:: cout << correlations[i].wss1->filename << " " << correlations[i].wss2->filename << " offset " << correlations[i].offset/double(correlations[i].wss1->samplerate) << " num std " << correlations[i].num_std << std::endl;

    correlation_data cd = correlations[i];
    // is the correlation reliable enough?
    if (cd.num_std < 10) { continue; }


    // the two wss are already on the same non-null timeline.
    if (cd.wss1->timeline && (cd.wss1->timeline == cd.wss2->timeline)) {
      continue;
    }

    long int tl_offset1=0;
    if (!cd.wss1->timeline) {
      std::shared_ptr<Timeline> timeline(new Timeline());
      timeline->addClip(cd.wss1, 0);
      cd.wss1->timeline = timeline;
      tl_offset1 = 0;
    } else {
      tl_offset1 = cd.wss1->timeline->wsOffset(cd.wss1);
    }

    long int tl_offset2=0;
    if (!cd.wss2->timeline) {
      std::shared_ptr<Timeline> timeline(new Timeline);
      timeline->addClip(cd.wss2, 0);
      cd.wss2->timeline = timeline;
      tl_offset2 = 0;
    } else {
      tl_offset2 = cd.wss2->timeline->wsOffset(cd.wss2);
    }

    if ((tl_offset1+cd.offset) > tl_offset2) {
      mergeTimelines(cd.wss1->timeline, cd.wss2->timeline,
                     tl_offset1+cd.offset-tl_offset2);
    } else {
      mergeTimelines(cd.wss2->timeline, cd.wss1->timeline,
                     tl_offset2-(tl_offset1+cd.offset));

    }
  }

  std::set<std::shared_ptr<Timeline> > timelines;
  BOOST_FOREACH(wav_samples& ws, wss) {
    timelines.insert(ws.timeline);
  }

  std::cout << "num timelines " << timelines.size() << std::endl;
  int tlnum=0;
  BOOST_FOREACH(auto tl, timelines) {
    std::cout << "timeline" << std::endl;
    long int length = 0;
    int samplerate = 0;
    BOOST_FOREACH(auto clip, tl->getClips()) {
      std:: cout << "  " << clip.wss->filename << " offset " << clip.offset << std::endl;
      length = std::max(length, clip.wss->frames+clip.offset);
      samplerate = clip.wss->samplerate;
    }
    const int channels = wss.size();
    std::vector<double> samples(length*channels, 0);
    int clipnum=0;
    BOOST_FOREACH(auto clip, tl->getClips()) {
      for(int i=0; i<clip.wss->frames; i++) {
        samples[(i+clip.offset)*channels+clipnum] = clip.wss->samples[i];
      }
      clipnum++;
    }

    write_wav(samples, length, samplerate, channels, std::string("aligned") + std::to_string(tlnum) + ".wav");
    tlnum++;
  }

}
