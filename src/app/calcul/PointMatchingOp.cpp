// APP
#include <app/calcul/PointMatchingOp.h>
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
#include <ign/geometry/graph/builder/SimpleGraphBuilder.h>
// #include <ign/sql/SqlResultSet.h>



namespace app
{
    namespace calcul
    {
        ///
        ///
        ///
        PointMatchingOp::PointMatchingOp(
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
        PointMatchingOp::~PointMatchingOp()
        {
            _shapeLogger->closeShape("pm_displacements");
            _shapeLogger->closeShape("pm_removed_point");
        }

        ///
        ///
        ///
        void PointMatchingOp::Compute(
            std::string borderCode,
			bool verbose
		) {
            PointMatchingOp op(borderCode, verbose);
            op._compute();
        }

        ///
        ///
        ///
        void PointMatchingOp::_init()
        {
            //--
            _logger = epg::log::EpgLoggerS::getInstance();
            _logger->log(epg::log::INFO, "[START] initialization: " + epg::tools::TimeTools::getTime());

            //--
            _shapeLogger = epg::log::ShapeLoggerS::getInstance();
            _shapeLogger->addShape("pm_displacements", epg::log::ShapeLogger::LINESTRING);
            _shapeLogger->addShape("pm_removed_point", epg::log::ShapeLogger::POINT);

            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const idName = epgParams.getValue(ID).toString();
            std::string const geomName = epgParams.getValue(GEOM).toString();
            std::string const pointTableName = epgParams.getValue(VERTEX_TABLE).toString();

            // app parameters
            params::ThemeParameters *themeParameters = params::ThemeParametersS::getInstance();
            std::string const edgeTableName = themeParameters->getValue(NET_TABLE_MATCHED).toString();

            //--
            _fsPoint = context->getDataBaseManager().getFeatureStore(pointTableName, idName, geomName);

            //--
            _fsEdge = context->getDataBaseManager().getFeatureStore(edgeTableName, idName, geomName);
            
            //--
            _logger->log(epg::log::INFO, "[END] initialization: " + epg::tools::TimeTools::getTime());
        };

        ///
        ///
        ///
        void PointMatchingOp::_compute() const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            // epg parameters
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
		    double const maxMatchDist = themeParameters->getParameter(PM_MAX_MATCHING_DIST).getValue().toDouble();

            //--
            std::map<std::string, std::list<std::string>> mAdjacency;
            _loadAdjacency(mAdjacency);

            //--
            std::list<std::string> lPointToRemove;

            // chargement et indexation du reseau
            std::vector<std::string> vCountry;
		    epg::tools::StringTools::Split(_borderCode, "#", vCountry);

            ign::geometry::Envelope edgeBounds = _fsEdge->getBounds();
            
            for ( std::vector<std::string>::const_iterator vit = vCountry.begin() ; vit != vCountry.end() ; ++vit ) {

                //--
                GraphType graph;
                std::vector<std::string> vEdgeIndexNatId;
                _loadGraph(graph, vEdgeIndexNatId, *vit);

                //--
                ign::feature::FeatureFilter filterPoint( countryCodeName +"='"+*vit+"'" );

                int numPoints = ome2::feature::sql::NotDestroyedTools::NumFeatures(*_fsPoint, filterPoint);
                boost::progress_display display(numPoints, std::cout, "[ computing point matching ["+*vit+"] % complete ]\n");

                ign::feature::FeatureIteratorPtr itPoint = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsPoint, filterPoint);
                while (itPoint->hasNext())
                {
                    ++display;

                    ign::feature::Feature const& fPoint = itPoint->next();
                    ign::geometry::Point const& pointGeom = fPoint.getGeometry().asPoint();
                    std::string pointId = fPoint.getId();

                    std::map<std::string, std::list<std::string>>::const_iterator mitAdj = mAdjacency.find(pointId);
                    if (mitAdj == mAdjacency.end())
                        continue;
                    std::list<std::string> const& lAdjacentEdges = mitAdj->second;

                    std::vector< vertex_descriptor > vCandidates = graph.verticesIntersectingBox( pointGeom.getEnvelope().expandBy(maxMatchDist) );
                    std::map< double, vertex_descriptor> mDistCandidate;
                    for ( size_t i = 0 ; i < vCandidates.size() ; ++i ) {
                        double dist = pointGeom.distance(graph.getGeometry(vCandidates[i]));
                        if (dist < maxMatchDist)
                            mDistCandidate.insert(std::make_pair(dist, vCandidates[i]));
                    }

                    std::map<double, vertex_descriptor> mScoredVertex;
                    for ( std::map< double, vertex_descriptor>::const_iterator mit = mDistCandidate.begin() ; mit != mDistCandidate.end() ; ++mit ) {
                        std::vector< edge_descriptor > vIncidentEdges = graph.incidentEdges(mit->second);
                        std::list<std::string> lNewAdjacentEdges;
                        for ( size_t i = 0 ; i < vIncidentEdges.size() ; ++i ) {
                            lNewAdjacentEdges.push_back(vEdgeIndexNatId[ign::data::String(graph.origins(vIncidentEdges[i]).front()).toInteger()]);
                        }

                        if( lNewAdjacentEdges.size() > lAdjacentEdges.size() )
                            continue;
                        
                        size_t count = 0;
                        for ( std::list<std::string>::const_iterator lit = lAdjacentEdges.begin() ; lit != lAdjacentEdges.end() ; ++lit ) {
                            for ( std::list<std::string>::const_iterator lit2 = lNewAdjacentEdges.begin() ; lit2 != lNewAdjacentEdges.end() ; ++lit2 ) {
                                if( *lit == *lit2 ) {
                                    ++count;
                                    lNewAdjacentEdges.erase(lit2);
                                    break;
                                }
                            }
                        }
                        if ( count == 0 ) continue;
                            
                        double score = mit->first * (lAdjacentEdges.size()/count);
                        mScoredVertex.insert(std::make_pair( score, mit->second ));
                    }

                    if( mScoredVertex.size() == 0 ) {
                        lPointToRemove.push_back(pointId);

                        //DEBUG
                        _shapeLogger->writeFeature("pm_removed_point", fPoint);

                        continue;
                    }

                    if( mScoredVertex.begin()->first > 0 ) {
                        ign::feature::Feature newFeat = fPoint;
                        newFeat.setGeometry(graph.getGeometry(mScoredVertex.begin()->second));
                        _fsPoint->modifyFeature(newFeat);

                        //DEBUG
                        ign::feature::Feature feat;
                        feat.setGeometry(ign::geometry::LineString(pointGeom, graph.getGeometry(mScoredVertex.begin()->second)));
                        _shapeLogger->writeFeature("pm_displacements", feat);
                    }
                }
            }

