#include <hj/log/log.hpp>
#include <hj/os/env.h>
#include <hj/os/options.hpp>
#include <hj/os/signal.hpp>

#include <iostream>
#if CRASH_HANDLER_ENABLE == 1
#include <hj/testing/crash.hpp>
#endif

#if TELEMETRY_ENABLE == 1
#include <hj/testing/telemetry.hpp>
#endif

#if LIC_ENABLE == 1
#include <hj/util/license.hpp>
#endif

// add your code here...
#define OPENSSL_ENABLE 1
#include <hj/net/http/http_client.hpp>
#include <hj/encoding/json.hpp>

#include <hj/testing/error.hpp>
#include <hj/testing/error_handler.hpp>
#include <hj/util/init.hpp>

// add your code here...
#define OK 0
#define ERR_FAIL 1
#define ERR_INVALID_SUBCMD 2
#define ERR_ARGC_TOO_LESS 3

using err_t = std::error_code;

static inline err_t error(const int e, const char *category = "app")
{
    return hj::make_err(e, category);
}

INIT(hj::register_err("app", ERR_INVALID_SUBCMD, "invalid subcmd");
     hj::register_err("app", ERR_ARGC_TOO_LESS, "argc too less");)

int main(int argc, char *argv[])
{
#if CRASH_HANDLER_ENABLE == 1
// add crash handle support
#pragma message("crash handler enabled, initializing crash handler...")
    hj::crash_handler::instance()->prevent_set_unhandled_exception_filter();
    hj::crash_handler::instance()->set_local_path("./");
#endif

#if TELEMETRY_ENABLE == 1
// add telemetry support
#pragma message("telemetry enabled, initializing tracer...")
    auto tracer =
        hj::telemetry::make_otlp_file_tracer("otlp_call", "./telemetry.json");
#endif

#if LIC_ENABLE == 1
// add license check support
#pragma message("license check enabled, verifying license...")
    hj::license::verifier vef{LIC_ISSUER, hj::license::sign_algo::none, {}};
    auto                  verify_err = vef.verify_file(LIC_FPATH, PACKAGE, 1);
    if(verify_err)
    {
        std::cerr << "license verify failed with err: " << verify_err.message()
                  << ", please check your license file: " << LIC_FPATH
                  << std::endl;
        return -1;
    }
#endif

    // add your code here...
    //
    // add options parse support
    hj::options opts;

    // add log support
#ifdef DEBUG
    hj::log::logger::instance()->set_level(hj::log::level::debug);
#else
    hj::log::logger::instance()->set_level(hj::log::level::info);
#endif

    // add signals handle support
    hj::sighandler::instance().sigcatch({SIGABRT, SIGTERM}, [](int sig) {});

    // error handler
    hj::error_handler<err_t> h{[](const char *src, const char *dst) {
        LOG_DEBUG("error handler state transition: {} -> {}", src, dst);
    }};

    // parse options
    opts.add<std::string>("subcmd", std::string(""), "subcommand");
    opts.add_positional("subcmd", 1);
    opts.add<std::string>("key,k", std::string("none"), "key");
    opts.add<std::string>("content", "", "input content");
    opts.add_positional("content", 2);

    std::string subcmd  = opts.parse<std::string>(argc, argv, "subcmd");
    auto        key     = opts.parse<std::string>(argc, argv, "key");
    auto        content = opts.parse<std::string>(argc, argv, "content");
    if(subcmd == "deepseek")
    {
        // http.exe deepseek -k xxx "tell me a short story no more than 250 words"
        std::string         api_key = key;
        std::string         addr    = "api.deepseek.com";
        hj::http_ssl_client client(addr);
        client.set_read_timeout(5, 0); // 5 seconds

        hj::json req_body;
        req_body["model"]    = "deepseek-chat";
        req_body["messages"] = {
            {{"role", "user"}, {"content", "User: Tell me a joke.\nAI:"}}};
        req_body["stream"] = true;

        // create request
        httplib::Request req;
        req.method  = "POST";
        req.path    = "/v1/chat/completions";
        req.headers = {{"Authorization", "Bearer " + api_key},
                       {"Content-Type", "application/json"},
                       {"Accept", "text/event-stream"}};
        req.body    = req_body.dump();

        // handle response
        bool        ok   = true;
        bool        done = false;
        std::string buf;
        req.response_handler = [&](const httplib::Response &res) -> bool {
            LOG_DEBUG("Response status: {}, headers.size(): {}",
                      res.status,
                      res.headers.size());
            if(res.status != 200)
            {
                ok = false;
                LOG_ERROR("Request failed with status: {}, headers.size(): {}",
                          res.status,
                          res.headers.size());
                return false; // Stop processing
            }
            return true; // Continue processing
        };
        req.content_receiver = [&](const char *data,
                                   size_t      data_length,
                                   size_t      offset,
                                   size_t
                                       total_length) { // Handle streaming data
            LOG_DEBUG(
                "Received data chunk: {} bytes, offset: {}, total_length: {}",
                data_length,
                offset,
                total_length);
            if(!ok)
                return false;

            buf.append(data, data_length);

            size_t pos = 0;
            while((pos = buf.find("\n")) != std::string::npos)
            {
                std::string line = buf.substr(0, pos);
                buf.erase(0, pos + 1);
                if(!line.empty() && line.back() == '\r')
                    line.pop_back();

                if(line.empty() || line[0] == ':')
                    continue;

                if(line.rfind("data: ", 0) == 0)
                {
                    std::string json_str = line.substr(6);
                    if(json_str == "[DONE]")
                    {
                        done = true;
                        LOG_DEBUG("Stream finished.");
                        return true;
                    }

                    try
                    {
                        auto json_data = hj::json::parse(json_str);
                        if(json_data.contains("choices")
                           && !json_data["choices"].empty()
                           && json_data["choices"][0].contains("delta")
                           && json_data["choices"][0]["delta"].contains(
                               "content"))
                        {
                            std::string content_piece =
                                json_data["choices"][0]["delta"]["content"];
                            std::cout << content_piece << std::flush;
                        }
                    }
                    catch(const std::exception &e)
                    {
                        LOG_ERROR("JSON parse error: {}, Line content: {}",
                                  e.what(),
                                  json_str);
                    }
                }
            }
            return true;
        };

        // send request
        auto result = client.send(req);

        // check if the request was successful
        if(!result)
        {
            LOG_ERROR("Request failed: {}", to_string(result.error()));
            return -1;
        } else if(!ok && !done)
        {
            LOG_ERROR("Request failed during streaming.");
            return -1;
        }
    } else
    {
        h.match(error(ERR_INVALID_SUBCMD), [&](const err_t &e) {
            LOG_ERROR("Error: unknown subcommand: {}, we expected one of these "
                      "subcommands: [deepseek]",
                      subcmd);
        });
        return 1;
    }
}