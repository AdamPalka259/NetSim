#include "factory.hpp"
#include <istream>
#include <string>
#include <sstream>

bool Factory::has_reachable_storehouse(const PackageSender *sender, std::map<const PackageSender *, NodeColor> &node_color) {
    if (node_color[sender] == NodeColor::Verified) {
        return true;
    }

    node_color[sender] = NodeColor::Visited;

    if (sender->receiver_preferences_.get_preferences().empty()){
        throw std::logic_error("Exception: This sender has no receivers!!!");
    }

    bool has_receivers = false;
    for (auto& [rec, pri] : sender->receiver_preferences_.get_preferences()) {
        if (rec->get_receiver_type() == ReceiverType::Storehouse) {
            has_receivers = true;
        } else if (rec->get_receiver_type() == ReceiverType::Worker) {
            IPackageReceiver* receiver_ptr = rec;
            auto worker_ptr = dynamic_cast<Worker*>(receiver_ptr);
            auto sendrecv_ptr = dynamic_cast<PackageSender*>(worker_ptr);
            if (sendrecv_ptr == sender) {
                continue;
            }
            has_receivers = true;
            if (node_color[sendrecv_ptr] == NodeColor::Univisited) {
                has_reachable_storehouse(sendrecv_ptr, node_color);
            }
        }
    }
    node_color[sender] = NodeColor::Verified;

    if (has_receivers) {
        return true;
    }
    else throw std::logic_error("Exception: Sender has no receivers!!!!");
}

bool Factory::is_consistent(void){
    std::map<const PackageSender*, NodeColor> node_color;

    for (Worker &worker : Workers) {
        const PackageSender *worker_ptr = &worker;
        node_color[worker_ptr] = NodeColor::Univisited;
    }

    for (Ramp &ramp : Ramps) {
        const PackageSender *ramp_ptr = &ramp;
        node_color[ramp_ptr] = NodeColor::Univisited;

        //Checking for exception from auxiliary function
        try {
            has_reachable_storehouse(ramp_ptr, node_color);
        } catch (std::logic_error &error) {
            return false;
        }
    }
    return true;
};

void Factory::do_delivieries(Time t) {
    for (auto& ramp : Ramps) {
        ramp.deliver_goods(t);
    }
};

void Factory::do_package_passing(void) {
    for (auto& ramp : Ramps) {
        ramp.send_package();
    }

    for(auto& worker : Workers) {
        worker.send_package();
    }
};

void Factory::do_work(Time t) {
    for (auto& worker : Workers) {
        worker.do_work(t);
    }
};

void Factory::remove_worker(ElementID id) {
    remove_receiver(Workers, id);
    Workers.remove_by_id(id);
}

void Factory::remove_storehouse(ElementID id){
    remove_receiver(Storehouses, id);
    Storehouses.remove_by_id(id);
}

template <class Node>
void Factory::remove_receiver(NodeCollection<Node> &collection, ElementID id) {
    auto ptr = dynamic_cast<IPackageReceiver*>(&(*collection.find_by_id(id)));
    for (Worker& sender : Workers) {
        sender.receiver_preferences_.remove_receiver(ptr);
    }
    if constexpr (std::is_same<Node, Worker>::value) {
        for (Ramp& sender : Ramps) {
            sender.receiver_preferences_.remove_receiver(ptr);
        }
    }
}

ParsedLineData parse_line(std::string line){
    ParsedLineData out;
    std::vector<std::string> tokens;
    std::string token;
 
    std::istringstream token_stream(line);
    char delimiter = ' ';
    char id_delimiter = '=';
    std::getline(token_stream, token, delimiter);
    if(token == "LOADING_RAMP"){
        out.type = RAMP;
    }
    if(token == "WORKER"){
        out.type = WORKER;
    }
    if(token == "STOREHOUSE"){
        out.type = STOREHOUSE;
    }
    if(token == "LINK"){
        out.type = LINK;
    }
    
    while (std::getline(token_stream, token, delimiter)) {
        std::string key = token.substr(0, token.find(id_delimiter));
        std::string id = token.substr(token.find(id_delimiter), token.size()-1);
        out.map.insert(std::make_pair(key,id));
    }
    return out;
}

Factory load_factory_structure(std::istream& is) {
    Factory factory;
    std::string line;
    while (std::getline(is, line)) {
        if (line.empty() || line[0] == ';') {
            continue;
        }
        ParsedLineData elem = parse_line(line);
        if (elem.type == RAMP) {
            ElementID id = std::stoi(elem.map.at("ramp-id"));
            TimeOffset di = std::stoi(elem.map.at("delivery-interval"));
            factory.add_ramp(Ramp(id, di));
        } else if (elem.type == WORKER) {
            ElementID id = std::stoi(elem.map.at("worker-id"));
            TimeOffset pt = std::stoi(elem.map.at("processing-time"));
            PackageQueueType type = (elem.map.at("queue-type") == "LIFO") ? PackageQueueType::LIFO : PackageQueueType::FIFO;
            factory.add_worker(Worker(id, pt, type));
        } else if (elem.type == STOREHOUSE) {
            ElementID id = std::stoi(elem.map.at("storehouse-id"));
            factory.add_storehouse(Storehouse(id));
        } else if (elem.type == LINK) {
            // Implement handling of LINK type if needed
        }
    }
    return factory;
}

void save_factory_structure(Factory& factory, std::ostream& os){
    os << "; == LOADING RAMPS" << std::endl;
    os.put(os.widen('\n'));

}