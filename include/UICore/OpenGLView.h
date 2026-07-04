#pragma once

#include <juce_opengl/juce_opengl.h>

#include <memory>
#include <vector>

namespace rp::uicore
{

    // An OpenGL component that renders a 3D model (a human head loaded from a
    // bundled OBJ file) standing inside a wireframe grid room. The camera orbits
    // the model: horizontal mouse drags rotate around it, while vertical drags
    // raise or lower the camera within a fixed elevation range. The mouse wheel,
    // or a Ctrl + vertical drag, zooms the camera in and out towards the central
    // model within a fixed distance range.
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

        unsigned int modelBuffer_;
        unsigned int gridBuffer_;
        std::vector<float> modelVertices_;
        std::vector<float> gridVertices_;
        int modelVertexCount_;
        int gridVertexCount_;

        float cameraAzimuth_;
        float cameraElevation_;
        float cameraDistance_;
        juce::Point<int> lastDragPosition_;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OpenGLView)
    };

}
