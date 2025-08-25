#include <app/step/520_PointMatching.h>

// EPG
#include <ome2/utils/CopyTableUtils.h>

// APP
#include <app/calcul/PointMatchingOp.h>

namespace app {
	namespace step {

		///
		///
		///
		void PointMatching::init()
		{
			addWorkingEntity(POINT_TABLE);
		}

		///
		///
		///
		void PointMatching::onCompute(bool verbose = false)
		{
			// copie 
			_epgParams.setParameter(VERTEX_TABLE, ign::data::String(getCurrentWorkingTableName(POINT_TABLE)));
			ome2::utils::CopyTableUtils::copyVertexTable(getLastWorkingTableName(POINT_TABLE), "", false, true);

			// traitement
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
			std::string countryCodeW = themeParameters->getParameter(COUNTRY_CODE_W).getValue().toString();

			app::calcul::PointMatchingOp::Compute(countryCodeW, verbose);

		}

	}
}