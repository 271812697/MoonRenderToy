#include "GizmoObject.h"
#include "ExecuteCommand.h"

#include <algorithm>
#include <sstream>
#include <vector>

namespace MOON {
	class GizmoObserver
	{
	public:
		GizmoObserver()
			: Command(nullptr)
			, Event(0)
			, Tag(0)
			, Next(nullptr)
			, Priority(0.0)
		{
		}
		~GizmoObserver();


		ExecuteCommand* Command;
		unsigned long Event;
		unsigned long Tag;
		GizmoObserver* Next;
		float Priority;
	};

	class SubjectHelper
	{
	public:
		SubjectHelper()
			: Focus1(nullptr)
			, Focus2(nullptr)
			, Start(nullptr)
			, Count(1)
		{
		}
		~SubjectHelper();

		unsigned long AddObserver(unsigned long event, ExecuteCommand* cmd, float p);
		void RemoveObserver(unsigned long tag);
		void RemoveObservers(unsigned long event);
		void RemoveObservers(unsigned long event, ExecuteCommand* cmd);
		void RemoveAllObservers();
		bool InvokeEvent(unsigned long event, void* callData, GizmoObject* self);
		ExecuteCommand* GetCommand(unsigned long tag);
		unsigned long GetTag(ExecuteCommand*);
		bool HasObserver(unsigned long event);
		bool HasObserver(unsigned long event, ExecuteCommand* cmd);
		void GrabFocus(ExecuteCommand* c1, ExecuteCommand* c2)
		{
			this->Focus1 = c1;
			this->Focus2 = c2;
		}
		void ReleaseFocus()
		{
			this->Focus1 = nullptr;
			this->Focus2 = nullptr;
		}

		std::vector<bool> ListModified;

		ExecuteCommand* Focus1;
		ExecuteCommand* Focus2;

	protected:
		GizmoObserver* Start;
		unsigned long Count;
	};

	GizmoObject* GizmoObject::New()
	{
		GizmoObject* ret = new GizmoObject;

		return ret;
	}


	GizmoObject::GizmoObject()
	{

		this->subjectHelper = nullptr;

	}

	GizmoObject::~GizmoObject()
	{

		delete this->subjectHelper;
		this->subjectHelper = nullptr;
	}

	GizmoObserver::~GizmoObserver()
	{

	}

	SubjectHelper::~SubjectHelper()
	{
		GizmoObserver* elem = this->Start;
		GizmoObserver* next;
		while (elem)
		{
			next = elem->Next;
			delete elem;
			elem = next;
		}
		this->Start = nullptr;
		this->Focus1 = nullptr;
		this->Focus2 = nullptr;
	}

	unsigned long SubjectHelper::AddObserver(unsigned long event, ExecuteCommand* cmd, float p)
	{
		GizmoObserver* elem;

		// initialize the new observer element
		elem = new GizmoObserver;
		elem->Priority = p;
		elem->Next = nullptr;
		elem->Event = event;
		elem->Command = cmd;

		elem->Tag = this->Count;
		this->Count++;

		// now insert into the list
		// if no other elements in the list then this is Start
		if (!this->Start)
		{
			this->Start = elem;
		}
		else
		{
			// insert high priority first
			GizmoObserver* prev = nullptr;
			GizmoObserver* pos = this->Start;
			while (pos->Priority >= elem->Priority && pos->Next)
			{
				prev = pos;
				pos = pos->Next;
			}
			// pos is Start and elem should not be start
			if (pos->Priority > elem->Priority)
			{
				pos->Next = elem;
			}
			else
			{
				if (prev)
				{
					prev->Next = elem;
				}
				elem->Next = pos;
				// check to see if the new element is the start
				if (pos == this->Start)
				{
					this->Start = elem;
				}
			}
		}
		return elem->Tag;
	}

