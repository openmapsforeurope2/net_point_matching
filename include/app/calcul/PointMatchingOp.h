#ifndef _APP_CALCUL_POINTMATCHINGOP_H_
#define _APP_CALCUL_POINTMATCHINGOP_H_

//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>
#include <ign/geometry/graph/GeometryGraph.h>

//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>


namespace app{
namespace calcul{

	/// @brief
	class PointMatchingOp {

		typedef ign::geometry::graph::GeometryGraph< ign::geometry::graph::PunctualVertexProperties, ign::geometry::graph::LinearEdgeProperties >  GraphType;
		typedef typename GraphType::edge_descriptor edge_descriptor;
		typedef typename GraphType::oriented_edge_descriptor oriented_edge_descriptor;
		typedef typename GraphType::vertex_descriptor vertex_descriptor;
        typedef typename GraphType::edge_iterator edge_iterator;

	public:

		/// @brief Constructeur
		/// @param verbose Mode verbeux
		PointMatchingOp(
			std::string borderCode,
            bool verbose
        );

        /// @brief Destructeur
        ~PointMatchingOp();

		/// @brief 
		/// @param verbose Mode verbeux
		static void Compute(
			std::string borderCode,
			bool verbose
		);

	private:

		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsPoint;
		//--
		ign::feature::sql::FeatureStorePostgis*                  _fsEdge;
		//--
		epg::log::EpgLogger*                                     _logger;
		//--
		epg::log::ShapeLogger*                                   _shapeLogger;
		//--
		std::string                                              _borderCode;
		//--
		bool                                                     _verbose;


	private:

		//--
		void _init();

        //--
		void _compute() const;

		//--
		void _loadGraph(
            GraphType & graph,
			std::vector<std::string> & vEdgeIndexNatId,
            std::string const& country        
        ) const;

		//--
		void _loadAdjacency(
			std::map<std::string, std::list<std::string>> & mAdjacency
		) const;
    };
}
}

#endif
