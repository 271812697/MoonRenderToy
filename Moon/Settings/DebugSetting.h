#pragma once
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <typeinfo>
class QWidget;
namespace MOON {
class NodeBase {
public:
	NodeBase() = default;
	NodeBase(void* p, size_t t, const char* n) :
		mData(p),
		mType(t),
		mName(n)
	{

	}
	virtual ~NodeBase() {
	}
	virtual bool valueChange();
	virtual QWidget* createWidget(QWidget*parent);
	std::string& getName() {
		return mName;
	}
	std::string getType();
	template<typename T>
	T* getPtr() {
		if (typeid(T).hash_code() == mType && mData) {
			return reinterpret_cast<T*>(mData);
		}
		return nullptr;
	}
	template<typename T>
	T* getPtrFast() {
		return reinterpret_cast<T*>(mData);
	}
	template<typename T>
	T& getData() {
		if (typeid(T).hash_code() == mType && mData) {
			return *(reinterpret_cast<T*>(mData));
		}
		
		return *(reinterpret_cast<T*>(mData));
	}
	template<typename T>
	void setData(T* ptr) {
		if (ptr) {
			if (typeid(T).hash_code() == mType) {
				if (mData) {
					*(reinterpret_cast<T*>(mData))=*ptr;
				}
			}
		}
	}
	template<typename T>
	void setData(const T&data) {
		if (typeid(T).hash_code() == mType) {
			if (mData) {
				T old=*(reinterpret_cast<T*>(mData));
				if (old != data) {
                   *(reinterpret_cast<T*>(mData)) = data;
				   submitCallBack();
				}
			}
		}
	}

	void addCallBack(const std::string& name,std::function<void(NodeBase* self)> fn) {
		mCallList[name] = fn;
	}
	void submitCallBack();
protected:
	std::unordered_map<std::string, std::function<void(NodeBase* self)>>mCallList;
	
	void* mData = nullptr;
	size_t mType=0;
	std::string mName="";
};
template<typename T>
class NodeData : public NodeBase {
public:
	NodeData(const T& value,const std::string&n) :data(value){
		mData = &data;
		mType = typeid(T).hash_code();
		mName = n;
	}
	virtual ~NodeData() override{
	}

	T& get() {
		return data;
	}
	T* getPtr() {
		return &data;
	}
protected:
	T data;
};
class DebugSettings {
public:
	static DebugSettings& instance();
	template<typename T>
	T getOrDefault(const std::string& key,T value=T()) {
		if (mIndexMap.find(key) != mIndexMap.end()) {
			return mRegistry[mIndexMap[key]]->getData<T>();
		}
		return value;
	}
	bool valueChange(const std::string& key) {
		if (mIndexMap.find(key) != mIndexMap.end()) {
			return mRegistry[mIndexMap[key]]->valueChange();
		}
		return false;
	}
	template<typename T>
	T* getPtr(const std::string& key) {
		if (mIndexMap.find(key) != mIndexMap.end()) {
			return mRegistry[mIndexMap[key]]->getPtr<T>();
		}
		return nullptr;
	}
	template<typename T>
	bool tryGet(const std::string& key,T& outValue) {
		if (mIndexMap.find(key) != mIndexMap.end()) {
			outValue= mRegistry[mIndexMap[key]]->getData<T>();
		}
		return false;
	}
	template <typename T>
	T& getOrCreate(const std::string&group,const std::string& key) {
		if (mIndexMap.find(key) != mIndexMap.end()) {
			return mRegistry[mIndexMap[key]]->getData<T>();
		}
		T value;
		NodeData<T>* ptr = new NodeData<T>(value,key);
		mIndexMap[key] = mNextIndex;
		mGroup[group].push_back(mNextIndex++);
		mRegistry.push_back(ptr);
		return value;
	}
	template <typename T>
	bool setData(const std::string& key, const T& value) {
		if (mIndexMap.find(key) != mIndexMap.end()) {
			mRegistry[mIndexMap[key]]->setData<T>(value);
			return true;
		}
		return false;
	}
	template <typename T>
	bool setData(const std::string& key,  T*ptr) {
		return setData(key,*ptr);
	}
	template <typename T>
	bool add(const std::string& group,const std::string& key,const T& value) {
		if (mIndexMap.find(key) != mIndexMap.end()) {
			return false;
		}
		NodeData<T>* ptr = new NodeData<T>(value, key);
		mIndexMap[key] = mNextIndex;
		mRegistry.push_back(ptr);
		mGroup[group].push_back(mNextIndex++);
		return true;
	}
	bool add(const std::string& group,NodeBase* ptr);
	bool registerFloatReference(const std::string& group,const std::string& key,float& value,float minValue,float maxValue);
	bool registerDoubleReference(const std::string& group, const std::string& key, double& value, double minValue, double maxValue);
	bool registerColorEdit(const std::string& group, const std::string& key,float color[3]);
	bool registerBoolReference(const std::string& group, const std::string& key, bool& value);
	bool addCallBack(const std::string& key, const std::string& name,std::function<void(NodeBase* self)>fn);
	NodeBase* getNode(const std::string& key);
	~DebugSettings();
	std::unordered_map<std::string, std::vector<int>>& getGroup();
	std::vector<NodeBase*>& getRegistry();
private:
	DebugSettings();
private:
	int mNextIndex = 0;
	std::vector<NodeBase*>mRegistry;
	std::unordered_map<std::string, int>mIndexMap;
	std::unordered_map<std::string, std::vector<int>>mGroup;
};
}