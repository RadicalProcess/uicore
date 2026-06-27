#pragma once

#include <juce_opengl/juce_opengl.h>

#include <memory>
#include <vector>

namespace rp::uicore
{

    // An OpenGL component that renders a cube standing on a reference floor grid.
    // The camera orbits the cube: horizontal mouse drags rotate around it, while
    // vertical drags raise or lower the camera within a fixed elevation range.
    // The mouse wheel, or a Ctrl + vertical drag, zooms the camera in and out
    // towards the central cube within a fixed distance range.
    class OpenGLView : public juce::Component,
                       private juce::OpenGLRenderer
    {
    public:
        OpenGLView();
        ~OpenGLView() override;

        void newOpenGLContextCreated() override;
        void renderOpenGL() override;
        void openGLContextClosing() override;

    private:
        void mouseDown(const juce::MouseEvent& event) override;
        void mouseDrag(const juce::MouseEvent& event) override;
        void mouseWheelMove(const juce::MouseEvent& event, const juce::MouseWheelDetails& wheel) override;

        void zoomBy(float amount);

        void buildGeometry();

        juce::OpenGLContext openGLContext_;
        std::unique_ptr<juce::OpenGLShaderProgram> shader_;
        std::unique_ptr<juce::OpenGLShaderProgram::Attribute> position_;
        std::unique_ptr<juce::OpenGLShaderProgram::Attribute> sourceColour_;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> projectionMatrix_;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> viewMatrix_;

        unsigned int cubeBuffer_;
        unsigned int gridBuffer_;
        std::vector<float> cubeVertices_;
        std::vector<float> gridVertices_;
        int cubeVertexCount_;
        int gridVertexCount_;

        float cameraAzimuth_;
        float cameraElevation_;
        float cameraDistance_;
        juce::Point<int> lastDragPosition_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLView)
    };

}
