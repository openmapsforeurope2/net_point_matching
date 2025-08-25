#ifndef _APP_STEP_INITNETPOINTADJACENCY_H_
#define _APP_STEP_INITNETPOINTADJACENCY_H_

#include <epg/step/StepBase.h>
#include <app/params/ThemeParameters.h>

namespace app{
namespace step{

	class InitNetPointAdjacency : public epg::step::StepBase< app::params::ThemeParametersS > {

	public:

		/// \brief
		int getCode() { return 510; };

		/// \brief
		std::string getName() { return "InitNetPointAdjacency"; };

		/// \brief
		void onCompute( bool );

		/// \brief
		void init();

	};

}
}

#endif