#ifndef _APP_CALCUL_ADJACENCYTABLEINITIALIZATIONOP_H_
#define _APP_CALCUL_ADJACENCYTABLEINITIALIZATIONOP_H_

//SOCLE
#include <ign/feature/sql/FeatureStorePostgis.h>


//EPG
#include <epg/log/EpgLogger.h>
#include <epg/log/ShapeLogger.h>


namespace app{
namespace calcul{

	/// @brief
	class AdjacencyTableInitializationOp {

	public:

		/// @brief Constructeur
		/// @param verbose Mode verbeux
		AdjacencyTableInitializationOp(
			std::string borderCode,
            bool verbose
        );

        /// @brief Destructeur
        ~AdjacencyTableInitializationOp();

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
    };
}
}

#endif
