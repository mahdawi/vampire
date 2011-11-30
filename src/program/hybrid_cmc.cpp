///
/// @file
/// @brief Contains the Curie Temperature program
///
/// @details Performs a temperature loop to calculate temperature dependent magnetisation
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2010. All Rights Reserved.
///
/// @section info File Information
/// @author  Richard Evans, richard.evans@york.ac.uk
/// @version 1.1
/// @date    09/03/2011
/// @internal
///	Created:		05/02/2011
///	Revision:	09/03/2011
///=====================================================================================
///

// Standard Libraries
#include <iostream>

// Vampire Header files
#include "atoms.hpp"
#include "errors.hpp"
#include "material.hpp"
#include "program.hpp"
#include "random.hpp"
#include "sim.hpp"
#include "stats.hpp"
#include "vio.hpp"
#include "vmath.hpp"
#include "vmpi.hpp"

namespace program{

/// @brief Function to calculate the temperature dependence of the anisotropy and magnetisation
///
/// @callgraph
/// @callergraph
///
/// @details Consists of a sequence of sub-calculations of fixed temperature, where the constraint angles 
/// are cycled. The system is initialised all spins along the constraint direction. After initialisation 
/// the sytem is equilibrated for sim::equilibration timesteps before statistics are collected.
///
/// @section License
/// Use of this code, either in source or compiled form, is subject to license from the authors.
/// Copyright \htmlonly &copy \endhtmlonly Richard Evans, 2009-2011. All Rights Reserved.
///
/// @section Information
/// @author  Richard Evans, richard.evans@york.ac.uk
/// @version 1.0
/// @date    15/09/2011
///
/// @return void
/// 
/// @internal
///	Created:		02/11/2011
///	Revision:	--/--/----
///=====================================================================================
///
void hybrid_cmc(){

	// check calling of routine if error checking is activated
	if(err::check==true){std::cout << "program::hybrid_cmc has been called" << std::endl;}

	// Check integrator is CMC, if not then exit disgracefully
	if(sim::integrator!=4){
		std::cerr << "Error! cmc-anisotropy program requires Hybrid Constrained Monte Carlo as the integrator. Exiting." << std::endl; 
		err::vexit();
	}
	
	// resize cmc array to include correct number of materials
	cmc::cmc_mat.resize(mp::num_materials);
	
	// loop over all materials
	for (int mat=0;mat<mp::num_materials;mat++){
	std::cout << "Hybrid CMC loop for material " << mat << std::endl; 
	// set minimum rotational angle
	cmc::cmc_mat[mat].constraint_theta=cmc::cmc_mat[mat].constraint_theta_min;

	// perform rotational angle sweep
	while(cmc::cmc_mat[mat].constraint_theta<=cmc::cmc_mat[mat].constraint_theta_max){

		// set minimum azimuthal angle
		cmc::cmc_mat[mat].constraint_phi=cmc::cmc_mat[mat].constraint_phi_min;

		// perform azimuthal angle sweep
		while(cmc::cmc_mat[mat].constraint_phi<=cmc::cmc_mat[mat].constraint_phi_max){
			
			// Re-initialise spin moments for CMC
			sim::CMCMCinit();
			
			// Set starting temperature
			sim::temperature=sim::Tmin;
			
			// Perform Temperature Loop
			while(sim::temperature<=sim::Tmax){

				// Equilibrate system
				sim::integrate(sim::equilibration_time);
				
				// Reset mean magnetisation counters
				stats::mag_m_reset();
				
				// Reset start time
				int start_time=sim::time;

				// Simulate system
				while(sim::time<sim::loop_time+start_time){
					
					// Integrate system
					sim::integrate(sim::partial_time);
				
					// Calculate magnetisation statistics
					stats::mag_m();

				}
				
				// Output data
				vout::data();
				
				// Increment temperature
				sim::temperature+=sim::delta_temperature;
				
			} // End of temperature loop
			
			// Increment azimuthal angle
			if(cmc::cmc_mat[mat].constraint_phi+cmc::cmc_mat[mat].constraint_phi_delta>cmc::cmc_mat[mat].constraint_phi_max) break;
			cmc::cmc_mat[mat].constraint_phi+=cmc::cmc_mat[mat].constraint_phi_delta;
			
		} // End of azimuthal angle sweep
		
		// Increment rotational angle
		if(cmc::cmc_mat[mat].constraint_theta+cmc::cmc_mat[mat].constraint_theta_delta>cmc::cmc_mat[mat].constraint_theta_max) break;
		cmc::cmc_mat[mat].constraint_theta+=cmc::cmc_mat[mat].constraint_theta_delta;
			
	} // End of rotational angle sweep

	} // end of material loop
	return;
}

}//end of namespace program