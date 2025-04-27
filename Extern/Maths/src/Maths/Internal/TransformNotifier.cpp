
#include "Maths/Internal/TransformNotifier.h"

Maths::Internal::TransformNotifier::NotificationHandlerID Maths::Internal::TransformNotifier::AddNotificationHandler(NotificationHandler p_notificationHandler)
{
	NotificationHandlerID handlerID = m_availableHandlerID++;
	m_notificationHandlers.emplace(handlerID, p_notificationHandler);
	return handlerID;
}

void Maths::Internal::TransformNotifier::NotifyChildren(ENotification p_notification)
{
	if (!m_notificationHandlers.empty())
		for (auto const&[id, handler] : m_notificationHandlers)
			handler(p_notification);
}

bool Maths::Internal::TransformNotifier::RemoveNotificationHandler(const NotificationHandlerID& p_notificationHandlerID)
{
	return m_notificationHandlers.erase(p_notificationHandlerID) != 0;
}
