#ifndef _APP_PARAMS_THEMEPARAMETERS_H_
#define _APP_PARAMS_THEMEPARAMETERS_H_

//STL
#include <string>

//EPG
#include <epg/params/ParametersT.h>
#include <epg/SingletonT.h>



	enum TH_PARAMETERS{
		DB_CONF_FILE,
		WORKING_SCHEMA,
		POINT_TABLE,
		POINT_TABLE_BASE,
		NET_TABLE_INIT,
		NET_TABLE_INIT_BASE,
		NET_TABLE_MATCHED,
		NET_TABLE_MATCHED_BASE,
		COUNTRY_CODE_W,
		LANDMASK_TABLE,
		LAND_COVER_TYPE_NAME,
		TYPE_LAND_AREA,
		TYPE_INLAND_WATER,
		NATIONAL_IDENTIFIER_NAME,
		W_TAG_NAME,

		ADJACENCY_TABLE,
		ADJACENCY_TABLE_SUFFIX,
		POINT_ID_NAME,
		EDGE_ID_NAME,

		AI_MAX_ASSOCIATION_DIST,

		PM_MAX_MATCHING_DIST
	};


namespace app{
namespace params{

	class ThemeParameters : public epg::params::ParametersT< TH_PARAMETERS >
	{
		typedef  epg::params::ParametersT< TH_PARAMETERS > Base;

		public:

			/// \brief
			ThemeParameters();

			/// \brief
			~ThemeParameters();

			/// \brief
			virtual std::string getClassName()const;

	};

	typedef epg::Singleton< ThemeParameters >   ThemeParametersS;

}
}

#endif