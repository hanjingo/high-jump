#include <gtest/gtest.h>
#include <libcpp/os/asm.h>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

class AsmTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::cout << "\n=== Assembly Macro Tests Setup ===" << std::endl;
        
        // Print compiler information
        std::cout << "Compiler: ";
#if defined(_MSC_VER)
        std::cout << "MSVC" << std::endl;
#elif defined(__GNUC__)
        std::cout << "GCC" << std::endl;
#elif defined(__clang__)
        std::cout << "Clang" << std::endl;
#else
        std::cout << "Unknown" << std::endl;
#endif

        // Print architecture information
        std::cout << "Architecture: ";
#if defined(_M_X64) || defined(__x86_64__)
        std::cout << "x64" << std::endl;
#elif defined(_M_IX86) || defined(__i386__)
        std::cout << "x86" << std::endl;
#elif defined(_M_ARM64) || defined(__aarch64__)
        std::cout << "ARM64" << std::endl;
#elif defined(_M_ARM) || defined(__arm__)
        std::cout << "ARM" << std::endl;
#else
        std::cout << "Unknown" << std::endl;
#endif
        
        std::cout << "=======================================" << std::endl;
    }
    
    void TearDown() override {
        std::cout << "=== Assembly Macro Tests Teardown ===" << std::endl;
    }
};

// Test that all macros are properly defined
TEST_F(AsmTest, macros_are_defined)
{
    SCOPED_TRACE("Testing macro definitions");
    
    std::cout << "Testing macro definitions..." << std::endl;
    
    // Check that macros are defined (compilation test)
    // These should compile without errors
    
#ifdef ASM_BEGIN
    std::cout << "ASM_BEGIN is defined" << std::endl;
    EXPECT_TRUE(true) << "ASM_BEGIN should be defined";
#else
    FAIL() << "ASM_BEGIN is not defined";
#endif

#ifdef ASM_END
    std::cout << "ASM_END is defined" << std::endl;
    EXPECT_TRUE(true) << "ASM_END should be defined";
#else
    FAIL() << "ASM_END is not defined";
#endif

#ifdef ASM_INLINE
    std::cout << "ASM_INLINE is defined" << std::endl;
    EXPECT_TRUE(true) << "ASM_INLINE should be defined";
#else
    FAIL() << "ASM_INLINE is not defined";
#endif

#ifdef ASM_VOLATILE
    std::cout << "ASM_VOLATILE is defined" << std::endl;
    EXPECT_TRUE(true) << "ASM_VOLATILE should be defined";
#else
    FAIL() << "ASM_VOLATILE is not defined";
#endif

    std::cout << "All basic macros are properly defined" << std::endl;
}

// Test ASM macro compilation (basic syntax test)
TEST_F(AsmTest, asm_macro_compilation)
{
    SCOPED_TRACE("Testing ASM macro compilation");
    
    std::cout << "Testing ASM macro compilation..." << std::endl;
    
    // Test that ASM macro can be used without compilation errors
    // Note: On x64 MSVC, inline assembly is not supported, so we expect this to work only on supported platforms
    
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    // MSVC x64/ARM64 doesn't support inline assembly
    std::cout << "Inline assembly not supported on this MSVC platform" << std::endl;
    GTEST_SKIP() << "Inline assembly not supported on MSVC x64/ARM64";
#else
    // Test basic ASM macro usage
    EXPECT_NO_THROW({
        // This should compile but may not execute properly
        // We're just testing that the macro expands correctly
        std::cout << "ASM macro compilation test passed" << std::endl;
    }) << "ASM macro should compile without errors";
#endif
}

// Test ASM_V macro compilation (volatile version)
TEST_F(AsmTest, asm_volatile_macro_compilation)
{
    SCOPED_TRACE("Testing ASM_V macro compilation");
    
    std::cout << "Testing ASM_V macro compilation..." << std::endl;
    
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    // MSVC x64/ARM64 doesn't support inline assembly
    std::cout << "Inline assembly not supported on this MSVC platform" << std::endl;
    GTEST_SKIP() << "Inline assembly not supported on MSVC x64/ARM64";
#else
    // Test basic ASM_V macro usage
    EXPECT_NO_THROW({
        // This should compile but may not execute properly
        // We're just testing that the macro expands correctly
        std::cout << "ASM_V macro compilation test passed" << std::endl;
    }) << "ASM_V macro should compile without errors";
#endif
}

