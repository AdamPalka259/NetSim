#ifndef NETSIM_TYPES_HPP
#define NETSIM_TYPES_HPP
#include <functional>

using ElementID = int;
using TimeOffset = int;
using Time = int;
using ProbabilityGenerator = std::function<double()>;

#endif //NETSIM_TYPES_HPP