	void SubjectHelper::RemoveObserver(unsigned long tag)
	{
		GizmoObserver* elem;
		GizmoObserver* prev;
		GizmoObserver* next;

		elem = this->Start;
		prev = nullptr;
		while (elem)
		{
			if (elem->Tag == tag)
			{
				if (prev)
				{
					prev->Next = elem->Next;
					next = prev->Next;
				}
				else
				{
					this->Start = elem->Next;
					next = this->Start;
				}
				delete elem;
				elem = next;
			}
			else
			{
				prev = elem;
				elem = elem->Next;
			}
		}

		if (!this->ListModified.empty())
		{
			this->ListModified.assign(this->ListModified.size(), true);
		}
	}

	void SubjectHelper::RemoveObservers(unsigned long event)
	{
		GizmoObserver* elem;
		GizmoObserver* prev;
		GizmoObserver* next;

		elem = this->Start;
		prev = nullptr;
		while (elem)
		{
			if (elem->Event == event)
			{
				if (prev)
				{
					prev->Next = elem->Next;
					next = prev->Next;
				}
				else
				{
					this->Start = elem->Next;
					next = this->Start;
				}
				delete elem;
				elem = next;
			}
			else
			{
				prev = elem;
				elem = elem->Next;
			}
		}

		if (!this->ListModified.empty())
		{
			this->ListModified.assign(this->ListModified.size(), true);
		}
	}

	void SubjectHelper::RemoveObservers(unsigned long event, ExecuteCommand* cmd)
	{
		GizmoObserver* elem;
		GizmoObserver* prev;
		GizmoObserver* next;

		elem = this->Start;
		prev = nullptr;
		while (elem)
		{
			if (elem->Event == event && elem->Command == cmd)
			{
				if (prev)
				{
					prev->Next = elem->Next;
					next = prev->Next;
				}
				else
				{
					this->Start = elem->Next;
					next = this->Start;
				}
				delete elem;
				elem = next;
			}
			else
			{
				prev = elem;
				elem = elem->Next;
			}
		}

		if (!this->ListModified.empty())
		{
			this->ListModified.assign(this->ListModified.size(), true);
		}
	}

	void SubjectHelper::RemoveAllObservers()
	{
		GizmoObserver* elem = this->Start;
		GizmoObserver* next;
		while (elem)
		{
			next = elem->Next;
			delete elem;
			elem = next;
		}
		this->Start = nullptr;

		if (!this->ListModified.empty())
		{
			this->ListModified.assign(this->ListModified.size(), true);
		}
	}

	bool SubjectHelper::HasObserver(unsigned long event)
	{
		GizmoObserver* elem = this->Start;
		while (elem)
		{
			if (elem->Event == event || elem->Event == ExecuteCommand::AnyEvent)
			{
				return true;
			}
			elem = elem->Next;
		}
		return false;
	}

	bool SubjectHelper::HasObserver(unsigned long event, ExecuteCommand* cmd)
	{
		GizmoObserver* elem = this->Start;
		while (elem)
		{
			if ((elem->Event == event || elem->Event == ExecuteCommand::AnyEvent) && elem->Command == cmd)
			{
				return true;
			}
			elem = elem->Next;
		}
		return false;
	}

