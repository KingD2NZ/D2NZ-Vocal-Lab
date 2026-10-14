cmake_minimum_required(VERSION 3.22)
# JUCE's plugin wrappers include C sources. Enable C at the top level so
# CMake's compile rules are also available outside FetchContent's scope.
project(D2NZVocalLab VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(D2NZ_BUILD_PLUGIN "Build JUCE plugin (requires platform SDKs and JUCE licence review)" OFF)
add_library(d2nz_dsp INTERFACE)
target_include_directories(d2nz_dsp INTERFACE Source)
include(CTest)
if(BUILD_TESTING)
  add_executable(d2nz_dsp_tests tests/DspTests.cpp)
  target_link_libraries(d2nz_dsp_tests PRIVATE d2nz_dsp)
  add_test(NAME vocal_dsp COMMAND d2nz_dsp_tests)
endif()
if(D2NZ_BUILD_PLUGIN)
  include(FetchContent)
  FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 51a8a6d7aeae7326956d747737ccf1575e61e209)
  FetchContent_MakeAvailable(JUCE)
  set(formats VST3)
  if(APPLE)
    list(APPEND formats AU)
  endif()
  juce_add_plugin(D2NZVocalLab
    COMPANY_NAME "D2NZ" BUNDLE_ID "nl.d2nzmusic.vocallab"
    PLUGIN_MANUFACTURER_CODE D2NZ PLUGIN_CODE DvLb
    PRODUCT_NAME "D2NZ Vocal Lab" FORMATS ${formats}
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE COPY_PLUGIN_AFTER_BUILD FALSE)
  target_sources(D2NZVocalLab PRIVATE Source/PluginProcessor.cpp Source/PluginProcessor.h)
  target_link_libraries(D2NZVocalLab PRIVATE d2nz_dsp juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
  target_compile_definitions(D2NZVocalLab PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
  if(BUILD_TESTING)
    juce_add_console_app(d2nz_plugin_tests PRODUCT_NAME "D2NZ Plugin Tests")
    target_sources(d2nz_plugin_tests PRIVATE tests/PluginTests.cpp)
    target_include_directories(d2nz_plugin_tests PRIVATE Source)
    target_link_libraries(d2nz_plugin_tests PRIVATE D2NZVocalLab juce::juce_audio_utils)
    add_test(NAME plugin_state COMMAND d2nz_plugin_tests)
  endif()
endif()
