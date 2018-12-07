#ifndef _parameters_hpp_
#define _parameters_hpp_

#include <cmath>
#include <limits>
#include <string>

const std::string UNINIT_STR = "uninitialized";

class Parameters {
 public:
  Parameters(const std::string config_file);

  int iterations = -1;
  int maxiter    = -1;

  std::string name        = UNINIT_STR;
  std::string start_name  = UNINIT_STR;
  std::string end_name    = UNINIT_STR;
  std::string surfdatadir = UNINIT_STR;
  std::string initdatadir = UNINIT_STR;
  std::string region      = UNINIT_STR;
  std::string HAD         = UNINIT_STR;
  std::string run_type    = UNINIT_STR;
  std::string time_start  = UNINIT_STR;
  std::string time_end    = UNINIT_STR;

  const int    dltxy = 120; //There are 120 30 arc-second pieces in one degree
  const double dy    = 6370000.*M_PI/(180.*dltxy); //radius of the earth * pi / number of possible cells in the y-direction. This should equal the height of each cell in the N-S direction.
  const double dx    = dy;

  bool interpolated = false;
  
  double sedge  = std::numeric_limits<double>::signaling_NaN();
  double deltat = std::numeric_limits<double>::signaling_NaN();
  double wtdmax = std::numeric_limits<double>::signaling_NaN();

  //Set for convenience within the code
  int width  = -1;
  int height = -1;

  bool run_to_equilibrium = false;

  void print() const;
};

#endif