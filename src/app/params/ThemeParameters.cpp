
//APP
#include <app/params/ThemeParameters.h>

//SOCLE
#include <ign/Exception.h>


namespace app{
namespace params{


	///
	///
	///
	ThemeParameters::ThemeParameters()
	{
		_initParameter( DB_CONF_FILE, "DB_CONF_FILE");
		_initParameter( WORKING_SCHEMA, "WORKING_SCHEMA");
		_initParameter( POINT_TABLE, "POINT_TABLE");
		_initParameter( NET_TABLE_INIT, "NET_TABLE_INIT");
		_initParameter( NET_TABLE_MATCHED, "NET_TABLE_MATCHED");
		_initParameter( COUNTRY_CODE_W, "COUNTRY_CODE_W");
		_initParameter( LANDMASK_TABLE, "LANDMASK_TABLE" );
		_initParameter( LAND_COVER_TYPE_NAME, "LAND_COVER_TYPE_NAME" );
		_initParameter( TYPE_LAND_AREA, "TYPE_LAND_AREA");
		_initParameter( TYPE_INLAND_WATER, "TYPE_INLAND_WATER" );
		_initParameter( NATIONAL_IDENTIFIER_NAME, "NATIONAL_IDENTIFIER_NAME");
		_initParameter( W_TAG_NAME, "W_TAG_NAME");

		_initParameter( ADJACENCY_TABLE, "ADJACENCY_TABLE");
		_initParameter( ADJACENCY_TABLE_SUFFIX, "ADJACENCY_TABLE_SUFFIX");
		_initParameter( POINT_ID_NAME, "POINT_ID_NAME");
		_initParameter( EDGE_ID_NAME, "EDGE_ID_NAME");

		_initParameter( AI_MAX_ASSOCIATION_DIST, "AI_MAX_ASSOCIATION_DIST");

		_initParameter( PM_MAX_MATCHING_DIST, "PM_MAX_MATCHING_DIST");
	}

	///
	///
	///
	ThemeParameters::~ThemeParameters()
	{
	}

	///
	///
	///
	std::string ThemeParameters::getClassName()const
	{
		return "app::params::ThemeParameters";
	}


}
}