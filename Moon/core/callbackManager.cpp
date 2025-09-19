#include "callbackManager.h"
namespace MOON {
	CallBackManager& CallBackManager::instance()
	{
		static CallBackManager callbackManager;
		return callbackManager;
	}
	void CallBackManager::exectue()
	{
		for (int i = 0;i < mCallBacks.size();i++) {
			mCallBacks[i].function(mCallBacks[i].storage,&mCallBacks[i]);
		}
		mCallBacks.clear();
	}
	CallBackManager::CallBack& CallBackManager::create(CallBackFunc func)
	{
		mCallBacks.emplace_back();
		mCallBacks.back().function = func;
		return mCallBacks.back();
	}
	CallBackManager::CallBackManager()
	{
	}

}