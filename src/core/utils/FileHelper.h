#pragma once
#include <filesystem>
#include <string>
#include "core/utils/Debug.h"

class FileHelper
{
  public:

    inline static void DeleteFile(const std::string& filePath)
    {
        try
        {
            if (!std::filesystem::exists(filePath))
            {
                Log_Warning("File does not exist: " + filePath);
            }
            if (!std::filesystem::remove(filePath))
            {
                Log_Warning("File could not be deleted: " + filePath);
            }
        }
        catch (std::filesystem::filesystem_error& e)
        {
            Log_Error("Filesystem error: " + std::string(e.what()));
        }
    }

    inline static void RenameFile(const std::string& oldName, const std::string& oldPath, const std::string& newName, const std::string& newPath)
    {
        try
        {
            if (!std::filesystem::exists(oldPath))
            {
                Log_Warning("File path does not exist: " + oldPath);
                return;
            }

            if (std::filesystem::exists(newPath))
            {
                Log_Warning("File path does not exist: " + newPath);
                return;
            }

            std::filesystem::rename(oldPath, newPath);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            Log_Error("Filesystem error: " + std::string(e.what()));
        }
    }
};
