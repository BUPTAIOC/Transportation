#pragma once
#include <string>
#include <iostream>
#include <vector>
using namespace std;

//记录每个发车时刻点与行程耗时的类
//只应该记录任务的直接输入，一切处理后的数据应放在park里
class timepoint
{
public:
        //总体序号
        int Number;

        string tstr;
        int tot;
        int triptime;
        //哪个车站的时刻点，开往哪里
        int fromCP;
        int gotoCP;

        //第几个发车点(同CP内)
        int SlotY;
        //休息车辆的目标车库,如果一天结束，为-1
        int Nextrest_pointX;
        //发车车辆的目标车库,如果到后一天结束，为-1
        int nextrip_firstParkX;

        
        timepoint(string instr = "00:00,0,0,0")
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
                this->tot = hour * 60 + min;
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
              
                fromCP = vals[0];
                gotoCP = vals[1];
                triptime = vals[2];
        }

        int Clock_to_tot()
        {
                //std::cout << hour << ":" << min << "\n";
                return tot;
        }

        int ShowClock()
        {
                std::cout << tstr << "\n";
                return tot;
        }

};

class TIMETABLE
{
public:
        vector<timepoint> tps;
        string linename;
};