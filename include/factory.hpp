#ifndef IMPLEMENTATION_NETSIM_FACTORY_HPP
#define IMPLEMENTATION_NETSIM_FACTORY_HPP

#include <vector>
#include <algorithm>
#include "types.hpp"
#include "nodes.hpp"
#include <stdexcept>
#include <string>



template <typename Node> class NodeCollection {
public:
    using container_t = typename std::vector<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    void add(Node&& node) { nodes_.push_back(std::move(node)); };

    NodeCollection<Node>::iterator find_by_id(ElementID id) {
        auto it = std::find_if(nodes_.begin(),nodes_.end(),
                               [id](auto& elem){ return (elem.get_id() == id);});
        return it;
    }
    NodeCollection<Node>::const_iterator find_by_id(ElementID id) const {
        auto it = std::find_if(nodes_.cbegin(),nodes_.cend(),
                               [id](const auto& elem){ return (elem.get_id() == id);});
        return it;
    }


    void remove_by_id(ElementID id){
        auto it = std::find_if(nodes_.begin(),nodes_.end(),
                               [id](auto& elem){ return (elem.get_id() == id);});
        if (it != nodes_.end()){
            nodes_.erase(it);
        }
    }

    iterator begin() {return nodes_.begin();}
    iterator end() {return nodes_.end();}
    const_iterator cbegin() const {return nodes_.cbegin();}
    const_iterator cend() const {return nodes_.cend();}
    const_iterator begin() const {return nodes_.begin();}
    const_iterator end() const {return nodes_.end();}
private:
    container_t nodes_;
};


class Factory {
public:
    void add_ramp(Ramp &&ramp) { Ramps_.add(std::move(ramp)); }

    void remove_ramp(ElementID id) { Ramps_.remove_by_id(id); }

    void add_worker(Worker &&worker) { Workers_.add(std::move(worker)); }

    void remove_worker(ElementID id) { remove_receiver<Worker>(Workers_, id); };

    void add_storehouse(Storehouse &&storehouse) { Storehouses_.add(std::move(storehouse)); }

    void remove_storehouse(ElementID id) { remove_receiver<Storehouse>(Storehouses_, id); };

    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) { return Ramps_.find_by_id(id); }

    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const { return Ramps_.find_by_id(id); }

    NodeCollection<Ramp>::const_iterator ramp_cbegin() const { return Ramps_.cbegin(); }

    NodeCollection<Ramp>::const_iterator ramp_cend() const { return Ramps_.end(); }

    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) { return Workers_.find_by_id(id); }

    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const { return Workers_.find_by_id(id); }

    NodeCollection<Worker>::const_iterator worker_cbegin() const { return Workers_.cbegin(); }

    NodeCollection<Worker>::const_iterator worker_cend() const { return Workers_.cend(); }

    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) { return Storehouses_.find_by_id(id); }

    NodeCollection<Storehouse>::const_iterator
    find_storehouse_by_id(ElementID id) const { return Storehouses_.find_by_id(id); }

    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const { return Storehouses_.cbegin(); }

    NodeCollection<Storehouse>::const_iterator storehouse_cend() const { return Storehouses_.cend(); }

    bool is_consistent(void);

    void do_delivieries(Time t) { for (auto &ramp: Ramps_) { ramp.deliver_goods(t); }};

    void do_package_passing(void);

    void do_work(Time t) { for (auto &worker: Workers_) { worker.do_work(t); }};

private:
    template<typename Node>
    void remove_receiver(NodeCollection<Node> &collection, ElementID id) {
        collection.remove_by_id(id);
        for (auto &ramp: Ramps_) {
            ramp.receiver_preferences_.remove_receiver(&(*collection.find_by_id(id)));
        }
        for (auto &worker: Workers_) {
            worker.receiver_preferences_.remove_receiver(&(*collection.find_by_id(id)));
        }
    }

    NodeCollection<Ramp> Ramps_;
    NodeCollection<Worker> Workers_;
    NodeCollection<Storehouse> Storehouses_;

    enum class NodeColor {
        Unvisited,
        Visited,
        Verified
    };

    bool has_reachable_storehouse(const PackageSender* sender, std::map<const PackageSender*, NodeColor> &node_colors);
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