            //--
            for ( std::list<std::string>::const_iterator lit = lPointToRemove.begin() ; lit != lPointToRemove.end() ; ++lit ) {
                _fsPoint->deleteFeature(*lit);
            }
        }

        ///
        ///
        ///
        void PointMatchingOp::_loadAdjacency(std::map<std::string, std::list<std::string>> & mAdjacency) const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            //--
			app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
            std::string const adjacencyTableName = themeParameters->getParameter(ADJACENCY_TABLE).getValue().toString();
		    std::string const pointIdName = themeParameters->getParameter(POINT_ID_NAME).getValue().toString();
		    std::string const edgeIdName = themeParameters->getParameter(EDGE_ID_NAME).getValue().toString();

            std::string query = "SELECT "+pointIdName+", "+edgeIdName+" from "+adjacencyTableName;
            ign::sql::SqlResultSetPtr resultPtr = context->getDataBaseManager().getConnection()->query(query);

            for( size_t i = 0 ; i < resultPtr->size() ; ++i ) {
                std::string pointId = resultPtr->getFieldValue(i, 0).toString();
                std::string edgeNatId = resultPtr->getFieldValue(i, 1).toString();

                std::map<std::string, std::list<std::string>>::iterator mit = mAdjacency.find(pointId);
                if( mit ==  mAdjacency.end() )
                    mit = mAdjacency.insert(std::make_pair(pointId, std::list<std::string>())).first;
                mit->second.push_back(edgeNatId);
            }
        }

        ///
        ///
        ///
        void PointMatchingOp::_loadGraph(
            GraphType & graph,
            std::vector<std::string> & vEdgeIndexNatId,
            std::string const& country        
        ) const {
            //--
            epg::Context *context = epg::ContextS::getInstance();

            //--
            epg::params::EpgParameters const& epgParams = context->getEpgParameters();
            std::string const countryCodeName = epgParams.getValue(COUNTRY_CODE).toString();

            //--
            app::params::ThemeParameters* themeParameters = app::params::ThemeParametersS::getInstance();
            std::string const natIdName = themeParameters->getParameter(NATIONAL_IDENTIFIER_NAME).getValue().toString();

            // chargement des edges
            ign::feature::FeatureFilter filterEdge( countryCodeName +" LIKE '%"+country+"%'" );
            //patch
            epg::tools::FilterTools::addAndConditions(filterEdge, epgParams.getValue(GEOM).toString() + " IS NOT NULL");

            ign::geometry::graph::builder::SimpleGraphBuilder<GraphType> builder(graph, 1e-5);

            // patience
            size_t numFeatures = ome2::feature::sql::NotDestroyedTools::NumFeatures(*_fsEdge, filterEdge);
            boost::progress_display display(numFeatures, std::cout, "[ loading graph % complete ]\n");

            ign::feature::FeatureIteratorPtr itEdge = ome2::feature::sql::NotDestroyedTools::GetFeatures(*_fsEdge, filterEdge);
            while (itEdge->hasNext())
            {
                ++display;
                ign::feature::Feature const& fEdge = itEdge->next();

                ign::geometry::LineString const& ls = fEdge.getGeometry().asLineString();
                std::string fNatId = fEdge.getAttribute(natIdName).toString();
                std::string const& fCountry = fEdge.getAttribute(countryCodeName).toString();

                if(fCountry.find("#") != std::string::npos) {
                    std::vector<std::string> vCountry;
		            epg::tools::StringTools::Split(fCountry, "#", vCountry);

                    size_t index = country == vCountry.front() ? 0 : 1;

                    if(fNatId.find("#") != std::string::npos) {
                        std::vector<std::string> vNatId;
                        epg::tools::StringTools::Split(fNatId, "#", vNatId);

                        fNatId = vNatId[index];
                    }
                }

                builder.addEdge(ign::geometry::LineString(ls.startPoint(), ls.endPoint()), ign::data::Integer(vEdgeIndexNatId.size()).toString());
                vEdgeIndexNatId.push_back(fNatId);
            }
        }
    }
}