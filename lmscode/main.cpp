#include <iostream>
#include <vector>

#include "Parser.h"
#include "Compiler.h"
#include "Utils.h"

LMS::Parser *LMS::CurrentParserInstance;

void ParseN()
{
	LMS::ParserSourceFileInfo info;
	info.Text = "#LMS #use mrk #nms xz class MRK : CLAZZ { fld int mrk fld Player Lercio ";
	info.Text += "func int mrktest { MRK.CLIENT.SENDMSG(mymsg) if x { } var string msg var int szVec } } ";
	info.Text += "#nms xy class MRK { func MRK constructor(MRK inst MRK newInst MRK inst) { var MRK myinstance }  func MRK constructor(MRK inst MRK newInst) { var MRK n } } ";
	LMS::Parser(_STD vector<LMS::ParserSourceFileInfo> {
		info
	}).Start();
}

LMS::CompilerResult CompileN()
{
	LMS::ParserSourceFileInfo info;
	info.Text = "#LMS #use mrk #nms xz class int { native func int Call() {} } class string {} class MRK : CLAZZ { fld int value func int constr { MRK.value.Call(0 21 LMS) } } ";
	return LMS::Compiler(_STD vector<LMS::ParserSourceFileInfo> {
		info
	}, LMS::CompilerOptions {
		LMS::CompilerBuildTarget::LMS, 
		"C:\\Users\\Mohamed\\Desktop\\lms_test.lms" }
	).Start();
}

int main(int argc, char **argv)
{
	printf("LMSCode v%d\n", LMSCODE_VERSION);
	// -c "x" -c "y" -n "myproj"
	if (argc == 0)
		return 0;
	_STD string name = "unknown";
	_STD string outpath;
	_STD vector<_STD string> paths;
	for (int i = 0; i < argc; i++)
	{
		char *current = argv[i];
		if (!strcmp(current, "-c"))
		{
			i++;
			paths.push_back(argv[i]);
		}
		else if (!strcmp(current, "-n"))
		{
			i++;
			name = argv[i];
		}
		else if (!strcmp(current, "-o"))
		{
			i++;
			outpath = argv[i];
		}
	}
	if (paths.size() == 0)
		return 0;
	_STD vector<LMS::ParserSourceFileInfo> vec;
	for (_STD string path : paths)
	{
		bool error = false;
		_STD string txt = LMS::Utils::ReadFile(path, &error);
		if (error)
		{
			_STD cout << "Cannot read file " << path << "\nCompilation Failed\n";
			return 0;
		}
		size_t ns = path.find_last_of('\\') + 1;
		size_t es = path.find_last_of('.');
		vec.push_back(LMS::ParserSourceFileInfo{
			txt,
			path,
			path.substr(ns, es - ns),
			path.substr(es + 1)
		});
	}
	LMS::CompilerResult res = LMS::Compiler(vec, LMS::CompilerOptions{
		LMS::CompilerBuildTarget::LMS,
		(outpath + "\\" + name + ".lms").c_str(),
		name == "unknown" ? 0 : name.c_str()
	}, &LMS::CurrentParserInstance).Start();
	if (res.Errors.size() > 0)
	{
		for (LMS::ParserError error : res.Errors)
		{
			_STD cout << "Error: " << error.Message << "\n";
		}
		_STD cout << "Compilation Failed!\n";
	}
	else _STD cout << "Compilation Succeeded!\n";
	system("pause");
	return 0;
}