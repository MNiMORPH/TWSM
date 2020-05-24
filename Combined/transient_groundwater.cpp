#include <chrono>
#include <thread>
#include "transient_groundwater.hpp"

///////////////////////
// PRIVATE FUNCTIONS //
///////////////////////

double FanDarcyGroundwater::computeTransmissivity(ArrayPack &arp, uint32_t x,
                                                  uint32_t y){
    using namespace std::this_thread;     // sleep_for, sleep_until
    using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

    double T;
    if(arp.fdepth(x,y)>0){
        // Equation S6 from the Fan paper
        if(arp.wtd(x,y)<-1.5){
            T = arp.fdepth(x,y) * arp.ksat(x,y) \
                       * std::exp( (arp.wtd(x,y)+1.5)/arp.fdepth(x,y) );
        }
        // If wtd is greater than 0, max out rate of groundwater movement
        // as though wtd were 0. The surface water will get to move in
        // FillSpillMerge.
        else if(arp.wtd(x,y) > 0){
            T = arp.ksat(x,y) * (0+1.5+arp.fdepth(x,y));
        }
        //Equation S4 from the Fan paper
        else{
            T = arp.ksat(x,y) * (arp.wtd(x,y) + 1.5 + arp.fdepth(x,y));
        }
    }
    // If the fdepth is zero, there is no water transmission below the surface
    // soil layer.
    // If it is less than zero, it is incorrect -- but no water transmission
    // also seems an okay thing to do in this case.
    else{
        T = 0;
    }
    return T;
}

void FanDarcyGroundwater::computeNeighborTransmissivity(ArrayPack &arp,
                                                        uint32_t x, uint32_t y){
    double transmissivityTargetCell = computeTransmissivity(arp, x, y);
    transmissivityN = ( transmissivityTargetCell
                          + computeTransmissivity(arp, x,  y+1) ) / 2.;
    transmissivityS = ( transmissivityTargetCell
                          + computeTransmissivity(arp, x,  y-1) ) / 2.;
    transmissivityW = ( transmissivityTargetCell
                          + computeTransmissivity(arp, x-1,y  ) ) / 2.;
    transmissivityE = ( transmissivityTargetCell
                          + computeTransmissivity(arp, x+1,y  ) ) / 2.;
}

double FanDarcyGroundwater::computeArrayMax(double *_val[], uint8_t size){
    double maxValue = std::numeric_limits<double>::min();
    for (uint32_t i = 0; i < size; i++){
        if(*_val[i] > maxValue){
            maxValue = *_val[i];
        }
    }
    return maxValue;
}

float FanDarcyGroundwater::computeArrayMin(float *_val[], uint8_t size){
    float minValue = std::numeric_limits<double>::max();
    for (uint32_t i = 0; i < size; i++){
        if(*_val[i] < minValue){
            minValue = *_val[i];
        }
    }
    return minValue;
}

double FanDarcyGroundwater::computeMaxStableTimeStep(Parameters &params,
                                                     ArrayPack &arp,
                                                     uint32_t x,
                                                     uint32_t y){
    // Transmissivity is an effective diffusivity
    // Use the highest transmissivity for this time-step calculation
    double dt_max_diffusion_basic;
    double dt_max_diffusion_withPorosity;
    double *Tarray[4] = { &transmissivityN, &transmissivityS,
                             &transmissivityW, &transmissivityE };
    double Dmax = computeArrayMax( Tarray, 4 ); // Max diffusivity
    dt_max_diffusion_basic = ( pow(arp.cellsize_e_w_metres[y], 2)
                                  * pow(params.cellsize_n_s_metres, 2) ) \
                                  / ( 2 * Dmax \
                                    * ( pow(arp.cellsize_e_w_metres[y], 2) \
                                      + pow(params.cellsize_n_s_metres, 2) ) );
    // Now let's add in the porosity differences
    // Porosity differences will AMPLIFY the WTD changes.
    // Let us choose a time step that is also based on the LOWEST porosity
    // (highest WTD change per water volume transfer)
    float *PhiArray[5] = { &arp.porosity(x, y),
                             &arp.porosity(x+1, y), &arp.porosity(x-1, y),
                             &arp.porosity(x, y+1), &arp.porosity(x, y-1) };
    float PhiMin = computeArrayMin( PhiArray, 5 ); // Minimum porosity
    if(PhiMin < 0.2)
        PhiMin = 0.2;
    // Porosity is a linear amplifier of WTD change, and it amplifies change
    // in both the giving and receiving cells.
    // Amplification goes as 1 / phi.
    // We need to consider this going up and down, so the amplification becomes
    // 2/Phi, and therefore we multiply our stable dt by Phi/2
    dt_max_diffusion_withPorosity = dt_max_diffusion_basic * PhiMin / 2.;
    // In order to avoid operating at the very maximum time step possible,
    // we apply a factor of safety of 2

    return dt_max_diffusion_withPorosity/2.;
}


