#pragma 
#include "timepoint.h"
#include "readfile.h"
#include "onecase.h"
#include "pso.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////////////////////////
//排班前，给下个排班的属性约束。会根据上个排班结果动态调整
class Constrains
{
public:
    //从哪个时间开始排班。用以变异。这个时间之后的车辆都是幽灵车，要在新的变异方案中清除。
    int breaktime;
    //这一块是chosecar函数的参数
    int ifrand;
    int ifbacktrack;
    int stationum;
    //最多不能超过多少个trip
    int max_tripnum;//////////////////使用PSO优化
    //trip后至少休息多久
    int atlist_one_rest;
    //是否有工作时长约束
    int max_worktime;
    //是否有驾驶时长约束
    int max_drivetime;
    //高峰车数
    int if_use_peakcar;
    int peakcars_used;
    int to_peakcar_restime;/////使用PSO优化

    //在latest_new_car_time之前，必须发够min_newcar_num个新车
    int if_min_newcar;
    int latest_new_car_time;
    int min_newcar_num;
    //动态调度时可用的最大车辆数
    int max_carnum;
    //加速安排最后trip的加速系数
    int Wplus_backhome;///////////////使用PSO优化
    // particle swarm，particle
    P P_used;

    Constrains()
    {
        breaktime = 0;
        ifrand = 0;
        ifbacktrack = 0;
        stationum = 0;
        max_tripnum = 0;
        atlist_one_rest = 1;
        max_worktime = 0;
        max_drivetime = 0;
        to_peakcar_restime = 0;
        if_use_peakcar = 0;
        peakcars_used = 0;
        this->max_carnum = 999999;
        if_min_newcar = 0;
        latest_new_car_time = 0;
        min_newcar_num = 0;
        Wplus_backhome = 0;
    }
    Constrains(
        int inbreaktime,
        int inifrand,
        int inifbacktrack,
        int inifmincar,
        int instationum,
        int ifgaofeng,
        int max_carnum,

        int inatlist_one_rest,
        int inmax_work_time,
        int inmax_drive_time
    )
    {
        breaktime = inbreaktime;
        ifrand = inifrand;
        ifbacktrack = inifbacktrack;

        stationum = instationum;

        atlist_one_rest = inatlist_one_rest;

        max_worktime = inmax_work_time;

        max_drivetime = inmax_drive_time;

        if_use_peakcar = ifgaofeng;

        this->max_carnum = max_carnum;

        peakcars_used = 0;

        if_min_newcar = inifmincar;
        latest_new_car_time = 0;
        min_newcar_num = 0;

        Wplus_backhome = 0;
    }

    void change_Particle(P* inP)
    {
        this->max_tripnum = int(inP->ds[0].val()); if (max_tripnum % 2 == 1) { max_tripnum--; }
        this->to_peakcar_restime = int(inP->ds[1].val());
        this->Wplus_backhome = int(inP->ds[2].val());
        this->latest_new_car_time = int(inP->ds[6].val());
        this->min_newcar_num = int(inP->ds[7].val());
        P_used = *inP;
    }
};

// Used to represent the result attributes of the completed scheduling, 
// including the number of vehicles, peak numbers, rest numbers, final score, etc.
class scheme_result_info
{
public:
    int carsused;
    int drivsused;

    // The number of time points not covered by any vehicle.
    int nocar_slotnum;
    // The number of instances where rest time is less than the threshold.
    int buisy_restnum;
    // The penalty value due to insufficient rest time.
    int buisy_punish;

    // The number of drivers whose number of trips is not even.
    int unback_drivernum;

    // The number of backtracks needed to obtain a better solution.
    int backtrack_num;

    // The higher the punish value, the worse it is.
    int punish;

    scheme_result_info(
        int incarnum = 0,
        int indrivernum = 0,
        int inafter_togaofengrestime = 0,
        int inocar_slotnum = 0,
        int inbadrestnum = 0,
        int inbadrestpunish = 0,
        int inunback_drivernum = 0)
    {
        carsused = incarnum;
        drivsused = indrivernum;

        nocar_slotnum = inocar_slotnum;
        buisy_restnum = inbadrestnum;
        buisy_punish = inbadrestpunish;

        unback_drivernum = inunback_drivernum;

        backtrack_num = 0;

        punish = 0;
    }
    //evaluation function of a scheme
    int scorer()
    {
        punish = 0;
        punish += carsused * 1000;
        //punish += drivsused *100;
        //punish += nocar_slotnum * 1000;
        //punish += buisy_punish / 10;
        punish += unback_drivernum * 2000;
        //punish += backtrack_num * 1;
        return punish;
    }
};


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

        Constrains constrains;

        vector<Timepoint> TimeTable;

        Swarm S;

        Setting()
        {
                line_name = "lineNanJing";//line name,include 
                //linek1
                //line4 
                //line15 
                //line52
                //line59 
                //line60 
                //line70 
                //line71 
                //line85
                //line803 
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
                ///////////////////////////////////////////////////////////////////constrains
                S = Swarm(swam_size, evaluation_times, 0.8, 0.3, 0.3);//swam_size，epoch，oumiga, Personal best attraction, swarm best attraction.
                S.cin_new_d(max_tripnum, max_tripnum + 1, 1);//constrains.P_used.ds[0]//tripnum
                S.cin_new_d(20, 90, 10);//constrains.P_used.ds[1]//to peakcar restime
                S.cin_new_d(100, 1500, 200);//constrains.P_used.ds[2]// b plus
                S.cin_new_d(19, 20, 5);//constrains.P_used.ds[3]//sleep minus 
                S.cin_new_d(0, 5, 1);//constrains.P_used.ds[4]//pow1
                S.cin_new_d(0, 5, 1);//constrains.P_used.ds[5]//pow2
                S.cin_new_d(300, 660, 60);//constrains.P_used.ds[6]//last new car time
                S.cin_new_d(5, 60, 5);//constrains.P_used.ds[7]//min newcar num

                S.init_swa();

                constrains = Constrains(
                        0,//break down time
                        0,//if rand modle
                        0,//if backtrack modle (forword detection approach)
                        0,//if enough cars on morning
                        2,//CP num
                        1, //if use peak cars
                        99999,//manimum cars available
                        atlist_one_rest,
                        max_work_time,
                        max_drive_time
                );
        }


};