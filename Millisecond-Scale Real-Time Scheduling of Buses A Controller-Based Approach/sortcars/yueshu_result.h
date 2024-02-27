#pragma once
#include"pso.h"
#include"rbf.h"
#include <iomanip>
#include <cmath>
#include <algorithm>



/////////////////////////////////////////////////////////////////////////////////////////////////
//排班前，给下个排班的属性约束。会根据上个排班结果动态调整
class pb_before_yueshu
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
        int if_gaofeng;
        int gaofengused;
        int to_peakcar_restime;/////使用PSO优化

        //在latest_new_car_time之前，必须发够min_newcar_num个新车
        int if_min_newcar;
        int latest_new_car_time;
        int min_newcar_num;
        //动态调度时可用的最大车辆数
        int max_carnum;
        //加速安排最后trip的加速系数
        int Wplus_backhome;///////////////使用PSO优化

        P P_used;
        hiden_layer H;

        pb_before_yueshu()
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

                if_gaofeng = 0;

                gaofengused = 0;

                this->max_carnum = 999999;

                if_min_newcar = 0;
                latest_new_car_time = 0;
                min_newcar_num = 0;

                Wplus_backhome = 0;
        }
        pb_before_yueshu(
                int inbreaktime,
                int inifrand,
                int inifbacktrack,
                int inifmincar,
                int instationum,
                int ifgaofeng,
                int max_carnum,

                int inatlist_one_rest,
                int inmax_work_time,
                int inmax_drive_time,
                hiden_layer & inH
        )
        {
                breaktime = inbreaktime;
                ifrand = inifrand;
                ifbacktrack = inifbacktrack;

                stationum = instationum;

                atlist_one_rest = inatlist_one_rest;

                max_worktime = inmax_work_time;

                max_drivetime = inmax_drive_time;

                if_gaofeng = ifgaofeng;

                this->max_carnum = max_carnum;

                gaofengused = 0;

                if_min_newcar = inifmincar;
                latest_new_car_time = 0;
                min_newcar_num = 0;

                Wplus_backhome = 0;

                H = inH;
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

//用以表示已经排完的pb的结果属性。包括车数高峰数休息数最终得分等。
class pb_result_shuxing
{
public:
        int carsused;
        int drivsused;

        //超过建议的新车最晚发车时间的新发车的数量，用以优化buisylevel
        //int after_finalnewcartime;
        //所有试图超过高峰转换线的司机数，用以调控togaofengrestime
        int after_togaofengrestime;

        //没有车覆盖的时刻点数
        int nocar_slotnum;
        //休息时间少于阈值的数目。
        int buisy_restnum;
        //休息时间不足导致的惩罚值
        int buisy_punish;

        //trip数不是双数的司机数
        int unback_drivernum;

        //回溯了几次才得到较优解
        int backtrack_num;

        int punish;//数值越高，越不好

        pb_result_shuxing(
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

                after_togaofengrestime = inafter_togaofengrestime;

                nocar_slotnum = inocar_slotnum;
                buisy_restnum = inbadrestnum;
                buisy_punish = inbadrestpunish;

                unback_drivernum = inunback_drivernum;

                backtrack_num = 0;

                punish = 0;
        }

        int scorer()
        {
                punish = 0;
                punish += carsused * 1000;
                punish += drivsused *100;

                punish += nocar_slotnum * 1000;
                //punish += buisy_punish / 10;
                punish += unback_drivernum * 2000;

                //punish += backtrack_num * 1;

                return punish;
        }
};