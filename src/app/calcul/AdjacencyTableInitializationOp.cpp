// APP
#include <app/calcul/AdjacencyTableInitializationOp.h>
#include <app/params/ThemeParameters.h>

// BOOST
#include <boost/progress.hpp>

// EPG
#include <epg/Context.h>
#include <epg/params/EpgParameters.h>
#include <ome2/feature/sql/NotDestroyedTools.h>
#include <epg/sql/DataBaseManager.h>
#include <epg/tools/StringTools.h>
#include <epg/tools/TimeTools.h>
#include <epg/tools/FilterTools.h>

// SOCLE
#include <ign/geometry/index/QuadTree.h>



namespace app
{
    namespace calcul
    {
        ///
        ///
        ///
        AdjacencyTableInitializationOp::AdjacencyTableInitializationOp(
            std::string borderCode,
            bool verbose
        ) : 
            _borderCode(borderCode),
            _verbose(verbose)
        {
            _init();
        }

        ///
        ///
        ///
        AdjacencyTableInitializationOp::~AdjacencyTableInitializationOp()
        {
            // _shapeLogger->closeShape("cbl_working_zone");
        }

        ///
        ///
        ///
        void AdjacencyTableInitializationOp::Compute(
            std::string borderCode,
			bool verbose
		) {
            AdjacencyTableInitializationOp op(borderCode, verbose);
            op._compute();
        }

        ///
        ///
        ///
        void AdjacencyTableInitializationOp::_init()
        {
            //--
            _logger = epg::log::EpgLoggerS::getInstance();
            _logger->log(epg::log::INFO, "[START] initialization: " + epg::tools::TimeTools::getTime());

            //--
            _shapeLogger = epg::log::ShapeLoggerS::getInstance();
            // _shapeLogger->addShape("cbl_working_zone", epg::log::ShapeLogger::POLYGON);

            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const idName = epgParams.getValue(ID).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();

            // app parameters
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();
            std::string const pointTableName = themeParameters->getValue(POINT_TABLE).toString();
            std::string const edgeTableName = themeParameters->getValue(NET_TABLE_INIT).toString();
            std::string const adjacencyTableName = themeParameters->getParameter(ADJACENCY_TABLE).getValue().toString();

            //--
            _fsPoint = context->getDataBaseManager().getFeatureStore(pointTableName, idName, geomName);

            //--
            _fsEdge = context->getDataBaseManager().getFeatureStore(edgeTableName, idName, geomName);

            //--
            context->getDataBaseManager().getConnection()->update("DELETE FROM "+adjacencyTableName);
            
            //--
            _logger->log(epg::log::INFO, "[END] initialization: " + epg::tools::TimeTools::getTime());
        };

        ///
        ///
        ///
        void AdjacencyTableInitializationOp::_compute() const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
		    double const maxAssDist = themeParameters->getParameter(AI_MAX_ASSOCIATION_DIST).getValue().toDouble();
		    std::string const adjacencyTableName = themeParameters->getParameter(ADJACENCY_TABLE).getValue().toString();
		    std::string const pointIdName = themeParameters->getParameter(POINT_ID_NAME).getValue().toString();
		    std::string const edgeIdName = themeParameters->getParameter(EDGE_ID_NAME).getValue().toString();
            std::string const natIdIdName = themeParameters->getValue(NATIONAL_IDENTIFIER_NAME).toString();

            //--
            std::list<std::pair<std::string, std::string>> lAdjacency;

            // chargement et indexation du reseau
            std::vector<std::string> vCountry;
		    epg::tools::StringTools::Split(_borderCode, "#", vCountry);

            ign::geometry::Envelope edgeBounds = _fsEdge->getBounds();
            
            for ( std::vector<std::string>::const_iterator vit = vCountry.begin() ; vit != vCountry.end() ; ++vit ) {

                ign::geometry::index::QuadTree<size_t> qtree;
                std::vector<std::pair<std::string, ign::geometry::MultiPoint>> vIdEndings;

                qtree.ensureExtent(edgeBounds);

                //--
                ign::feature::FeatureFilter filterEdge( countryCodeName +"='"+*vit+"'" );

                int numEdges = ome2::feature::sql::NotDestroyedTools::NumFeatures(*_fsEdge, filterEdge);
                boost::progress_display displayCountry(numEdges, std::cout, "[ loading network ["+*vit+"] % complete ]\n");

                ign::feature::FeatureIteratorPtr itEdge = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsEdge, filterEdge);
                while (itEdge->hasNext())
                {
                    ++displayCountry;

                    ign::feature::Feature const& fEdge = itEdge->next();
                    ign::geometry::LineString const& edgeGeom = fEdge.getGeometry().asLineString();
                    std::string const& natId = fEdge.getAttribute(natIdIdName).toString();

                    qtree.insert(vIdEndings.size(), edgeGeom.getEnvelope());
                    vIdEndings.push_back(std::make_pair(natId, ign::geometry::MultiPoint(edgeGeom.startPoint())));
                    vIdEndings.back().second.addGeometry(edgeGeom.endPoint());
                }

                //--
                ign::feature::FeatureFilter filterPoint( countryCodeName +"='"+*vit+"'" );

                int numPoints = ome2::feature::sql::NotDestroyedTools::NumFeatures(*_fsPoint, filterPoint);
                boost::progress_display display(numPoints, std::cout, "[ computing adjacency ["+*vit+"] % complete ]\n");

                ign::feature::FeatureIteratorPtr itPoint = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsPoint, filterPoint);
                while (itPoint->hasNext())
                {
                    ++display;

                    ign::feature::Feature const& fPoint = itPoint->next();
                    ign::geometry::Point const& pointGeom = fPoint.getGeometry().asPoint();
                    std::string pointId = fPoint.getId();

                    //DEBUG
                    // if( pointId == "888eec32-f6d4-4b49-b724-58e8c8fa8447") {
                    //     bool test = true;
                    // }
                    // if( pointId == "02764c12-a755-47c3-9ee9-a36a201c6b65") {
                    //     bool test = true;
                    // }

                    std::set< size_t > sIndex;
                    qtree.query( pointGeom.getEnvelope().expandBy( maxAssDist ), sIndex );

                    for ( std::set< size_t >::const_iterator sit = sIndex.begin() ; sit != sIndex.end() ; ++sit )
                        if( pointGeom.distance(vIdEndings[*sit].second) < maxAssDist )
                            lAdjacency.push_back(std::make_pair(pointId, vIdEndings[*sit].first));
                }
            }

            //--
            boost::progress_display displayRecording(lAdjacency.size(), std::cout, "[ recording adjacency tuples % complete ]\n");

            size_t batchSize = 100;
            std::string query = "";
            size_t count = 0;
            for ( std::list<std::pair<std::string, std::string>>::const_iterator lit = lAdjacency.begin() ; lit != lAdjacency.end() ; ++lit, ++displayRecording, ++count ) {
                query += "INSERT INTO "+adjacencyTableName+" ("+pointIdName+", "+edgeIdName+") VALUES ('"+lit->first+"', '"+lit->second+"');";

                if ( count == batchSize ) {
                    context->getDataBaseManager().getConnection()->update(query);

                    count = 0;
                    query = "";
                }
            }
            if (!query.empty())
                context->getDataBaseManager().getConnection()->update(query);
        }
    }
}