double FanDarcyGroundwater::calculateWaterVolume(const float wtd_change,
                                          const float center_wtd,
                                          const float neighbour_wtd,
                                          const int x1,
                                          const int y1,
                                          const int x2,
                                          const int y2,
                                          const ArrayPack &arp){


    double volume_change = 0.;

    // We have the change in the water table between the two cells.
    // We will use this to calculate the volume that moves between the two,
    // based on the average porosity and average water table depths
    // of the two cells. 
    // This volume can then be used to calculate the actual 
    // change in water table height in each of the two cells. 

    float mean_wtd = (center_wtd + neighbour_wtd) / 2.;
    float mean_porosity = (arp.porosity(x1,y1) + arp.porosity(x2,y2)) / 2.;
    float mean_fdepth = (arp.fdepth(x1,y1) + arp.fdepth(x2,y2)) / 2.;
    float mean_area = (arp.cell_area[y1] + arp.cell_area[y2]) / 2.;

    float upper_edge = mean_wtd + wtd_change/2.;
    float lower_edge = mean_wtd - wtd_change/2.;
    //since I am using averages between the two cells, I have to take
    //water both moving up or moving down into account. 
    //So, I treat the average wtd as the middle of where water is
    //with respect to changing porosity. 

  
    // First, we check to see if the mean wtd was above the surface.

    // If wtd stays above 0 once the change has occurred, then no need to worry
    // about porosity.
    if(lower_edge > 0){
        // The volume change is just the height change multiplied by the cell's
        // area.
        volume_change = wtd_change * mean_area;
        // The water table drops below the surface during this iteration, so we
        // need to consider porosity for part of the water.
    }
    else if(lower_edge < 0 && upper_edge > 0){
    // There is a portion of the water that is above the land surface.
        volume_change = upper_edge * mean_area;
        // The portion that is below the land surface.
        // -= because this comes out as a negative number.
        volume_change -= mean_area * mean_porosity * mean_fdepth * \
                            ( exp( (lower_edge) / mean_fdepth ) \
                              - 1);
        
    }
    // the water table is below the surface to start with, therefore it is
    // below the surface the whole time and we need porosity for all the change.
    else{
        volume_change = -mean_area * mean_porosity * mean_fdepth * \
                          ( exp(lower_edge / mean_fdepth) - \
                            exp(upper_edge / mean_fdepth) );
        }

    // So now we have the volume change that occurs in each of the two cells
    // as a positive value  whether it was all above ground, all below ground, 
    // or a combination.
   
   return volume_change;
}



