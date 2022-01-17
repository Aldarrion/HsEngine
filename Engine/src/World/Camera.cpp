#include "World/Camera.h"

#include "Input/Input.h"
#include "Render/Render.h"
#include "Common/Logging.h"
#include "Resources/Serialization.h"
#include "Engine.h"

namespace hs
{

//------------------------------------------------------------------------------
static void CameraUpdateCameraVectors(Camera* camera)
{
    static constexpr float PITCH_LIMIT{ DegToRad(89.0f) };

    camera->angles_.x  = Clamp(camera->angles_.x, -PITCH_LIMIT, PITCH_LIMIT);

    camera->forward_.x = cos(camera->angles_.y) * cos(camera->angles_.x);
    camera->forward_.y = sin(camera->angles_.x);
    camera->forward_.z = sin(camera->angles_.y) * cos(camera->angles_.x);

    camera->right_ = Vec3::UP().Cross(camera->forward_).Normalized();
}

//------------------------------------------------------------------------------
static void CameraUpdateMatrices(Camera* camera)
{
    if (camera->projectionType_ == ProjectionType::Orthographic)
    {
        camera->toProjection_ = MakeOrthographicProjection(
            -camera->horizontalExtent_,
            camera->horizontalExtent_,
            -camera->horizontalExtent_ / g_Render->GetAspect(),
            camera->horizontalExtent_ / g_Render->GetAspect(),
            camera->near_,
            camera->far_
        );
    }
    else
    {
        camera->toProjection_ = MakePerspectiveProjection(DegToRad(camera->fovy_), g_Render->GetAspect(), camera->near_, camera->far_);
    }

    camera->toCamera_ = MakeLookAt(camera->pos_, camera->pos_ + camera->forward_);
}

//------------------------------------------------------------------------------
void CameraInitAsPerspective(Camera* camera, const Vec3& pos, const Vec3& target, float fovY, float near, float far)
{
   camera->projectionType_ = ProjectionType::Perspective;

    camera->pos_ = pos;
    camera->forward_ = (target - pos).Normalized();
    camera->forward_.y = Clamp(camera->forward_.y, -0.99f, 0.99f);

    camera->right_ = Vec3::UP().Cross(camera->forward_).Normalized();
    camera->fovy_ = fovY;
    camera->near_ = near;
    camera->far_ = far;

    CameraUpdateMatrices(camera);

    camera->angles_.x = asinf(camera->forward_.y);
    float cosX = cosf(camera->angles_.x);

    float y1 = acosf(camera->forward_.x / cosX);
    float y2 = 2 * HS_PI - y1;

    float d1 = abs(sin(y1) - (camera->forward_.z / cosX));
    float d2 = abs(sin(y2) - (camera->forward_.z / cosX));

    if (d1 < d2)
        camera->angles_.y = y1;
    else
        camera->angles_.y = y2;
}

//------------------------------------------------------------------------------
void CameraInit(Camera* camera, const PropertyContainer& data)
{
    camera->pos_ = data.GetValue(CameraDef::POSITION).V3;
    camera->angles_ = data.GetValue(CameraDef::ANGLES).V2;
    CameraUpdateCameraVectors(camera);
}

//------------------------------------------------------------------------------
void CameraFillData(const Camera* camera, PropertyContainer& data)
{
    data.Insert({ CameraDef::POSITION, PropertyValue(PropertyType::Vec3, camera->pos_) });
    data.Insert({ CameraDef::ANGLES, PropertyValue(PropertyType::Vec2, camera->angles_) });
}

//------------------------------------------------------------------------------
void CameraUpdateFreeFly(CameraFreelyController* freeflyCamera)
{
    Camera* camera = freeflyCamera->camera_;
    bool isMoveMode = g_Input->GetState(BTN_RIGHT);
    if (isMoveMode)
    {
        g_Input->SetMouseMode(MouseMode::Relative);
        Vec2 mouseDelta = g_Input->GetMouseDelta();
        camera->angles_.x -= mouseDelta.y * 0.0025f;
        camera->angles_.y -= mouseDelta.x * 0.0025f;

        if (mouseDelta != Vec2{})
        {
            CameraUpdateCameraVectors(camera);
        }
    }
    else
    {
        g_Input->SetMouseMode(MouseMode::Absolute);
    }

    if (isMoveMode)
    {
        float freeflySpeed = freeflyCamera->freeflySpeed_;
        if (g_Input->GetState(KC_W))
        {
            camera->pos_ += camera->forward_ * freeflySpeed * g_Engine->GetDTime();
        }
        else if (g_Input->GetState(KC_S))
        {
            camera->pos_ -= camera->forward_ * freeflySpeed * g_Engine->GetDTime();
        }

        if (g_Input->GetState(KC_D))
        {
            camera->pos_ += camera->right_ * freeflySpeed * g_Engine->GetDTime();
        }
        else if (g_Input->GetState(KC_A))
        {
            camera->pos_ -= camera->right_ * freeflySpeed * g_Engine->GetDTime();
        }

        if (g_Input->GetState(KC_E))
        {
            camera->pos_ += Vec3::UP() * freeflySpeed * g_Engine->GetDTime();
        }
        else if (g_Input->GetState(KC_Q))
        {
            camera->pos_ -= Vec3::UP() * freeflySpeed * g_Engine->GetDTime();
        }
    }

    //float extent = 20;
    //projection_ = MakeOrthographicProjection(-extent, extent, -extent / g_Render->GetAspect(), extent / g_Render->GetAspect(), 0.1f, 1000);
    camera->toProjection_ = MakePerspectiveProjection(DegToRad(camera->fovy_), g_Render->GetAspect(), camera->near_, camera->far_);

    camera->toCamera_ = MakeLookAt(camera->pos_, camera->pos_ + camera->forward_);
}

//------------------------------------------------------------------------------
void CameraUpdate(Camera* camera)
{
    CameraUpdateMatrices(camera);
}

//------------------------------------------------------------------------------
Box2D CameraGetOrthoFrustum(const Camera* camera)
{
    const Vec2 extents = Vec2(camera->horizontalExtent_, camera->horizontalExtent_ / g_Render->GetAspect());
    const Box2D frustum = MakeBox2DMinMax(
        Vec2(camera->pos_.x - extents.x, camera->pos_.y - extents.y),
        Vec2(camera->pos_.x + extents.x, camera->pos_.y + extents.y)
    );
    return frustum;
}

}
