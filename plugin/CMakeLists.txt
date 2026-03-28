# plugin/CMakeLists.txt
# cmake -B build -DCMAKE_BUILD_TYPE=Debug
# cmake --build build --parallel 4

# rm -rf ~/Library/Caches/AudioUnitCache/* 
# sudo rm -rf /Library/Caches/AudioUnitCache/*
# killall -9 AudioComponentRegistrar
# sudo pkgutil --forget com.dank.vocalgate

cmake_minimum_required(VERSION 3.15)

# Starting fresh at v0.1.0 for the prototype
project(FrictionalExciter VERSION 0.1.0)
message(STATUS "Building Local Prototype: ${PROJECT_VERSION}")

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.12 
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(FrictionalExciter
    COMPANY_NAME "DanK"                 
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE  # We can change this to TRUE later for Pillar A tuning
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE  
    VST3_AUTO_MANIFEST FALSE          
    PLUGIN_MANUFACTURER_CODE "DanK"
    PLUGIN_CODE "FrcX"      # Must be exactly 4 characters, unique                 
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Frictional Exciter"
)

juce_generate_juce_header(FrictionalExciter)

# Source files
# Notice: No PluginEditor.cpp! The Generic UI handles it.
target_sources(FrictionalExciter
    PRIVATE
        Source/PluginProcessor.cpp
        ../core_dsp/FrictionalEngine.cpp
        ../core_dsp/FrictionalEngine.h
)

# Include directories bridging the JUCE wrapper to your pure DSP
target_include_directories(FrictionalExciter
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
        ${CMAKE_CURRENT_SOURCE_DIR}/../core_dsp
)

# Standard JUCE definitions 
target_compile_definitions(FrictionalExciter
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

# Base JUCE linking 
target_link_libraries(FrictionalExciter
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Mac-Specific Local Auto-Install
if(APPLE)
    # Auto-copy VST3 to your local Mac directory
    add_custom_command(TARGET FrictionalExciter_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_FILE_DIR:FrictionalExciter_VST3>/../.."
            "$ENV{HOME}/Library/Audio/Plug-Ins/VST3/Frictional Exciter.vst3"
    )
    
    # Auto-copy AU to your local Mac directory
    add_custom_command(TARGET FrictionalExciter_AU POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_FILE_DIR:FrictionalExciter_AU>/../.."
            "$ENV{HOME}/Library/Audio/Plug-Ins/Components/Frictional Exciter.component"
    )
endif() 

# Minimal CTest setup for your core DSP logic
enable_testing()

add_executable(RunDSPTests 
    ../tests/CPP/test_main.cpp
)

target_link_libraries(RunDSPTests 
    PRIVATE 
        juce::juce_audio_basics 
        juce::juce_core
)

target_include_directories(RunDSPTests 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../core_dsp
)

add_test(NAME CoreDSPMathTest COMMAND RunDSPTests)

