#pragma once

namespace MOON{
class SubjectHelper;
class ExecuteCommand;

class  GizmoObject 
{
public:
  GizmoObject* New();
  unsigned long AddObserver(unsigned long event, ExecuteCommand*, float priority = 0.0f);
  unsigned long AddObserver(const char* event, ExecuteCommand*, float priority = 0.0f);
  ExecuteCommand* GetExecuteCommand(unsigned long tag);

  void RemoveObserver(ExecuteCommand*);
  void RemoveObservers(unsigned long event, ExecuteCommand*);
  void RemoveObservers(const char* event, ExecuteCommand*);
  bool HasObserver(unsigned long event, ExecuteCommand*);
  bool HasObserver(const char* event, ExecuteCommand*);

  void RemoveObserver(unsigned long tag);
  void RemoveObservers(unsigned long event);
  void RemoveObservers(const char* event);
  void RemoveAllObservers(); // remove every last one of them
  bool HasObserver(unsigned long event);
  bool HasObserver(const char* event);

  bool InvokeEvent(unsigned long event, void* callData);
  bool InvokeEvent(const char* event, void* callData);
  ///@}

  bool InvokeEvent(unsigned long event) { return this->InvokeEvent(event, nullptr); }
  bool InvokeEvent(const char* event) { return this->InvokeEvent(event, nullptr); }
protected:
  GizmoObject();
  ~GizmoObject() ;
  void InternalGrabFocus(ExecuteCommand* mouseEvents, ExecuteCommand* keypressEvents = nullptr);
  void InternalReleaseFocus();

private:
    void ObjectFinalize();
protected:
    SubjectHelper* subjectHelper;
};

}


