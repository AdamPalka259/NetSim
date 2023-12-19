#ifndef IMPLEMENTATION_NETSIM_FACTORY_HPP
#define IMPLEMENTATION_NETSIM_FACTORY_HPP

#include <vector>
#include <algorithm>
#include "types.hpp"
#include "nodes.hpp"
#include <stdexcept>
#include <string>



template<class Node>
class NodeCollection {
public:
    using container_t = typename std::list<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    void add(Node&& node) { nodes_.push_back(std::move(node)); };

    NodeCollection<Node>::iterator find_by_id(ElementID id);
    NodeCollection<Node>::const_iterator find_by_id(ElementID id) const;


    void remove_by_id(ElementID id);

    iterator begin() {return nodes_.begin();}
    iterator end() {return nodes_.end();}
    const_iterator cbegin() const {return nodes_.cbegin();}
    const_iterator cend() const {return nodes_.cend();}
    const_iterator begin() const {return nodes_.begin();}
    const_iterator end() const {return nodes_.end();}
private:
    std::list<Node> nodes_;
};


class Factory{
public:
    void add_ramp(Ramp&& ramp) {Ramps.add(std::move(ramp));};
    void remove_ramp(ElementID id) {Ramps.remove_by_id(id);}

    void add_worker(Worker&& worker) {Workers.add(std::move(worker));}
    void remove_worker(ElementID id);

    void add_storehouse(Storehouse&& storehouse){Storehouses.add(std::move(storehouse));}
    void remove_storehouse(ElementID id);

    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) {return Ramps.find_by_id(id);}
    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const {return Ramps.find_by_id(id);}
    NodeCollection<Ramp>::const_iterator ramp_cbegin() {return Ramps.cbegin();}
    NodeCollection<Ramp>::const_iterator ramp_cend() {return Ramps.end();}

    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id){return Workers.find_by_id(id);}
    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const {return Workers.find_by_id(id);}
    NodeCollection<Worker>::const_iterator worker_cbegin() {return Workers.cbegin();}
    NodeCollection<Worker>::const_iterator worker_cend() {return Workers.cend();}

    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id){return Storehouses.find_by_id(id);}
    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const {return Storehouses.find_by_id(id);}
    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() {return Storehouses.cbegin();}
    NodeCollection<Storehouse>::const_iterator storehouse_cend() {return Storehouses.cend();}

    bool is_consistent(void);
    void do_deliveries(Time t);
    void do_package_passing(void);
    void do_work(Time t);

private:
    template <class Node>
    void remove_receiver(NodeCollection<Node>& collection, ElementID id);

    NodeCollection<Ramp> Ramps;
    NodeCollection<Worker> Workers;
    NodeCollection<Storehouse> Storehouses;

    enum class NodeColor {
        Univisited,
        Visited,
        Verified
    };

    bool has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor> &node_colors);
};

Factory load_factory_structure(std::istream& is);
void save_factory_structure(Factory& factory, std::ostream& os);
#endif