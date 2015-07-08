#include "pch.h"
#include <codecvt>
using namespace ModelPipeline;

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc == 3)
	{
		std::wstring objPathW = std::wstring(argv[1]);
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converterX;
		std::string objPath = std::string(converterX.to_bytes(objPathW));
		std::unique_ptr<Model> model(new Model(objPath, true));
		
		std::wstring outFileW = std::wstring(argv[2]);
		std::string outFile = std::string(converterX.to_bytes(outFileW));
		model->SaveModel(outFile.c_str());
	}
	else
	{
		throw std::exception("Incorrect number of arguments");
		return 1;
	}

	return 0;
}

