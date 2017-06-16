

#include <kdenlive_utils.hxx>
#include <wav_utils.hxx>
#include <stdlib.h>
#include <set>
#include <boost/foreach.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <libgen.h>
#include <unistd.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "need filename" << std::endl;
    exit(-1);
  }

  std::string filename(argv[1]);
  char path[filename.size()+1];
  strcpy(path, filename.c_str());
  std::string kdendir = dirname(path);
  
  tinyxml2::XMLDocument doc;
  doc.LoadFile(filename.c_str());

#if 0
  std::cout << "chaning to dir: " << kdendir << std::endl;
  if (chdir(kdendir.c_str())) {
    std::cout << "unable to change to dir " << path << std::endl;
    exit(-1);
  }
#endif
  
  kdenlive_data kdata;
  doc.Accept(&kdata);

  boost::accumulators::accumulator_set<long int, boost::accumulators::features<boost::accumulators::tag::variance,
                                                                               boost::accumulators::tag::max,
                                                                               boost::accumulators::tag::min> > acc;
  
  std::vector<wav_samples> clips;
  std::set<std::string> producers;

  std::cout << "getting track resources" << std::endl;
  BOOST_FOREACH(auto &track, kdata.GetTracks()) {
    std::cout << "track " << track.GetName() << std::endl;

    BOOST_FOREACH(auto &entry, track.GetEntries()) {
      if (producers.find(entry.producer) != producers.end()) {
        continue;
      }
      std::cout << "clip " << entry.producer << "(" << entry.resource << ")" << std::endl;
      wav_samples wss;
      wss.producer = entry.producer;
      wss.resource = kdendir + "/" + entry.resource;
      wss.td = &track;
      
      char tmp[] = "convertedXXXXXX.wav";
      int fp = mkstemps(tmp, 4);
      wss.filename = kdendir + "/" + tmp;
      close(fp);
      std::string cmd = std::string("ffmpeg -i ") + wss.resource + " -map 0:a -ac 1 -ar 5000 -y " + wss.filename;
      std::cout << "executing " << cmd << std::endl;
      if (std::system(cmd.c_str())) {
        std::cerr << "got non-zero when running " << cmd << std::endl;
      }
    
      read_wav(wss);

      std::remove(wss.filename.c_str());
    
      acc(wss.frames);

      clips.push_back(wss);
    }
  }

  std::cout << "mean " << boost::accumulators::mean(acc)
            << " variance " << sqrt(boost::accumulators::variance(acc))
            << " max " << boost::accumulators::max(acc)
            << " min " << boost::accumulators::min(acc)
            << std::endl;
  
  std::sort(clips.begin(), clips.end(),
        [] (const wav_samples& struct1, const wav_samples& struct2)
        {
            return (struct1.frames > struct2.frames);
        }
    );

  
  
  // I multiple by two so I can tell the different between a shifted after b and
  // b shifted after a.
  int fftsize = boost::accumulators::max(acc)*2;
  std::cout << "need fft size of " << fftsize << std::endl;

  mytimer mt;
  BOOST_FOREACH(auto &res, clips) {
    add_fft(res, fftsize);
  }
  std::cout << "time for all ffts " << mt.duration() << std::endl;

  std::vector<correlation_data> correlations;
  mt.reset();
  for(unsigned int i=0; i<clips.size(); i++) {
    for (unsigned int j=i+1; j<clips.size(); j++) {
      correlation_data cd = correlate_wavs(clips[i], clips[j], fftsize, clips[i].samplerate);
      //std:: cout << wss[i].filename << " " << wss[j].filename << " offset " << offset << " num std " << num_std << std::endl;
      correlations.push_back(cd);
    }
  }
  std::cout << "time for all pairs correlations " << mt.duration() << std::endl;

  std::sort(correlations.begin(), correlations.end(), [](auto const &t1, auto const &t2) {
      return t1.num_std > t2.num_std;
    });

  for(unsigned int i=0; i<correlations.size(); i++) {
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
  BOOST_FOREACH(auto &res, clips) {
    timelines.insert(res.timeline);
    res.td->ClearEntriesAndBlanks();
  }

  std::cout << "num timelines " << timelines.size() << std::endl;
  int tlnum=0;
  BOOST_FOREACH(auto tl, timelines) {
    std::cout << "timeline" << std::endl;
    long int length = 0;
    int samplerate = 0;

    // compute the overall length needed to save all of the aligned clips.
    BOOST_FOREACH(auto clip, tl->getClips()) {
      std:: cout << "  " << clip.wss->filename << " offset " << clip.offset << std::endl;
      length = std::max(length, clip.wss->frames+clip.offset);
      samplerate = clip.wss->samplerate;
    }

    // here we put everything back together.
    const int channels = tl->getClips().size();
    std::vector<double> samples(length*channels, 0);
    int clipnum=0;
    BOOST_FOREACH(auto clip, tl->getClips()) {
      // the offset and samplerate are both of the downsampled version.
      std::cout << clip.wss->td->GetName() << " "
                << clip.wss->producer << "(" << clip.wss->resource << ") offset "
                << clip.offset/double(clip.wss->samplerate)
                << " duration " << clip.wss->frames/double(clip.wss->samplerate) << std::endl;

      const double fps = 30.0;
      int frame_offset = round(clip.offset/double(clip.wss->samplerate)*fps);
      frame_offset = std::max(frame_offset, clip.wss->td->GetLastPosition());

      if (frame_offset > clip.wss->td->GetLastPosition()) {
        clip.wss->td->AddBlank(frame_offset-clip.wss->td->GetLastPosition());
        clip.wss->td->SetLastPosition(frame_offset);
      }

      int frame_end = round((clip.offset+clip.wss->frames)/double(clip.wss->samplerate)*fps);
      tinyxml2::XMLElement* entry = clip.wss->td->AddEntry(0, frame_end-frame_offset, clip.wss->producer);
      clip.wss->td->AddZoomPan(entry, clip.wss->td->GetId());
      clip.wss->td->SetLastPosition(frame_end);
      
      for(int i=0; i<clip.wss->frames; i++) {
        samples[(i+clip.offset)*channels+clipnum] = clip.wss->samples[i];
      }
      clipnum++;
    }

    write_wav(samples, length, samplerate, channels, std::string("aligned") + std::to_string(tlnum) + ".wav");
    tlnum++;
  }

  
  doc.SaveFile((filename + std::string(".new")).c_str());

}
