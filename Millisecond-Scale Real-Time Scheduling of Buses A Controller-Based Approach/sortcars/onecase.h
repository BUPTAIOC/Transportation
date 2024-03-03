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
        // Initial scheduling task loop.
        onecase(string line,
                vector<Timepoint>& inTimeTable,
                Constrains inyueshu,
                Swarm& inS
        )
        {
                line_name = line;
                TimeTable = inTimeTable;
                constrains = inyueshu;
                S = inS;

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
        // Create a rescheduling scheduling environment.
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
        // Model: -2 Fixed parameter mode, limited number of vehicles to MaxCarNum, simulate vehicle failures, allow uncovered
        // Model: -1 Fixed parameter mode, limited number of vehicles to MaxCarNum, simulate random congestion, allow uncovered
        // Model: 0 Fixed parameter mode, parameters use optimized parameters, and remain fixed thereafter
        // Model: 1 Continuing optimization mode, parameters use optimized parameters, and continue optimization thereafter
        // Model: 2 Re-optimization mode, parameters use random parameters, and continue optimization thereafter
        void resort_oldcase(int mod, int MaxCarNum)
        {
                vector<int> empint;
               
                TimeTable_make_link();               

                Scheme scheduleempty(TimeTable, constrains, -1);//初始化的空排班方案
                scheme_result_info result;//单次排班
                switch (mod)
                {
                case -2://modle：-2 固定参数模式，模拟车辆故障，允许未覆盖
                    schedules.push_back(scheduleempty); schedules.back().clear();
                    schedules.back().constrains.change_Particle(S.sw_bestP());//将粒子群参数更新到约束结构内
                    schedules.back().constrains.max_carnum = MaxCarNum;//最大车辆约束
                    result = schedules.back().Schedule_parks(initScheme);//单次排班
                    result.scorer();//回写目标函数值
                    break;

                case -1://modle：-1 固定参数模式，车辆数受限为原方案，允许未覆盖
                    schedules.push_back(scheduleempty); schedules.back().clear();
                    schedules.back().constrains.change_Particle(S.sw_bestP());//将粒子群参数更新到约束结构内
                    schedules.back().constrains.max_carnum = MaxCarNum;//最大车辆约束
                    result = schedules.back().Schedule_parks(initScheme);//单次排班
                    result.scorer();//回写目标函数值
                    break;

                case 0://modle：0 固定参数模式，参数使用优化后的参数，且之后固定。
                        schedules.push_back(scheduleempty); schedules.back().clear();
                        schedules.back().constrains.change_Particle(S.sw_bestP());//将粒子群参数更新到约束结构内
                        result = schedules.back().Schedule_parks(initScheme);//单次排班
                        result.scorer();//回写目标函数值
                        break;


                case 1://modle：1 继续优化模式，参数使用优化后的参数，且之后继续优化。                      
                        S.init_swa(S);
                        for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////排班主循环
                        {
                                for (int j = 0; j < S.grs.size(); j++)
                                {
                                        schedules.push_back(scheduleempty); schedules.back().clear(); schedules.back().ScheduleID = j;
                                        schedules.back().constrains.change_Particle(&S.grs[j].ps[i]);//将粒子群参数更新到约束结构内
                                        result = schedules.back().Schedule_parks(initScheme);//单次排班
                                        S.grs[j].ps[i].punish = result.scorer();//回写目标函数值                                       
                                }
                                S.get_next_ps(i);
                                printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f \n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                        }
                        //print_to_excel(line_name + "/ofline_test_mod1", S, schedules);//把粒子轨迹输入文件
                        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        break;


                case 2://modle：2 重新优化模式，参数使用随机参数，且之后继续优化。
                        S.init_swa();
                        for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////排班主循环
                        {
                                for (int j = 0; j < S.grs.size(); j++)
                                {
                                        schedules.push_back(scheduleempty); schedules.back().clear(); schedules.back().ScheduleID = j;
                                        schedules.back().constrains.change_Particle(&S.grs[j].ps[i]);//将粒子群参数更新到约束结构内
                                        result = schedules.back().Schedule_parks(initScheme);//单次排班
                                        S.grs[j].ps[i].punish = result.scorer();//回写目标函数值
                                }
                                S.get_next_ps(i);
                                printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f\n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                        }
                        //print_to_excel(line_name + "/ofline_test_mod2", S, schedules);//把粒子轨迹输入文件
                        break;

                default:
                        break;
                }

                /////////////////////////////////////////////////////分数排名部分
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
                print_scheme_tofile(line_name + "/best_RE_scheme.txt", schedules[schedule_i_ofscoreth[0]]);//打印重调度的最优排班
                //print_result_sts(line_name + "/RE_result.csv", schedules[schedulei_ofscoreth[0]]);//打印统计数据
                cout << endl;
        }
};