add_subdirectory(UITestApp)


add_library(UICore
        source/Font.cpp
        source/StepSlider.cpp
        source/GlissonSlider.cpp
        source/Label.cpp)


add_library(rp::UICore ALIAS UICore)

target_include_directories(UICore
        PRIVATE include/UICore
        PUBLIC include)

juce_add_binary_data(UIResource
        SOURCES
        resource/RobotoCondensed.ttf
        resource/Logo.png)

target_link_libraries(UICore
        PUBLIC
        UIResource
        juce::juce_gui_basics)

target_compile_features(UICore PUBLIC cxx_std_17)
