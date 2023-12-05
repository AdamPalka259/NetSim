# include "storage_types.hpp"

Package PackageQueue::pop() {
    Package package;
    if (queue_type_ == PackageQueueType::FIFO) {
        package = std::move(queue_.front());
        queue_.pop_front();
    } else if (queue_type_ == PackageQueueType::LIFO) {
        package = std::move(queue_.back());
        queue_.pop_back();
    }
    return package;
}
