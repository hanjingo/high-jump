#include <gtest/gtest.h>
#include <libcpp/os/dll.h>

typedef int (*hello) (void);
typedef int (*world) (void);

TEST (dll, dll_open)
{
#ifdef _WIN32
    ASSERT_EQ (
      dll_open (std::string ("./dll_example").append (DLL_EXT).c_str (),
                DLL_RTLD_LAZY)
        != NULL,
      true);
#else
    ASSERT_EQ (
      dll_open (std::string ("./libdll_example").append (DLL_EXT).c_str (),
                DLL_RTLD_LAZY)
        != NULL,
      true);
#endif
}

TEST (dll, dll_get)
{
#ifdef _WIN32
    void *example = dll_open (
      std::string ("./dll_example").append (DLL_EXT).c_str (), DLL_RTLD_LAZY);
#else
    void *example =
      dll_open (std::string ("./libdll_example").append (DLL_EXT).c_str (),
                DLL_RTLD_LAZY);
#endif
    ASSERT_EQ (example != NULL, true);

    hello fn1 = (hello) dll_get (example, "hello");
    ASSERT_EQ (fn1 != NULL, true);
    ASSERT_EQ (fn1 (), 1);

    world fn2 = (world) dll_get (example, "world");
    ASSERT_EQ (fn2 != NULL, true);
    ASSERT_EQ (fn2 (), 2);
}

TEST (dll, dll_close)
{
#ifdef _WIN32
    void *example = dll_open (
      std::string ("./dll_example").append (DLL_EXT).c_str (), DLL_RTLD_LAZY);
#else
    void *example =
      dll_open (std::string ("./libdll_example").append (DLL_EXT).c_str (),
                DLL_RTLD_LAZY);
#endif
    ASSERT_EQ (example != NULL, true);

    ASSERT_EQ (dll_close (example), true);
    ASSERT_EQ (example != NULL, true);
}

TEST (dll, dll_ext)
{
    ASSERT_EQ (!std::string (DLL_EXT).empty (), true);
}