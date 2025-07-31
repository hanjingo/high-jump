#include <gtest/gtest.h>
#include <libcpp/hardware/ram.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <memory>

class RamTest : public ::testing::Test {
protected:
    void SetUp() override {
        std::cout << "\n=== Random Access Memory (RAM) Tests Setup ===" << std::endl;
        
        // Initialize RAM subsystem
        ram_error_t result = ram_init();
        ASSERT_EQ(result, RAM_SUCCESS) << "Failed to initialize RAM subsystem: " 
                                       << ram_get_error_string(result);
        
        // Print platform information
        std::cout << "Platform: ";
#if defined(RAM_PLATFORM_WINDOWS)
        std::cout << "Windows" << std::endl;
#elif defined(RAM_PLATFORM_LINUX)
        std::cout << "Linux" << std::endl;
#elif defined(RAM_PLATFORM_MACOS)
        std::cout << "macOS" << std::endl;
#else
        std::cout << "Unknown" << std::endl;
#endif
        
        std::cout << "=============================================" << std::endl;
    }
    
    void TearDown() override {
        std::cout << "=== Random Access Memory (RAM) Tests Teardown ===" << std::endl;
        ram_cleanup();
    }
};

// Test RAM subsystem initialization and cleanup
TEST_F(RamTest, initialization_and_cleanup)
{
    SCOPED_TRACE("Testing RAM initialization and cleanup");
    
    std::cout << "Testing RAM subsystem initialization..." << std::endl;
    
    // Cleanup and re-initialize to test the functions
    ram_cleanup();
    
    ram_error_t result = ram_init();
    EXPECT_EQ(result, RAM_SUCCESS) << "RAM initialization should succeed";
    
    // Test multiple initializations (should be safe)
    result = ram_init();
    EXPECT_EQ(result, RAM_SUCCESS) << "Multiple RAM initializations should be safe";
    
    // Test cleanup (should be safe to call multiple times)
    ram_cleanup();
    ram_cleanup();
    
    // Re-initialize for other tests
    result = ram_init();
    EXPECT_EQ(result, RAM_SUCCESS) << "RAM re-initialization should succeed";
    
    std::cout << "RAM initialization and cleanup tests completed" << std::endl;
}

// Test error string functionality
TEST_F(RamTest, error_string_functionality)
{
    SCOPED_TRACE("Testing error string functionality");
    
    std::cout << "Testing error string conversion..." << std::endl;
    
    // Test all error codes
    EXPECT_STREQ(ram_get_error_string(RAM_SUCCESS), "Success");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_INVALID_PARAMETER), "Invalid parameter");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_ACCESS_DENIED), "Access denied");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_NOT_FOUND), "Memory region not found");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_INSUFFICIENT_BUFFER), "Insufficient buffer");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_SYSTEM_ERROR), "System error");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_NOT_SUPPORTED), "Operation not supported");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_INSUFFICIENT_MEMORY), "Insufficient memory");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_ALLOCATION_FAILED), "Memory allocation failed");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_INVALID_ADDRESS), "Invalid memory address");
    EXPECT_STREQ(ram_get_error_string(RAM_ERROR_PROTECTION_VIOLATION), "Memory protection violation");
    
    // Test unknown error code
    EXPECT_STREQ(ram_get_error_string(static_cast<ram_error_t>(-999)), "Unknown error");
    
    std::cout << "Error string tests completed" << std::endl;
}

