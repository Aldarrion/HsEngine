#pragma once

#include "Math/hs_Math.h"

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
class Camera
{
public:
    void InitAsPerspective(const Vec3& pos, const Vec3& target, float fovY = 75, float near = 0.01f, float far = 1000.0f);

    const Mat44& ToCamera() const;
    const Mat44& ToProjection() const;
    const Vec3& Position() const;

    void SetPosition(const Vec3& pos);
    void SetPosition(const Vec2& pos);

    void Init(const PropertyContainer& data);
    void FillData(PropertyContainer& data);

    void UpdateFreeFly();
    void Update();
    void UpdateMatrices();

    Box2D GetOrthoFrustum() const;

    float GetHorizontalExtent() const;
    void SetHorizontalExtent(float extent);

private:
    Mat44 toCamera_;
    Mat44 projection_;

    Vec3 pos_{ 0, 0, -5 };
    Vec3 forward_{ Vec3::FORWARD() };
    Vec3 right_{ Vec3::RIGHT() };
    float freeflySpeed_{ 20 };

    Vec2 angles_{ 0, HS_PI_HALF };
    float fovy_{ 75 };
    float near_{ 0.01f };
    float far_{ 1000 };
    float horizontalExtent_{ 128 };

    ProjectionType projectionType_{ ProjectionType::Orthographic };

    void UpdateCameraVectors();
};

}
