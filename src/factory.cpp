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
        if (rec->get_receiver_type() == ReceiverType::STOREHOUSE) {
            has_receivers = true;
        } else if (rec->get_receiver_type() == ReceiverType::WORKER) {
            IPackageReceiver* receiver_ptr = rec;
            auto worker_ptr = dynamic_cast<Worker*>(receiver_ptr);
            auto sendrecv_ptr = dynamic_cast<PackageSender*>(worker_ptr);
            if (sendrecv_ptr == sender) {
                continue;
            }
            has_receivers = true;
            if (node_color[sendrecv_ptr] == NodeColor::Unvisited) {
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

    for (Worker &worker : Workers_) {
        const PackageSender *worker_ptr = &worker;
        node_color[worker_ptr] = NodeColor::Unvisited;
    }

    for (Ramp &ramp : Ramps_) {
        const PackageSender *ramp_ptr = &ramp;
        node_color[ramp_ptr] = NodeColor::Unvisited;

        //Checking for exception from auxiliary function
        try {
            has_reachable_storehouse(ramp_ptr, node_color);
        } catch (std::logic_error &error) {
            return false;
        }
    }
    return true;
};

void Factory::do_package_passing(void) {
    for (auto& ramp : Ramps_) {
        ramp.send_package();
    }

    for(auto& worker : Workers_) {
        worker.send_package();
    }
};

ParsedLineData parse_line(std::string line){
    ParsedLineData out;
    std::vector<std::string> tokens;
    std::string token;

    std::istringstream token_stream(line);

    while(std::getline(token_stream,token,' ')) {
        tokens.push_back(token);
    }

    if(tokens[0] == "LOADING_RAMP") {
        out.TAG = LOADING_RAMP;

        std::istringstream id_stream(tokens[1]);
        std::string id;
        while(std::getline(id_stream,id,'=')){}

        std::istringstream interval_stream(tokens[2]);
        std::string interval;
        while(std::getline(interval_stream,interval,'=')){}

        out.params["id"] = id;
        out.params["delivery-interval"] = interval;
    }

    else if(tokens[0] == "WORKER"){
        out.TAG = WORKER;

        std::istringstream id_stream(tokens[1]);
        std::string id;
        while(std::getline(id_stream,id,'=')){}

        std::istringstream time_stream(tokens[2]);
        std::string time;
        while(std::getline(time_stream,time,'=')){}

        std::istringstream queue_stream(tokens[3]);
        std::string queue;
        while(std::getline(queue_stream,queue,'=')){}

        out.params["id"] = id;
        out.params["processing-time"] = time;
        out.params["queue-type"] = queue;
    }

    else if(tokens[0] == "STOREHOUSE"){
        out.TAG = STOREHOUSE;

        std::istringstream id_stream(tokens[1]);
        std::string id;
        while(std::getline(id_stream,id,'=')){}

        out.params["id"] = id;
    }

    else if(tokens[0] == "LINK"){
        out.TAG = LINK;

        std::istringstream src_stream(tokens[1]);
        std::string src;
        while(std::getline(src_stream,src,'=')){}

        std::istringstream dest_stream(tokens[2]);
        std::string dest;
        while(std::getline(dest_stream,dest,'=')){}

        out.params["src"] = src;
        out.params["dest"] = dest;
    }

    else{
        out.TAG = NONE;
    }

    return out;
}

Factory load_factory_structure(std::istream& is) {
    Factory factory;

    std::string line;
    while (std::getline(is, line)) {
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }
        ParsedLineData elem = parse_line(line);

        if (elem.TAG == LOADING_RAMP) {
            Ramp ramp(std::stoi(elem.params["id"]), std::stoi(elem.params["delivery-interval"]));
            factory.add_ramp(std::move(ramp));

        } else if (elem.TAG == WORKER) {
            if(elem.params["queue-type"] == "FIFO"){
                Worker worker(std::stoi(elem.params["id"]),std::stoi(elem.params["processing-time"]),
                              std::make_unique<PackageQueue>(PackageQueueType::FIFO));
                factory.add_worker(std::move(worker));
            }
            if(elem.params["queue-type"] == "LIFO"){
                Worker worker(std::stoi(elem.params["id"]),std::stoi(elem.params["processing-time"]),
                              std::make_unique<PackageQueue>(PackageQueueType::LIFO));
                factory.add_worker(std::move(worker));
            }

        } else if (elem.TAG == STOREHOUSE) {
            Storehouse storehouse(std::stoi(elem.params["id"]));
            factory.add_storehouse(std::move(storehouse));

        } else if (elem.TAG == LINK) {
            std::istringstream src_stream(elem.params["src"]);
            std::string src;
            std::vector<std::string> src_v;
            while(std::getline(src_stream,src,'-')){src_v.push_back(src);}

            std::istringstream dest_stream(elem.params["dest"]);
            std::string dest;
            std::vector<std::string> dest_v;
            while(std::getline(dest_stream,dest,'-')){dest_v.push_back(dest);}

            if(src_v[0] == "ramp"){
                Ramp& r = *(factory.find_ramp_by_id(std::stoi(src_v[1])));
                if(dest_v[0] == "worker"){
                    r.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(std::stoi(dest_v[1]))));

                }
                else if(dest_v[0] == "store"){
                    r.receiver_preferences_.add_receiver(&(*factory.find_storehouse_by_id(std::stoi(dest_v[1]))));

                }
            }

            if(src_v[0] == "worker"){
                Worker& w = *(factory.find_worker_by_id(std::stoi(src_v[1])));
                if(dest_v[0] == "worker"){
                    w.receiver_preferences_.add_receiver(&(*factory.find_worker_by_id(std::stoi(dest_v[1]))));
                }
                else if(dest_v[0] == "store"){
                    w.receiver_preferences_.add_receiver(&(*factory.find_storehouse_by_id(std::stoi(dest_v[1]))));
                }
            }
        }
    }

    return factory;
}

