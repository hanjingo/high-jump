#ifndef ISSUER_H
#define ISSUER_H

#define LICENSE_ERR_START 3200
#include <hj/util/license.hpp>

#include "lic_core.h"
#include "comm.h"

class issuer_mgr
{
public:
    issuer_mgr()
        : _issuers{}
    {
    }
    ~issuer_mgr()
    {
        _issuers.clear();
    }

    static issuer_mgr& instance()
    {
        static issuer_mgr inst;
        return inst;
    }

    const std::vector<std::string> supported_issuers();

    int add(std::unique_ptr<hj::license::issuer>&& issuer);

    int issue(std::string& output,
              const std::string& licensee,
              const std::string& issuer_id,
              const std::size_t leeway_days,
              const std::vector<hj::license::pair_t>& claims = {});

private:
    std::vector<std::unique_ptr<hj::license::issuer>> _issuers;
};

#endif