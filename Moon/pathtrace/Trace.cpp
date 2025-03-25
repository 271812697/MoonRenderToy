#include "Trace.h"

namespace PathTrace
{
	Vec4 Trace(const Ray& r, const RenderOptions& opt)
	{
		Vec3 radiance = Vec3(0.0);
		Vec3 throughput = Vec3(1.0);

		State state;
		LightSampleRec lightSample;
		ScatterSampleRec scatterSample;
		// FIXME: alpha from material opacity/medium density
		float alpha = 1.0;
		// For medium tracking
		bool inMedium = false;
		bool mediumSampled = false;
		bool surfaceScatter = false;
		for (state.depth = 0;; state.depth++) {

		}

		return Vec4();
	}
}