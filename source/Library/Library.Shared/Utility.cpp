#include "pch.h"
#include "Utility.h"
#if(WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#include "Shlwapi.h"
#endif
#include <fstream>

namespace Library
{
	std::string Utility::CurrentDirectory()
	{
		WCHAR buffer[MAX_PATH];
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		GetCurrentDirectory(MAX_PATH, buffer);
#endif // (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)

		std::wstring currentDirectoryW(buffer);

		return std::string(currentDirectoryW.begin(), currentDirectoryW.end());
	}

	std::wstring Utility::ExecutableDirectory()
	{
		WCHAR buffer[MAX_PATH];
#if (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
		GetModuleFileName(nullptr, buffer, MAX_PATH);
		PathRemoveFileSpec(buffer);
#endif // (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)


		return std::wstring(buffer);
	}

	void Utility::GetFileName(const std::string& inputPath, std::string& filename)
	{
		std::string fullPath(inputPath);
		std::replace(fullPath.begin(),fullPath.end(),'\\','/');

		std::string::size_type lastSlashIndex = fullPath.find_last_of('/');

		if (lastSlashIndex == std::string::npos)
		{
			filename = fullPath;
		}
		else
		{
			filename = fullPath.substr(lastSlashIndex + 1, fullPath.size() - lastSlashIndex- 1);
		}
	}

	void Utility::GetDirectory(const std::string& inputPath, std::string& directory)
	{
		std::string fullPath(inputPath);
		std::replace(fullPath.begin(),fullPath.end(),'\\','/');

		std::string::size_type lastSlashIndex = fullPath.find_last_of('/');

		if (lastSlashIndex == std::string::npos)
		{
			directory = "";
		}
		else
		{		
			directory = fullPath.substr(0, lastSlashIndex);
		}
	}

	void Utility::GetFileNameAndDirectory(const std::string& inputPath, std::string& directory, std::string& filename)
	{
		std::string fullPath(inputPath);
		std::replace(fullPath.begin(),fullPath.end(),'\\','/');

		std::string::size_type lastSlashIndex = fullPath.find_last_of('/');

		if (lastSlashIndex == std::string::npos)
		{
			directory = "";
			filename = fullPath;
		}
		else
		{
			directory = fullPath.substr(0, lastSlashIndex);
			filename = fullPath.substr(lastSlashIndex + 1, fullPath.size() - lastSlashIndex- 1);
		}
	}
	
	void Utility::LoadBinaryFile(const std::wstring& filename, std::vector<char>& data)
	{
		std::ifstream file(filename.c_str(), std::ios::binary);
		if (file.bad())
		{
			throw std::exception("Could not open file.");
		}

		file.seekg(0, std::ios::end);
		UINT size = (UINT)file.tellg();

		if (size > 0)
		{
			data.resize(size);
			file.seekg(0, std::ios::beg);			
			file.read(&data.front(), size);
		}

		file.close();
	}

	void Utility::ToWideString(const std::string& source, std::wstring& dest)
	{
		dest.assign(source.begin(), source.end());
	}

	std::wstring Utility::ToWideString(const std::string& source)
	{
		std::wstring dest;
		dest.assign(source.begin(), source.end());

		return dest;
	}

	std::string Utility::LoadBinaryFile(const std::string &filename)
	{
		std::ifstream file(filename, std::ios::binary);
		if (file.bad())
		{
			throw std::exception("Could not open file");
		}

		std::string data = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

		file.close();
		return data;
	}
}