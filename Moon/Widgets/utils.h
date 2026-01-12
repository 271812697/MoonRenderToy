#pragma once 
#include <QWidget>
namespace MOON {
	// In a non high dpi system an 'M' measures 11 px. Hence all our old pixels sizes, can be converted
// to Em sizes by dividing by 11
	constexpr int refEm() { return 11; }
	constexpr double refSpaceEm() { return 7.0 / refEm(); }
	template <typename T>
	int decimals([[maybe_unused]] T inc) {
		if constexpr (std::is_floating_point_v<T>) {
			if (inc == T{ 0.0 } || inc >= T{ 1.0 }) return 0;
			return static_cast<int>(-std::log10(inc));
		}
		else {
			return 0;
		}
	}
	int emToPPx(const QFontMetrics& m, double em);
	int emToPx(const QWidget* w, double em);
	int refSpacePx(const QWidget* w);

}