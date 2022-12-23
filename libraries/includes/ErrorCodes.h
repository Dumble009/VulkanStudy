﻿#pragma once

namespace fbxAgent
{
    enum class FbxAgentErrorCode
    {
        FBX_AGENT_SUCCESS,
        FBX_AGENT_ERROR_FAILED_TO_CREATE_FBX_MANAGER,
        FBX_AGENT_ERROR_FAILED_TO_CREATE_FBX_IMPORETR,
        FBX_AGENT_ERROR_FAILED_TO_CREATE_FBX_SCENE,
        FBX_AGENT_ERROR_FAILED_TO_LOAD_FILE,
        FBX_AGENT_ERROR_FAILED_TO_IMPORT,
        FBX_AGENT_ERROR_AGENT_IS_NOT_INITIALIZED,
        FBX_AGENT_ERROR_FAILED_TO_LOAD_MESH_DATA,
        FBX_AGENT_ERROR_MODEL_INDEX_OUT_OF_RANGE
    };
}