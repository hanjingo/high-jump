#include <iostream>
#include <thread>

#include <libcpp/net/http.hpp>

int main(int argc, char* argv[])
{
    std::thread([]() {
        libcpp::http_server srv;
        srv.Get(
            "/hello",
            [](const libcpp::http_request& req, libcpp::http_response& resp) {
                std::cout << "On GET /hello" << std::endl;
                resp.set_content("world", "text/plain");
            });
        srv.Post("/person",
                 [](const libcpp::http_request& req,
                    libcpp::http_response& resp,
                    const libcpp::http_content_reader& reader) {
                     std::cout << "On POST /person" << std::endl;

                     libcpp::http_multipart_form_data_items items;
                     if (req.is_multipart_form_data())
                     {
                         reader(
                             [&](const libcpp::http_multipart_form_data& item) {
                                 items.push_back(item);
                                 return true;
                             },
                             [&](const char* data, size_t sz) {
                                 items.back().content.append(data, sz);
                                 return true;
                             });

                         std::cout << "multi part form:" << std::endl;
                         std::cout << "size=" << items.size() << std::endl;
                         std::cout << "text1=" << items.at(0).content
                                   << std::endl;
                     }
                     else
                     {
                         std::string body;
                         reader([&](const char* data, size_t data_length) {
                             body.append(data, data_length);
                             return true;
                         });

                         std::cout << "body form:" << body << std::endl;
                     }
                 });
        srv.listen("localhost", 10086);
    }).detach();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "http server listen localhost:10086" << std::endl;

    std::thread([]() {
        libcpp::http_client cli{ "http://localhost:10086" };
        auto resp = cli.Get("/hello");
        std::cout << "http cli GET http://localhost:10086/hello with >>"
                  << std::endl;
        std::cout << "status: " << resp->status << std::endl;
        std::cout << "body: " << resp->body << std::endl;
        std::cout << "\n" << std::endl;

        libcpp::http_params params{
            { "name", "he"    },
            { "note", "coder" }
        };
        resp = cli.Post("/person", params);
        std::cout << "http cli POST http://localhost:10086/person with with "
                     "http_params>>"
                  << std::endl;
        std::cout << "\n" << std::endl;

        httplib::MultipartFormDataItems items = {
            { "text1", "text default",      "",           ""                         },
            { "text2", "aωb",               "",           ""                         },
            { "file1", "h\ne\n\nl\nl\no\n", "hello.txt",  "text/plain"               },
            { "file2",
             "{\n  \"world\", true\n}\n",   "world.json",
             "application/json"                                                      },
            { "file3", "",                  "",           "application/octet-stream" },
        };
        resp = cli.Post("/person", items);
        std::cout << "http cli POST http://localhost:10086/person with with "
                     "MultipartFormDataItems>>"
                  << std::endl;
        std::cout << "\n" << std::endl;
    }).detach();


    std::cin.get();
    return 0;
}