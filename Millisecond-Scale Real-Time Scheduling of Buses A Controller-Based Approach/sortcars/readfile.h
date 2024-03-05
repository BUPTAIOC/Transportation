#pragma once
#include "timepoint.h"
#include <fstream>
#include <sstream>
#include <vector>
using namespace std;


void make_link(vector<Timepoint> &TimeTable,int atlist_onerest)
{
        for (int i = 0; i < TimeTable.size(); i++)
        {
                for (int j = i + 1; j < TimeTable.size(); j++)
                {
                        if (TimeTable[i].time > TimeTable[j].time)
                        {
                                Timepoint ttp = TimeTable[i];
                                TimeTable[i] = TimeTable[j];
                                TimeTable[j] = ttp;
                        }
                }
        }

        // Find the first park after the car arrives at the station, i.e., the park that occurs after arrivetime.
        // If the car arrives at the departure time, do not return this timepoint.
        for (int i = 0; i < TimeTable.size(); i++)
        {
                TimeTable[i].nextrip_firstParkX = -1;

                int arrivetime = TimeTable[i].time + TimeTable[i].triptime + atlist_onerest;

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
        // The next timepointX after the same CP.
        vector<int>y; y.push_back(0); y.push_back(0);

        for (int i = 0; i < TimeTable.size(); i++)
        {
                TimeTable[i].Nextrest_pointX = -1;
                TimeTable[i].Number = i;
                TimeTable[i].SlotY = y[TimeTable[i].start_CP]; y[TimeTable[i].start_CP]++;

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

//Read timetable information into TimeTable.
int ReadFile_TimeTable_merged_Message(
        string line_name,
        vector<Timepoint>& TimeTable_merged,
        int& max_tripnum,
        int& min_restime,
        int& max_worktime,
        int& max_drivetime,
        int& swam_size,
        int& evaluation_times
)
{
        TimeTable_merged.clear();    

        char ch[64];
        FILE* file_tp; errno_t err_tp; int line_tp = 0;
        string file_head = line_name + "/TimeTable_merged.csv";
        if ((err_tp = fopen_s(&file_tp, file_head.c_str(), "r")) != 0)
        {
                cout << "CANNOT OPEN message FILE\n";     
                cin >> ch;
                exit(0);                    
        }
        else
        {
                while (!feof(file_tp))
                {
                        if (fgets(ch, 64, file_tp) != NULL)
                        {
                                string str = ch;
                                TimeTable_merged.push_back(Timepoint(str));
                                line_tp++;
                        }
                }
                fclose(file_tp);
        }

        double sum = 0;
        int count = 0;
        FILE* file3; errno_t err3; int line3 = 0;
        ///  ///   ////////////////////////////////////////////////////////////////////////////////////////////////////
        vector<int> avg_triptime_map;
        file_head = line_name + "/tmessage.txt";
        if ((err3 = fopen_s(&file3, file_head.c_str(), "r")) != 0)
        {
                cout << "CANNOT OPEN message FILE\n"; 
                cin >> ch;
                exit(0);                        
        }
        else
        {
                /*
                Maximum single driver trip limit: 
                8
                Minimum rest time: 
                3
                Maximum working time: 
                480
                Maximum driving time: 
                390
                Population size: 
                50
                Evolution rounds: 
                20
                */

                string str;
                fgets(ch, 64, file3);
                fgets(ch, 64, file3);
                str = ch;
                max_tripnum = std::stoi(str);

                fgets(ch, 64, file3);
                fgets(ch, 64, file3);
                str = ch;
                min_restime = std::stoi(str);

                fgets(ch, 64, file3);
                fgets(ch, 64, file3);
                str = ch;
                max_worktime = std::stoi(str);

                fgets(ch, 64, file3);
                fgets(ch, 64, file3);
                str = ch;
                max_drivetime = std::stoi(str);

                fgets(ch, 64, file3);
                fgets(ch, 64, file3);
                str = ch;
                swam_size = std::stoi(str);

                fgets(ch, 64, file3);
                fgets(ch, 64, file3);
                str = ch;
                evaluation_times = std::stoi(str);

                fclose(file3);
        }
        /// /// //////////////////////////////////////////////////////////////////////////////////////

        /// /// /////////////////////////////////////////////////////////////////////////////////

        // Establish links, including resting target TIMEPOINT and departure target TIMEPOINT.
        make_link(TimeTable_merged, min_restime);

        return sum / count;
}

