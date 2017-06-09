
#define BOOST_NUMERIC_FUNCTIONAL_STD_COMPLEX_SUPPORT 1
#define BOOST_NUMERIC_FUNCTIONAL_STD_VALARRAY_SUPPORT 1
#define BOOST_NUMERIC_FUNCTIONAL_STD_VECTOR_SUPPORT 1
#include <complex>
#include <iostream>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics.hpp>

// this code comes from:
// http://hiankun.blogspot.de/2011/09/using-boost-accumulators-to-calculate.html
int main(){
  //using namespace boost::accumulators;
    boost::accumulators::accumulator_set< double, boost::accumulators::stats<boost::accumulators::tag::variance> > acc_variance;
 
    for (int i = 0; i < 10; i++){
        std::cout << i << ", ";
        acc_variance(i);
    }
 
    std::cout << std::endl << "Variance = "
              << sqrt(boost::accumulators::variance(acc_variance)) << std::endl;

    std::cout << std::endl << "Mean = "
        << boost::accumulators::mean(acc_variance) << std::endl;

    return 0;
}