double FanDarcyGroundwater::computeNewWTD(const float volume,
                                          const float my_wtd,
                                          const int x,
                                          const int y,
                                          const int direction, 
                                          const ArrayPack &arp){

    //We convert the known change in water volume to a change
	//change in water table depth for this cell. 

    //We have a volume, which is positive; we also need to know
    //if this cell is gaining or losing water. The 'direction' 
    //tells us this. If 'direction' is 1, it is gaining water, 
    //if 'direction' is 0, it is losing water. 


    double change_in_wtd = 0.;

    if(my_wtd > 0){
    	//at least some of the change is above the surface. 
    	change_in_wtd = volume / arp.cell_area[y];

    	if(direction == 0){
    		change_in_wtd = -volume / arp.cell_area[y];

    	    if(my_wtd + change_in_wtd < 0){
    	    	//how much of the volume is used up above the surface?
    	    	double SW_portion = my_wtd * arp.cell_area[y];
    	    	//this is the volume used in water above the surface. 
    	    	//The total volume minus this is left over to 
    	    	//decrease groundwater.
    	    	change_in_wtd = -arp.fdepth(x,y) * \
    	    	            log( exp(my_wtd / arp.fdepth(x,y)) \
    	    	            + (volume - SW_portion) / (arp.cell_area[y] \
    	    	            	* arp.porosity(x,y) \
    	    	            *arp.fdepth(x,y)) ) + my_wtd;
    
        	    //the cell is losing water, and it's losing enough
        	    //that some of the change is below the surface and
    	    	//so we need to take porosity into account. 
    	    }
        }
    }
    else{
    	//either all of the change is below the surface, 
    	//or it's a combination of above and below. 
    	//we start off assuming that it will all be below the surface:
    	if(direction == 1){
    		change_in_wtd = -arp.fdepth(x,y) * \
    		            log( exp(my_wtd / arp.fdepth(x,y)) \
    		            - volume / (arp.cell_area[y] * arp.porosity(x,y) \
    		            *arp.fdepth(x,y)) ) + my_wtd;

        	if((my_wtd + change_in_wtd > 0) || \
        	exp(my_wtd / arp.fdepth(x,y)) < \
        	(volume/(arp.cell_area[y] * arp.porosity(x,y)*arp.fdepth(x,y)))  ){ 
        	    //it is gaining enough water
        		//that some will be above the surface. 
        		//we want to calculate how much of the water is used up 
        		//in the ground, i.e. the portion between the starting
        		//wtd and 0. 
        		double GW_portion = -arp.cell_area[y] \
                                    * arp.porosity(x,y) \
                                    * arp.fdepth(x,y) \
                                    * ( exp( my_wtd /arp.fdepth(x,y) )
                                    - 1);
              //this is the volume of water used up in filling in the ground.
             //the volume minus GW_portion is left over to fill surface water.
                change_in_wtd = ( (volume - GW_portion)
                                      / arp.cell_area[y] ) \
                                      - my_wtd;  //-my_wtd because this
                                       //was a negative number and we are
                                      //getting the total change in wtd.
        	}
        }
        else{  //it is losing water, so it is definitely all below the surface. 
        	change_in_wtd = -arp.fdepth(x,y) * \
        	            log( exp(my_wtd / arp.fdepth(x,y)) \
    		            + volume / (arp.cell_area[y] * arp.porosity(x,y) \
    		            *arp.fdepth(x,y)) ) + my_wtd;
        }
    }

    if(direction == 1)
	    assert (change_in_wtd>-1e-4);
    if(direction == 0)
	    assert(change_in_wtd<1e-4);

    return change_in_wtd;
}



