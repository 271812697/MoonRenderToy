#include "DebugSetting.h"
#include "dataDispatcher.h"
#include "Qtimgui/imgui/imgui.h"
#include "core/callbackManager.h"
namespace MOON {
	class DragFloat :public NodeData<float> {
	public:
		DragFloat(float v,float a,float b,const char* n) :NodeData(v,n),mMinValue(a),mMaxValue(b){
		}
		virtual void draw() override {
			ImGui::SliderFloat(mName.c_str(),&data,mMinValue,mMaxValue);
		}
	private:
		float mMinValue;
		float mMaxValue;
	};
	class InputString :public NodeData<std::string> {
	public:
		InputString(const std::string& s, const char* n) :NodeData(s, n) {
			mValueChange = false;
		}
		virtual void draw()override {
			char str[200];
			strcpy(str,data.c_str());
			mValueChange = ImGui::InputText(mName.c_str(),str,200,ImGuiInputTextFlags_EnterReturnsTrue);
			data = str;
			if (ImGui::BeginDragDropTarget()) {
				auto filePath = ImGui::AcceptDragDropPayload("FilePath",ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
				if (filePath) {
					data = *(std::string*)filePath->Data;
					mValueChange = true;

				}
				ImGui::EndDragDropTarget();
			}
			if (mValueChange) {
				submitCallBack();
			}
		}
		virtual bool valueChange() {
			return mValueChange;
		}
	   
	private:
		bool mValueChange;
	};
	template <typename T>
	class DataSetting :public NodeData<T> {
	public:
		DataSetting(T&value,const char* n):NodeData<T>(value,n)
		{
			dispatcher.RegisterReference(value);
		}
		DataSetting(std::function<void(T)>provider,std::function<T(void)>gather,const char* n) :
		NodeData(T(),n)
		{
			dispatcher.RegisterGather(gather);
			dispatcher.RegisterProvider(provider);
		}
	protected:
		DataDispatcher<T>dispatcher;
	};
	class BoolSetting :public DataSetting<bool>
	{
	public:
		BoolSetting(bool& value, const char* name) :DataSetting(value, name) {

		}
		virtual void draw() override{
			data = dispatcher.Gather();
			if(ImGui::Checkbox(mName.c_str(),&data)){
				dispatcher.NotifyChange();
			}
			dispatcher.Provide(data);
		}
	};
	template<typename T>
	class RangeSetting :public DataSetting<T> {
	public:
		RangeSetting(T& value, T minV, T maxV, const char* n) :
			DataSetting<T>(value,n),minValue(minV),maxValue(maxV)
			
