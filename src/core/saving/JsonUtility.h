#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
#include "core/utils/Debug.h"

using Json = nlohmann::json;

class JsonUtility
{
  public:

    inline static bool SaveJsonObjectToFile(Json& jsonObject, const std::string& filePath)
    {

        std::ofstream file(filePath);
        if (!file.is_open())
        {
            Log_Error("Failed to open file for saving scene: " + filePath);
            return false;
        }

        file << jsonObject.dump(2);
        file.close();
        return true;
    }

    inline static Json LoadJsonObjectFromFile(const std::string& filePath)
    {
        Json jsonObject;

        std::ifstream file(filePath);
        if (!file.is_open())
        {
            Log_Error("Failed to open scene file: " + filePath);
            return jsonObject;
        }

        file >> jsonObject;
        return jsonObject;
    }
};
