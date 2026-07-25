#include "UICore/OpenGLView.h"
#include "UICore/ObjModel.h"
#include "UICore/Style.h"

#include <BinaryData.h>

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
        constexpr float minDistance = 3.0f;
        constexpr float maxDistance = 20.0f;
        constexpr float wheelZoomSensitivity = 4.0f;
        constexpr float dragZoomSensitivity = 0.02f;
        constexpr float minRoomSize = 1.0f;
        constexpr float maxRoomSize = 30.0f;
        constexpr float defaultRoomSize = 12.0f;

        constexpr float sourceSphereRadius = 0.15f;
        constexpr int sourceSphereLatitudeSteps = 8;
        constexpr int sourceSphereLongitudeSteps = 12;

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
    }

    OpenGLView::OpenGLView()
    : modelBuffer_(0)
    , gridBuffer_(0)
    , trajectoryBuffer_(0)
    , sourceBuffer_(0)
    , modelVertexCount_(0)
    , gridVertexCount_(0)
    , trajectoryVertexCount_(0)
    , sourceVertexCount_(0)
    , roomWidth_(defaultRoomSize)
    , roomHeight_(defaultRoomSize)
    , roomDepth_(defaultRoomSize)
    , gridGeometryDirty_(false)
    , sceneGeometryDirty_(false)
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
        buildModelGeometry();
        buildGridGeometry();
    }

    void OpenGLView::buildModelGeometry()
    {
        modelVertices_.clear();

        // Load the bundled head model and normalise it so its centre sits at the
        // origin regardless of the source file's units or origin.
        ObjModel model;
        model.loadFromString(juce::String::createStringFromData(BinaryData::dummyhead_obj,
                                                                BinaryData::dummyhead_objSize));

        const auto centre = model.getCentre();
        const auto maxExtent = model.getMaxExtent();

        // Target size of the model's largest dimension in world units, chosen so
        // the head is roughly as tall as the reference cube used to be.
        const auto targetSize = 2.2f;
        const auto scale = (maxExtent > 0.0f) ? targetSize / maxExtent : 1.0f;

        const auto transform = [centre, scale](const ObjModel::Position& p)
        {
            return std::array<float, 3>{ (p[0] - centre[0]) * scale,
                                         (p[1] - centre[1]) * scale,
                                         (p[2] - centre[2]) * scale };
        };

        // A single directional light baked into per-vertex colours so the flat
        // shader still conveys the model's shape. Lighting is two-sided so faces
        // are lit regardless of their winding order.
        const auto lightDirection = normalise({ 0.35f, 0.8f, 0.5f });
        const auto baseColour = juce::Colour(222, 186, 165); // warm skin tone
        const auto baseRed = baseColour.getFloatRed();
        const auto baseGreen = baseColour.getFloatGreen();
        const auto baseBlue = baseColour.getFloatBlue();

        for (const auto& triangle : model.getTriangles())
        {
            const auto a = transform(triangle.a);
            const auto b = transform(triangle.b);
            const auto c = transform(triangle.c);

            const std::array<float, 3> edge1 = { b[0] - a[0], b[1] - a[1], b[2] - a[2] };
            const std::array<float, 3> edge2 = { c[0] - a[0], c[1] - a[1], c[2] - a[2] };
            const auto normal = normalise(cross(edge1, edge2));

            const auto diffuse = std::abs(dot(normal, lightDirection));
            const auto shade = 0.3f + 0.7f * diffuse;

            const auto colour = juce::Colour::fromFloatRGBA(baseRed * shade, baseGreen * shade, baseBlue * shade, 1.0f);

            appendVertex(modelVertices_, a[0], a[1], a[2], colour);
            appendVertex(modelVertices_, b[0], b[1], b[2], colour);
            appendVertex(modelVertices_, c[0], c[1], c[2], colour);
        }

        modelVertexCount_ = static_cast<int>(modelVertices_.size()) / 6;
    }

    void OpenGLView::buildGridGeometry()
    {
        gridVertices_.clear();

        // Wireframe grid room enclosing the model: the floor, ceiling and four
        // walls are each drawn as a 1 m grid so the model appears to sit inside a
        // room. The room is a box centred on the origin, so both the room and the
        // head share (0, 0, 0) as their centre. Its extents are driven by the
        // width (x), height (y) and depth (z) set through the public setters.
        const auto gridColour = juce::Colour(70, 90, 100);

        const auto halfWidth = roomWidth_ * 0.5f;
        const auto halfHeight = roomHeight_ * 0.5f;
        const auto halfDepth = roomDepth_ * 0.5f;

        // Grid-line coordinates along one axis: the two outer edges plus a line at
        // every whole metre in between, so the room stays subdivided into 1 m
        // cells at any size.
        const auto axisCoords = [](float half)
        {
            std::vector<float> coords;
            coords.push_back(-half);

            const auto first = static_cast<int>(std::ceil(-half));
            const auto last = static_cast<int>(std::floor(half));
            for (auto i = first; i <= last; ++i)
            {
                const auto coord = static_cast<float>(i);
                if (coord > -half && coord < half)
                    coords.push_back(coord);
            }

            coords.push_back(half);
            return coords;
        };

        const auto xs = axisCoords(halfWidth);
        const auto ys = axisCoords(halfHeight);
        const auto zs = axisCoords(halfDepth);

        const auto floorY = -halfHeight;
        const auto ceilY = halfHeight;

        const auto addLine = [this, &gridColour](float x0, float y0, float z0, float x1, float y1, float z1)
        {
            appendVertex(gridVertices_, x0, y0, z0, gridColour);
            appendVertex(gridVertices_, x1, y1, z1, gridColour);
        };

        // Floor and ceiling grids (x-z planes at y = +/- halfHeight).
        for (const auto x : xs)
        {
            addLine(x, floorY, -halfDepth, x, floorY, halfDepth);
            addLine(x, ceilY, -halfDepth, x, ceilY, halfDepth);
        }
        for (const auto z : zs)
        {
            addLine(-halfWidth, floorY, z, halfWidth, floorY, z);
            addLine(-halfWidth, ceilY, z, halfWidth, ceilY, z);
        }

        // Vertical grid lines running up the four walls.
        for (const auto x : xs)
        {
            addLine(x, floorY, -halfDepth, x, ceilY, -halfDepth); // back wall  (z = -halfDepth)
            addLine(x, floorY, halfDepth, x, ceilY, halfDepth);   // front wall (z = +halfDepth)
        }
        for (const auto z : zs)
        {
            addLine(-halfWidth, floorY, z, -halfWidth, ceilY, z); // left wall  (x = -halfWidth)
            addLine(halfWidth, floorY, z, halfWidth, ceilY, z);   // right wall (x = +halfWidth)
        }

        // Horizontal rings around the walls at each height level.
        for (const auto y : ys)
        {
            addLine(-halfWidth, y, -halfDepth, halfWidth, y, -halfDepth); // back wall
            addLine(-halfWidth, y, halfDepth, halfWidth, y, halfDepth);   // front wall
            addLine(-halfWidth, y, -halfDepth, -halfWidth, y, halfDepth); // left wall
            addLine(halfWidth, y, -halfDepth, halfWidth, y, halfDepth);   // right wall
        }

        gridVertexCount_ = static_cast<int>(gridVertices_.size()) / 6;
    }

    void OpenGLView::buildTrajectoryGeometry()
    {
        trajectoryVertices_.clear();

        const auto lineColour = styles::highlight;

        for (const auto& point : trajectoryPoints_)
            appendVertex(trajectoryVertices_, point.x, point.y, point.z, lineColour);

        trajectoryVertexCount_ = static_cast<int>(trajectoryVertices_.size()) / 6;
    }

    void OpenGLView::buildSourceGeometry()
    {
        sourceVertices_.clear();

        // A small UV sphere marking the source position, drawn as filled
        // triangles so it reads clearly against the wireframe grid.
        const auto sphereColour = styles::highlight;

        const auto vertexAt = [&sourcePosition = sourcePosition_](float latitude, float longitude)
        {
            return std::array<float, 3>{
                sourcePosition.x + sourceSphereRadius * std::cos(latitude) * std::sin(longitude),
                sourcePosition.y + sourceSphereRadius * std::sin(latitude),
                sourcePosition.z + sourceSphereRadius * std::cos(latitude) * std::cos(longitude)
            };
        };

        for (auto latStep = 0; latStep < sourceSphereLatitudeSteps; ++latStep)
        {
            const auto latitude0 = -pi / 2.0f + pi * static_cast<float>(latStep) / sourceSphereLatitudeSteps;
            const auto latitude1 = -pi / 2.0f + pi * static_cast<float>(latStep + 1) / sourceSphereLatitudeSteps;

            for (auto lonStep = 0; lonStep < sourceSphereLongitudeSteps; ++lonStep)
            {
                const auto longitude0 = 2.0f * pi * static_cast<float>(lonStep) / sourceSphereLongitudeSteps;
                const auto longitude1 = 2.0f * pi * static_cast<float>(lonStep + 1) / sourceSphereLongitudeSteps;

                const auto a = vertexAt(latitude0, longitude0);
                const auto b = vertexAt(latitude1, longitude0);
                const auto c = vertexAt(latitude1, longitude1);
                const auto d = vertexAt(latitude0, longitude1);

                appendVertex(sourceVertices_, a[0], a[1], a[2], sphereColour);
                appendVertex(sourceVertices_, b[0], b[1], b[2], sphereColour);
                appendVertex(sourceVertices_, c[0], c[1], c[2], sphereColour);

                appendVertex(sourceVertices_, a[0], a[1], a[2], sphereColour);
                appendVertex(sourceVertices_, c[0], c[1], c[2], sphereColour);
                appendVertex(sourceVertices_, d[0], d[1], d[2], sphereColour);
            }
        }

        sourceVertexCount_ = static_cast<int>(sourceVertices_.size()) / 6;
    }

    void OpenGLView::newOpenGLContextCreated()
    {
        buildGeometry();
        buildTrajectoryGeometry();
        buildSourceGeometry();

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

        glGenBuffers(1, &modelBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, modelBuffer_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(modelVertices_.size() * sizeof(float)),
                     modelVertices_.data(),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &gridBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, gridBuffer_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(gridVertices_.size() * sizeof(float)),
                     gridVertices_.data(),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &trajectoryBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, trajectoryBuffer_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(trajectoryVertices_.size() * sizeof(float)),
                     trajectoryVertices_.data(),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &sourceBuffer_);
        glBindBuffer(GL_ARRAY_BUFFER, sourceBuffer_);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(sourceVertices_.size() * sizeof(float)),
                     sourceVertices_.data(),
                     GL_STATIC_DRAW);
    }

    void OpenGLView::renderOpenGL()
    {
        const auto background = styles::background;
        juce::OpenGLHelpers::clear(background);

        if (shader_ == nullptr)
            return;

        // A room dimension changed since the last frame: rebuild the grid and
        // re-upload it here, on the GL thread, before drawing.
        if (gridGeometryDirty_)
        {
            buildGridGeometry();
            glBindBuffer(GL_ARRAY_BUFFER, gridBuffer_);
            glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(gridVertices_.size() * sizeof(float)),
                         gridVertices_.data(),
                         GL_STATIC_DRAW);
            gridGeometryDirty_ = false;
        }

        // The trajectory line and source sphere follow the active gesture and
        // playhead: rebuilt and re-uploaded here, on the GL thread, whenever
        // setTrajectory/setSourcePosition changed them since the last frame.
        if (sceneGeometryDirty_)
        {
            buildTrajectoryGeometry();
            glBindBuffer(GL_ARRAY_BUFFER, trajectoryBuffer_);
            glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(trajectoryVertices_.size() * sizeof(float)),
                         trajectoryVertices_.data(),
                         GL_STATIC_DRAW);

            buildSourceGeometry();
            glBindBuffer(GL_ARRAY_BUFFER, sourceBuffer_);
            glBufferData(GL_ARRAY_BUFFER,
                         static_cast<GLsizeiptr>(sourceVertices_.size() * sizeof(float)),
                         sourceVertices_.data(),
                         GL_STATIC_DRAW);
            sceneGeometryDirty_ = false;
        }

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
        drawBuffer(modelBuffer_, GL_TRIANGLES, modelVertexCount_);
        drawBuffer(trajectoryBuffer_, GL_LINE_STRIP, trajectoryVertexCount_);
        drawBuffer(sourceBuffer_, GL_TRIANGLES, sourceVertexCount_);
    }

    void OpenGLView::openGLContextClosing()
    {
        if (modelBuffer_ != 0)
            glDeleteBuffers(1, &modelBuffer_);
        if (gridBuffer_ != 0)
            glDeleteBuffers(1, &gridBuffer_);
        if (trajectoryBuffer_ != 0)
            glDeleteBuffers(1, &trajectoryBuffer_);
        if (sourceBuffer_ != 0)
            glDeleteBuffers(1, &sourceBuffer_);

        modelBuffer_ = 0;
        gridBuffer_ = 0;
        trajectoryBuffer_ = 0;
        sourceBuffer_ = 0;

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

        // Ctrl + vertical drag zooms; a plain drag orbits the camera.
        if (event.mods.isCtrlDown())
        {
            zoomBy(deltaY * dragZoomSensitivity);
            return;
        }

        cameraAzimuth_ -= degreesToRadians(deltaX * orbitSensitivity);

        const auto newElevation = cameraElevation_ + degreesToRadians(deltaY * orbitSensitivity);
        cameraElevation_ = juce::jlimit(degreesToRadians(minElevation), degreesToRadians(maxElevation), newElevation);

        openGLContext_.triggerRepaint();
    }

    void OpenGLView::mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
    {
        zoomBy(-wheel.deltaY * wheelZoomSensitivity);
    }

    void OpenGLView::zoomBy(float amount)
    {
        cameraDistance_ = juce::jlimit(minDistance, maxDistance, cameraDistance_ + amount);
        openGLContext_.triggerRepaint();
    }

    void OpenGLView::setWidth(float widthMetres)
    {
        const auto clamped = juce::jlimit(minRoomSize, maxRoomSize, widthMetres);
        if (roomWidth_ == clamped)
            return;

        roomWidth_ = clamped;
        markGridDirty();
    }

    void OpenGLView::setHeight(float heightMetres)
    {
        const auto clamped = juce::jlimit(minRoomSize, maxRoomSize, heightMetres);
        if (roomHeight_ == clamped)
            return;

        roomHeight_ = clamped;
        markGridDirty();
    }

    void OpenGLView::setDepth(float depthMetres)
    {
        const auto clamped = juce::jlimit(minRoomSize, maxRoomSize, depthMetres);
        if (roomDepth_ == clamped)
            return;

        roomDepth_ = clamped;
        markGridDirty();
    }

    void OpenGLView::markGridDirty()
    {
        gridGeometryDirty_ = true;
        openGLContext_.triggerRepaint();
    }

    void OpenGLView::setTrajectory(std::vector<juce::Vector3D<float>> points)
    {
        trajectoryPoints_ = std::move(points);
        markSceneDirty();
    }

    void OpenGLView::setSourcePosition(juce::Vector3D<float> position)
    {
        sourcePosition_ = position;
        markSceneDirty();
    }

    void OpenGLView::markSceneDirty()
    {
        sceneGeometryDirty_ = true;
        openGLContext_.triggerRepaint();
    }

}
