#include "comm.h"

int hj_err_to_int(hj::license::err_t e)
{
    switch(static_cast<hj::license::err>(e.value()))
    {
        case 0:
            return OK;
        case hj::license::err::invalid_times:
            return LIC_ERR_INVALID_TIMES;
        case hj::license::err::claim_mismatch:
            return LIC_ERR_CLAIM_MISMATCH;
        case hj::license::err::keys_not_changed:
            return LIC_ERR_KEYS_NOT_CHANGED;
        case hj::license::err::keys_not_enough:
            return LIC_ERR_KEYS_NOT_ENOUGH;
        case hj::license::err::file_not_exist:
            return LIC_ERR_FILE_NOT_EXIST;
        case hj::license::err::input_stream_invalid:
            return LIC_ERR_INPUT_STREAM_INVALID;
        case hj::license::err::output_stream_invalid:
            return LIC_ERR_OUTPUT_STREAM_INVALID;
        default:
            return LIC_ERR_FAIL;
    }
}

hj::license::sign_algo str_to_sign_algo(const std::string &algo)
{
    if(algo == "none")
        return hj::license::sign_algo::none;
    else if(algo == "rsa256")
        return hj::license::sign_algo::rsa256;
    else
        return hj::license::sign_algo::none;
}