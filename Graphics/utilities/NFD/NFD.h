#pragma once
/*
  MOFIED VERSION OF NATIVE FILE DIALOGUE: https://github.com/mlabbe/nativefiledialog/tree/master/src

  Native File Dialog

  http://www.frogtoss.com/labs
 */

#include <shobjidl.h>
#include <string>
static class NativeFileDialogue {
public:
	static bool NFD_OpenDialog(std::string& outPath);
};