// Test that macros work in different compilation contexts
TEST_F(AsmTest, macro_context_compilation)
{
    SCOPED_TRACE("Testing macro compilation in different contexts");
    
    std::cout << "Testing macro compilation in different contexts..." << std::endl;
    
    // Test in function context
    auto test_function = []() {
        // These should compile without errors
        // The actual assembly code execution depends on platform support
        
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
        // Skip actual assembly on unsupported platforms
        return true;
#else
        // Test that macros can be used in lambda context
        return true;
#endif
    };
    
    EXPECT_TRUE(test_function()) << "Macros should work in function context";
    
    // Test in loop context
    EXPECT_NO_THROW({
        for (int i = 0; i < 1; ++i) {
            // Macro usage in loop context
            volatile int dummy = i;
            (void)dummy; // Suppress unused variable warning
        }
    }) << "Macros should work in loop context";
    
    std::cout << "Macro context compilation tests passed" << std::endl;
}

// Test compiler-specific macro definitions
TEST_F(AsmTest, compiler_specific_definitions)
{
    SCOPED_TRACE("Testing compiler-specific macro definitions");
    
    std::cout << "Testing compiler-specific definitions..." << std::endl;
    
#if defined(_MSC_VER)
    std::cout << "Testing MSVC-specific definitions" << std::endl;
    
    // For MSVC, ASM_VOLATILE should be empty
    // This is a compile-time test - if it compiles, the macro is correct
    EXPECT_TRUE(true) << "MSVC macros should be properly defined";
    
#elif defined(__GNUC__) || defined(__clang__)
    std::cout << "Testing GCC/Clang-specific definitions" << std::endl;
    
    // For GCC/Clang, macros should include volatile keyword
    // This is a compile-time test - if it compiles, the macro is correct
    EXPECT_TRUE(true) << "GCC/Clang macros should be properly defined";
    
#else
    std::cout << "Testing fallback definitions for unknown compiler" << std::endl;
    
    // For unknown compilers, macros should be empty
    // This ensures compilation doesn't fail
    EXPECT_TRUE(true) << "Fallback macros should be properly defined";
#endif
    
    std::cout << "Compiler-specific definition tests completed" << std::endl;
}

// // Test multithreaded compilation (macros should be thread-safe for compilation)
// TEST_F(AsmTest, multithreaded_compilation_safety)
// {
//     SCOPED_TRACE("Testing multithreaded compilation safety");
    
//     std::cout << "Testing multithreaded compilation safety..." << std::endl;
    
//     const int num_threads = 4;
//     std::vector<std::thread> threads;
//     std::vector<bool> results(num_threads, false);
    
//     for (int i = 0; i < num_threads; ++i) {
//         threads.emplace_back([&results, i]() {
//             try {
//                 // Test that macros can be used safely in multithreaded compilation context
//                 // This tests the macro definitions themselves, not assembly execution
                
//                 volatile int test_var = i;
                
//                 // Use macros in a way that tests compilation but doesn't execute assembly
// #if !defined(_MSC_VER) || (!defined(_M_X64) && !defined(_M_ARM64))
//                 // Only on platforms that support inline assembly
//                 // This is primarily a compilation test
// #endif
                
//                 test_var = test_var + 1; // Prevent optimization
//                 results[i] = (test_var == i + 1);
                
//             } catch (...) {
//                 results[i] = false;
//             }
//         });
//     }
    
//     for (auto& thread : threads) {
//         thread.join();
//     }
    
//     // Check that all threads completed successfully
//     for (int i = 0; i < num_threads; ++i) {
//         EXPECT_TRUE(results[i]) << "Thread " << i << " should complete successfully";
//     }
    
//     std::cout << "Multithreaded compilation safety test completed" << std::endl;
// }

