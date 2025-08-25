// APP
#include <app/utils/createAdjacencyTable.h>
#include <app/params/ThemeParameters.h>

// EPG
#include <epg/Context.h>


namespace app{
namespace utils{

    //--
    void createAdjacencyTable() {
        epg::Context* context = epg::ContextS::getInstance();
        app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();

        std::string const idName = context->getEpgParameters().getValue(ID).toString();
	    std::string const geomName = context->getEpgParameters().getValue(GEOM).toString();
        std::string const countryCodeName = context->getEpgParameters().getValue(COUNTRY_CODE).toString();

        std::string const adjacencyTableName = themeParameters->getValue(ADJACENCY_TABLE).toString();
        std::string const pointIdName = themeParameters->getValue(POINT_ID_NAME).toString();
        std::string const edgeIdName = themeParameters->getValue(EDGE_ID_NAME).toString();

        if (!context->getDataBaseManager().tableExists(adjacencyTableName)) {
            std::ostringstream ss;
            ss << "CREATE TABLE " << adjacencyTableName
                << " (" 
                << pointIdName << " varchar(255), "
                << edgeIdName << " varchar(255)"
                << ");"
                << " CREATE INDEX IF NOT EXISTS " + adjacencyTableName+"_"+pointIdName+"_idx ON " + adjacencyTableName
                << " USING btree ("+pointIdName+" ASC NULLS LAST);";

            context->getDataBaseManager().getConnection()->update(ss.str());
        }
    }
}
}