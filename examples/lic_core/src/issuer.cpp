#include "issuer.h"

#include <iostream>

const std::vector<std::string> issuer_mgr::supported_issuers()
{
    std::vector<std::string> ret;
    for(const auto &e : _issuers)
        ret.emplace_back(e->id());

    return ret;
}

int issuer_mgr::add(std::unique_ptr<hj::license::issuer> &&issuer)
{
    for(const auto &e : _issuers)
        if(e->id() == issuer->id())
            return LIC_ERR_ISSUER_EXISTED;

    _issuers.emplace_back(std::move(issuer));
    return OK;
}

int issuer_mgr::issue(std::string                            &output,
                      const std::string                      &licensee,
                      const std::string                      &issuer_id,
                      const std::size_t                       leeway_days,
                      const std::vector<hj::license::pair_t> &claims)
{
    hj::license::err_t err;
    for(const auto &issuer : _issuers)
    {
        if(issuer->id() != issuer_id)
            continue;

        if(output.empty()) // to mem
            err = issuer->issue(output, licensee, leeway_days, claims);
        else
            err = issuer->issue_file(output, licensee, leeway_days, claims);

        return hj_err_to_int(err);
    }

    return LIC_ERR_ISSUER_NOT_EXIST;
}