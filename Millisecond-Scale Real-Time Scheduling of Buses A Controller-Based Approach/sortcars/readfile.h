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

        //找到车到站之后的首个park,即将将大于arrivetime的park
        //刚好在发车slot时刻到站时，不返回此slot
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
        //同CP的后一个slotX
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



//读取时刻表信息到TimeTable
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
                cout << "无法打开 TimeTable_merged.txt 文件\n";      //如果打不开，就输出打不开
                cin >> ch;
                exit(0);                               //终止程序
        }
        else
        {
                while (!feof(file_tp))//判定文件是否结尾
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
        vector<int> avg_triptime_map;//平均triptime部分
        file_head = line_name + "/tmessage.txt";
        if ((err3 = fopen_s(&file3, file_head.c_str(), "r")) != 0)
        {
                cout << "无法打开 message  文件\n";            //如果打不开，就输出打不开
                cin >> ch;
                exit(0);                               //终止程序
        }
        else
        {
                /*
                第三部分
                单司机最大行程数：
                8
                至少休息时间：
                3
                最大工作时间：
                480
                最大驾驶时间：
                390
                种群大小：
                50
                进化轮次：
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

        //建立链接,包括休息目标车库和 发车目标车库
        make_link(TimeTable_merged, min_restime);

        return sum / count;
}

int chang_triptime(vector<timepoint>& TimeTable,int min_restime)
{
        //每小时的平均trip用时。可能出现超车现象。
        vector<int> triptimeofhour =
        //{ 00, 01, 02, 03, 04, 05, 06, 07, 08, 09, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23}
        //{ 27, 26, 25, 26, 28, 31, 35, 51, 49, 39, 34, 33, 32, 33, 33, 35, 37, 42, 37, 33, 31, 30, 27, 27 };//实际
        //{ 27, 26, 25, 26, 28, 31, 30, 31, 40, 38, 38, 38, 38, 38, 38, 38, 38, 38, 40, 38, 38, 33, 33, 30};//人工
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


// 定义一个类型别名，表示单个数据条目
using DataEntry = std::tuple<double, double, int>;

// 将一行数据转换为一个DataEntry
DataEntry parseDataEntry(const std::string& line) {
    std::istringstream ss(line);
    double start, duration;
    int delay;
    char coma;
    ss >> start >> coma >> duration >> coma >> delay;
    return std::make_tuple(start, duration, delay);
}

// 从文件读取数据
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