void FanDarcyGroundwater::computeWTDchangeAtCell( Parameters &params,
                                                  ArrayPack &arp,
                                                  int32_t x, int32_t y,
                                                  double dt){
    // Update WTD change

    // We do this instead of using a staggered grid to approx. double CPU time
    // in exchange for using less memory.

    // First, compute elevation head at center cell and all neighbours
    // This equals topography plus the water table depth
    // (positive upwards; negative if water table is below Earth surface)
    double headCenter = arp.topo(x,y) + wtdCenter;
    double headN      = arp.topo(x,y+1) + wtdN;
    double headS      = arp.topo(x,y-1) + wtdS;
    double headW      = arp.topo(x-1,y) + wtdW;
    double headE      = arp.topo(x+1,y) + wtdE;

    double mycell_change = 0.;

    // Then, compute the discharges
    double QN = transmissivityN * (headN - headCenter) \
                    / params.cellsize_n_s_metres \
                    * arp.cellsize_e_w_metres[y];
    double QS = transmissivityS * (headS - headCenter) \
                    / params.cellsize_n_s_metres \
                    * arp.cellsize_e_w_metres[y];
    double QE = transmissivityE * (headE - headCenter) \
                    / arp.cellsize_e_w_metres[y] \
                    * params.cellsize_n_s_metres;
    double QW = transmissivityW * (headW - headCenter) \
                    / arp.cellsize_e_w_metres[y] \
                    * params.cellsize_n_s_metres;

    // Update water-table depth, but only in the "internal" variables to
    // handle internal time stepping to maintain stability.
    // dH = sum(discharges) times time step, divided by cell area,
    //      divided by porosity.
    double wtd_change_N = QN * dt / ( arp.cell_area[y+1]);
    double wtd_change_S = QS * dt / ( arp.cell_area[y-1]);
    double wtd_change_W = QW * dt / ( arp.cell_area[y] );
    double wtd_change_E = QE * dt / ( arp.cell_area[y] );

   //We use the calculated wtd change fluxes to calculate a volume 
    //change between the 2 cells. This is based on average
    //porosity, e-folding depth, wtd, and cell area of the 2 cells. 

    double volume_N = calculateWaterVolume(fabs(wtd_change_N), 
    	 wtdCenter, wtdN, x, y, x, y+1, arp);

    double volume_S = calculateWaterVolume(fabs(wtd_change_S), 
    	 wtdCenter, wtdS, x, y, x, y-1, arp);

    double volume_E = calculateWaterVolume(fabs(wtd_change_E), 
    	 wtdCenter, wtdE, x, y, x+1, y, arp);

    double volume_W = calculateWaterVolume(fabs(wtd_change_W), 
    	 wtdCenter, wtdW, x, y, x-1, y, arp);

  //we now use these volumes to compute the actual changes in 
    //water table depths in the target cell and each of the neighbouring cells:

    
    // Using the wtd_changes from above, we need to calculate how much change
    // will occur in the target cell, accounting for porosity.

    // The current cell is receiving water from the North, so (x,y+1) is the
    // giving cell.
    // Target cell is the receiving cell.
    if(wtd_change_N > 0){
        wtdN           += computeNewWTD( volume_N, wtdN,      x, y+1, 0, arp);
        mycell_change  += computeNewWTD( volume_N, wtdCenter, x, y,   1, arp);
    }
    // The current cell is giving water to the North.
    // The North is the receiving cell.
    else{
        wtdN           += computeNewWTD( volume_N, wtdN,      x, y+1, 1, arp);
        mycell_change  += computeNewWTD( volume_N, wtdCenter, x, y,   0, arp);
    }
    

    if(wtd_change_S > 0){
         wtdS          += computeNewWTD( volume_S, wtdS,      x, y-1, 0, arp);
         mycell_change += computeNewWTD( volume_S, wtdCenter, x, y,   1, arp);
     }
    else {
        wtdS           += computeNewWTD( volume_S, wtdS,      x, y-1, 1, arp);
        mycell_change  += computeNewWTD( volume_S, wtdCenter, x, y,   0, arp);
    }
    

    if(wtd_change_E > 0){
        wtdE           += computeNewWTD( volume_E, wtdE,      x+1, y, 0, arp);
        mycell_change  += computeNewWTD( volume_E, wtdCenter, x,   y, 1, arp);
     }
    else {
        wtdE           += computeNewWTD( volume_E, wtdE,      x+1, y, 1, arp);
        mycell_change  += computeNewWTD( volume_E, wtdCenter, x,   y, 0, arp);
     }
    

    if(wtd_change_W > 0){
        wtdW           += computeNewWTD( volume_W, wtdW,      x-1, y, 0, arp);
        mycell_change  += computeNewWTD( volume_W, wtdCenter, x,   y, 1, arp);
     }
    else{
        wtdW           += computeNewWTD( volume_W, wtdW,      x-1, y, 1, arp);
        mycell_change  += computeNewWTD( volume_W, wtdCenter, x,   y, 0, arp);
     }
    
    // Now we have the height changes that will take place in the target cell
    // and each of the four neighbours.
    wtdCenter += mycell_change;
}



