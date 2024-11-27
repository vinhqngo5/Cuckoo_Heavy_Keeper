#include "module1/module1.hpp"
#include <gtest/gtest.h>

// Test for the public function of Module1
TEST(Module1Test, RunPublicModule1) {
    // Create an instance of Module1
    Module1 module;

    // Capture the output
    testing::internal::CaptureStdout();
    module.run_public_module1();
    std::string output = testing::internal::GetCapturedStdout();

    // Check if the output matches the expected string
    EXPECT_EQ(output, "Running public module1\n");
}