// Test system memory information
TEST_F(RamTest, system_memory_information)
{
    SCOPED_TRACE("Testing system memory information");
    
    std::cout << "Testing system memory information retrieval..." << std::endl;
    
    // Test invalid parameter
    ram_error_t result = ram_get_system_info(nullptr);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null parameter";
    
    // Test valid system info retrieval
    ram_system_info_t info;
    result = ram_get_system_info(&info);
    EXPECT_EQ(result, RAM_SUCCESS) << "System info retrieval should succeed";
    
    if (result == RAM_SUCCESS) {
        std::cout << "System Memory Information:" << std::endl;
        
        char buffer[64];
        ram_format_size(info.total_physical, buffer, sizeof(buffer));
        std::cout << "  Total Physical RAM: " << buffer << std::endl;
        
        ram_format_size(info.available_physical, buffer, sizeof(buffer));
        std::cout << "  Available Physical RAM: " << buffer << std::endl;
        
        ram_format_size(info.used_physical, buffer, sizeof(buffer));
        std::cout << "  Used Physical RAM: " << buffer << std::endl;
        
        ram_format_size(info.total_virtual, buffer, sizeof(buffer));
        std::cout << "  Total Virtual Memory: " << buffer << std::endl;
        
        ram_format_size(info.available_virtual, buffer, sizeof(buffer));
        std::cout << "  Available Virtual Memory: " << buffer << std::endl;
        
        ram_format_size(info.total_swap, buffer, sizeof(buffer));
        std::cout << "  Total Swap Space: " << buffer << std::endl;
        
        ram_format_size(info.available_swap, buffer, sizeof(buffer));
        std::cout << "  Available Swap Space: " << buffer << std::endl;
        
        std::cout << "  Memory Load: " << info.memory_load << "%" << std::endl;
        std::cout << "  Page Size: " << info.page_size << " bytes" << std::endl;
        std::cout << "  Large Page Size: " << info.large_page_size << " bytes" << std::endl;
        std::cout << "  Module Count: " << info.module_count << std::endl;
        
        // Validate the information
        EXPECT_GT(info.total_physical, 0) << "Total physical memory should be greater than 0";
        EXPECT_LE(info.used_physical, info.total_physical) << "Used memory should not exceed total";
        EXPECT_LE(info.available_physical, info.total_physical) << "Available memory should not exceed total";
        EXPECT_GE(info.memory_load, 0.0) << "Memory load should be non-negative";
        EXPECT_LE(info.memory_load, 100.0) << "Memory load should not exceed 100%";
        EXPECT_GT(info.page_size, 0) << "Page size should be greater than 0";
        EXPECT_TRUE(info.page_size == 4096 || info.page_size == 8192 || info.page_size == 16384 || info.page_size == 65536) 
            << "Page size should be a common page size";
    }
    
    std::cout << "System memory information tests completed" << std::endl;
}

// Test RAM module enumeration
TEST_F(RamTest, ram_module_enumeration)
{
    SCOPED_TRACE("Testing RAM module enumeration");
    
    std::cout << "Testing RAM module enumeration..." << std::endl;
    
    // Test invalid parameters
    uint32_t actual_count = 0;
    ram_error_t result = ram_get_modules(nullptr, 4, &actual_count);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null modules array";
    
    ram_module_info_t modules[4];
    result = ram_get_modules(modules, 4, nullptr);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null actual_count";
    
    result = ram_get_modules(modules, 0, &actual_count);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for zero max_modules";
    
    // Test valid module enumeration
    result = ram_get_modules(modules, RAM_MAX_MODULES, &actual_count);
    if (result == RAM_SUCCESS) {
        std::cout << "Found " << actual_count << " RAM modules:" << std::endl;
        
        for (uint32_t i = 0; i < actual_count; i++) {
            const ram_module_info_t& module = modules[i];
            
            std::cout << "  Module " << i + 1 << ":" << std::endl;
            std::cout << "    Manufacturer: " << module.manufacturer << std::endl;
            std::cout << "    Part Number: " << module.part_number << std::endl;
            std::cout << "    Serial Number: " << module.serial_number << std::endl;
            std::cout << "    Type: " << ram_type_to_string(module.type) << std::endl;
            
            char capacity_str[64];
            ram_format_size(module.capacity, capacity_str, sizeof(capacity_str));
            std::cout << "    Capacity: " << capacity_str << std::endl;
            
            std::cout << "    Speed: " << module.speed << " MHz" << std::endl;
            std::cout << "    Voltage: " << module.voltage << " mV" << std::endl;
            std::cout << "    Slot Number: " << module.slot_number << std::endl;
            std::cout << "    ECC: " << (module.is_ecc ? "Yes" : "No") << std::endl;
            std::cout << "    Registered: " << (module.is_registered ? "Yes" : "No") << std::endl;
            
            // Validate module data
            EXPECT_GT(module.capacity, 0) << "Module capacity should be greater than 0";
            EXPECT_GT(module.slot_number, 0) << "Slot number should be greater than 0";
        }
        
        EXPECT_GE(actual_count, 1) << "Should have at least one RAM module";
        EXPECT_LE(actual_count, RAM_MAX_MODULES) << "Module count should not exceed maximum";
    } else {
        std::cout << "RAM module enumeration not supported on this platform" << std::endl;
    }
    
    std::cout << "RAM module enumeration tests completed" << std::endl;
}

