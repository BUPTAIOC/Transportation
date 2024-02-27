#pragma once
#include "timepoint.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>

using namespace std;



void make_link(vector<timepoint> &TimeTable,int atlist_onerest)
{
        for (int i = 0; i < TimeTable.size(); i++)
        {
                for (int j = i + 1; j < TimeTable.size(); j++)
                {
                        if (TimeTable[i].tot > TimeTable[j].tot)
                        {
                                timepoint ttp = TimeTable[i];
                                TimeTable[i] = TimeTable[j];
                                TimeTable[j] = ttp;
                        }
                }
        }

        //�ҵ�����վ֮����׸�park,����������arrivetime��park
        //�պ��ڷ���slotʱ�̵�վʱ�������ش�slot
        for (int i = 0; i < TimeTable.size(); i++)
        {
                TimeTable[i].nextrip_firstParkX = -1;

                int arrivetime = TimeTable[i].tot + TimeTable[i].triptime + atlist_onerest;

                for (int j = i + 1; j < TimeTable.size(); j++)
                {
                        if (TimeTable[j].tot > arrivetime && 
                            TimeTable[j].fromCP == TimeTable[i].gotoCP)
                        {
                                TimeTable[i].nextrip_firstParkX = j;
                                break;
                        }
                }
        }
        //ͬCP�ĺ�һ��slotX
        vector<int>y; y.push_back(0); y.push_back(0);

        for (int i = 0; i < TimeTable.size(); i++)
        {
                TimeTable[i].Nextrest_pointX = -1;
                TimeTable[i].Number = i;
                TimeTable[i].SlotY = y[TimeTable[i].fromCP]; y[TimeTable[i].fromCP]++;

                for (int j = i + 1; j < TimeTable.size(); j++)
                {
                        if (TimeTable[i].fromCP == TimeTable[j].fromCP)
                        {
                                TimeTable[i].Nextrest_pointX = j;
                                break;
                        }
                }               
        }
}



//��ȡʱ�̱���Ϣ��TimeTable
int ReadFile_TimeTable_merged_Message(
        string line_name,
        vector<timepoint>& TimeTable_merged,
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
                cout << "�޷��� TimeTable_merged.txt �ļ�\n";      //����򲻿���������򲻿�
                cin >> ch;
                exit(0);                               //��ֹ����
        }
        else
        {
                while (!feof(file_tp))//�ж��ļ��Ƿ��β
                {
                        if (fgets(ch, 64, file_tp) != NULL)
                        {
                                string str = ch;
                                TimeTable_merged.push_back(timepoint(str));
                                line_tp++;
                        }
                }
                fclose(file_tp);
        }

        double sum = 0;
        int count = 0;
        FILE* file3; errno_t err3; int line3 = 0;
        ///  ///   ////////////////////////////////////////////////////////////////////////////////////////////////////
        vector<int> avg_triptime_map;//ƽ��triptime����
        file_head = line_name + "/tmessage.txt";
        if ((err3 = fopen_s(&file3, file_head.c_str(), "r")) != 0)
        {
                cout << "�޷��� message  �ļ�\n";            //����򲻿���������򲻿�
                cin >> ch;
                exit(0);                               //��ֹ����
        }
        else
        {
                /*
                ��������
                ��˾������г�����
                8
                ������Ϣʱ�䣺
                3
                �����ʱ�䣺
                480
                ����ʻʱ�䣺
                390
                ��Ⱥ��С��
                50
                �����ִΣ�
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

        //��������,������ϢĿ�공��� ����Ŀ�공��
        make_link(TimeTable_merged, min_restime);

        return sum / count;
}

int chang_triptime(vector<timepoint>& TimeTable,int min_restime)
{
        //ÿСʱ��ƽ��trip��ʱ�����ܳ��ֳ�������
        vector<int> triptimeofhour =
        //{ 00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23}
        //{ 27, 26, 25, 26, 28, 31, 35, 51, 49, 39, 34, 33, 32, 33, 33, 35, 37, 42, 37, 33, 31, 30, 27, 27 };//ʵ��
        //{ 27, 26, 25, 26, 28, 31, 30, 31, 40, 38, 38, 38, 38, 38, 38, 38, 38, 38, 40, 38, 38, 33, 33, 30};//�˹�
        //{ 31, 31, 31, 31, 31, 31, 32, 33, 34, 33, 33, 33, 32, 32, 33, 33, 33, 32, 32, 32, 32, 31, 31, 31 };//line70human
        //{ 30, 30, 30, 30, 30, 30, 39, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 38, 35, 32 };//line71human
        //{ 30, 30, 30, 30, 30, 30, 30, 33, 36, 37, 38, 38, 38, 38, 38, 38, 38, 38, 38, 37, 37, 36, 33, 30 };//line85human
        //{70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70};//line4human
        {35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35};//line59human
        //{50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50};//line60human
        //{41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41};//line803human
        //{65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65};//linek1human


        double sum = 0;
        int count = 0;
        vector<int> triptime_map;
        for (int i = 0; i < triptimeofhour.size(); i++)
        {
                for (int j = 0; j < 60; j++)
                {
                        triptime_map.push_back(triptimeofhour[i]);
                        //trip_time_map.push_back(10);
                }
        }
        for (int i = 0; i < TimeTable.size(); i++)
        {
                TimeTable[i].triptime = triptime_map[TimeTable[i].tot];
                sum += TimeTable[i].triptime;
                count++;
        }

        make_link(TimeTable, min_restime);

        return sum/count;
}


// ����һ�����ͱ�������ʾ����������Ŀ
using DataEntry = std::tuple<double, double, int>;

// ��һ������ת��Ϊһ��DataEntry
DataEntry parseDataEntry(const std::string& line) {
    std::istringstream ss(line);
    double start, duration;
    int delay;
    char coma;
    ss >> start >> coma >> duration >> coma >> delay;
    return std::make_tuple(start, duration, delay);
}

// ���ļ���ȡ����
std::vector<std::vector<DataEntry>> readDataFromFile(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::vector<DataEntry>> data;
    std::string line;

    while (std::getline(file, line)) {
        int numEntries = std::stoi(line);
        std::vector<DataEntry> entries;

        for (int i = 0; i < numEntries; ++i) {
            std::getline(file, line);
            entries.push_back(parseDataEntry(line));
        }

        data.push_back(entries);
    }

    return data;
}
