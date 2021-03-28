#pragma once
#include "Engine/Core/StringUtils.hpp"

typedef std::string FilePath;

Strings GetFileNamesInFolder( const FilePath& folderPath, const char* filePattern = nullptr );
 