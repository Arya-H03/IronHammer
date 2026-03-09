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
                LOG_WARNING("File does not exist: " + filePath);
            }
            if (!std::filesystem::remove(filePath))
            {
                LOG_WARNING("File could not be deleted: " + filePath);
            }
        }
        catch (std::filesystem::filesystem_error& e)
        {
            LOG_ERROR("Filesystem error: " + std::string(e.what()));
        }
    }

    inline static void RenameFile(const std::string& oldName, const std::string& oldPath, const std::string& newName, const std::string& newPath)
    {
        try
        {
            if (!std::filesystem::exists(oldPath))
            {
                LOG_WARNING("File path does not exist: " + oldPath);
                return;
            }

            if (std::filesystem::exists(newPath))
            {
                LOG_WARNING("File path does not exist: " + newPath);
                return;
            }

            std::filesystem::rename(oldPath, newPath);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            LOG_ERROR("Filesystem error: " + std::string(e.what()));
        }
    }
};
