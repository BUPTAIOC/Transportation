#pragma once
#include <string>
#include <iostream>
#include <vector>
using namespace std;

//��¼ÿ������ʱ�̵����г̺�ʱ����
//ֻӦ�ü�¼�����ֱ�����룬һ�д���������Ӧ����park��
class timepoint
{
public:
        //�������
        int Number;

        string tstr;
        int tot;
        int triptime;
        //�ĸ���վ��ʱ�̵㣬��������
        int fromCP;
        int gotoCP;

        //�ڼ���������(ͬCP��)
        int SlotY;
        //��Ϣ������Ŀ�공��,���һ�������Ϊ-1
        int Nextrest_pointX;
        //����������Ŀ�공��,�������һ�������Ϊ-1
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