#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <thread>
#include "common.h"

struct ServiceResult
{
    std::uint64_t reqId;
    Vector<std::uint64_t>* triggers;
};

using ServiceCallback = std::function<void(ServiceResult*)>;

class Service
{
public:
    explicit Service(const std::string& spec, ServiceCallback cb) 
    : triggerSpec(spec), callback(cb) {}
    
    void start();
    void shutdown();

private:
    std::string triggerSpec;
    Vector<std::thread> workers;
    ServiceCallback callback;
};

#endif
