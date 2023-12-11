#ifndef NETSIM_TYPES_HPP
#define NETSIM_TYPES_HPP
#include <functional>

using ElementID = int;
using TimeOffset = unsigned int;
using Time = unsigned int;
using ProbabilityGenerator = std::function<double()>;

#endif //NETSIM_TYPES_HPP
