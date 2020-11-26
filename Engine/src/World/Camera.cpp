#include "World/Camera.h"

#include "Input/Input.h"
#include "Render/Render.h"
#include "Common/Logging.h"
#include "Resources/Serialization.h"
#include "Engine.h"

namespace hs
{

//------------------------------------------------------------------------------
void Camera::InitAsPerspective(const Vec3& pos, const Vec3& target, float fovY, float near, float far)
{
    projectionType_ = ProjectionType::Perspective;

    pos_ = pos;
    forward_ = (target - pos).Normalized();
    forward_.y = Clamp(forward_.y, -0.99f, 0.99f);

    right_ = Vec3::UP().Cross(forward_).Normalized();
    fovy_ = fovY;
    near_ = near;
    far_ = far;

    UpdateMatrices();

    angles_.x = asinf(forward_.y);
    float cosX = cosf(angles_.x);

    float y1 = acosf(forward_.x / cosX);
    float y2 = 2 * HS_PI - y1;

    float d1 = abs(sin(y1) - (forward_.z / cosX));
    float d2 = abs(sin(y2) - (forward_.z / cosX));

    if (d1 < d2)
        angles_.y = y1;
    else
        angles_.y = y2;
}

//------------------------------------------------------------------------------
void Camera::Init(const PropertyContainer& data)
{
    pos_ = data.GetValue(CameraDef::POSITION).V3;
    angles_ = data.GetValue(CameraDef::ANGLES).V2;
    UpdateCameraVectors();
}

//------------------------------------------------------------------------------
void Camera::FillData(PropertyContainer& data)
{
    data.Insert({ CameraDef::POSITION, PropertyValue(PropertyType::Vec3, pos_) });
    data.Insert({ CameraDef::ANGLES, PropertyValue(PropertyType::Vec2, angles_) });
}

//------------------------------------------------------------------------------
void Camera::UpdateCameraVectors()
{
    static constexpr float PITCH_LIMIT{ DegToRad(89.0f) };

    angles_.x  = Clamp(angles_.x, -PITCH_LIMIT, PITCH_LIMIT);

    forward_.x = cos(angles_.y) * cos(angles_.x);
    forward_.y = sin(angles_.x);
    forward_.z = sin(angles_.y) * cos(angles_.x);

    right_ = Vec3::UP().Cross(forward_).Normalized();
}

//------------------------------------------------------------------------------
void Camera::UpdateMatrices()
{
    if (projectionType_ == ProjectionType::Orthographic)
    {
        projection_ = MakeOrthographicProjection(-horizontalExtent_, horizontalExtent_, -horizontalExtent_ / g_Render->GetAspect(), horizontalExtent_ / g_Render->GetAspect(), near_, far_);
    }
    else
    {
        projection_ = MakePerspectiveProjection(DegToRad(fovy_), g_Render->GetAspect(), near_, far_);
    }

    toCamera_ = MakeLookAt(pos_, pos_ + forward_);
}

//------------------------------------------------------------------------------
void Camera::UpdateFreeFly()
{
    bool isMoveMode = g_Input->GetState(VK_RBUTTON);
    if (isMoveMode)
    {
        g_Input->SetMouseMode(MouseMode::Relative);
        Vec2 mouseDelta = g_Input->GetMouseDelta();
        angles_.x -= mouseDelta.y * 0.0025f;
        angles_.y -= mouseDelta.x * 0.0025f;
        
        if (mouseDelta != Vec2{})
        {
            UpdateCameraVectors();
        }
    }
    else
    {
        g_Input->SetMouseMode(MouseMode::Absolute);
    }

    if (isMoveMode)
    {
        if (g_Input->GetState('W'))
        {
            pos_ += forward_ * freeflySpeed_ * g_Engine->GetDTime();
        }
        else if (g_Input->GetState('S'))
        {
            pos_ -= forward_ * freeflySpeed_ * g_Engine->GetDTime();
        }
    
        if (g_Input->GetState('D'))
        {
            pos_ += right_ * freeflySpeed_ * g_Engine->GetDTime();
        }
        else if (g_Input->GetState('A'))
        {
            pos_ -= right_ * freeflySpeed_ * g_Engine->GetDTime();
        }

        if (g_Input->GetState('Q'))
        {
            pos_ += Vec3::UP() * freeflySpeed_ * g_Engine->GetDTime();
        }
        else if (g_Input->GetState('E'))
        {
            pos_ -= Vec3::UP() * freeflySpeed_ * g_Engine->GetDTime();
        }
    }

    float extent = 20;
    //projection_ = MakeOrthographicProjection(-extent, extent, -extent / g_Render->GetAspect(), extent / g_Render->GetAspect(), 0.1f, 1000);
    projection_ = MakePerspectiveProjection(DegToRad(fovy_), g_Render->GetAspect(), near_, far_);

    toCamera_ = MakeLookAt(pos_, pos_ + forward_);
}

//------------------------------------------------------------------------------
void Camera::Update()
{
    UpdateMatrices();
}

//------------------------------------------------------------------------------
const Mat44& Camera::ToCamera() const
{
    return toCamera_;
}

//------------------------------------------------------------------------------
const Mat44& Camera::ToProjection() const
{
    return projection_;
}

//------------------------------------------------------------------------------
const Vec3& Camera::Position() const
{
    return pos_;
}

//------------------------------------------------------------------------------
void Camera::SetPosition(const Vec3& pos)
{
    pos_ = pos;
}

//------------------------------------------------------------------------------
void Camera::SetPosition(const Vec2& pos)
{
    pos_ = Vec3{ pos.x, pos.y, pos_.z };
}

//------------------------------------------------------------------------------
Box2D Camera::GetOrthoFrustum() const
{
    const Vec2 extents = Vec2(horizontalExtent_, horizontalExtent_ / g_Render->GetAspect());
    const Box2D frustum = MakeBox2DMinMax(
        Vec2(pos_.x - extents.x, pos_.y - extents.y),
        Vec2(pos_.x + extents.x, pos_.y + extents.y)
    );
    return frustum;
}

//------------------------------------------------------------------------------
float Camera::GetHorizontalExtent() const
{
    return horizontalExtent_;
}

//------------------------------------------------------------------------------
void Camera::SetHorizontalExtent(float extent)
{
    horizontalExtent_ = extent;
}

}