	bool SubjectHelper::InvokeEvent(unsigned long event, void* callData, GizmoObject* self)
	{
		bool focusHandled = false;


		this->ListModified.push_back(false);

		using  VisitedListType = std::vector<unsigned long>;
		VisitedListType visited;
		GizmoObserver* elem = this->Start;

		const unsigned long maxTag = this->Count;


		GizmoObserver* next;
		while (elem)
		{
			// store the next pointer because elem could disappear due to Command
			next = elem->Next;
			if (elem->Command->GetPassiveObserver() &&
				(elem->Event == event || elem->Event == ExecuteCommand::AnyEvent) && elem->Tag < maxTag)
			{
				VisitedListType::iterator vIter = std::lower_bound(visited.begin(), visited.end(), elem->Tag);
				if (vIter == visited.end() || *vIter != elem->Tag)
				{
					// Sorted insertion by tag to speed-up future searches at limited
					// insertion cost because it reuses the search iterator already at the
					// correct location
					visited.insert(vIter, elem->Tag);
					ExecuteCommand* command = elem->Command;

					elem->Command->Execute(self, event, callData);

				}
			}
			if (this->ListModified.back())
			{
				// Passive observer should not call AddObserver or RemoveObserver in callback.;
				elem = this->Start;
				this->ListModified.back() = false;
			}
			else
			{
				elem = next;
			}
		}

		// 1. Focus loop
		//
		if (this->Focus1 || this->Focus2)
		{
			elem = this->Start;
			while (elem)
			{
				// store the next pointer because elem could disappear due to Command
				next = elem->Next;
				if (((this->Focus1 == elem->Command) || (this->Focus2 == elem->Command)) &&
					(elem->Event == event || elem->Event == ExecuteCommand::AnyEvent) && elem->Tag < maxTag)
				{
					VisitedListType::iterator vIter =
						std::lower_bound(visited.begin(), visited.end(), elem->Tag);
					if (vIter == visited.end() || *vIter != elem->Tag)
					{
						// Don't execute the remainder loop
						focusHandled = true;
						// Sorted insertion by tag to speed-up future searches at limited
						// insertion cost because it reuses the search iterator already at the
						// correct location
						visited.insert(vIter, elem->Tag);
						ExecuteCommand* command = elem->Command;

						command->SetAbortFlag(false);
						elem->Command->Execute(self, event, callData);
						// if the command set the abort flag, then stop firing events
						// and return
						if (command->GetAbortFlag())
						{

							this->ListModified.pop_back();
							return true;
						}

					}
				}
				if (this->ListModified.back())
				{
					elem = this->Start;
					this->ListModified.back() = false;
				}
				else
				{
					elem = next;
				}
			}
		}

		// 2. Remainder loop
		//
		if (!focusHandled)
		{
			elem = this->Start;
			while (elem)
			{
				// store the next pointer because elem could disappear due to Command
				next = elem->Next;
				if ((elem->Event == event || elem->Event == ExecuteCommand::AnyEvent) && elem->Tag < maxTag)
				{
					VisitedListType::iterator vIter =
						std::lower_bound(visited.begin(), visited.end(), elem->Tag);
					if (vIter == visited.end() || *vIter != elem->Tag)
					{
						// Sorted insertion by tag to speed-up future searches at limited
						// insertion cost because it reuses the search iterator already at the
						// correct location
						visited.insert(vIter, elem->Tag);
						ExecuteCommand* command = elem->Command;

						command->SetAbortFlag(false);
						elem->Command->Execute(self, event, callData);
						// if the command set the abort flag, then stop firing events
						// and return
						if (command->GetAbortFlag())
						{

							this->ListModified.pop_back();
							return true;
						}

					}
				}
				if (this->ListModified.back())
				{
					elem = this->Start;
					this->ListModified.back() = false;
				}
				else
				{
					elem = next;
				}
			}
		}

		this->ListModified.pop_back();
		return false;
	}

	unsigned long SubjectHelper::GetTag(ExecuteCommand* cmd)
	{
		GizmoObserver* elem = this->Start;
		while (elem)
		{
			if (elem->Command == cmd)
			{
				return elem->Tag;
			}
			elem = elem->Next;
		}
		return 0;
	}

	ExecuteCommand* SubjectHelper::GetCommand(unsigned long tag)
	{
		GizmoObserver* elem = this->Start;
		while (elem)
		{
			if (elem->Tag == tag)
			{
				return elem->Command;
			}
			elem = elem->Next;
		}
		return nullptr;
	}

