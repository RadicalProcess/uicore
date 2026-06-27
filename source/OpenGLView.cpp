#include "UICore/OpenGLView.h"
#include "UICore/Style.h"

#include <array>
#include <cmath>

using namespace juce::gl;

namespace rp::uicore
{

    namespace
    {
        constexpr float pi = 3.14159265358979323846f;
        constexpr float minElevation = -60.0f;
        constexpr float maxElevation = 60.0f;
        constexpr float orbitSensitivity = 0.4f;

        using Mat4 = std::array<float, 16>;

        float degreesToRadians(float degrees)
        {
            return degrees * (pi / 180.0f);
        }

        Mat4 makePerspective(float fovYDegrees, float aspect, float nearPlane, float farPlane)
        {
            auto result = Mat4{};
            const auto f = 1.0f / std::tan(degreesToRadians(fovYDegrees) * 0.5f);
            result[0] = f / aspect;
            result[5] = f;
            result[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
            result[11] = -1.0f;
            result[14] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
            return result;
        }

        std::array<float, 3> normalise(const std::array<float, 3>& v)
        {
            const auto length = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
            if (length <= 0.0f)
                return { 0.0f, 0.0f, 0.0f };
            return { v[0] / length, v[1] / length, v[2] / length };
        }

        std::array<float, 3> cross(const std::array<float, 3>& a, const std::array<float, 3>& b)
        {
            return { a[1] * b[2] - a[2] * b[1],
                     a[2] * b[0] - a[0] * b[2],
                     a[0] * b[1] - a[1] * b[0] };
        }

        float dot(const std::array<float, 3>& a, const std::array<float, 3>& b)
        {
            return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
        }

        Mat4 makeLookAt(const std::array<float, 3>& eye, const std::array<float, 3>& centre, const std::array<float, 3>& up)
        {
            const auto forward = normalise({ centre[0] - eye[0], centre[1] - eye[1], centre[2] - eye[2] });
            const auto side = normalise(cross(forward, up));
            const auto trueUp = cross(side, forward);

            auto result = Mat4{};
            result[0] = side[0];
            result[4] = side[1];
            result[8] = side[2];
            result[1] = trueUp[0];
            result[5] = trueUp[1];
            result[9] = trueUp[2];
            result[2] = -forward[0];
            result[6] = -forward[1];
            result[10] = -forward[2];
            result[12] = -dot(side, eye);
            result[13] = -dot(trueUp, eye);
            result[14] = dot(forward, eye);
            result[15] = 1.0f;
            return result;
        }

        void appendVertex(std::vector<float>& target, float x, float y, float z, const juce::Colour& colour)
        {
            target.push_back(x);
            target.push_back(y);
            target.push_back(z);
            target.push_back(colour.getFloatRed());
            target.push_back(colour.getFloatGreen());
            target.push_back(colour.getFloatBlue());
        }

        void appendQuad(std::vector<float>& target,
                        const std::array<float, 3>& a,
                        const std::array<float, 3>& b,
                        const std::array<float, 3>& c,
                        const std::array<float, 3>& d,
                        const juce::Colour& colour)
        {
            appendVertex(target, a[0], a[1], a[2], colour);
            appendVertex(target, b[0], b[1], b[2], colour);
            appendVertex(target, c[0], c[1], c[2], colour);

            appendVertex(target, a[0], a[1], a[2], colour);
            appendVertex(target, c[0], c[1], c[2], colour);
            appendVertex(target, d[0], d[1], d[2], colour);
        }
    }

    OpenGLView::OpenGLView()
    : cubeBuffer_(0)
    , gridBuffer_(0)
    , cubeVertexCount_(0)
    , gridVertexCount_(0)
    , cameraAzimuth_(degreesToRadians(35.0f))
    , cameraElevation_(degreesToRadians(20.0f))
    , cameraDistance_(8.0f)
    {
        openGLContext_.setRenderer(this);
        openGLContext_.setContinuousRepainting(false);
        openGLContext_.attachTo(*this);
    }

    OpenGLView::~OpenGLView()
    {
        openGLContext_.detach();
    }

