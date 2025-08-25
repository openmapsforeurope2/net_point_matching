#ifndef _APP_STEP_POINTMATCHING_H_
#define _APP_STEP_POINTMATCHING_H_

#include <epg/step/StepBase.h>
#include <app/params/ThemeParameters.h>

namespace app{
namespace step{

	class PointMatching : public epg::step::StepBase< app::params::ThemeParametersS > {

	public:

		/// \brief
		int getCode() { return 520; };

		/// \brief
		std::string getName() { return "PointMatching"; };

		/// \brief
		void onCompute( bool );

		/// \brief
		void init();

	};

}
}

#endif