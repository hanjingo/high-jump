
#include <zlib.h>

#include <iostream>
#include <string>

std::string compressData(const std::string &data)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK)
    {
        throw(std::runtime_error("deflateInit failed"));
    }

    zs.next_in = (Bytef *)data.data();
    zs.avail_in = data.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    do
    {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out)
        {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
    {
        throw(std::runtime_error("Exception during zlib compression"));
    }

    return outstring;
}

std::string decompressData(const std::string &str)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (inflateInit(&zs) != Z_OK)
    {
        throw(std::runtime_error("inflateInit failed"));
    }

    zs.next_in = (Bytef *)str.data();
    zs.avail_in = str.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    do
    {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = inflate(&zs, 0);

        if (outstring.size() < zs.total_out)
        {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    inflateEnd(&zs);

    if (ret != Z_STREAM_END)
    {
        throw(std::runtime_error("Exception during zlib decompression"));
    }

    return outstring;
}

int main()
{
    std::string original = "This is a test string to demonstrate zlib "
                           "compression and decompression in C++";
    std::cout << "Original: " << original << std::endl;
    std::cout << "Size: " << original.size() << " bytes" << std::endl;

    std::string compressed = compressData(original);
    std::cout << "Compressed: " << compressed.size() << " bytes" << std::endl;

    std::string decompressed = decompressData(compressed);
    std::cout << "Decompressed: " << decompressed << std::endl;
    std::cout << "Size: " << decompressed.size() << " bytes" << std::endl;

    return 0;
}
