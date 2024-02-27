#pragma once
//#include "excel.h"
#include "schedule.h"
#include <fstream>
//指针对某种客户要求下，按照某种优化方法得到的排班方案集合。集合拥有某些性质：至少的车辆数等。
class onecase
{
public:
        string line_name;

        pb_before_yueshu yueshu;

        vector<timepoint> TimeTable;

        Swa S;

        Schedule oldPB;//如果有方案上的继承，那么把方案样例存储在这里
        vector<Schedule> pbs;

        //pbi的得分属于什么名次
        vector<int> scoreth_ofpbi;
        //第i名是pb几
        vector<int> pbi_ofscoreth;
        //pbi的得分属于什么名次
        vector<int> scoreth_of_broken_pbi;
        //第i名是pb几
        vector<int> pbi_of_broken_scoreth;

        /////////////////////////////////////////////////////
        void TimeTable_make_link()
        {
                //找到车到站之后的首个park,即将将大于arrivetime的park
                //刚好在发车slot时刻到站时，不返回此slot
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        TimeTable[i].nextrip_firstParkX = -1;

                        int arrivetime = TimeTable[i].tot + TimeTable[i].triptime + yueshu.atlist_one_rest;

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
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        TimeTable[i].Nextrest_pointX = -1;

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

        //打印进化曲线
        void print_to_excel(string fname, Swa& S, vector<Schedule>& pbs)
        {
                ofstream oFile;

                oFile.open(fname + ".csv", ios::out | ios::trunc);//

                for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////排班主循环
                {
                        int total_back = 0;
                        for (int j = 0; j < S.gr_num; j++)
                        {
                                oFile << "No epo p:" <<","<<i* S.Max_epoch + j + 1 <<"," << i + 1 << "," << j + 1<< ",";//总序号、epoch、粒子序号
                                oFile << S.grs[j].ps[i].punish << ",";
                                if (j == S.grs.size() - 1)
                                {
                                        //printf("i:%d %d\t", i*S.grs.size() + j, S.grs[j].ps[i].punish);
                                        oFile << S.sw_avgPunish(i);
                                }
                                else
                                {
                                        //printf("i:%d %d\t", i*S.grs.size() + j, S.grs[j].ps[i].punish);
                                        oFile << "";
                                }

                                for (int k = 0; k < S.p_standard.ds.size(); k++)
                                {
                                        //printf("%.2f\t", S.grs[j].ps[i].ds[k].value);
                                        if (S.grs[j].ps[i].punish == 12000)
                                        {
                                                oFile << "," << S.grs[j].ps[i].ds[k].value << "," << S.grs[j].ps[i].ds[k].value;
                                        }
                                        else
                                        {
                                                oFile << "," << S.grs[j].ps[i].ds[k].value << "," << "";
                                        }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
                                }

                                total_back += pbs[i * S.gr_num + j].result.backtrack_num;
                                if (j == S.grs.size() - 1)
                                {
                                        //printf("avg punish: %d\t", totpunish / S.grs.size());
                                        oFile << "," << "," << pbs[i * S.gr_num + j].result.backtrack_num << "," << double(total_back) / S.gr_num;
                                }
                                else
                                {
                                        oFile << "," << "," << pbs[i * S.gr_num + j].result.backtrack_num << "," << "";
                                }
                                //printf("\n");
                                oFile << endl;
                        }
                }
                oFile.close();
        }
        //打印最优方案
        void print_scheme(string fname, Schedule pb)
        {
            ofstream SchedulFile;//输出排班结果
            SchedulFile.open(fname, ios::out | ios::trunc);//
            for (int car_i = 0; car_i < pb.tripque.size(); car_i++)//i是车号
            {
                SchedulFile << pb.tripque[car_i][0]->tp.tot - pb.tripque[0][0]->tp.tot + 1;
                SchedulFile << " ";
                SchedulFile << pb.tripque[car_i][0]->tp.tot;
                SchedulFile << " ";
                SchedulFile << pb.tripque[car_i][0]->tp.fromCP;
                SchedulFile << " ";
                for (int k = 0; k < pb.tripque[car_i].size(); k++)//k是trip
                {
                    SchedulFile << pb.tripque[car_i][k]->tp.tot;
                    SchedulFile << " ";
                    SchedulFile << pb.tripque[car_i][k]->tp.triptime;
                    if(k < pb.tripque[car_i].size()-1)SchedulFile << " ";
                }
                SchedulFile << endl;
            }
            SchedulFile.close();
        }
        //打印重调度结果数据统计
        void print_result_sts(string fname, Schedule pb)
        {
            ofstream oFile;

            oFile.open(fname, ios::out | ios::app);//

            oFile << "," << "cars" << "," << pb.result.carsused
                  << "," << "dirs" << "," << pb.result.drivsused
                  << "," << "emps" << "," << pb.result.nocar_slotnum
                  << "," << "evens" << "," << pb.result.unback_drivernum;

            oFile << endl;
        }
        /////////////////////////////////////////////////////
        //初始排班任务循环
        onecase(string line,
                vector<timepoint>& inTimeTable,
                pb_before_yueshu inyueshu,
                Swa& inS
        )
        {
                line_name = line;
                TimeTable = inTimeTable;
                yueshu = inyueshu;
                S = inS;

                TimeTable_make_link();

                Schedule pb(TimeTable,yueshu,-1);//初始化的空排班方案
                for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////排班主循环
                {
                        for (int j = 0; j < S.grs.size(); j++)
                        {
                                pbs.push_back(pb); pbs.back().clear(); pbs.back().ScheduleID = j;
                                pbs.back().yueshu.change_Particle(&S.grs[j].ps[i]);//将粒子群参数更新到约束结构内
                                S.grs[j].ps[i].punish = pbs.back().Schedule_parks(oldPB).punish;                             
                        }
                        S.get_next_ps(i);

                        printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f \n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////分数排名部分
                for (int i = 0; i < pbs.size(); i++)
                {
                        int num = 0;
                        for (int j = 0; j < pbs.size(); j++)
                        {
                                if (pbs[i].result.punish > pbs[j].result.punish)
                                {
                                        num++;
                                }
                        }
                        scoreth_ofpbi.push_back(num);
                }
                ///////////////////////////////////////////////////////
                for (int i = 0; i < pbs.size(); i++)
                {
                        for (int j = 0; j < pbs.size(); j++)
                        {
                                if (scoreth_ofpbi[j] == i)
                                {
                                        pbi_ofscoreth.push_back(j);
                                }
                        }
                        //cout << i << " th:car " << pbi_ofscoreth[i] << endl;
                }
                print_scheme(line_name + "/best_scheme.txt", pbs[pbi_ofscoreth[0]]);//打印最优排班
                print_to_excel(line_name + "/ofline_train", S, pbs);//把粒子轨迹输入文件
                cout << endl;
        }
        //创建重调度排班环境
        onecase(onecase& oldcase)
        {
                line_name = oldcase.line_name;
                yueshu = oldcase.yueshu;
                TimeTable = oldcase.TimeTable;
                S = oldcase.S;
                ///////////////////////////////////////////////////////////////////////
                //初始化的空排班方案,继承上次调度的最优方案
                oldPB = oldcase.pbs[oldcase.pbi_ofscoreth[0]];
        }

        //将当前行程时长随机上下波动 r% 
        void change_triptime_simprand_triptime(float r)
        {
                //更新时刻表
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        int radis = int(r * TimeTable[i].triptime);
                        int rplus = rand() % (2*radis + 1);

                        TimeTable[i].triptime += rplus - radis;
                }
        }
        //更新阻塞行程时长。trip增加分钟，用于重调度
        void change_triptime_blocked(double breakbeginH, double breaklenH, int triplusM)
        {
                //更新时刻表
            int break_t = -1;
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        //阻塞时段不包括区间边界。所以在阻塞时段左边界的原方案park也要复制过来。
                        if (TimeTable[i].tot > 60 * breakbeginH && TimeTable[i].tot < 60 * (breakbeginH + breaklenH))
                        {
                                TimeTable[i].triptime += triplusM;
                                if(break_t == -1)break_t = TimeTable[i].tot;
                        }
                }
                yueshu.breaktime = break_t;
        }
        //更新车辆故障开始时间与时长。通过延长单个时刻点triptime来实现
        void change_triptime_vehicle_failure(double failbeginH, double len, double faillenM)
        {
            //更新时刻表
            int break_t = -1;
            for (int i = 0; i < TimeTable.size(); i++)
            {
                //fail时刻后首个时刻点认为是车辆故障点
                if (TimeTable[i].tot > 60 * failbeginH)
                {
                    TimeTable[i].triptime += faillenM;
                    if (break_t == -1)break_t = TimeTable[i].tot;
                    break;
                }
            }
            yueshu.breaktime = break_t;
        }

