#include "package.hpp"

std::set<ElementID> Package::assigned_IDs = {};
std::set<ElementID> Package::freed_IDs = {};

Package::Package() {
    ElementID new_id;
    if(!freed_IDs.empty){
        new_id = *freed_IDs.begin();
        freed_IDs.erase(new_id);
    }
    else{
        new_id = std::prev(assigned_IDs.end()) + 1;
    }
    assigned_IDs.insert(new_id)
    id_ = new_id;
}

Package::Package(Package &&p){
    id_ = p.id_;
    p.id = -1;
}


Package &Package::operator=(Package &&p){
    if(id_ = -1){
        freed_IDs.insert(id_);
        assigned_IDs.erase(id_);
    }
    d_ = p.id_;
    p.id_ = -1;
}


Package::~Package() {
    id(id_ = -1){
        freed_IDs.insert(id_);
        assigned_IDs.erase(id_);
    }
}