// Test aligned memory allocation
TEST_F(RamTest, aligned_memory_allocation)
{
    SCOPED_TRACE("Testing aligned memory allocation");
    
    std::cout << "Testing aligned memory allocation..." << std::endl;
    
    // Test invalid parameters
    void* ptr = nullptr;
    ram_error_t result = ram_allocate_aligned(0, 16, &ptr);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for zero size";
    
    result = ram_allocate_aligned(1024, 0, &ptr);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for zero alignment";
    
    result = ram_allocate_aligned(1024, 16, nullptr);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null pointer";
    
    result = ram_allocate_aligned(1024, 15, &ptr);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for non-power-of-2 alignment";
    
    // Test valid allocations with different alignments
    const size_t test_size = 4096;
    const size_t alignments[] = {16, 32, 64, 128, 256, 512, 1024};
    const size_t num_alignments = sizeof(alignments) / sizeof(alignments[0]);
    
    for (size_t i = 0; i < num_alignments; i++) {
        void* aligned_ptr = nullptr;
        result = ram_allocate_aligned(test_size, alignments[i], &aligned_ptr);
        EXPECT_EQ(result, RAM_SUCCESS) << "Aligned allocation should succeed for alignment " << alignments[i];
        
        if (result == RAM_SUCCESS) {
            EXPECT_NE(aligned_ptr, nullptr) << "Allocated pointer should not be null";
            
            // Check alignment
            uintptr_t addr = (uintptr_t)aligned_ptr;
            EXPECT_EQ(addr % alignments[i], 0) << "Memory should be aligned to " << alignments[i] << " bytes";
            
            // Test that we can write to the memory
            memset(aligned_ptr, 0xAA, test_size);
            
            // Verify the memory content
            uint8_t* byte_ptr = (uint8_t*)aligned_ptr;
            for (size_t j = 0; j < test_size; j++) {
                EXPECT_EQ(byte_ptr[j], 0xAA) << "Memory content should be preserved";
            }
            
            // Free the memory
            result = ram_free_aligned(aligned_ptr);
            EXPECT_EQ(result, RAM_SUCCESS) << "Aligned memory deallocation should succeed";
        }
    }
    
    // Test freeing null pointer
    result = ram_free_aligned(nullptr);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for freeing null pointer";
    
    std::cout << "Aligned memory allocation tests completed" << std::endl;
}

// Test large page allocation
TEST_F(RamTest, large_page_allocation)
{
    SCOPED_TRACE("Testing large page allocation");
    
    std::cout << "Testing large page allocation..." << std::endl;
    
    // Test invalid parameters
    void* ptr = nullptr;
    ram_error_t result = ram_allocate_large_pages(0, &ptr);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for zero size";
    
    result = ram_allocate_large_pages(1024, nullptr);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null pointer";
    
    // Test large page allocation (may fail if not supported or insufficient privileges)
    const size_t large_page_size = 2 * 1024 * 1024; // 2 MB
    result = ram_allocate_large_pages(large_page_size, &ptr);
    
    if (result == RAM_SUCCESS) {
        std::cout << "Large page allocation succeeded" << std::endl;
        
        EXPECT_NE(ptr, nullptr) << "Large page pointer should not be null";
        
        // Test that we can write to the memory
        memset(ptr, 0x55, large_page_size);
        
        // Verify some of the memory content
        uint8_t* byte_ptr = (uint8_t*)ptr;
        for (size_t i = 0; i < 1024; i += 64) {
            EXPECT_EQ(byte_ptr[i], 0x55) << "Large page memory content should be preserved";
        }
        
        // Free the large pages
        result = ram_free_large_pages(ptr, large_page_size);
        EXPECT_EQ(result, RAM_SUCCESS) << "Large page deallocation should succeed";
        
    } else {
        std::cout << "Large page allocation not supported or insufficient privileges: " 
                  << ram_get_error_string(result) << std::endl;
        // This is expected on many systems without special configuration
    }
    
    // Test freeing with invalid parameters
    result = ram_free_large_pages(nullptr, large_page_size);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null pointer";
    
    result = ram_free_large_pages((void*)0x1000, 0);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for zero size";
    
    std::cout << "Large page allocation tests completed" << std::endl;
}

