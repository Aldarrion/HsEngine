#pragma once

#include "Math/Math.h"

namespace hs
{

//------------------------------------------------------------------------------
struct PropertyContainer;

//------------------------------------------------------------------------------
enum class ProjectionType
{
    Perspective,
    Orthographic
};

//------------------------------------------------------------------------------
struct Camera
{
    Mat44 toCamera_;
    Mat44 toProjection_;

    Vec3 pos_{ 0, 0, -5 };
    Vec3 forward_{ Vec3::FORWARD() };
    Vec3 right_{ Vec3::RIGHT() };

    Vec2 angles_{ 0, HS_PI_HALF };
    float fovy_{ 75 };
    float near_{ 0.01f };
    float far_{ 1000 };
    float horizontalExtent_{ 128 };

    ProjectionType projectionType_{ ProjectionType::Orthographic };
};

//------------------------------------------------------------------------------
struct CameraFreelyController
{
    Camera* camera_;
    float freeflySpeed_{ 20 };
};

//------------------------------------------------------------------------------
void CameraInitAsPerspective(Camera* camera, const Vec3& pos, const Vec3& target, float fovY = 75, float near = 0.01f, float far = 1000.0f);
void CameraInit(Camera* camera, const PropertyContainer& data);
void CameraFillData(Camera* camera, PropertyContainer& data);
Box2D CameraGetOrthoFrustum(const Camera* camera);
void CameraUpdate(Camera* camera);

void CameraUpdateFreeFly(CameraFreelyController* freeflyCamera);

}