		{

		}
	protected:
		T minValue;
		T maxValue;
	};
	class RangeFLoat :public RangeSetting<float> {
	public:
		RangeFLoat(float& v, float minV, float maxV, const char* n):RangeSetting(v,minV,maxV,n) {

		}
		virtual void draw() override{
			data = dispatcher.Gather();
			if (ImGui::SliderFloat(mName.c_str(), &data,minValue,maxValue)) {
				dispatcher.NotifyChange();
			}
			dispatcher.Provide(data);
		}
	};
	class RangeDouble :public RangeSetting<double> {
	public:
		RangeDouble(double& v, double minV, double maxV, const char* n) :RangeSetting(v, minV, maxV, n) {

		}
		virtual void draw() override {
			data = dispatcher.Gather();
			if (ImGui::SliderScalar(mName.c_str(),ImGuiDataType_::ImGuiDataType_Double ,&data, &minValue, &maxValue)) {
				dispatcher.NotifyChange();
			}
			dispatcher.Provide(data);
		}
	};
	class DragInt :public NodeData<int> {
	public:
		DragInt(int v, int a, int b, const char* n):NodeData(v,n),mMinValue(a),mMaxValue(b) {
			
		}
		virtual void draw() override{
			ImGui::SliderInt(mName.c_str(),&data,mMinValue,mMaxValue);
		}
	private:
		int mMinValue;
		int mMaxValue;
	};
	template <typename T,size_t size>
	class RangeSettingN :public NodeData<std::array<T, size>> {
	public:
		RangeSettingN(const std::array<T,size>& v,T a,T b,const char* n):
		NodeData(v,n),
			mMinValue(a), mMaxValue(b)
		{
			
		}
	protected:
		T mMinValue;
		T mMaxValue;
	};
	template <size_t size>
	class RangeSettingNFloat :public RangeSettingN<float, size> {
	public:
		RangeSettingNFloat(const std::array<float, size>& v, float a, float b, const char* n)
			:RangeSettingN<float, size>(v,a,b,n)
		{
			

		}
		virtual void draw() override{
		

			ImGui::SliderScalarN(RangeSettingN<float, size>::mName.c_str(),
				ImGuiDataType_Float, RangeSettingN<float, size>::data.data(),size,
				&RangeSettingN<float, size>::mMinValue,
				&RangeSettingN<float, size>::mMaxValue);
		}
	};
	class ColorSetting :public NodeData<float*> {
	public:
		ColorSetting(float col[3],const char* n) :NodeData(col,n){

		}
		virtual void draw()override {
			ImGui::ColorEdit3(mName.c_str(),data);
		}
	};
	DebugSettings& DebugSettings::instance()
	{
		static DebugSettings instance;
		return instance;
	}
	bool DebugSettings::add(const std::string& group, NodeBase* ptr)
	{
		if (mIndexMap.find(ptr->getName()) != mIndexMap.end()) {
			return false;
		}
		mIndexMap[ptr->getName()] = mNextIndex;
		mGroup[group].push_back(mNextIndex++);
		mRegistry.push_back(ptr);
		return true;
	}
	bool DebugSettings::registerFloatReference(const std::string& group, const std::string& key, float& value, float minValue, float maxValue)
	{
		if (mIndexMap.find(key) != mIndexMap.end()) {
			return false;
		}
		RangeFLoat* ptr = new RangeFLoat(value,minValue,maxValue,key.c_str());
		mIndexMap[ptr->getName()] = mNextIndex;
		mGroup[group].push_back(mNextIndex++);
		mRegistry.push_back(ptr);
		return true;
	}
	bool DebugSettings::registerDoubleReference(const std::string& group, const std::string& key, double& value, double minValue, double maxValue)
	{
		if (mIndexMap.find(key) != mIndexMap.end()) {
			return false;
		}
		RangeDouble* ptr = new RangeDouble(value, minValue, maxValue, key.c_str());
		mIndexMap[ptr->getName()] = mNextIndex;
		mGroup[group].push_back(mNextIndex++);
		mRegistry.push_back(ptr);
		return true;
	}
	bool DebugSettings::registerColorEdit(const std::string& group, const std::string& key, float color[3])
	{
		if (mIndexMap.find(key) != mIndexMap.end()) {
			return false;
		}
		ColorSetting* ptr = new ColorSetting(color,key.c_str());
		mIndexMap[ptr->getName()] = mNextIndex;
		mGroup[group].push_back(mNextIndex++);
		mRegistry.push_back(ptr);
		return true;
	}
	bool DebugSettings::registerBoolReference(const std::string& group, const std::string& key, bool& value)
	{
		if (mIndexMap.find(key) != mIndexMap.end()) {
			return false;
		}
		BoolSetting* ptr = new BoolSetting(value, key.c_str());
		mIndexMap[ptr->getName()] = mNextIndex;
		mGroup[group].push_back(mNextIndex++);
		mRegistry.push_back(ptr);
		return true;
	}
	bool DebugSettings::addCallBack(const std::string& key, std::function<void()> fn)
	{
		auto it = mIndexMap.find(key);
		if (it == mIndexMap.end()) {
            return false;
		}
		mRegistry[it->second]->addCallBack(fn);
		return true;
	}
	NodeBase* DebugSettings::getNode(const std::string& key)
	{
		auto it = mIndexMap.find(key);
		if (it == mIndexMap.end()) {
			return nullptr;
		}
		return mRegistry[it->second];
	}
	DebugSettings::~DebugSettings()
	{
		for (auto& item : mRegistry) {
			delete item;
		}
	}
	void DebugSettings::drawImgui()
	{
		if (ImGui::Begin("DebugOptions")) {
			for (auto& g : mGroup) {
				if (ImGui::CollapsingHeader(g.first.c_str())) {
					ImGui::Columns(2,g.first.c_str());
					for (int& i : g.second) {
						mRegistry[i]->draw();
						ImGui::NextColumn();
					}
					ImGui::Columns(1);
				}
			}
		}
		ImGui::End();
	}
	DebugSettings::DebugSettings()
	{
		mHashCode["bool"] = typeid(bool).hash_code();
		mHashCode["int"] = typeid(int).hash_code();
		mHashCode["float"] = typeid(float).hash_code();
		add("View","showLight",false);

		add("Batch",new InputString("path","path"));
	}
	bool NodeBase::valueChange()
	{
		return true;
	}
	void NodeBase::draw()
	{
		size_t type = getType();
		if (type == typeid(bool).hash_code()) {
			if (ImGui::Checkbox(getName().c_str(), getPtrFast<bool>())) {
				submitCallBack();
			}
		}
	}
	void NodeBase::submitCallBack()
	{
		for (int i = 0;i < mCallList.size();i++) {
			createCallBack(CallBackManager::instance(), mCallList[i]);
		}
	}
}