void save_factory_structure(Factory& factory, std::ostream& os){
    os << "; == LOADING RAMPS ==\n \n";
    for(auto i = factory.ramp_cbegin(); i != factory.ramp_cend(); ++i) {
        const auto &ramp = *i;
        os << "LOADING_RAMP id=" << ramp.get_id() << " delivery-interval=" << ramp.get_delivery_interval() << "\n";
    }

    os << "\n; == WORKERS == \n\n";
    for(auto i = factory.worker_cbegin(); i != factory.worker_cend(); ++i){
        const auto& worker = *i;
        os << "WORKER id=" << worker.get_id() << " processing-time=" << worker.get_processing_duration();
        if(worker.get_queue()->get_queue_type() == PackageQueueType::FIFO){
            os << " queue-type=FIFO\n";
        }
        else if(worker.get_queue()->get_queue_type() == PackageQueueType::LIFO){
            os << " queue-type=LIFO\n";
        }
    }

    os << "\n; == STOREHOUSES == \n\n";
    for(auto i = factory.storehouse_cbegin(); i != factory.storehouse_cend(); ++i) {
        const auto &storehouse = *i;
        os << "STOREHOUSE id=" << storehouse.get_id() << "\n";
    }

    os << "\n; == LINKS ==\n\n";
    for(auto i = factory.ramp_cbegin(); i != factory.ramp_cend(); ++i) {
        const auto &ramp = *i;
        for(auto rec : ramp.receiver_preferences_.get_preferences()){
            os << "LINK src=ramp-" << ramp.get_id() << " dest=";
            if (rec.first->get_receiver_type() == ReceiverType::WORKER){
                os << "worker-" << rec.first->get_id() << "\n";
            }
            else if(rec.first->get_receiver_type() == ReceiverType::STOREHOUSE){
                os << "storehouse-" << rec.first->get_id() << "\n";
            }
        }
        os << "\n";
    }
    for(auto i = factory.worker_cbegin(); i != factory.worker_cend(); ++i) {
        const auto &worker = *i;
        for(auto rec : worker.receiver_preferences_.get_preferences()){
            os << "LINK src=worker-" << worker.get_id() << " dest=";
            if (rec.first->get_receiver_type() == ReceiverType::WORKER){
                os << "worker-" << rec.first->get_id() << "\n";
            }
            else if(rec.first->get_receiver_type() == ReceiverType::STOREHOUSE){
                os << "store-" << rec.first->get_id() << "\n";
            }
        }
        if(i==std::prev(factory.worker_cend())){
            continue;
        }
        os << "\n";
    }

}