    void OpenGLView::buildGeometry()
    {
        cubeVertices_.clear();
        gridVertices_.clear();

        // Unit cube centred on the origin, each face a distinct colour.
        const auto h = 1.0f;
        const std::array<float, 3> nnn = { -h, -h, -h };
        const std::array<float, 3> pnn = { h, -h, -h };
        const std::array<float, 3> ppn = { h, h, -h };
        const std::array<float, 3> npn = { -h, h, -h };
        const std::array<float, 3> nnp = { -h, -h, h };
        const std::array<float, 3> pnp = { h, -h, h };
        const std::array<float, 3> ppp = { h, h, h };
        const std::array<float, 3> npp = { -h, h, h };

        appendQuad(cubeVertices_, nnp, pnp, ppp, npp, juce::Colour(214, 90, 90));   // front (+z)
        appendQuad(cubeVertices_, pnn, nnn, npn, ppn, juce::Colour(90, 170, 110));  // back  (-z)
        appendQuad(cubeVertices_, nnn, nnp, npp, npn, juce::Colour(80, 130, 210));  // left  (-x)
        appendQuad(cubeVertices_, pnp, pnn, ppn, ppp, juce::Colour(225, 190, 90));  // right (+x)
        appendQuad(cubeVertices_, npp, ppp, ppn, npn, juce::Colour(110, 200, 210)); // top   (+y)
        appendQuad(cubeVertices_, nnn, pnn, pnp, nnp, juce::Colour(190, 110, 200)); // bottom(-y)

        cubeVertexCount_ = static_cast<int>(cubeVertices_.size()) / 6;

        // Floor grid laid out on the y = -1 plane so the cube rests on it.
        const auto gridExtent = 10;
        const auto gridY = -h;
        const auto gridColour = juce::Colour(70, 90, 100);
        const auto axisColour = styles::foreground;

        for (auto i = -gridExtent; i <= gridExtent; ++i)
        {
            const auto coord = static_cast<float>(i);
            const auto extent = static_cast<float>(gridExtent);
            const auto& colour = (i == 0) ? axisColour : gridColour;

            appendVertex(gridVertices_, coord, gridY, -extent, colour);
            appendVertex(gridVertices_, coord, gridY, extent, colour);

            appendVertex(gridVertices_, -extent, gridY, coord, colour);
            appendVertex(gridVertices_, extent, gridY, coord, colour);
        }

        gridVertexCount_ = static_cast<int>(gridVertices_.size()) / 6;
    }

    void OpenGLView::newOpenGLContextCreated()
    {
        buildGeometry();

        auto newShader = std::make_unique<juce::OpenGLShaderProgram>(openGLContext_);

        const auto vertexShader = R"(
            attribute vec3 position;
            attribute vec3 sourceColour;
            uniform mat4 projectionMatrix;
            uniform mat4 viewMatrix;
            varying vec3 fragColour;
            void main()
            {
                gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
                fragColour = sourceColour;
            }
        )";

        const auto fragmentShader = R"(
            varying vec3 fragColour;
            void main()
            {
                gl_FragColor = vec4(fragColour, 1.0);
            }
        )";

        if (newShader->addVertexShader(vertexShader)
            && newShader->addFragmentShader(fragmentShader)
            && newShader->link())
        {
            shader_ = std::move(newShader);
            shader_->use();

            position_ = std::make_unique<juce::OpenGLShaderProgram::Attribute>(*shader_, "position");
            sourceColour_ = std::make_unique<juce::OpenGLShaderProgram::Attribute>(*shader_, "sourceColour");
            projectionMatrix_ = std::make_unique<juce::OpenGLShaderProgram::Uniform>(*shader_, "projectionMatrix");
            viewMatrix_ = std::make_unique<juce::OpenGLShaderProgram::Uniform>(*shader_, "viewMatrix");
        }