// Test memory protection
TEST_F(RamTest, memory_protection)
{
    SCOPED_TRACE("Testing memory protection");
    
    std::cout << "Testing memory protection..." << std::endl;
    
    // Allocate memory for protection tests
    const size_t test_size = 4096;
    void* ptr = nullptr;
    ram_error_t result = ram_allocate_aligned(test_size, 4096, &ptr);
    ASSERT_EQ(result, RAM_SUCCESS) << "Memory allocation should succeed";
    ASSERT_NE(ptr, nullptr) << "Allocated pointer should not be null";
    
    // Test invalid parameters
    result = ram_protect_memory(nullptr, test_size, RAM_PROTECTION_READ_WRITE);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null pointer";
    
    result = ram_protect_memory(ptr, 0, RAM_PROTECTION_READ_WRITE);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for zero size";
    
    // Test different protection modes
    const ram_protection_t protections[] = {
        RAM_PROTECTION_READ,
        RAM_PROTECTION_READ_WRITE,
        RAM_PROTECTION_READ_EXECUTE,
        RAM_PROTECTION_NONE
    };
    const size_t num_protections = sizeof(protections) / sizeof(protections[0]);
    
    for (size_t i = 0; i < num_protections; i++) {
        result = ram_protect_memory(ptr, test_size, protections[i]);
        
        if (result == RAM_SUCCESS) {
            std::cout << "Successfully set protection mode " << protections[i] << std::endl;
            
            // Test read access (except for NONE protection)
            if (protections[i] != RAM_PROTECTION_NONE) {
                // Try to read from memory
                volatile uint8_t* byte_ptr = (volatile uint8_t*)ptr;
                volatile uint8_t dummy = byte_ptr[0];
                (void)dummy; // Suppress unused variable warning
            }
            
            // Test write access (only for protections that allow write)
            if (protections[i] & RAM_PROTECTION_WRITE) {
                uint8_t* byte_ptr = (uint8_t*)ptr;
                byte_ptr[0] = 0x42;
                EXPECT_EQ(byte_ptr[0], 0x42) << "Write should succeed with write protection";
            }
        } else {
            std::cout << "Protection mode " << protections[i] << " not supported: " 
                      << ram_get_error_string(result) << std::endl;
        }
    }
    
    // Restore read-write protection for cleanup
    ram_protect_memory(ptr, test_size, RAM_PROTECTION_READ_WRITE);
    
    // Clean up
    ram_free_aligned(ptr);
    
    std::cout << "Memory protection tests completed" << std::endl;
}

// Test memory locking and unlocking
TEST_F(RamTest, memory_locking)
{
    SCOPED_TRACE("Testing memory locking");
    
    std::cout << "Testing memory locking..." << std::endl;
    
    // Allocate memory for locking tests
    const size_t test_size = 4096;
    void* ptr = nullptr;
    ram_error_t result = ram_allocate_aligned(test_size, 4096, &ptr);
    ASSERT_EQ(result, RAM_SUCCESS) << "Memory allocation should succeed";
    ASSERT_NE(ptr, nullptr) << "Allocated pointer should not be null";
    
    // Test invalid parameters
    result = ram_lock_memory(nullptr, test_size);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null pointer";
    
    result = ram_lock_memory(ptr, 0);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for zero size";
    
    // Test memory locking
    result = ram_lock_memory(ptr, test_size);
    if (result == RAM_SUCCESS) {
        std::cout << "Memory locking succeeded" << std::endl;
        
        // Memory should still be accessible
        memset(ptr, 0x33, test_size);
        uint8_t* byte_ptr = (uint8_t*)ptr;
        EXPECT_EQ(byte_ptr[0], 0x33) << "Locked memory should be accessible";
        
        // Test memory unlocking
        result = ram_unlock_memory(ptr, test_size);
        EXPECT_EQ(result, RAM_SUCCESS) << "Memory unlocking should succeed";
        
        if (result == RAM_SUCCESS) {
            std::cout << "Memory unlocking succeeded" << std::endl;
            
            // Memory should still be accessible after unlocking
            EXPECT_EQ(byte_ptr[0], 0x33) << "Unlocked memory should still be accessible";
        }
    } else {
        std::cout << "Memory locking not supported or insufficient privileges: " 
                  << ram_get_error_string(result) << std::endl;
    }
    
    // Test invalid parameters for unlocking
    result = ram_unlock_memory(nullptr, test_size);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null pointer";
    
    result = ram_unlock_memory(ptr, 0);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for zero size";
    
    // Clean up
    ram_free_aligned(ptr);
    
    std::cout << "Memory locking tests completed" << std::endl;
}

