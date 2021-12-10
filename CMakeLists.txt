add_subdirectory(UITestApp)


add_library(UICore
        source/Font.cpp
        source/RotarySlider.cpp
        source/VerticalSlider.cpp
        source/Label.cpp)

target_include_directories(UICore
        PRIVATE include/UICore
        PUBLIC include)

juce_add_binary_data(UIResource
        SOURCES
        resource/JosefinSans-Bold.ttf
        resource/Logo.png)

target_link_libraries(UICore
        PUBLIC
        UIResource
        juce::juce_gui_basics)

target_compile_features(UICore PUBLIC cxx_std_17)