        glGenBuffers(1, &cubeBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, cubeBuffer_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(cubeVertices_.size() * sizeof(float)),
                     cubeVertices_.data(),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &gridBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, gridBuffer_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(gridVertices_.size() * sizeof(float)),
                     gridVertices_.data(),
                     GL_STATIC_DRAW);
    }

    void OpenGLView::renderOpenGL()
    {
        const auto background = styles::background;
        juce::OpenGLHelpers::clear(background);

        if (shader_ == nullptr)
            return;

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        const auto scale = static_cast<float>(openGLContext_.getRenderingScale());
        const auto pixelWidth = juce::roundToInt(scale * static_cast<float>(getWidth()));
        const auto pixelHeight = juce::roundToInt(scale * static_cast<float>(getHeight()));
        glViewport(0, 0, pixelWidth, pixelHeight);

        shader_->use();

        const auto aspect = (pixelHeight > 0) ? static_cast<float>(pixelWidth) / static_cast<float>(pixelHeight) : 1.0f;
        const auto projection = makePerspective(45.0f, aspect, 0.1f, 100.0f);

        const std::array<float, 3> eye = {
            cameraDistance_ * std::cos(cameraElevation_) * std::sin(cameraAzimuth_),
            cameraDistance_ * std::sin(cameraElevation_),
            cameraDistance_ * std::cos(cameraElevation_) * std::cos(cameraAzimuth_)
        };
        const auto view = makeLookAt(eye, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f });

        if (projectionMatrix_ != nullptr)
            projectionMatrix_->setMatrix4(projection.data(), 1, GL_FALSE);
        if (viewMatrix_ != nullptr)
            viewMatrix_->setMatrix4(view.data(), 1, GL_FALSE);

        const auto stride = static_cast<GLsizei>(6 * sizeof(float));

        const auto drawBuffer = [this, stride](unsigned int buffer, GLenum mode, int vertexCount)
        {
            glBindBuffer(GL_ARRAY_BUFFER, buffer);

            if (position_ != nullptr)
            {
                glVertexAttribPointer(static_cast<GLuint>(position_->attributeID), 3, GL_FLOAT, GL_FALSE, stride, nullptr);
                glEnableVertexAttribArray(static_cast<GLuint>(position_->attributeID));
            }

            if (sourceColour_ != nullptr)
            {
                glVertexAttribPointer(static_cast<GLuint>(sourceColour_->attributeID), 3, GL_FLOAT, GL_FALSE, stride,
                                      reinterpret_cast<GLvoid*>(3 * sizeof(float)));
                glEnableVertexAttribArray(static_cast<GLuint>(sourceColour_->attributeID));
            }

            glDrawArrays(mode, 0, vertexCount);

            if (position_ != nullptr)
                glDisableVertexAttribArray(static_cast<GLuint>(position_->attributeID));
            if (sourceColour_ != nullptr)
                glDisableVertexAttribArray(static_cast<GLuint>(sourceColour_->attributeID));
        };

        drawBuffer(gridBuffer_, GL_LINES, gridVertexCount_);
        drawBuffer(cubeBuffer_, GL_TRIANGLES, cubeVertexCount_);
    }

    void OpenGLView::openGLContextClosing()
    {
        if (cubeBuffer_ != 0)
            glDeleteBuffers(1, &cubeBuffer_);
        if (gridBuffer_ != 0)
            glDeleteBuffers(1, &gridBuffer_);

        cubeBuffer_ = 0;
        gridBuffer_ = 0;

        shader_.reset();
        position_.reset();
        sourceColour_.reset();
        projectionMatrix_.reset();
        viewMatrix_.reset();
    }

    void OpenGLView::mouseDown(const juce::MouseEvent& event)
    {
        lastDragPosition_ = event.getPosition();
    }

    void OpenGLView::mouseDrag(const juce::MouseEvent& event)
    {
        const auto position = event.getPosition();
        const auto deltaX = static_cast<float>(position.getX() - lastDragPosition_.getX());
        const auto deltaY = static_cast<float>(position.getY() - lastDragPosition_.getY());
        lastDragPosition_ = position;

        cameraAzimuth_ -= degreesToRadians(deltaX * orbitSensitivity);

        const auto newElevation = cameraElevation_ + degreesToRadians(deltaY * orbitSensitivity);
        cameraElevation_ = juce::jlimit(degreesToRadians(minElevation), degreesToRadians(maxElevation), newElevation);

        openGLContext_.triggerRepaint();
    }

}