	unsigned long GizmoObject::AddObserver(unsigned long event, ExecuteCommand* cmd, float p)
	{
		if (!this->subjectHelper)
		{
			this->subjectHelper = new SubjectHelper;
		}
		return this->subjectHelper->AddObserver(event, cmd, p);
	}

	unsigned long GizmoObject::AddObserver(const char* event, ExecuteCommand* cmd, float p)
	{
		return this->AddObserver(ExecuteCommand::GetEventIdFromString(event), cmd, p);
	}

	ExecuteCommand* GizmoObject::GetExecuteCommand(unsigned long tag)
	{
		if (this->subjectHelper)
		{
			return this->subjectHelper->GetCommand(tag);
		}
		return nullptr;
	}

	void GizmoObject::RemoveObserver(unsigned long tag)
	{
		if (this->subjectHelper)
		{
			this->subjectHelper->RemoveObserver(tag);
		}
	}

	void GizmoObject::RemoveObserver(ExecuteCommand* c)
	{
		if (this->subjectHelper)
		{
			unsigned long tag = this->subjectHelper->GetTag(c);
			while (tag)
			{
				this->subjectHelper->RemoveObserver(tag);
				tag = this->subjectHelper->GetTag(c);
			}
		}
	}

	void GizmoObject::RemoveObservers(unsigned long event)
	{
		if (this->subjectHelper)
		{
			this->subjectHelper->RemoveObservers(event);
		}
	}

	void GizmoObject::RemoveObservers(const char* event)
	{
		this->RemoveObservers(ExecuteCommand::GetEventIdFromString(event));
	}

	void GizmoObject::RemoveObservers(unsigned long event, ExecuteCommand* cmd)
	{
		if (this->subjectHelper)
		{
			this->subjectHelper->RemoveObservers(event, cmd);
		}
	}

	void GizmoObject::RemoveObservers(const char* event, ExecuteCommand* cmd)
	{
		this->RemoveObservers(ExecuteCommand::GetEventIdFromString(event), cmd);
	}

	void GizmoObject::RemoveAllObservers()
	{
		if (this->subjectHelper)
		{
			this->subjectHelper->RemoveAllObservers();
		}
	}

	bool GizmoObject::InvokeEvent(unsigned long event, void* callData)
	{
		if (this->subjectHelper)
		{
			return this->subjectHelper->InvokeEvent(event, callData, this);
		}
		return false;
	}

	bool GizmoObject::InvokeEvent(const char* event, void* callData)
	{
		return this->InvokeEvent(ExecuteCommand::GetEventIdFromString(event), callData);
	}

	bool GizmoObject::HasObserver(unsigned long event)
	{
		if (this->subjectHelper)
		{
			return this->subjectHelper->HasObserver(event);
		}
		return false;
	}

	bool GizmoObject::HasObserver(const char* event)
	{
		return this->HasObserver(ExecuteCommand::GetEventIdFromString(event));
	}

	bool GizmoObject::HasObserver(unsigned long event, ExecuteCommand* cmd)
	{
		if (this->subjectHelper)
		{
			return this->subjectHelper->HasObserver(event, cmd);
		}
		return false;
	}

	bool GizmoObject::HasObserver(const char* event, ExecuteCommand* cmd)
	{
		return this->HasObserver(ExecuteCommand::GetEventIdFromString(event), cmd);
	}

	void GizmoObject::InternalGrabFocus(ExecuteCommand* mouseEvents, ExecuteCommand* keypressEvents)
	{
		if (this->subjectHelper)
		{
			this->subjectHelper->GrabFocus(mouseEvents, keypressEvents);
		}
	}


	void GizmoObject::InternalReleaseFocus()
	{
		if (this->subjectHelper)
		{
			this->subjectHelper->ReleaseFocus();
		}
	}


	void GizmoObject::ObjectFinalize()
	{

		this->InvokeEvent(ExecuteCommand::DeleteEvent, nullptr);

		this->RemoveAllObservers();
	}
}