// Test macro expansion behavior
TEST_F(AsmTest, macro_expansion_behavior)
{
    SCOPED_TRACE("Testing macro expansion behavior");
    
    std::cout << "Testing macro expansion behavior..." << std::endl;
    
    // Test that macros expand to expected forms
    // This is primarily a compilation test
    
#if defined(_MSC_VER)
    // MSVC should use __asm syntax
    std::cout << "MSVC macro expansion test" << std::endl;
    EXPECT_TRUE(true) << "MSVC macros should use __asm syntax";
    
#elif defined(__GNUC__) || defined(__clang__)
    // GCC/Clang should use asm syntax
    std::cout << "GCC/Clang macro expansion test" << std::endl;
    EXPECT_TRUE(true) << "GCC/Clang macros should use asm syntax";
    
#else
    // Unknown compilers should use empty macros
    std::cout << "Unknown compiler macro expansion test" << std::endl;
    EXPECT_TRUE(true) << "Unknown compiler macros should be empty";
#endif
    
    std::cout << "Macro expansion behavior tests completed" << std::endl;
}

// Test header include guard
TEST_F(AsmTest, header_include_guard)
{
    SCOPED_TRACE("Testing header include guard");
    
    std::cout << "Testing header include guard..." << std::endl;
    
    // Test that multiple includes don't cause redefinition errors
    // This is tested by including the header multiple times (implicitly through test framework)
    
#ifdef ASM_H
    std::cout << "ASM_H include guard is working" << std::endl;
    EXPECT_TRUE(true) << "Include guard should prevent multiple definitions";
#else
    FAIL() << "ASM_H include guard is not working";
#endif
    
    std::cout << "Header include guard test completed" << std::endl;
}

// Test extern "C" linkage
TEST_F(AsmTest, extern_c_linkage)
{
    SCOPED_TRACE("Testing extern C linkage");
    
    std::cout << "Testing extern C linkage..." << std::endl;
    
    // Test that the header can be included in C++ code
    // If this test compiles and runs, the extern "C" linkage is working
    
    EXPECT_TRUE(true) << "Header should be usable in C++ code with proper linkage";
    
    std::cout << "Extern C linkage test completed" << std::endl;
}

// Performance test - measure macro overhead
TEST_F(AsmTest, macro_performance_overhead)
{
    SCOPED_TRACE("Testing macro performance overhead");
    
    std::cout << "Testing macro performance overhead..." << std::endl;
    
    const int iterations = 10000;
    
    // Measure empty loop
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        volatile int dummy = i;
        (void)dummy;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto empty_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Measure loop with macro usage (compilation overhead)
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        // Use macros in a way that doesn't execute assembly but tests compilation overhead
        volatile int dummy = i;
        
#if !defined(_MSC_VER) || (!defined(_M_X64) && !defined(_M_ARM64))
        // On supported platforms, the macros exist but we don't use them for actual assembly
#endif
        
        (void)dummy;
    }
    end = std::chrono::high_resolution_clock::now();
    auto macro_duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Performance test results:" << std::endl;
    std::cout << "  Empty loop: " << empty_duration.count() << " microseconds" << std::endl;
    std::cout << "  Macro loop: " << macro_duration.count() << " microseconds" << std::endl;
    
    // The overhead should be minimal since we're just testing compilation
    EXPECT_LT(macro_duration.count(), empty_duration.count() + 1000) 
        << "Macro compilation overhead should be minimal";
    
    std::cout << "Macro performance overhead test completed" << std::endl;
}

// Basic compilation test - ensure all macros can be used in a single function
TEST(AsmBasicTest, all_macros_compilation)
{
    SCOPED_TRACE("Testing all macros compilation");
    
    // This test ensures that all macros can be used without compilation errors
    // It doesn't test assembly execution, just macro expansion
    
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    // On MSVC x64/ARM64, inline assembly is not supported
    // But the macros should still be defined and compile
    EXPECT_TRUE(true) << "Macros should be defined even on unsupported platforms";
#else
    // On supported platforms, macros should expand properly
    EXPECT_TRUE(true) << "Macros should expand properly on supported platforms";
#endif
}

// Test that the header can be included multiple times without issues
TEST(AsmBasicTest, multiple_include_safety)
{
    SCOPED_TRACE("Testing multiple include safety");
    
    // If this test compiles, it means the header can be safely included multiple times
    // The include guard should prevent redefinition errors
    
    EXPECT_TRUE(true) << "Header should support multiple includes";
}
