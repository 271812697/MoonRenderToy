#pragma once
#include <vector>
#include <limits> // 支持 numeric_limits
#define GetSelection MOON::SelectionManager::instance()
namespace MOON
{
	struct SelectID
	{
		static constexpr size_t InvalidID = std::numeric_limits<size_t>::max();
		SelectID() = default;
		SelectID(int64_t actid) { actorId = static_cast<size_t>(actid); }
		SelectID(int actid) { actorId = static_cast<size_t>(actid); }
		SelectID(size_t actid) { actorId = actid; }
		operator int64_t() const {
			return static_cast<int64_t>(actorId);
		}

		void reset() { actorId = InvalidID; }
		bool isValid() const { return actorId != InvalidID; }
		size_t actorId = InvalidID;
	};

	class SelectionManager
	{
	public:
		static SelectionManager& instance();
		void setPreselect(SelectID id);
		void addSelect(const std::vector<SelectID>& selectIdLists);
		void setSelect(const std::vector<SelectID>& selectIdLists);
		void clearSelect();
		void clearPreselect();
		SelectID getPreselect();
		std::vector<SelectID> getSelect();
		~SelectionManager();
	private:
		SelectionManager();
		class Internal;
		Internal* mInternal = nullptr;
	};
}