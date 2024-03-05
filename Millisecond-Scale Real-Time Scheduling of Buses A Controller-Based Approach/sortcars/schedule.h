#pragma once
#include "park.h"

#define DELETE_OR_SLEEP_working_car if (car->cannot_convert_to_long_vehicle(triptime)){park.Cars.erase(park.Cars.begin() + i);i--;}else{car->sleep_to_long(oldtime);}

// If backtracking is needed, it is only an internal matter of this schedule and does not affect other schedules.
class Scheme
{
public:
        int ScheduleID;
        // Departure point garage, used to obtain real-time garage information.
        vector<Park> parks;
        // The end time of the last journey of the vehicle. It may not be the last bus, but an earlier vehicle that departs later and arrives earlier.
        int endtime_ofday;
        // Vehicle scheduling results, the index represents the vehicle id, which trip, and the element represents which Slot this trip departs from.
        // car[].trip[]->(slot)
        vector<vector<Park*>> tripque;
        // The number of Slots that have been scheduled.
        int sorted;

        Constrains constrains; // After initialization, it will not change within this class.
        scheme_result_info result;

        // Constructor, establishes connections between time points, 
        // mainly the garage links of adjacent time points within the same CP.
        Scheme(vector<Timepoint>& TimeTable, Constrains& inyueshu, int S_id)
        {
                ScheduleID = S_id;

                sorted = 0;

                endtime_ofday = 0;

                constrains = inyueshu;
                result = scheme_result_info();

                //initial parks
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        parks.push_back(Park(TimeTable[i]));
                        if (TimeTable[i].time + TimeTable[i].triptime > endtime_ofday)
                        {
                                endtime_ofday = TimeTable[i].time + TimeTable[i].triptime;
                        }
                }
        }    

        Scheme()
        {

        }

        void clear()
        {
                sorted = 0;
                //initial slos
                for (int i = 0; i < parks.size(); i++)
                {
                        parks[i].clear();
                }
                tripque.clear();
        }

        int VehicleSelector(Park& park, int endtime_ofday)
        {               
            // Considered as a vehicle participating in the weight calculation.
            // The comprehensive score of all attributes of this vehicle is calculated into the total score.
            // Even if there are vehicles in the garage, a new vehicle may still be dispatched.
            int newcar_score = 0;
            int weak_i = 0; // Count the index set of awake vehicles.
            int sleep_type1 = 0; // Count the index set of sleeping vehicles of type 1.
            int sleep_type0 = 0; // Count the index set of sleeping vehicles of type 0.

                for (int i = 0; i < park.Cars.size(); i++)
                {
                        if (park.Cars[i].state == 0)
                        {
                                weak_i++;
                        }
                        else
                        {
                                if (park.Cars[i].type == 1)
                                {
                                        sleep_type1++;
                                }
                                else if (park.Cars[i].type == 0)
                                {
                                        sleep_type0++;
                                }
                        }
                }
                /////////////////////////////////////////////////////////////////////////
                //total score
                vector<double>scores;
                vector<int> weight;
                int weight_tot = 0;
                // Below are the single-step evaluation criteria. They do not represent the final evaluation
                // and are only used for the vehicle dispatch decision from the garage. 
                // First, calculate the scores for various attributes separately, and then add whichever you want to use later.
                /////////////////////////////////////////////////////////////////////////
                double sco = -1;
                double maxsco = 0.0;
                double minsco = 999999999.0;
                for (int i = 0; i < park.Cars.size(); i++)
                {
                        sco = park.Cars[i].simple_scorer(
                                endtime_ofday,
                                park.tp.time,park.tp.triptime,
                                weak_i,
                                sleep_type1,
                                sleep_type0,
                                constrains.Wplus_backhome,
                                constrains.P_used.ds[3].val(),
                                constrains.P_used.ds[4].val(),
                                constrains.P_used.ds[5].val());

                        scores.push_back(sco);
                        if (sco < minsco && sco > 0)
                        {
                                minsco = sco;
                        }
                        if (sco > maxsco)
                        {
                                maxsco = sco;
                        }
                }
                for (int i = 0; i < park.Cars.size(); i++)
                {
                        int intsco = int((100 * scores[i]) / minsco);
                        weight.push_back(intsco);
                        weight_tot += intsco;
                }
                if (weight_tot == 0)
                {
                        cout << "weight_tot == 0 error"<<endl;
                }
                /////////////////////////////////////////////////////////////////////////
                int chosen_i = 0;
                double choseni_weight = weight[chosen_i];
                int randnum = rand() % weight_tot;
                int numleft = randnum;
                switch (constrains.ifrand)
                {
                case 0:
                       // Strategy: Not random, choose the vehicle with the highest score to dispatch. 
                       // Will not dispatch new vehicles but will awaken existing ones.
                        for (int i = 0; i < park.Cars.size(); i++)
                        {
                                if (weight[i] > choseni_weight)
                                {
                                        choseni_weight = weight[i];
                                        chosen_i = i;
                                }
                        }
                        break;
                case 1:
                       // Strategy: Score-weighted random. The score itself is the weight. 
                       // Might dispatch new vehicles.
                        for (int i = 0; i < park.Cars.size(); i++)
                        {
                                numleft -= weight[i];
                                if (numleft <= 0)
                                {
                                        chosen_i = i;
                                        break;
                                }
                        }
                        if (numleft > 0)
                        {
                                chosen_i = -1;
                        }
                
                        break;
                default:
                        return 0;
                        break;
                }
                return chosen_i;
        }

        // Update garage information, clear finished vehicles.
        // Use selector to decide how to dispatch vehicles for a given slot: 
        // new vehicle? old vehicle? wake up a vehicle? which vehicle number?
        // Transfer vehicles, construct subsequent time point garage information.
        // copychose is used to copy the old selection before the breakpoint.
        // If backtracking is needed, return the target parkX; otherwise, return -1.
        int mover(Park& park,int copychose)
        {
                // Update the information of used vehicles.
                park.carsused = result.carsused;
                park.driverused = result.drivsused;
                ////////////////////////////////////////////////////////////////////////////
                // Find the slot on the opposite shore where the vehicle will arrive after departure.
                // The departure time of this trip.
                int oldtime = park.tp.time;
                // Duration of the trip.
                int triptime = park.tp.triptime;
                // Predict the arrival time of the vehicle's next trip.
                int next_arrivetime = oldtime + triptime;
                // Where the vehicle goes on its next trip.
                int next_TableX = park.tp.gotoCP;
                // The sloty of the next available slot after arrival. If there is no next slot, then -1.
                int nextrip_firstSlotY = park.tp.nextrip_firstParkX; // find_nextrip_firstSlotY4(TimeTable[next_TableX], next_arrivetime);
                // How long the selected vehicle rests before the next slot on the opposite shore arrives after completing this trip.
                int partrest; if (nextrip_firstSlotY >= 0) partrest = parks[nextrip_firstSlotY].tp.time - next_arrivetime;
                // Represents the position of the selected vehicle in the garage. 
                // -1 means dispatching a new vehicle, not using an old one from the garage.
                int chosen;
                int chosencarx;
                /////////////////////////////////////////////////////////////////////////////
                // Below is the cleaning of vehicles in the garage that cannot be used:
                // Duration exceeds 8, 6.5, 1.5 hours.
                // int atlistrest = 2; // The minimum number of minutes the driver must rest.
                // Clean up the vehicles in the garage that can no longer be used, or change drivers, let drivers rest. 
                // Also, start the sleep mode for peak-time vehicles.
                for (int i = 0; i < park.Cars.size(); i++)
                {
                        Car* car = &park.Cars[i];

                        switch (car->state)
                        {
                        case 0:
                            // State 0 means the car was just working. In the constraints below, 
                            // it may violate multiple constraints, such as maxtrips and worktime.
                            if (park.marked_to_sleep(car->carx))
                            {
                                // If this car is found to have a backtrack mark, it means it should 
                                // go off duty at this moment, instead of being selected for departure.
                                DELETE_OR_SLEEP_working_car
                            }
                            else if (car->break_maxtrip() || car->canot_fulltrip(triptime)) // Vehicles that normally go off duty, or vehicles that go off duty after an even number of trips.
                            {
                                DELETE_OR_SLEEP_working_car
                            }
                            else if (car->break_worktime(triptime) || // Exceeding worktime, may only exceed after resting for several slots.
                                car->break_drivetime(triptime)) // When triggering this condition, consider odd and even trip scenarios.
                            {// These vehicles definitely do not have a full number of trips. If the trip is even, just go off duty. If the trip is odd, backtrack to the last even trip and go off duty there.
                                 if (car->type != 1 && car->tripnum % 2 == 1)// car->type != 1
                                 {
                                    if (constrains.ifbacktrack == 1)// Backtrack mode
                                    {
                                        Park* backto_park = tripque[car->carx][car->tripnum - 1];
                                        if (backto_park->tp.time >= constrains.breaktime)// Backtrack now, otherwise, if we don't backtrack, we can't solve the problem, just continue scheduling.
                                        {
                                            backto_park->ShouldSleep_Cars.push_back(car->carx);
                                            return backto_park->tp.Number;// Should backtrack
                                        }
                                        else// Backtracking goes before the breakpoint, can't solve the problem with backtracking, just continue scheduling.
                                        {
                                            DELETE_OR_SLEEP_working_car
                                        }
                                    }
                                    else// Non-backtrack mode
                                    {
                                        DELETE_OR_SLEEP_working_car
                                    }
                                 }
                                 else// Generally, short-route vehicles may likely convert to long-route vehicles, no need to ensure an even number of trips.
                                 {
                                    DELETE_OR_SLEEP_working_car
                                 }
                            }
                            else if (car->type == 1 &&
                                car->resthis >= constrains.to_peakcar_restime &&
                                car->tripnum % 2 == 0)// Convert to a peak vehicle and enter sleep mode.
                            {
                                car->sleep_to_peak(oldtime);
                                constrains.peakcars_used++;
                            }
                            break;

                        case 1: // State 1 means the vehicle is sleeping.
                            if (park.marked_to_sleep(car->carx))
                            {// If this car is found to have a backtrack mark, it means it should go off duty at this moment, rather than being selected for departure.
                                park.Cars.erase(park.Cars.begin() + i); i--;
                            }
                            // After the first driver of the short-route bus finishes their shift, sleep causes the first driver's work time to be exhausted.
                            else if (car->type == 1 && car->break_worktime(triptime))
                            {
                                if (car->worktime() + 2 * triptime >= car->workmax * 2 ||
                                    car->drivetime + 2 * triptime >= car->drivemax * 2)
                                {
                                    park.Cars.erase(park.Cars.begin() + i); i--;
                                }
                                ////////////////////////////////////////////////////////////////////////// Situations that a new long bus will encounter.
                            }
                            break;

                        default:
                            cout << "car state error";
                            break;
                        }
                }
                /////////////////////////////////////// Whether to dispatch, dispatch a new vehicle, dispatch an old vehicle, which old vehicle
                if (copychose >= 0)
                {
                    chosen = copychose;
                }
                else if (constrains.if_min_newcar == 1 &&
                    result.carsused < constrains.min_newcar_num &&
                    park.tp.time >= constrains.latest_new_car_time)
                {// By the time there must be at least a certain number of vehicles dispatched, usually first turned off, can use line70, line85, line803.
                    chosen = -1;
                }
                else if (park.Cars.size() == 0)
                {
                    chosen = -1;
                }
                else if (park.Cars.size() == 1)
                {
                    chosen = 0;
                }
                else
                {// If there are old vehicles in the garage and the number of dispatched vehicles has met the requirement, consider whether to dispatch an old vehicle or wake one up.
                    chosen = VehicleSelector(park, endtime_ofday);
                }
                // During dynamic scheduling, the maximum number of vehicles can be limited in yueshu.max_carnum
                if (chosen == -1 && result.carsused >= constrains.max_carnum)
                {
                    chosen = -3;
                }
                if (chosen >= 0 && park.Cars.size() <= chosen)
                {
                    chosen = -3;
                }
                // By this point, chosen is determined
                //////////////////////////////////////////////////////////////////////////
                park.chosen = chosen;
                //////////////////////////////////////////////////////////////////////////
                if (chosen == -1)// Decide to dispatch a new vehicle.
                {
                        chosencarx = result.carsused;
                        result.carsused++;
                        result.drivsused++;
                        // Record the update of the new vehicle, update the slot node information.
                        park.ifnewcar = 1;
                        park.ifnewdriver = 1;
                        park.carx = chosencarx;
                        // This part updates the vehicle's scheduling queue.
                        vector<Park*> ca;
                        tripque.push_back(ca);
                        tripque[chosencarx].push_back(&park);

                        if (nextrip_firstSlotY >= 0) // The vehicle enters the garage on the opposite side.
                        {
                                parks[nextrip_firstSlotY].Cars.push_back(
                                        Car(chosencarx, 1, 0, park.tp.Number,
                                                constrains.max_drivetime, constrains.max_worktime, constrains.max_tripnum,
                                                triptime, partrest, 1, -1));
                        }
                }
                else if (chosen == -3)
                {
                        result.nocar_slotnum++;
                }
                else//use old car
                {
                        Car* chosencar = &park.Cars[chosen];
                        chosencarx = chosencar->carx;
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        // This section updates the vehicle's scheduling queue, tripque.
                        switch (chosencar->state)
                        {
                        case 0: // The selected vehicle is working
                                park.ifnewdriver = 0;
                                break;
                        case 1: // The selected vehicle is sleeping

                                switch (chosencar->type)
                                {
                                case 2:
                                        park.ifnewdriver = 1;
                                        if (chosencar->type == 1)// If the vehicle is sleeping and can still add new drivers, it becomes a long-route bus.                           
                                        {

                                        }
                                        break;
                                case 1:
                                        park.ifnewdriver = 1;
                                        if (chosencar->type == 1) // If the vehicle is sleeping and can still add new drivers,                              
                                        {

                                        }
                                        break;
                                case 0:
                                        park.ifnewdriver = 0;// Select and wake up the peak-time vehicle.
                                        break;
                                default:
                                        break;
                                }
                                break;
                        default:
                                break;
                        }
                        tripque[chosencarx].push_back(&park); // Record tripque update
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        // Record the update for a new vehicle, update slot node information
                        park.ifnewcar = 0;
                        park.carx = chosencarx;
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        // After arriving, if there are subsequent time points, the old vehicle enters the next slot's garage.
                        if (nextrip_firstSlotY >= 0)
                        {
                                Park* aimslo = &parks[nextrip_firstSlotY];
                                aimslo->Cars.push_back(*chosencar); // First drive the car over, then create new car initial information
                                Car* car = &aimslo->Cars.back();
                                if (chosencar->state == 1) // If a sleeping car was selected, wake it up at the next station (and add a new driver)
                                {
                                        // Early on, push the new driver resource into this slot's chosen car, the strategy will be asynchronous with the car's garage entry strategy. New car won't be pushed into this slot.
                                        // Thus, the chosen car's driver in this slot remains unchanged, only after the car and driver reach the next station, then a new driver is added, and the state is updated. In short, drive the car over first, then find a new driver and wake the car up.
                                        aimslo->Cars.back().weakup(park.tp.Number, 1, triptime, partrest);
                                }
                                else // If the selected car is working, then update directly
                                {
                                        aimslo->Cars.back().update_drive(park.tp.Number, 1, triptime, partrest);
                                }
                        }

                }
                ///////////////////////////////////////////////////////////////////////////////////////////////////
                // Move the remaining vehicles in the park to the next tp on this CP. Sleep peak vehicles do not count as rest, others do.
                if (park.tp.Nextrest_pointX >= 0)
                {
                        Park* next_restpark = & parks[park.tp.Nextrest_pointX];
                        int whole_rest = next_restpark->tp.time - park.tp.time;
                        for (int i = 0; i < park.Cars.size(); i++)
                        {
                                if (i != chosen)
                                {
                                        next_restpark->Cars.push_back(park.Cars[i]);
                                        Car* car = &next_restpark->Cars.back();

                                        switch (next_restpark-> Cars.back().state)
                                        {
                                        case 0: // For vehicles in the garage that are working, increase rest time
                                                car->update_rest(park.tp.Number, whole_rest);
                                                break;

                                        case 1: // For vehicles in the garage that are sleeping, do not add rest time for peak-time vehicles, add for others.

                                                if (car->type == 0)//peak car
                                                {

                                                }
                                                else if (car->type == 1)//sleep short car
                                                {
                                                        next_restpark->Cars.back().update_rest(park.tp.Number, whole_rest);
                                                }
                                                break;

                                        default:
                                                cout << "car state error";
                                                break;
                                        }
                                }
                        }
                }
                else
                {

                }
                sorted++;
                return -1;
        }

        // Backtrack the scheduling scene to before the dispatch at position X.
        // Reconstruct the scene, clear the backtrack records after X.
        void backtrack_rebuild(int toparkX)
        {
                vector<int> oldchoses;// Copy the original time point park
                for (int i = 0; i < parks.size(); i++)
                {
                        oldchoses.push_back(parks[i].chosen);
                        parks[i].clear();
                }               
                sorted = 0;

                tripque.clear();

                int backnum = result.backtrack_num;
                result = scheme_result_info();
                result.backtrack_num = backnum + 1;
                
                // Copy each original action
                // Backtrack to the moment just before the target input, not repeating the action of the target park.
                for (int i = 0; i <= toparkX - 1; i++)
                {
                    mover(parks[i], oldchoses[i]);
                }
                // Clear the backtrack records after X, because the environment has changed
                for (int i = toparkX + 1; i < parks.size(); i++)
                {
                    parks[i].ShouldSleep_Cars.clear();
                }
        }
        // Backtrack the scheduling scene to before the dispatch at position X. This does not handle updating the backtrack signal.
        // Delete the modifications that follow.
        void backtrack_delet(int toparkX)
        {
                
        }

        scheme_result_info Schedule_parks(Scheme& initScheme)
        {
                int backtrack_to_parkx = -1;// -1 indicates no backtracking, corresponding to the backtracking parkx.
                for (int pki = 0; pki < parks.size();)
                {
                        if (parks[pki].tp.time < constrains.breaktime)
                        {
                                backtrack_to_parkx = mover(parks[pki], initScheme.parks[pki].chosen);                               
                        }
                        else    
                        {
                                backtrack_to_parkx = mover(parks[pki], -1);
                        }         
                        // After adding the breakpoint mechanism, scheduling is only done from the breakpoint onwards. 
                        // Slots that coincide with breaktime also need to be rescheduled. 
                        if (backtrack_to_parkx >= 0)
                        {
                            // At this time, backtrack to park[shouldsleep_parkx], 
                            // and the vehicle dispatched should be changed to go to sleep or finish the shift.
                                backtrack_rebuild(backtrack_to_parkx);
                                pki = backtrack_to_parkx;
                        }
                        else
                        {
                                pki++;
                        }
                }
                /////////////////////////////////////////////////////////////////////////////////
                // After scheduling is completed, summarize the result parameters to determine the punishment value.
                for (int i = 0; i < tripque.size(); i++)//i is car number
                {
                        /////////////////////////////////// Scheduling results summary.
                        Park* last_trip = tripque[i].back();
                        int chosen = last_trip->chosen;
                        if (chosen >= 0)
                        {
                                Car* chocar = &last_trip->Cars[chosen];
                        }
                        if (tripque[i].size() % 2 == 1)
                        {
                                result.unback_drivernum++;
                        }
                        if (tripque[i].size() > constrains.max_tripnum)
                        {
                                result.drivsused++;
                        }
                        ////////////////////////////////////////////////////////
                }
                result.scorer();
                return result;
        }
};
