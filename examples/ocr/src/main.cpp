#include <hj/log/log.hpp>
#include <hj/os/env.h>
#include <hj/os/options.hpp>
#include <hj/os/signal.hpp>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

// #include <hj/ai/ocr.hpp>

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

int main(int argc, char *argv[])
{
#if CRASH_HANDLER_ENABLE == 1
// add crash handle support
#pragma message("crash handler enabled, initializing crash handler...")
    hj::crash_handler::instance().init("./");
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

    // add tesseract OCR support here...
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    if(api->Init(nullptr, "eng", tesseract::OEM_LSTM_ONLY))
    {
        std::cerr << "Could not initialize tesseract." << std::endl;
        return -1;
    }

    Pix *image = pixRead("example.png");
    if(!image)
    {
        std::cerr << "Could not read image." << std::endl;
        return -1;
    }
    api->SetImage(image);

    char *outText = api->GetUTF8Text();
    std::cout << "OCR output: " << outText << std::endl;

    // cleanup
    api->End();
    delete[] outText;
    pixDestroy(&image);
    delete api;

    // std::error_code ec;
    // auto            img = hj::ocr::image::load("example.png", ec);
    // if(ec)
    // {
    //     std::cerr << "Failed to load image: " << ec.message() << std::endl;
    //     return -1;
    // }

    // auto parser = hj::ocr::parser::make_unique("eng", "", ec);
    // if(ec)
    // {
    //     std::cerr << "Failed to create OCR parser: " << ec.message()
    //               << std::endl;
    //     return -1;
    // }

    // auto text = parser->parse(img, ec);
    // if(ec)
    // {
    //     std::cerr << "Failed to parse image: " << ec.message() << std::endl;
    //     return -1;
    // }
    // std::cout << "OCR output: " << text << std::endl;

    return 0;
}
