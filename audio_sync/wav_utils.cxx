
#include <wav_utils.hxx>
#include <fftw3.h>
#include <sndfile.h>

#include <iostream>
#include <iomanip>
#include <chrono>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>

// read a wav file and put the file's properties into wav samples
bool
read_wav(wav_samples& ws)
{
  // http://www.mega-nerd.com/libsndfile/api.html#open
  // When opening a file for read, the format field should be set to zero before calling sf_open().
  SF_INFO sfinfo;
  sfinfo.format = 0x0;

  std::cout << "reading wav " << ws.filename << std::endl;
  SNDFILE* sfile = sf_open(ws.filename.c_str(), SFM_READ, &sfinfo);
  if (sfile == NULL) {
    std::cout << "unable to open file " << ws.filename << std::endl;
    std::cout << "error " << sf_strerror(sfile) << std::endl;
    exit(-1);
  }

  std::cout << "num channels    " << sfinfo.channels;
  std::cout << " frames         " << sfinfo.frames;
  std::cout << " sample rate    " << sfinfo.samplerate << std::endl;
   
  double *samples = (double*) malloc(sizeof(double)*sfinfo.frames*sfinfo.channels);
  sf_readf_double(sfile, samples, sfinfo.frames);

  ws.channels = sfinfo.channels;
  ws.frames = sfinfo.frames;
  ws.samplerate = sfinfo.samplerate;
  ws.samples = samples;
  
  return true;
}

// now that we know how long of a fft we want, do it on one set of data.
void
add_fft(wav_samples& ws, long int fftsize)
{

  auto begin = std::chrono::high_resolution_clock::now();

  // http://www.fftw.org/fftw3_doc/Complex-One_002dDimensional-DFTs.html#Complex-One_002dDimensional-DFTs
  std::complex<double> *in  = (std::complex<double>*) fftw_malloc(sizeof(std::complex<double>) * fftsize);
  std::complex<double> *out = (std::complex<double>*) fftw_malloc(sizeof(std::complex<double>) * fftsize);
  fftw_plan p = fftw_plan_dft_1d(fftsize,
                                 reinterpret_cast<fftw_complex*>(in),
                                 reinterpret_cast<fftw_complex*>(out),
                                           FFTW_FORWARD, FFTW_ESTIMATE);

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "create plan " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()/1000000000.0 << " sec ";
  begin = std::chrono::high_resolution_clock::now();

  
  for(int i=0; i<ws.frames; i++) {
    in[i] = ws.samples[i*ws.channels];
  }
  for(int i=ws.frames; i<fftsize; i++) {
    in[i] = 0;
  }

  fftw_execute(p);

  end = std::chrono::high_resolution_clock::now();
  std::cout << "execute " << std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()/1000000000.0 << "sec" << std::endl;
  
  ws.fft = out;

  fftw_destroy_plan(p);
  fftw_free(in);
  // don't free out. it's returned as part of ws.
}

void write_wav(std::vector<double>& samples,
               long int length,
               int samplerate,
               int channels,
               const std::string &filename)
{
  SF_INFO sfinfo;
  // http://www.mega-nerd.com/libsndfile/api.html#open
  sfinfo.format = SF_FORMAT_WAV|SF_FORMAT_PCM_16;
  sfinfo.samplerate = samplerate;
  sfinfo.channels = channels;

  std::cout << "writing wav " << filename << std::endl;
  SNDFILE* sfile = sf_open(filename.c_str(), SFM_WRITE, &sfinfo);
  if (sfile == NULL) {
    std::cout << "unable to open file " << filename << std::endl;
    std::cout << "error " << sf_strerror(sfile) << std::endl;
    exit(-1);
  }
  sf_writef_double(sfile, samples.data(), length);
  sf_close(sfile);
}

void write_wav_for_fft(std::complex<double> *fft,
                       long int fftsize,
                       int samplerate,
                       const char* filename)
{

  // use malloc instead of stack array (ie samples[numsamples*channel]) as this will likely
  // exceed stack size limit.
  const int channels = 2;
  std::vector<double> samples(fftsize*channels,0);

  // first need to find the right scale factor.
  double maxval=0;
  for(int i=0; i<fftsize; i++) {
    maxval = std::max(maxval, std::abs(fft[i]));
  }
  for(int i=0; i<fftsize; i++) {
    samples[2*i]   = fft[i].real()/maxval;
    samples[2*i+1] = fft[i].imag()/maxval;
  }

  write_wav(samples,
            fftsize,
            samplerate,
            channels,
            filename);
}

correlation_data
correlate_wavs(wav_samples& ws1,
               wav_samples& ws2,
               long int     fftsize,
               int          samplerate)
{
  std::complex<double> *in  = (std::complex<double>*) fftw_malloc(sizeof(std::complex<double>) * fftsize);
  std::complex<double> *out = (std::complex<double>*) fftw_malloc(sizeof(std::complex<double>) * fftsize);

  fftw_plan p_backward = fftw_plan_dft_1d(fftsize,
                                          reinterpret_cast<fftw_complex*>(in),
                                          reinterpret_cast<fftw_complex*>(out),
                                          FFTW_BACKWARD, FFTW_ESTIMATE);

  // piecewise multiple the FFT
  for (int i=0; i<fftsize; i++) {
    in[i] = std::conj(ws1.fft[i]) * ws2.fft[i];
  }
  fftw_execute(p_backward);

  boost::accumulators::accumulator_set< double,
                                        boost::accumulators::features<
                                          boost::accumulators::stats<boost::accumulators::tag::variance,
                                                                     boost::accumulators::tag::max> > > acc;
 
  //accumulator_set<int, stats<tag::variance> > acc2;
  int maxid = 0;
  for (int i=0; i<fftsize; i++) {
    double mag = std::abs(out[i]);
    acc(mag);
    if (mag == boost::accumulators::max(acc)) { maxid = i; }
  }

  double num_std = (boost::accumulators::max(acc)-
                    boost::accumulators::mean(acc))/sqrt(boost::accumulators::variance(acc));
  
  std::cout << std::fixed << std::setprecision(2)
            << ws1.filename << " " << ws2.filename
            << " mean "      << std::setw(14) << std::right << boost::accumulators::mean(acc)
            << "\tvariance " << std::setw(14) << std::right << sqrt(boost::accumulators::variance(acc))
            << "\tmax "      << std::setw(14) << std::right << boost::accumulators::max(acc)
            << "\tnum std "  << std::setw(14) << std::right << num_std << std::endl;

#if 0
  write_wav_for_fft(out,
                    fftsize,
                    samplerate,
                    "correlate.wav");
#endif
    
  // the max correlation id will be positive if the first input ahead of the second.
  // this doesn't appear to really be true. rather, it's the reverse.
  // ie if in1(t) ~= in2(t+maxcorrel_id)

  if (maxid>fftsize/2) {
    // the order to correlation data matters.
    return correlation_data(&ws1, &ws2, fftsize-maxid, num_std);
  } else {
    return correlation_data(&ws2, &ws1, maxid,         num_std);
  }
}

