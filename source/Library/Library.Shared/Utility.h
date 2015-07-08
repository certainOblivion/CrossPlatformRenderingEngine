#pragma once
#include <string>
#include <vector>
namespace Library
{
	class Utility
	{
	public:
		static std::string CurrentDirectory();
		static std::wstring ExecutableDirectory();
		static void GetFileName(const std::string& inputPath, std::string& filename);
		static void GetDirectory(const std::string& inputPath, std::string& directory);
		static void GetFileNameAndDirectory(const std::string& inputPath, std::string& directory, std::string& filename);
		static void LoadBinaryFile(const std::wstring& filename, std::vector<char>& data);
		static void ToWideString(const std::string& source, std::wstring& dest);
		static std::wstring ToWideString(const std::string& source);
		static std::string LoadBinaryFile(const std::string &filename);
		Utility() = delete;
		Utility(const Utility& rhs) = delete;
		Utility& operator=(const Utility& rhs) = delete;		
	};
}