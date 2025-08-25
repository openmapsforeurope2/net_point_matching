#include <app/step/510_InitNetPointAdjacency.h>

// EPG
#include <ome2/utils/CopyTableUtils.h>

// APP
#include <app/calcul/AdjacencyTableInitializationOp.h>

namespace app {
	namespace step {

		///
		///
		///
		void InitNetPointAdjacency::init()
		{
		}

		///
		///
		///
		void InitNetPointAdjacency::onCompute(bool verbose = false)
		{
			// traitement
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
			std::string countryCodeW = themeParameters->getParameter(COUNTRY_CODE_W).getValue().toString();

			app::calcul::AdjacencyTableInitializationOp::Compute(countryCodeW, verbose);

		}

	}
}