// Test memory prefetching
TEST_F(RamTest, memory_prefetching)
{
    SCOPED_TRACE("Testing memory prefetching");
    
    std::cout << "Testing memory prefetching..." << std::endl;
    
    // Allocate memory for prefetching tests
    const size_t test_size = 64 * 1024; // 64 KB
    void* ptr = nullptr;
    ram_error_t result = ram_allocate_aligned(test_size, 4096, &ptr);
    ASSERT_EQ(result, RAM_SUCCESS) << "Memory allocation should succeed";
    ASSERT_NE(ptr, nullptr) << "Allocated pointer should not be null";
    
    // Initialize memory with test data
    for (size_t i = 0; i < test_size; i++) {
        ((uint8_t*)ptr)[i] = (uint8_t)(i & 0xFF);
    }
    
    // Test invalid parameters
    result = ram_prefetch_memory(nullptr, test_size);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for null pointer";
    
    result = ram_prefetch_memory(ptr, 0);
    EXPECT_EQ(result, RAM_ERROR_INVALID_PARAMETER) << "Should return error for zero size";
    
    // Test memory prefetching
    auto start_time = std::chrono::high_resolution_clock::now();
    result = ram_prefetch_memory(ptr, test_size);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    if (result == RAM_SUCCESS) {
        std::cout << "Memory prefetching succeeded in " << duration.count() << " microseconds" << std::endl;
        
        // Verify that memory is still accessible and correct
        for (size_t i = 0; i < 1024; i += 64) {
            EXPECT_EQ(((uint8_t*)ptr)[i], (uint8_t)(i & 0xFF)) 
                << "Prefetched memory content should be preserved";
        }
    } else {
        std::cout << "Memory prefetching not supported: " << ram_get_error_string(result) << std::endl;
    }
    
    // Clean up
    ram_free_aligned(ptr);
    
    std::cout << "Memory prefetching tests completed" << std::endl;
}

// Test utility functions
TEST_F(RamTest, utility_functions)
{
    SCOPED_TRACE("Testing utility functions");
    
    std::cout << "Testing utility functions..." << std::endl;
    
    // Test RAM type to string conversion
    EXPECT_STREQ(ram_type_to_string(RAM_TYPE_DDR), "DDR");
    EXPECT_STREQ(ram_type_to_string(RAM_TYPE_DDR2), "DDR2");
    EXPECT_STREQ(ram_type_to_string(RAM_TYPE_DDR3), "DDR3");
    EXPECT_STREQ(ram_type_to_string(RAM_TYPE_DDR4), "DDR4");
    EXPECT_STREQ(ram_type_to_string(RAM_TYPE_DDR5), "DDR5");
    EXPECT_STREQ(ram_type_to_string(RAM_TYPE_SDRAM), "SDRAM");
    EXPECT_STREQ(ram_type_to_string(RAM_TYPE_SRAM), "SRAM");
    EXPECT_STREQ(ram_type_to_string(RAM_TYPE_RDRAM), "RDRAM");
    EXPECT_STREQ(ram_type_to_string(RAM_TYPE_UNKNOWN), "Unknown");
    
    // Test RAM size formatting
    char buffer[64];
    EXPECT_STREQ(ram_format_size(0, buffer, sizeof(buffer)), "0 bytes");
    EXPECT_STREQ(ram_format_size(1, buffer, sizeof(buffer)), "1 byte");
    EXPECT_STREQ(ram_format_size(1023, buffer, sizeof(buffer)), "1023 bytes");
    EXPECT_STREQ(ram_format_size(1024, buffer, sizeof(buffer)), "1 KB");
    EXPECT_STREQ(ram_format_size(2 * 1024, buffer, sizeof(buffer)), "2 KB");
    EXPECT_STREQ(ram_format_size(1024 * 1024, buffer, sizeof(buffer)), "1 MB");
    EXPECT_STREQ(ram_format_size(2 * 1024 * 1024, buffer, sizeof(buffer)), "2 MB");
    EXPECT_STREQ(ram_format_size(1024 * 1024 * 1024, buffer, sizeof(buffer)), "1 GB");
    EXPECT_STREQ(ram_format_size(2 * 1024 * 1024 * 1024, buffer, sizeof(buffer)), "2 GB");
    
    std::cout << "Utility function tests completed" << std::endl;
}