        //（根据实际道路每小时的行程时长范围）产生随机行程时长，以模拟另一天的道路情况
        void change_triptime_realrand_triptime()
        {
                //统计，用于mod3
                vector<int> max_triptime; max_triptime.resize(24, -9999);
                vector<int> avg_triptime; avg_triptime.resize(24, 0);
                vector<int> count; count.resize(24, 0);
                vector<int> min_triptime; min_triptime.resize(24, 99999);
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        int time = TimeTable[i].tot;
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
                        int time = TimeTable[i].tot;
                        int hour = time / 60;
                        TimeTable[i].triptime = min_triptime[hour] + rand() % (1 + max_triptime[hour] - min_triptime[hour]);
                }
        }
        //modle：-2 固定参数模式,车辆数受限为MaxCarNum，模拟车辆故障，允许未覆盖
        //modle:-1 固定参数模式，车辆数受限为MaxCarNum，模拟随机拥堵，允许未覆盖
        //modle：0 固定参数模式，参数使用优化后的参数，且之后固定。
        //modle：1 继续优化模式，参数使用优化后的参数，且之后继续优化。
        //modle：2 重新优化模式，参数使用随机参数，且之后继续优化。
        void resort_oldcase(int mod, int MaxCarNum)
        {
                vector<int> empint;
               
                //更新链接
                TimeTable_make_link();               

                Schedule PBempty(TimeTable, yueshu, -1);//初始化的空排班方案
                pb_result_shuxing result;//单次排班
                switch (mod)
                {
                case -2://modle：-2 固定参数模式，模拟车辆故障，允许未覆盖
                    pbs.push_back(PBempty); pbs.back().clear();
                    pbs.back().yueshu.change_Particle(S.sw_bestP());//将粒子群参数更新到约束结构内
                    pbs.back().yueshu.max_carnum = MaxCarNum;//最大车辆约束
                    result = pbs.back().Schedule_parks(oldPB);//单次排班
                    result.scorer();//回写目标函数值
                    break;

                case -1://modle：-1 固定参数模式，车辆数受限为原方案，允许未覆盖
                    pbs.push_back(PBempty); pbs.back().clear();
                    pbs.back().yueshu.change_Particle(S.sw_bestP());//将粒子群参数更新到约束结构内
                    pbs.back().yueshu.max_carnum = MaxCarNum;//最大车辆约束
                    result = pbs.back().Schedule_parks(oldPB);//单次排班
                    result.scorer();//回写目标函数值
                    break;

                case 0://modle：0 固定参数模式，参数使用优化后的参数，且之后固定。
                        pbs.push_back(PBempty); pbs.back().clear();
                        pbs.back().yueshu.change_Particle(S.sw_bestP());//将粒子群参数更新到约束结构内
                        result = pbs.back().Schedule_parks(oldPB);//单次排班
                        result.scorer();//回写目标函数值
                        break;


                case 1://modle：1 继续优化模式，参数使用优化后的参数，且之后继续优化。                      
                        S.init_swa(S);
                        for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////排班主循环
                        {
                                for (int j = 0; j < S.grs.size(); j++)
                                {
                                        pbs.push_back(PBempty); pbs.back().clear(); pbs.back().ScheduleID = j;
                                        pbs.back().yueshu.change_Particle(&S.grs[j].ps[i]);//将粒子群参数更新到约束结构内
                                        result = pbs.back().Schedule_parks(oldPB);//单次排班
                                        S.grs[j].ps[i].punish = result.scorer();//回写目标函数值                                       
                                }
                                S.get_next_ps(i);
                                printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f \n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                        }
                        //print_to_excel(line_name + "/ofline_test_mod1", S, pbs);//把粒子轨迹输入文件
                        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        break;


                case 2://modle：2 重新优化模式，参数使用随机参数，且之后继续优化。
                        S.init_swa();
                        for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////排班主循环
                        {
                                for (int j = 0; j < S.grs.size(); j++)
                                {
                                        pbs.push_back(PBempty); pbs.back().clear(); pbs.back().ScheduleID = j;
                                        pbs.back().yueshu.change_Particle(&S.grs[j].ps[i]);//将粒子群参数更新到约束结构内
                                        result = pbs.back().Schedule_parks(oldPB);//单次排班
                                        S.grs[j].ps[i].punish = result.scorer();//回写目标函数值
                                }
                                S.get_next_ps(i);
                                printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f\n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                        }
                        //print_to_excel(line_name + "/ofline_test_mod2", S, pbs);//把粒子轨迹输入文件
                        break;

                default:
                        break;
                }

                /////////////////////////////////////////////////////分数排名部分
                for (int i = 0; i < pbs.size(); i++)
                {
                        int num = 0;
                        for (int j = 0; j < pbs.size(); j++)
                        {
                                if (pbs[i].result.punish > pbs[j].result.punish)
                                {
                                        num++;
                                }
                        }
                        scoreth_ofpbi.push_back(num);
                }
                ///////////////////////////////////////////////////////
                for (int i = 0; i < pbs.size(); i++)
                {
                        for (int j = 0; j < pbs.size(); j++)
                        {
                                if (scoreth_ofpbi[j] == i)
                                {
                                        pbi_ofscoreth.push_back(j);
                                }
                        }
                }
                print_scheme(line_name + "/best_RE_scheme.txt", pbs[pbi_ofscoreth[0]]);//打印重调度的最优排班
                //print_result_sts(line_name + "/RE_result.csv", pbs[pbi_ofscoreth[0]]);//打印统计数据
                cout << endl;
        }
};