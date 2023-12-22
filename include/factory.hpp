#ifndef NETSIM_FACTORY_HPP
#define NETSIM_FACTORY_HPP

#include "nodes.hpp"
#include <memory>
#include <iostream>
#include <sstream>

template <typename Node> class NodeCollection{
public:
    using container_t = typename std::vector<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;
    void add(Node&& node) {node_vector_.push_back(std::move(node));}

    iterator find_by_id(ElementID id){
        auto it = std::find_if(node_vector_.begin(),node_vector_.end(),
                               [id](auto& elem){ return (elem.get_id() == id);});
        return it;
    }

    const_iterator find_by_id(ElementID id) const{
        auto it = std::find_if(node_vector_.cbegin(),node_vector_.cend(),
                               [id](const auto& elem){ return (elem.get_id() == id);});
        return it;
    }

    void remove_by_id(ElementID id){
        auto it = std::find_if(node_vector_.begin(),node_vector_.end(),
                               [id](auto& elem){ return (elem.get_id() == id);});
        if (it != node_vector_.end()){
            node_vector_.erase(it);
        }
    }

    const_iterator cbegin() const { return node_vector_.cbegin();}
    const_iterator cend() const { return node_vector_.cend();}
    const_iterator begin() const { return node_vector_.begin();}
    const_iterator end() const { return node_vector_.end();}
    iterator begin() { return node_vector_.begin();}
    iterator end() { return node_vector_.end();}
private:
    container_t node_vector_;
};

class Factory{
public:
        //Ramp related methods
    void add_ramp(Ramp&& new_ramp) {_Ramp_collection.add(std::move(new_ramp));}
    void remove_ramp(ElementID id) {_Ramp_collection.remove_by_id(id);};
    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) {return _Ramp_collection.find_by_id(id);}
    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const {return _Ramp_collection.find_by_id(id);}
    NodeCollection<Ramp>::const_iterator ramp_cbegin() const {return _Ramp_collection.cbegin();}
    NodeCollection<Ramp>::const_iterator ramp_cend() const {return _Ramp_collection.cend();}
        //Worker related methods
    void add_worker(Worker&& new_worker) {_Worker_collection.add(std::move(new_worker));}
    void remove_worker(ElementID id) { remove_receiver<Worker>(_Worker_collection,id);};
    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) {return _Worker_collection.find_by_id(id);}
    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const {return _Worker_collection.find_by_id(id);}
    NodeCollection<Worker>::const_iterator worker_cbegin() const {return _Worker_collection.cbegin();}
    NodeCollection<Worker>::const_iterator worker_cend() const {return _Worker_collection.cend();}
        //Storehouse related methods
    void add_storehouse(Storehouse&& new_storehouse) {_Storehouse_collection.add(std::move(new_storehouse));}
    void remove_storehouse(ElementID id) { remove_receiver<Storehouse>(_Storehouse_collection,id);};
    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) {return _Storehouse_collection.find_by_id(id);}
    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const {return _Storehouse_collection.find_by_id(id);}
    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const {return _Storehouse_collection.cbegin();}
    NodeCollection<Storehouse>::const_iterator storehouse_cend() const {return _Storehouse_collection.cend();}
        //Whole factory methods
    bool is_consistent();
    void do_deliveries(Time t) {for(auto& ramp : _Ramp_collection){ramp.deliver_goods(t);}};
    void do_package_passing();
    void do_work(Time t) {for(auto& worker : _Worker_collection){worker.do_work(t);}};

private:
    template<typename Node>
    void remove_receiver(NodeCollection<Node>& collection, ElementID id){
        collection.remove_by_id(id);
        for(auto &ramp : _Ramp_collection){
            ramp.receiver_preferences_.remove_receiver(&(*collection.find_by_id(id)));
        }
        for(auto &worker : _Worker_collection){
            worker.receiver_preferences_.remove_receiver(&(*collection.find_by_id(id)));
        }
    }
    NodeCollection<Ramp> _Ramp_collection;
    NodeCollection<Worker> _Worker_collection;
    NodeCollection<Storehouse> _Storehouse_collection;
};

enum Component {LOADING_RAMP, WORKER, STOREHOUSE, LINK, NONE};

struct ParsedLineData{
    Component TAG;
    std::map<std::string, std::string> params;
};

Factory load_factory_structure(std::istream& is);
void save_factory_structure(Factory& factory, std::ostream& os);
ParsedLineData parse_line(std::string line);
#endif //NETSIM_FACTORY_HPP
