#pragma once
#include <string>
#include <iostream>
#include <vector>
using namespace std;

// A class that records each departure time point and the duration of the trip.
// It should only record the direct input of the task; all processed data should be stored in the tp.
class Timepoint
{
public:
        // The index number in the merged timetable.
        int Number;

        string tstr;
        int time;
        int triptime;
        // Which station's time point, and where it is headed.
        int start_CP;
        int gotoCP;

        // Which departure point (within the same CP).
        int SlotY;
        // The target garage for resting vehicles; if the day ends, it is -1.
        int Nextrest_pointX;
        // The target garage for departing vehicles; if the day ends after arrival, it is -1.
        int nextrip_firstParkX;
        
        Timepoint(string instr = "00:00,0,0,0")
        {
                if (instr[instr.length() - 1] == '\0')
                {
                        this->tstr = instr.substr(0, instr.length() - 1);
                }
                else if (instr[instr.length() - 1] == '\n')
                {
                        this->tstr = instr.substr(0, instr.length() - 1);
                }
                else
                {
                        this->tstr = instr;
                }
                vector<int> blank;
                blank.push_back(tstr.find_first_of(","));
                string time = tstr.substr(0, blank[0]);
                int n = time.find(":");
                int hour = std::stoi(tstr.substr(0, n));
                int min = std::stoi(tstr.substr(n + 1, time.length()));
                this->time = hour * 60 + min;
                Nextrest_pointX = -1;
                nextrip_firstParkX = -1;

                vector<int>vals;
                for(int i = 0;i<3;i++)
                {      
                        n = 0;
                        if ((n = tstr.find_first_of(",", blank[i]+1)) != string::npos)
                        {
                                blank.push_back(n);
                                string intstr = tstr.substr(blank[i]+1, 1);

                                vals.push_back(std::stoi(intstr));
                        }
                        else
                        {
                                string intstr = tstr.substr(blank[i]+1, tstr.length());
                                vals.push_back(std::stoi(intstr));
                                break;
                        }
                }               
              
                start_CP = vals[0];
                gotoCP = vals[1];
                triptime = vals[2];
        }

        int Clock_to_tot()
        {
                //std::cout << hour << ":" << min << "\n";
                return time;
        }

        int ShowClock()
        {
                std::cout << tstr << "\n";
                return time;
        }
};