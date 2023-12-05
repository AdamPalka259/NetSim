#ifndef NETSIM_STORAGE_TYPES_HPP
#define NETSIM_STORAGE_TYPES_HPP

#include "package.hpp"

#include <list>

enum class PackageQueueType {
    FIFO,
    LIFO
};

class IPackageStockpile {
public:
    using const_iterator = std::list<Package>::const_iterator;

    virtual void push(Package&& package) = 0;

    virtual std::size_t size() const = 0;

    virtual bool empty() const = 0;

    virtual const_iterator cbegin() const = 0;

    virtual const_iterator cend() const = 0;

    virtual const_iterator begin() const = 0;

    virtual const_iterator end() const = 0;

    virtual ~IPackageStockpile() = default;
};

class IPackageQueue : public IPackageStockpile {
public:
    virtual Package pop() = 0;

    virtual PackageQueueType get_queue_type() const = 0;

    ~IPackageQueue() override = default;
};

class PackageQueue : public IPackageQueue {
public:
    explicit PackageQueue(PackageQueueType queue_type) : queue_(), queue_type_(queue_type) {}

    void push(Package&& package) override { queue_.emplace_back(std::move(package)); }

    std::size_t size() const override { return queue_.size(); }

    bool empty() const override { return queue_.empty(); }

    const_iterator cbegin() const override { return queue_.cbegin(); }

    const_iterator cend() const override { return queue_.cend(); }

    const_iterator begin() const override { return queue_.cbegin(); }

    const_iterator end() const override { return queue_.cend(); }

    Package pop() override;

    PackageQueueType get_queue_type() const override { return queue_type_; }

    ~PackageQueue() override = default;
private:
    std::list<Package> queue_;
    PackageQueueType queue_type_;
};

#endif //NETSIM_STORAGE_TYPES_HPP
