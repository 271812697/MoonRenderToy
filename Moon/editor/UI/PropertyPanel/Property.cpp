#pragma once
#include "editor/UI/PropertyPanel/Property.h"
namespace MOON {
	Property::Property(const QString& n, PropertyComponent* comp):mName(n), owner(comp)
	{
	}
	Property::~Property() {
	
	}
	QString Property::getPropertyName()
	{
		return mName;
	}
}