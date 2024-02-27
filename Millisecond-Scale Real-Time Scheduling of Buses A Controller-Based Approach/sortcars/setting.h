//settings in main.h
#pragma 
#include "timepoint.h"
#include "readfile.h"
#include "onecase.h"

#include "Coutshow.h"
#include "Qtshow.h"

#include "pso.h"
#include "rbf.h"
#include <iostream>

#include <QtWidgets/QApplication>
#include <QTime>

class Setting
{
public:
        string line_name;
        //司机每个trip后至少休息多久,硬性要求。如果为0，则实际至少休息1min。
        int atlist_one_rest;
        //司机最大上班时间，分钟计
        int max_work_time;
        //司机最大开车时间，分钟计
        int max_drive_time;
        int max_tripnum;
        int swam_size;
        int evaluation_times;

        pb_before_yueshu yueshu;

        vector<timepoint> TimeTable;

        Swa S;
        hiden_layer H;

        Setting()
        {
                line_name = "line59";//line name,include 
                //linek1
                //line4 line4_holiday_real
                //line15 line15_proj
                //line52
                //line59 line59_weekday_real
                //line60 
                //line70 line71 line85
                //line803 line803_real
                //lineNanJing

                int avg_triptime = ReadFile_TimeTable_merged_Message(
                        line_name,
                        TimeTable,
                        max_tripnum,
                        atlist_one_rest,
                        max_work_time,
                        max_drive_time,
                        swam_size,
                        evaluation_times);//从文件中读取时刻表到内存。

                atlist_one_rest;
                max_work_time;
                max_drive_time;
                max_tripnum;
                swam_size = 100;
                evaluation_times = 50;
                //cout << "平均trip时长::::::::::: :"<< avg_triptime <<endl;
                //司机总共最多休息时间,可以把6.5h挪到休息时间里。
                int max_rest_time = max_work_time - (max_tripnum * avg_triptime);
                int avg_rest_time = max_rest_time / (max_tripnum - 1);
                ///////////////////////////////////////////////////////////////////正常排班的初始约束。
                S = Swa(swam_size, evaluation_times, 0.8, 0.3, 0.3);//初始粒子数，epoch，oumiga，个人最佳引力，种群最佳引力
                S.cin_new_d(max_tripnum, max_tripnum + 1, 1);//yueshu.P_used.ds[0]//tripnum
                S.cin_new_d(20, 90, 10);//yueshu.P_used.ds[1]//to peakcar restime
                S.cin_new_d(100, 1500, 200);//yueshu.P_used.ds[2]// b plus
                S.cin_new_d(19, 20, 5);//yueshu.P_used.ds[3]//sleep minus 实际使用时 缩小100倍使用
                S.cin_new_d(0, 5, 1);//yueshu.P_used.ds[4]//pow1
                S.cin_new_d(0, 5, 1);//yueshu.P_used.ds[5]//pow2
                S.cin_new_d(300, 660, 60);//yueshu.P_used.ds[6]//last new car time
                S.cin_new_d(5, 60, 5);//yueshu.P_used.ds[7]//min newcar num
				//S = Swa(swam_size, evaluation_times, 0.8, 0.3, 0.3);//初始粒子数，epoch，oumiga，个人最佳引力，种群最佳引力
				//S.cin_new_d(6, 7, 1);//yueshu.P_used.ds[0]//tripnum
				//S.cin_new_d(46, 48, 1);//yueshu.P_used.ds[1]//to peakcar restime
				//S.cin_new_d(1386, 1388, 1);//yueshu.P_used.ds[2]// b plus
				//S.cin_new_d(19, 20, 5);//yueshu.P_used.ds[3]//sleep minus 实际使用时 缩小100倍使用
				//S.cin_new_d(3.2, 3.4, 1);//yueshu.P_used.ds[4]//pow1
				//S.cin_new_d(3.84, 3.86, 1);//yueshu.P_used.ds[5]//pow2
				//S.cin_new_d(300, 660, 60);//yueshu.P_used.ds[6]//last new car time
				//S.cin_new_d(10, 30, 2);//yueshu.P_used.ds[7]//min newcar num

              
                //inputs.push_back(double(tripleft));
                //inputs.push_back(double(wateleft));
                //inputs.push_back(resthis);
                H.add_input_size(0, 100, 20);
                //H.add_input_size(0, 100, 20);
                //H.add_input_size(0, 100, 20);
                H.set_inode_numb(20, S);

                S.init_swa();

                yueshu = pb_before_yueshu(
                        0,//break down time
                        0,//if rand modle
                        0,//if backtrack,一般not rand modle 使用回溯模式
                        0,//if 早高峰至少发出足够车辆
                        2,//CP num
                        1, //是否安排高峰车
                        99999,//可用最大车辆数约束,足量的话为99999
                        atlist_one_rest,//最少休息时长
                        max_work_time,//工作时长约束
                        max_drive_time,//驾驶时长约束
                        H
                );
        }


};