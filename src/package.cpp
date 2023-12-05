#include "package.hpp"

std::set<ElementID> Package::assigned_IDs = {};
std::set<ElementID> Package::freed_IDs = {};

Package::Package() {
    ElementID new_id;
    if (assigned_IDs.empty() && freed_IDs.empty()) {
        new_id = 1;
    }
    else if(!freed_IDs.empty()){
        new_id = *freed_IDs.begin();
        freed_IDs.erase(new_id);
    }
    else{
        new_id = *assigned_IDs.end() + 1;
    }
    assigned_IDs.insert(new_id);
    id_ = new_id;
}

Package::Package(Package &&p) noexcept{
    id_ = p.id_;
    p.id_ = -1;
}


Package &Package::operator=(Package &&p) noexcept{
    if(id_ != -1){
        freed_IDs.insert(id_);
        assigned_IDs.erase(id_);
    }
    id_ = p.id_;
    p.id_ = -1;
    return *this;
}


Package::~Package() {
    if(id_ != -1){
        freed_IDs.insert(id_);
        assigned_IDs.erase(id_);
    }
}

