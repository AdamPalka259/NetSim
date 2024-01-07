#include "nodes.hpp"

void ReceiverPreferences::add_receiver(IPackageReceiver* r) {
    double num_of_receivers = double(preferences_.size());
    if (num_of_receivers == 0) {
        preferences_[r] = 1.0;
    } else {
        for (auto& rec : preferences_) {
            rec.second = 1 / (num_of_receivers + 1);
        }
        preferences_[r] = 1 / (num_of_receivers + 1);
    }
}

void ReceiverPreferences::remove_receiver(IPackageReceiver* r) {
    double num_of_receivers = double(preferences_.size());
    if (num_of_receivers > 1) {
        for (auto& rec : preferences_) {
            if (rec.first != r) {
                rec.second = 1 / (num_of_receivers - 1);
            }
        }
    }
    preferences_.erase(r);
}

IPackageReceiver* ReceiverPreferences::choose_receiver() {
    auto probability = pg_();
    if (probability >= 0 && probability <= 1) {
        double distribution = 0.0;
        for (auto& rec : preferences_) {
            distribution += rec.second;
            if (probability <= distribution) {
                return rec.first;
            }
        }
    }
    return nullptr;
}

void PackageSender::send_package() {
    IPackageReceiver* receiver;
    if (bufor_) {
        receiver = receiver_preferences_.choose_receiver();
        receiver->receive_package(std::move(*bufor_));
        bufor_.reset();
    }
}

void Worker::do_work(Time t){
    if (!bufor_ && !q_->empty()) {
        bufor_.emplace(q_->pop());
        t_ = t;

        if (t - t_ + 1 == pd_) {
            push_package(Package(bufor_.value().get_id()));
            bufor_.reset();

            if (!q_->empty()) { bufor_.emplace(q_->pop()); }
        }
    }
    else {
        if (t - t_ + 1 == pd_) {
            push_package(Package(bufor_.value().get_id()));
            bufor_.reset();
            if (!q_->empty()) { bufor_.emplace(q_->pop()); }
        }
    }
}

Worker::Worker(const Worker &worker){
    id_ = worker.get_id();
    pd_ = worker.get_processing_duration();
    receiver_preferences_ = worker.receiver_preferences_;
    q_ = std::make_unique<PackageQueue>(worker.get_queue()->get_queue_type());
}

Worker& Worker::operator=(const Worker &worker) noexcept {
    id_ = worker.get_id();
    pd_ = worker.get_processing_duration();
    receiver_preferences_ = worker.receiver_preferences_;
    q_ = std::make_unique<PackageQueue>(worker.get_queue()->get_queue_type());
    return *this;
}

void Worker::receive_package(Package&& p) {
    q_->push(std::move(p));
}

void Storehouse::receive_package(Package&& p) {
    d_->push(std::move(p));
}

Ramp::Ramp(const Ramp &ramp) {
    id_ = ramp.get_id();
    di_ = ramp.get_delivery_interval();
    receiver_preferences_ = ramp.receiver_preferences_;
}

Ramp& Ramp::operator=(const Ramp &ramp) noexcept{
    id_ = ramp.get_id();
    di_ = ramp.get_delivery_interval();
    receiver_preferences_ = ramp.receiver_preferences_;
    return *this;
}

void Ramp::deliver_goods(Time t) {
    if (!bufor_) {
        push_package(Package());
        bufor_.emplace(id_);
        t_ = t;
    } else {
        if (t - di_ == t_) {
            push_package(Package());
        }
    }
}