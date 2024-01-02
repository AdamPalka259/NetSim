#ifndef NETSIM_SIMULATION_HPP
#define NETSIM_SIMULATION_HPP

#include "factory.hpp"
#include "types.hpp"

void simulate(Factory& factory, TimeOffset time, std::function<void>(Factory& f, TimeOffset t));

#endif //NETSIM_SIMULATION_HPP
