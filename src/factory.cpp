#include "factory.hpp"

void Factory::do_package_passing() {
    for(auto& worker : _Worker_collection){
        worker.send_package();
    }
    for(auto& ramp : _Ramp_collection){
        ramp.send_package();
    }
}
enum Color { NIEODWIEDZONY, ODWIEDZONY, ZWERYFIKOWANY };

bool is_storehouse_available(PackageSender* send, std::map<PackageSender*,Color>& map){
    auto it = map.find(send);
    if (it->second == Color::ZWERYFIKOWANY){
        return true;
    }
    it->second = Color::ODWIEDZONY;
    if(send->receiver_preferences_.get_preferences().empty()){
        return false;
    }
    bool different_send=false;
    for(auto pair : send->receiver_preferences_.get_preferences()){
        if(pair.first->get_receiver_type() == ReceiverType::STOREHOUSE){
            different_send=true;
        }
        else if(pair.first->get_receiver_type() == ReceiverType::WORKER){
            IPackageReceiver* receiver_ptr = pair.first;
            auto worker_ptr = dynamic_cast<Worker*>(receiver_ptr);
            auto sendrecv_ptr = dynamic_cast<PackageSender*>(worker_ptr);
            if (sendrecv_ptr != send){
                different_send = true;
            }
            if(map.find(sendrecv_ptr)->second == Color::NIEODWIEDZONY){
                bool is = is_storehouse_available(sendrecv_ptr,map);
                if(!is){
                    return false;
                }
            }
        }
    }
    it->second=Color::ZWERYFIKOWANY;
    if(different_send){
        return true;
    }
    else{
        return false;
    }
}

bool Factory::is_consistent(){
    std::map<PackageSender*,Color> color_collection;
    for(auto &ramp : _Ramp_collection){
        color_collection[&ramp]=Color::NIEODWIEDZONY;
    }
    for(auto &worker : _Worker_collection){
        color_collection[&worker]=Color::NIEODWIEDZONY;
    }
    for(auto &ramp : _Ramp_collection){
        bool is = is_storehouse_available(&ramp, color_collection);
        if(!is){
            return false;
        }
    }
    for(auto &worker : _Worker_collection){
        bool is = is_storehouse_available(&worker, color_collection);
        if(!is){
            return false;
        }
    }
    return true;
}

ParsedLineData parse_line(std::string line){
    ParsedLineData ParsedLine;
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream token_stream(line);

    while(std::getline(token_stream,token,' ')){
        tokens.push_back(token);
    }

    if(tokens[0] == "LOADING_RAMP") {
        ParsedLine.TAG = LOADING_RAMP;
        // id
        std::istringstream id_stream(tokens[1]);
        std::string id;
        while(std::getline(id_stream,id,'=')){}
        // delivery-interval
        std::istringstream interval_stream(tokens[2]);
        std::string interval;
        while(std::getline(interval_stream,interval,'=')){}

        ParsedLine.params["id"] = id;
        ParsedLine.params["delivery-interval"] = interval;
    }

    else if(tokens[0] == "WORKER"){
        ParsedLine.TAG = WORKER;
        // id
        std::istringstream id_stream(tokens[1]);
        std::string id;
        while(std::getline(id_stream,id,'=')){}
        // processing-time
        std::istringstream time_stream(tokens[2]);
        std::string time;
        while(std::getline(time_stream,time,'=')){}
        // queue-type
        std::istringstream queue_stream(tokens[3]);
        std::string queue;
        while(std::getline(queue_stream,queue,'=')){}

        ParsedLine.params["id"] = id;
        ParsedLine.params["processing-time"] = time;
        ParsedLine.params["queue-type"] = queue;
    }

    else if(tokens[0] == "STOREHOUSE"){
        ParsedLine.TAG = STOREHOUSE;
        // id
        std::istringstream id_stream(tokens[1]);
        std::string id;
        while(std::getline(id_stream,id,'=')){}

        ParsedLine.params["id"] = id;
    }

    else if(tokens[0] == "LINK"){
        ParsedLine.TAG = LINK;
        // src
        std::istringstream src_stream(tokens[1]);
        std::string src;
        while(std::getline(src_stream,src,'=')){}
        //dest
        std::istringstream dest_stream(tokens[2]);
        std::string dest;
        while(std::getline(dest_stream,dest,'=')){}

        ParsedLine.params["src"] = src;
        ParsedLine.params["dest"] = dest;
    }

    else{
        ParsedLine.TAG = NONE;
    }

    return ParsedLine;
}

Factory load_factory_structure(std::istream& is) {
    Factory factory;

    std::string line;
    while (std::getline(is, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        ParsedLineData parsed_data = parse_line(line);

        switch(parsed_data.TAG){
            case LOADING_RAMP: {
                Ramp ramp(std::stoi(parsed_data.params["id"]), std::stoi(parsed_data.params["delivery-interval"]));
                factory.add_ramp(std::move(ramp));
                break;
            }
            case WORKER: {
                if(parsed_data.params["queue-type"] == "FIFO"){
                    Worker worker(std::stoi(parsed_data.params["id"]),std::stoi(parsed_data.params["processing-time"]),
                                  std::make_unique<PackageQueue>(PackageQueueType::FIFO));
                    factory.add_worker(std::move(worker));
                }
                if(parsed_data.params["queue-type"] == "LIFO"){
                    Worker worker(std::stoi(parsed_data.params["id"]),std::stoi(parsed_data.params["processing-time"]),
                                  std::make_unique<PackageQueue>(PackageQueueType::LIFO));
                    factory.add_worker(std::move(worker));
                }
                break;
            }
            case STOREHOUSE:{
                Storehouse storehouse(std::stoi(parsed_data.params["id"]));
                factory.add_storehouse(std::move(storehouse));
                break;
            }
            case LINK:{
                // src
                std::istringstream src_stream(parsed_data.params["src"]);
                std::string src;
                std::vector<std::string> src_v;
                while(std::getline(src_stream,src,'-')){src_v.push_back(src);}
                // dest
                std::istringstream dest_stream(parsed_data.params["dest"]);
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
            case NONE:
                break;
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
        if(i==factory.worker_cend()-1){
            continue;
        }
        os << "\n";
    }

}
