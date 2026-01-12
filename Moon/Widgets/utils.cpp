#include "Widgets/utils.h"
namespace MOON {
	int emToPPx(const QFontMetrics& m, double em) {
		const auto pxPerEm = m.boundingRect(QString(100, 'M')).width() / 100.0;
		return static_cast<int>(std::round(pxPerEm * em));
	}
	int emToPx(const QWidget* w, double em) {
		w->ensurePolished();
		return emToPPx(w->fontMetrics(), em);
	}
	int refSpacePx(const QWidget* w) { return emToPx(w, refSpaceEm()); }
}