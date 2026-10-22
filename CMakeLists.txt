cmake_minimum_required(VERSION 3.22)

project(Instro VERSION 0.3.0 LANGUAGES C CXX)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(FetchContent)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/JUCE-9.0.4.zip")
    FetchContent_Declare(JUCE
        URL "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/JUCE-9.0.4.zip"
        URL_HASH SHA256=f8b33c9094c004a8888ae557374f32c4690618764bbb80431bae314cf34e064b
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
else()
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 9.0.4
    GIT_SHALLOW TRUE
    GIT_SUBMODULES_RECURSE TRUE
)
endif()
FetchContent_MakeAvailable(JUCE)

set(SFIZZ_JACK OFF CACHE BOOL "" FORCE)
set(SFIZZ_RENDER OFF CACHE BOOL "" FORCE)
set(SFIZZ_TESTS OFF CACHE BOOL "" FORCE)
set(SFIZZ_SHARED OFF CACHE BOOL "" FORCE)
set(ENABLE_LTO OFF CACHE BOOL "" FORCE)
set(INSTRO_SFIZZ_ARCHIVE "https://github.com/sfztools/sfizz/releases/download/1.2.3/sfizz-1.2.3.tar.gz")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/sfizz-1.2.3.tar.gz")
    set(INSTRO_SFIZZ_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/Dependencies/sfizz-1.2.3.tar.gz")
endif()
FetchContent_Declare(sfizz
    URL "${INSTRO_SFIZZ_ARCHIVE}"
    URL_HASH SHA256=a9339eac7620d7f0f6b44bdfe860680fab73e66efad4b5f15b21198dd9436822
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(sfizz)

juce_add_plugin(Instro
    COMPANY_NAME "Instro Audio"
    BUNDLE_ID com.instro.audio.natural
    PLUGIN_MANUFACTURER_CODE InAu
    PLUGIN_CODE In03
    PRODUCT_NAME "Instro Natural"
    FORMATS VST3 Standalone
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
)

juce_generate_juce_header(Instro)

target_sources(Instro PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/SampleEngine.cpp
)

target_compile_features(Instro PRIVATE cxx_std_17)
target_compile_definitions(Instro PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Instro PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    sfizz::sfizz
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
option(INSTRO_BUILD_TESTS "Build offline sampler smoke tests" OFF)
if(INSTRO_BUILD_TESTS)
    juce_add_console_app(InstroChecks PRODUCT_NAME "Instro checks")
    target_sources(InstroChecks PRIVATE tests/SmokeTest.cpp)
    target_include_directories(InstroChecks PRIVATE src "$<TARGET_PROPERTY:Instro,INCLUDE_DIRECTORIES>")
    target_link_libraries(InstroChecks PRIVATE Instro sfizz::sfizz juce::juce_audio_utils juce::juce_dsp)
    target_compile_definitions(InstroChecks PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_compile_features(InstroChecks PRIVATE cxx_std_17)
    juce_generate_juce_header(InstroChecks)
endif()
