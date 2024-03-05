#pragma once
#include "schedule.h"
#include <fstream>

// A collection of scheduling solutions obtained according to a 
// certain optimization method under certain customer requirements. 
// The collection has certain properties, such as a minimum number of vehicles.
class onecase
{
public:
        string line_name;
        Constrains constrains;
        vector<Timepoint> TimeTable;
        Swarm S;
        Scheme initScheme;// If there is inheritance in the scheme, store the scheme examples here.
        vector<Scheme> schedules;

        // The ranking of the score of schedul_i
        vector<int> scoreth_ofschedulei;
        // Which schedule corresponds to the ith rank
        vector<int> schedule_i_ofscoreth;
        // The ranking of the score of broken schedule_i
        vector<int> scoreth_of_broken_schedulei;
        // Which schedule corresponds to the ith rank of broken schedules
        vector<int> schedulei_of_broken_scoreth;


        /////////////////////////////////////////////////////
        void TimeTable_make_link()
        {
            // Find the first park after the car arrives at the station, i.e., the park that occurs after arrivetime.
            // If the car arrives at the departure slot, do not return this slot.
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        TimeTable[i].nextrip_firstParkX = -1;

                        int arrivetime = TimeTable[i].time + TimeTable[i].triptime + constrains.atlist_one_rest;

                        for (int j = i + 1; j < TimeTable.size(); j++)
                        {
                                if (TimeTable[j].time > arrivetime &&
                                        TimeTable[j].start_CP == TimeTable[i].gotoCP)
                                {
                                        TimeTable[i].nextrip_firstParkX = j;
                                        break;
                                }
                        }
                }
                // The next slotX after the same CP.
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        TimeTable[i].Nextrest_pointX = -1;

