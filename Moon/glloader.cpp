#include "glloader.h"
#include <glad/glad.h>

bool GlLoader::CustomLoadGL(void* load)
{
	static bool initFlag = false;
	if (!initFlag) {
		initFlag = true;
		if (!gladLoadGLLoader((GLADloadproc)load)) {
			return false;
		}
	}
	return true;
}