void FanDarcyGroundwater::updateCell( Parameters &params, ArrayPack &arp,
                                      uint32_t x, uint32_t y ){

    using namespace std::this_thread;     // sleep_for, sleep_until
    using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

    // Runs functions to compute time steps and update WTD for the center cell
    // and its neighbours until the outer time step has been completed

    // Initialize variables for dynamic time stepping
    double time_remaining = params.deltat;
    double dt_inner;

    // Initial water-table depths, prior to updating
    double wtdCenter_initial = arp.wtd(x,y);
    wtdCenter = arp.wtd(x,y);

    wtdN      = arp.wtd(x,y+1);
    wtdS      = arp.wtd(x,y-1);
    wtdE      = arp.wtd(x+1,y);
    wtdW      = arp.wtd(x-1,y);

    // Update water-table depths using dynamic time stepping
    while (time_remaining > 0){

        // Currently transmissivity is based on wtd, which does not change
        // during the while loop.
        computeNeighborTransmissivity(arp, x, y);
        //should we change it during the while loop? If not, we can move it out.
        double max_stable_time_step = computeMaxStableTimeStep( params, arp,
                                                                x, y);
        // Choose the inner-loop time step
        if(time_remaining <= max_stable_time_step){
            dt_inner = time_remaining;
        }
        else{
            dt_inner = max_stable_time_step;
        }

        computeWTDchangeAtCell(params, arp, x, y, dt_inner);
        time_remaining -= dt_inner;
    }

    // When exiting loop, the wtdCenter variable holds the final
    // water-table depth
    // This subtraction is unnecessary; could just give the new total instead
    arp.wtd_change_total(x,y) = wtdCenter - wtdCenter_initial;
}

/////////////////
// CONSTRUCTOR //
/////////////////

FanDarcyGroundwater::FanDarcyGroundwater(){
}

//////////////////////
// PUBLIC FUNCTIONS //
//////////////////////

void FanDarcyGroundwater::initialize(){

}

void FanDarcyGroundwater::update(Parameters &params, ArrayPack &arp){
    //cout << "Output sentence\n"; // prints Output sentence on screen
    //cout << params.ncells_x; // prints Output sentence on screen
    //cout << "\n";
    //cout << params.ncells_y; // prints Output sentence on screen
    //cout << "\n";
    // Updates water-table depth grid over one time step
    for(int32_t y=1; y<params.ncells_y-1; y++){
        for(int32_t x=1; x<params.ncells_x-1; x++){
            // Skip ocean cells
            if(arp.land_mask(x,y) == 0){
                continue;
            }
            // Otherwise, update the water-table depth change array
            updateCell( params, arp, x, y );
        }
    }
    // Once all the changes are known, update the WTD everywhere with the
    // difference array

    for(int32_t y=1; y<params.ncells_y-1; y++){
        for(int32_t x=1; x<params.ncells_x-1; x++){
            // Skip ocean cells
            if(arp.land_mask(x,y) == 0){
                continue;
            }
            // Update the whole wtd array at once.
            // This is the new water table after groundwater has moved
            // for delta_t seconds.
            arp.wtd(x,y) += arp.wtd_change_total(x,y);
        }
    }
}

// This can be populated if we intend to run this module on its own.
// Otherwise, will not be called
void FanDarcyGroundwater::run(){

}

// This can include functions to clean up / clear memory, if needed
void FanDarcyGroundwater::finalize(){

}
