#pragma once

namespace fbxAgent
{
    class Vector2
    {
    public:
        float x, y;
        Vector2();
        Vector2(float _x, float _y);

        bool operator==(const Vector2 &v) const;
    };
}