// tests/CPP/test_main.cpp
#include <juce_core/juce_core.h>

// --- 1. Define a Dummy Test Suite ---
class PlaceholderTest : public juce::UnitTest
{
public:
    PlaceholderTest() : juce::UnitTest("Placeholder Test") {}

    void runTest() override
    {
        beginTest("Initial CMake Pipeline Check");
        
        // expect() takes a boolean. True means pass, False means fail.
        // This will always pass and print our success message to the console.
        expect(true, "CMake pipeline is successfully wired up!");
    }
};

// --- 2. Register the Test ---
// By creating a static instance, JUCE automatically adds it to the list of tests to run.
static PlaceholderTest placeholderTest;

// --- 3. The Main Entry Point ---
// This is what CMake/CTest runs when it executes the test suite.
int main (int argc, char* argv[])
{
    // Initialize the JUCE test runner
    juce::UnitTestRunner runner;
    
    // Run all registered tests (just our PlaceholderTest for now)
    runner.runAllTests();
    
    // Returning 0 tells CMake/CTest that the executable finished successfully without crashing.
    return 0; 
}

