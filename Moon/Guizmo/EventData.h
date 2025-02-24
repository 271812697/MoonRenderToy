#pragma once

#include "ExecuteCommand.h"

namespace MOON {

	class vtkEventData
	{
	public:

		int GetType() const { return this->Type; }
		void SetType(int val) { this->Type = val; }

		// are two events equivalent
		bool operator==(const vtkEventData& a) const
		{
			return this->Type == a.Type && this->Equivalent(&a);
		}


	protected:
		vtkEventData() = default;
		~vtkEventData() = default;

		// subclasses override this to define their
		// definition of equivalent
		virtual bool Equivalent(const vtkEventData* ed) const = 0;

		int Type;

	private:
		vtkEventData(const vtkEventData& c) = delete;
	};




}