                        for (int j = i + 1; j < TimeTable.size(); j++)
                        {
                                if (TimeTable[i].start_CP == TimeTable[j].start_CP)
                                {
                                        TimeTable[i].Nextrest_pointX = j;
                                        break;
                                }
                        }
                }
        }

        //print the best scheduling scheme to fname.csv file
        void print_scheme_tofile(string fname, Scheme scheme)
        {
            ofstream outcomeFile;//output file
            outcomeFile.open(fname, ios::out | ios::trunc);
            outcomeFile << "car_number" << "," 
                << "work_time" << ","
                << "start_CP" << ","
                << "trips_count" << ","
                << "trips_start_time" << ","
                << endl;
            for (int car_number = 0; car_number < scheme.tripque.size(); car_number++)
            {
                outcomeFile << car_number + 1;//car_number
                outcomeFile << ",";
                outcomeFile << scheme.tripque[car_number].back()->tp.time - scheme.tripque[car_number][0]->tp.time;//work_time
                outcomeFile << ",";
                outcomeFile << scheme.tripque[car_number][0]->tp.start_CP;//vehicle start_CP
                outcomeFile << ",";
                outcomeFile << scheme.tripque[car_number].size();//the number of vehicle trips
                outcomeFile << ",";
                for (int trip_number = 0; trip_number < scheme.tripque[car_number].size(); trip_number++)
                {
                    outcomeFile << scheme.tripque[car_number][trip_number]->tp.time;//start time of each vehicle trip
                    if(trip_number < scheme.tripque[car_number].size()-1)
                    {
                        outcomeFile << ",";
                    }
                }
                outcomeFile << endl;
            }
            outcomeFile.close();
        }
        /////////////////////////////////////////////////////
        //original scheduling for initial timetable. Creating a onecase object, and get result.
        onecase(Setting setting)
        {
                line_name = setting.line_name;
                TimeTable = setting.TimeTable;
                constrains = setting.constrains;
                S = setting.S;

                TimeTable_make_link();

                Scheme schedule(TimeTable,constrains,-1);// Initialized empty scheduling scheme.
                for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////main loop
                {
                        for (int j = 0; j < S.grs.size(); j++)
                        {
                                schedules.push_back(schedule); schedules.back().clear(); schedules.back().ScheduleID = j;
                                schedules.back().constrains.change_Particle(&S.grs[j].ps[i]);/// Update particle group parameters to the constraint structure.
                                S.grs[j].ps[i].punish = schedules.back().Schedule_parks(initScheme).punish;                             
                        }
                        S.get_next_ps(i);

                        printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f \n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////Scheme Score ranking section.
                for (int i = 0; i < schedules.size(); i++)
                {
                        int num = 0;
                        for (int j = 0; j < schedules.size(); j++)
                        {
                                if (schedules[i].result.punish > schedules[j].result.punish)
                                {
                                        num++;
                                }
                        }
                        scoreth_ofschedulei.push_back(num);
                }
                ///////////////////////////////////////////////////////
                for (int i = 0; i < schedules.size(); i++)
                {
                        for (int j = 0; j < schedules.size(); j++)
                        {
                                if (scoreth_ofschedulei[j] == i)
                                {
                                        schedule_i_ofscoreth.push_back(j);
                                }
                        }
                        //cout << i << " th:car " << schedulei_ofscoreth[i] << endl;
                }
                print_scheme_tofile(line_name + "/best_scheme.csv", schedules[schedule_i_ofscoreth[0]]);// Print the optimal schedule.
                cout << endl;
        }
        // Create a rescheduling scheduling environment, and get reschedul result using function "onecase.rescheduling_oldcase(int mod, int MaxCarNum)" below.
        // The input "oldcase" is the original "onecase" object created created uper.
        onecase(onecase& oldcase)
        {
                line_name = oldcase.line_name;
                constrains = oldcase.constrains;
                TimeTable = oldcase.TimeTable;
                S = oldcase.S;
                ///////////////////////////////////////////////////////////////////////
                // Initialized empty scheduling scheme, inheriting the last optimal scheme from the previous scheduling.
                initScheme = oldcase.schedules[oldcase.schedule_i_ofscoreth[0]];
        }

        // Randomly fluctuate the current trip duration up and down by r%.
        void change_triptime_simprand_triptime(float r)
        {
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        int radis = int(r * TimeTable[i].triptime);
                        int rplus = rand() % (2*radis + 1);

                        TimeTable[i].triptime += rplus - radis;
                }
        }

        // Update the duration of the blocked trip. Increase the trip by minutes for rescheduling.
        void change_triptime_blocked(double breakbeginH, double breaklenH, int triplusM)
        {
            int break_t = -1;
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        if (TimeTable[i].time > 60 * breakbeginH && TimeTable[i].time < 60 * (breakbeginH + breaklenH))
                        {
                                TimeTable[i].triptime += triplusM;
                                if(break_t == -1)break_t = TimeTable[i].time;
                        }
                }
                constrains.breaktime = break_t;
        }

        // Update the start time and duration of vehicle breakdown. 
        // Achieve this by extending the trip time of individual time points.
        // The first timepoint after failbeginH is considered the point of vehicle failure.
        // the trip add faillenM minutes. only this trip is influenced. "len"is useless
        void change_triptime_vehicle_failure(double failbeginH, double len, double faillenM)
        {
            int break_t = -1;
            for (int i = 0; i < TimeTable.size(); i++)
            {
                // The first time point after the fail time is considered the point of vehicle failure.
                if (TimeTable[i].time > 60 * failbeginH)
                {
                    TimeTable[i].triptime += faillenM;
                    if (break_t == -1)break_t = TimeTable[i].time;
                    break;
                }
            }
            constrains.breaktime = break_t;
        }

        // Generate random trip durations (based on the actual range of travel times per hour on the roads)
        //  to simulate the road conditions of another day.
        void change_triptime_realrand_triptime()
        {
                //used by mod3
                vector<int> max_triptime; max_triptime.resize(24, -9999);
                vector<int> avg_triptime; avg_triptime.resize(24, 0);
                vector<int> count; count.resize(24, 0);
                vector<int> min_triptime; min_triptime.resize(24, 99999);
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        int time = TimeTable[i].time;
                        int hour = time / 60;
                        avg_triptime[hour] += TimeTable[i].triptime;
                        count[hour]++;
                        if (max_triptime[hour] < TimeTable[i].triptime)
                        {
                                max_triptime[hour] = TimeTable[i].triptime;
                        }
                        if (min_triptime[hour] > TimeTable[i].triptime)
                        {
                                min_triptime[hour] = TimeTable[i].triptime;
                        }
                }
                for (int i = 0; i < avg_triptime.size(); i++)
                {
                        if (count[i] > 0)avg_triptime[i] /= count[i];
                }

                for (int i = 0; i < TimeTable.size(); i++)
                {
                        int time = TimeTable[i].time;
                        int hour = time / 60;
                        TimeTable[i].triptime = min_triptime[hour] + rand() % (1 + max_triptime[hour] - min_triptime[hour]);
                }
        }
        //rescheduling function.
        // Model: -2 Fixed parameters like mod 0, limited number of vehicles to MaxCarNum, simulate vehicle failures, allow uncovered timepoint
        // Model: -1 Fixed parameters like mod 0, limited number of vehicles to MaxCarNum, simulate random congestion, allow uncovered timepoint
        // Model: 0 Fixed parameters(use optimized parameters), and remain fixed thereafter
        // Model: 1 Continuing optimization mode, parameters use optimized parameters, and continue optimization thereafter
        // Model: 2 Re-optimization mode, parameters use random parameters, and continue optimization thereafter
        void rescheduling_oldcase(int mod, int MaxCarNum)
        {
                vector<int> empint;
               
                TimeTable_make_link();               

                Scheme scheduleempty(TimeTable, constrains, -1); // Initialize an empty scheduling scheme with Scheme schedule
                scheme_result_info result;//one schedule result
                switch (mod)
                {
                case -2: // Mode: -2 Fixed parameter mode, simulate vehicle malfunction, allow uncovered scenarios
                        schedules.push_back(scheduleempty); // Add an empty schedule
                        schedules.back().clear(); // Clear the last schedule in the list
                        schedules.back().constrains.change_Particle(S.sw_bestP()); // Update particle swarm parameters into the constraints structure
                        schedules.back().constrains.max_carnum = MaxCarNum; // Set the maximum vehicle constraint
                        result = schedules.back().Schedule_parks(initScheme); // Perform single scheduling
                        result.scorer(); // Calculate and update the objective function value
                        break;

                case -1: // Mode: -1 Fixed parameter mode, the number of vehicles is limited to the original plan, allowing uncovered scenarios
                        schedules.push_back(scheduleempty); // Add an empty schedule
                        schedules.back().clear(); // Clear the last schedule in the list
                        schedules.back().constrains.change_Particle(S.sw_bestP()); // Update particle swarm parameters into the constraints structure
                        schedules.back().constrains.max_carnum = MaxCarNum; // Set the maximum vehicle constraint, alow uncovered timepoint
                        result = schedules.back().Schedule_parks(initScheme); // Perform single scheduling
                        result.scorer(); // Calculate and update the objective function value
                        break;

                case 0: // Mode: 0 Fixed parameter mode, use fix optimized parameters.
                        schedules.push_back(scheduleempty); // Add an empty schedule
                        schedules.back().clear(); // Clear the last schedule in the list
                        schedules.back().constrains.change_Particle(S.sw_bestP()); // Update particle swarm parameters into the constraints structure
                        result = schedules.back().Schedule_parks(initScheme); // Perform single scheduling
                        result.scorer(); // Calculate and update the objective function value
                        break;



                case 1: // Mode: 1 Continue optimization mode, use optimized parameters, and continue to optimize.
                        S.init_swa(S);
                        for (int i = 0; i < S.Max_epoch; i++) // Main scheduling loop
                        {
                                for (int j = 0; j < S.grs.size(); j++)
                                {
                                        schedules.push_back(scheduleempty); // Add an empty schedule
                                        schedules.back().clear(); // Clear the last schedule in the list
                                        schedules.back().ScheduleID = j;
                                        schedules.back().constrains.change_Particle(&S.grs[j].ps[i]); // Update particle swarm parameters into the constraints structure
                                        result = schedules.back().Schedule_parks(initScheme); // Perform single scheduling
                                        S.grs[j].ps[i].punish = result.scorer(); // Update the objective function value
                                }
                                S.get_next_ps(i); // Get next particle state
                                printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f \n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                        }
                        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        break;


                case 2: // Mode: 2 Re-optimization mode, parameters are initialized with random values, and further optimization continues afterwards.
                        S.init_swa();
                        for (int i = 0; i < S.Max_epoch; i++)////////////////////Scheduling Main Loop.
                        {
                                for (int j = 0; j < S.grs.size(); j++)
                                {
                                        schedules.push_back(scheduleempty); schedules.back().clear(); schedules.back().ScheduleID = j;
                                        schedules.back().constrains.change_Particle(&S.grs[j].ps[i]); // Update particle swarm parameters into the constraint structure
                                        result = schedules.back().Schedule_parks(initScheme); // Schedule parks for a single shift
                                        S.grs[j].ps[i].punish = result.scorer(); // Write back the objective function value
                                }
                                S.get_next_ps(i);
                                printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f\n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                        }
                        break;

                default:
                        break;
                }

                /// Scheduling schemes ranking by score.
                for (int i = 0; i < schedules.size(); i++)
                {
                        int num = 0;
                        for (int j = 0; j < schedules.size(); j++)
                        {
                                if (schedules[i].result.punish > schedules[j].result.punish)
                                {
                                        num++;
                                }
                        }
                        scoreth_ofschedulei.push_back(num);
                }
                ///////////////////////////////////////////////////////
                for (int i = 0; i < schedules.size(); i++)
                {
                        for (int j = 0; j < schedules.size(); j++)
                        {
                                if (scoreth_ofschedulei[j] == i)
                                {
                                        schedule_i_ofscoreth.push_back(j);
                                }
                        }
                }
                print_scheme_tofile(line_name + "/best_reschedulingMOD" + to_string(mod) + "_scheme.csv", schedules[schedule_i_ofscoreth[0]]);
                cout << endl;
        }
};