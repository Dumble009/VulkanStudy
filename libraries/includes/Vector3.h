#pragma once

namespace fbxAgent
{
    // FbxAgent内で使用する3次元のベクトルのデータを持つクラス
    class Vector3
    {
    public:
        float x, y, z;

        Vector3();
        Vector3(float _x, float _y, float _z);

        bool operator==(const Vector3 &v) const;
    };
}