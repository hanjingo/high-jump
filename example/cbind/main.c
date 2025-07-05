#include <stdio.h>

#include <libcpp/binding/c/any.h>
#include <libcpp/binding/c/bits.h>
#include <libcpp/binding/c/bytes.h>
#include <libcpp/binding/c/endian.h>
#include <libcpp/binding/c/filepath.h>
#include <libcpp/binding/c/hex.h>

void any()
{
    printf("any C API example >>>>>>>>>>>>>>\n");
}

void bits()
{
    printf("bits C API example >>>>>>>>>>>>>>\n");
    int n = 4;

    printf("libcpp_bits_get(n, 1) = %d\n", libcpp_bits_get(n, 1));
    printf("libcpp_bits_get(n, 2) = %d\n", libcpp_bits_get(n, 2));
    printf("libcpp_bits_get(n, 3) = %d\n", libcpp_bits_get(n, 3));
    printf("libcpp_bits_get(n, 4) = %d\n", libcpp_bits_get(n, 4));

    printf("libcpp_bits_put(n, 1, true) = %d\n", libcpp_bits_put(n, 1, true));

    printf("libcpp_bits_reset(n, 1) = %d\n", libcpp_bits_reset(n, 1));

    printf("libcpp_bits_flip(n) = %d\n", libcpp_bits_flip(-1));

    int sz = sizeof(unsigned long) * 8 + 1;
    char buf[sz];
    libcpp_bits_to_string(0xFF, buf);
    printf("libcpp_bits_to_string(0xFF, buf) = ");
    for (int i = 0; i < sz - 1; i++) {
        printf("%c", buf[i]);
    }

    printf("\n\n");
}

void bytes()
{
    printf("bytes C API example >>>>>>>>>>>>>>\n");
    unsigned char buf[8] = {1, 1, 1, 1, 1, 1, 1, 1};

    printf("libcpp_bytes_to_bool(buf) = %d\n", libcpp_bytes_to_bool(buf));
    printf("libcpp_bool_to_bytes(false, buf)[0] = %d\n", libcpp_bool_to_bytes(false, buf)[0]);
    printf("libcpp_bytes_to_int(buf, true) = %d\n", libcpp_bytes_to_int(buf, true));
    printf("libcpp_int_to_bytes(0xFFFFFFFF, buf, true)[0] = %d\n", libcpp_int_to_bytes(0xFFFFFFFF, buf, true)[0]);
    printf("libcpp_bytes_to_long(buf, true) = %ld\n", libcpp_bytes_to_long(buf, true));
    printf("libcpp_long_to_bytes(0xFF, buf, true)[0] = %d\n", libcpp_long_to_bytes(0xFF, buf, true)[0]);
    printf("libcpp_bytes_to_long_long(buf, true) = %lld\n", libcpp_bytes_to_long_long(buf, true));
    printf("libcpp_long_long_to_bytes(0xFF, buf, true)[0] = %d\n", libcpp_long_long_to_bytes(0xFF, buf, false)[0]);
    printf("libcpp_bytes_to_short(buf, false) = %d\n", libcpp_bytes_to_short(buf, false));
    printf("libcpp_short_to_bytes(0xFFFF, buf, false)[0] = %d\n", libcpp_short_to_bytes(0xFFFF, buf, false)[0]);
    printf("libcpp_bytes_to_double(buf) = %lf\n", libcpp_bytes_to_double(buf));
    printf("libcpp_double_to_bytes(123.456)[0] = %d\n", libcpp_double_to_bytes(123.456, buf)[0]);
    printf("libcpp_bytes_to_float(buf) = %f\n", libcpp_bytes_to_float(buf));
    printf("libcpp_float_to_bytes(123.456, buf)[0] = %d\n", libcpp_float_to_bytes(123.456, buf)[0]);

    printf("\n");
}

void endian()
{
    printf("endian C API example >>>>>>>>>>>>>>\n");

    printf("libcpp_convert_big_endian_short(0x1234) = 0x%x\n", libcpp_convert_big_endian_short(0x1234));
    printf("libcpp_convert_big_endian(0x12345678) = 0x%x\n", libcpp_convert_big_endian(0x12345678));
    // printf("libcpp_convert_big_endian_long(0x12345678) = 0x%lx\n", libcpp_convert_big_endian_long(0x12345678));

    printf("libcpp_convert_little_endian_short(0x1234) = 0x%x\n", libcpp_convert_little_endian_short(0x1234));
    printf("libcpp_convert_little_endian(0x12345678) = 0x%x\n", libcpp_convert_little_endian(0x12345678));
    // printf("libcpp_convert_little_endian_long(0x12345678) = 0x%lx\n", libcpp_convert_little_endian_long(0x12345678));

    printf("\n");
}

void filepath()
{
    printf("filepath C API example >>>>>>>>>>>>>>\n");

    printf("\n");
}

void hex()
{
    printf("hex C API example >>>>>>>>>>>>>>\n");

    char buf[] = {'F', 'F', 'A', 'A', 'B', 'B', 'C', 'C', '\0'};
    printf("libcpp_from_hex_str(buf) = %d\n", libcpp_from_hex_str(buf));
    printf("libcpp_to_hex_str(2147483647, buf)[1] = 0x%x\n", libcpp_to_hex_str(2147483647, buf)[1]);

    printf("\n");
}

int main(int argc, char* argv[])
{
    bits();

    bytes();

    endian();

    filepath();

    hex();

    return 0;
}