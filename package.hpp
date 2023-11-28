#ifndef NETSIM_PACKAGE_HPP
#define NETSIM_PACKAGE_HPP

#include "types.hpp"
#include <set>

class Package{
public:
    Package();
    Package(ElementID);
    Package(Package &&);
    Package & operator = (Package &&);
    ElementID get_id()const;
    ~Package();

private:
    ElementID id_;
    static std::set<ElementID> assigned_IDs;
    static std::set<ElementID> freed_IDs;

};

#endif //NETSIM_PACKAGE_HPP