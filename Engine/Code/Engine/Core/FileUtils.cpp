#include "Engine/Core/FileUtils.hpp"
#include <io.h>

Strings GetFileNamesInFolder( const FilePath& folderPath, const char* filePattern )
{
	Strings fileNamesInFolder;

	std::string fileNamepattern = filePattern ? filePattern : "*";
	FilePath filePath = folderPath + "/" + fileNamepattern;
	_finddata_t fileInfo;
	intptr_t searchHandle = _findfirst( filePath.c_str(), &fileInfo );
	while( searchHandle != -1 )
	{
		fileNamesInFolder.push_back( fileInfo.name );
		int errorCode = _findnext( searchHandle, &fileInfo );
		if( errorCode != 0 )
			break;
	}

	return fileNamesInFolder;
}
