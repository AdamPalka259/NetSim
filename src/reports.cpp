#include "reports.hpp"
#include "ostream"

void generate_structure_report(const Factory& f, std::ostream& os){
    os << "\n== LOADING RAMPS ==\n";
    for(auto r = f.ramp_cbegin(); r != f.ramp_cend(); ++r){
        auto &ramp = *r;
        os << "\nLOADING RAMP #" << ramp.get_id() << "\n";
        os << "  Delivery interval: " << ramp.get_delivery_interval() << "\n";
        os << "  Receivers:\n";
        for (auto rec : ramp.receiver_preferences_.get_preferences()){
            if(rec.first->get_receiver_type()==ReceiverType::WORKER){
                os << "    worker #" << rec.first->get_id() << "\n";
            }
            if(rec.first->get_receiver_type()==ReceiverType::STOREHOUSE){
                os << "    storehouse #" << rec.first->get_id() << "\n";
            }
        }
    }

    os << "\n\n== WORKERS ==\n";
    for(auto w = f.worker_cbegin(); w != f.worker_cend();w++){
        auto &worker = *w;
        os << "\nWORKER #" << worker.get_id() << "\n";
        os << "  Processing time: " << worker.get_processing_duration() << "\n";
        os << "  Queue type: ";
        if(worker.get_queue()->get_queue_type() == PackageQueueType::FIFO){
            os << "FIFO\n";
        }
        else if(worker.get_queue()->get_queue_type() == PackageQueueType::LIFO){
            os << "LIFO\n";
        }
        os << "  Receivers:\n";
        for (auto rec : worker.receiver_preferences_.get_preferences()){
            if(rec.first->get_receiver_type()==ReceiverType::WORKER){
                os << "    worker #" << rec.first->get_id() << "\n";
            }
            if(rec.first->get_receiver_type()==ReceiverType::STOREHOUSE){
                os << "    storehouse #" << rec.first->get_id() << "\n";
            }
        }
    }

    os << "\n\n== STOREHOUSES ==\n";
    for(auto s = f.storehouse_cbegin(); s != f.storehouse_cend();s++){
        auto &store = *s;
        os << "\nSTOREHOUSE #" << store.get_id() << "\n";
    }
    os << "\n";
}

void generate_simulation_turn_report(const Factory& f, std::ostream& os, Time t){
    os << "=== [ Turn: " << t << " ] ===";
    
    os << "\n\n== WORKERS ==\n";
    for(auto w = f.worker_cbegin(); w != f.worker_cend(); w++){
        const auto &worker = *w;
        os << "\nWORKER #" << worker.get_id() << "\n";
            //PBUFFER
        os << "  PBuffer: ";
        if(worker.get_processing_buffer())
            os << '#' << worker.get_processing_buffer()->get_id() << ' ' << "(pt = " << 1+t-worker.get_package_processing_start_time() << ")";
        else
            os << "(empty)";
            //QUEUE
        os << "\n  Queue:";
        if(worker.get_queue()->size() > 0) {
            for (auto it = worker.cbegin(); it != worker.cend(); ++it) {
                os << " #" << it->get_id();
                if (std::next(it, 1) != worker.cend())
                    os << ',';
            }
        } else {
            os << " (empty)";
        }
            //SBUFFER
        os << "\n  SBuffer: ";
        if(worker.get_sending_buffer())
            os << '#' << worker.get_sending_buffer()->get_id() << '\n';
        else
            os << "(empty)\n";
    }

    os << "\n\n== STOREHOUSES ==\n";
    for(auto s = f.storehouse_cbegin(); s != f.storehouse_cend();s++){
        auto &store = *s;
        os << "\nSTOREHOUSE #" << store.get_id() << "\n";
        os << "  Stock:";
        if(store.cbegin() != store.cend()) {
            for (auto elem = store.cbegin(); elem != store.cend(); ++elem) {
                os << " #" << elem->get_id();
                if (std::next(elem, 1) != store.cend())
                    os << ',';
            }
        } else {
            os << " (empty)";
        }
    }
    os << "\n\n";
}

class IntervalReportNotifier{
public:
    IntervalReportNotifier(TimeOffset to) : _to(to) {};

    bool should_generate_report(Time t){
        if ((t-1) % _to == 0) { return true;}
        else { return false;}
    }
private:
    TimeOffset _to;
};

class SpecificTurnsReportNotifier{
public:
    SpecificTurnsReportNotifier(std::set<Time> turns) : _turns(std::move(turns)) {}

    bool should_generate_report(Time t){
        auto it = _turns.find(t);
        if(it == _turns.cend()){ return false; }
        else { return true; }
    }
private:
    std::set<Time> _turns;
};
