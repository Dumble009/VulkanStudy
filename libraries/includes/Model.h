#pragma once
#include <vector>
#include "Vector2.h"
#include "Vector3.h"

namespace fbxAgent
{
    class Model
    {
    private:
        int vertexPositonCount; // 頂点座標データの数
        std::vector<Vector3> vertexPositions;

        int vertexIndexCount; // 頂点インデックスデータの数
        std::vector<int> vertexIndices;

        std::vector<std::vector<Vector2>> vertexUVs; // [vertexIndexCount][vertexUVLayerCount]の配列

    public:
        Model();
        Model(std::vector<Vector3> _vertexPositions,
              std::vector<int> _vertexIndices,
              std::vector<std::vector<Vector2>> _vertexUVs);

        int GetVertexPositionCount();
        std::vector<Vector3> *GetVertexPositions();

        int GetVertexIndexCount();
        std::vector<int> *GetVertexIndices();

        std::vector<std::vector<Vector2>> *GetVertexUVs();
    };
}