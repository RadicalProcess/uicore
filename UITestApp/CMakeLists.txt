cmake_minimum_required(VERSION 3.15)

juce_add_gui_app(UITest PRODUCT_NAME "UI Test")

target_include_directories(UITest
        PUBLIC include)

target_sources(UITest
        PRIVATE
        source/Main.cpp
        source/MainComponent.cpp)

target_compile_definitions(UITest
        PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:UITest,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:UITest,JUCE_VERSION>")

target_link_libraries(UITest
        PRIVATE
        juce::juce_gui_extra
        rp::UICore
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
