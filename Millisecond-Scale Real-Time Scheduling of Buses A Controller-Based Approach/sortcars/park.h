#pragma once
#include "timepoint.h"
#include "car.h"
//时刻点车库，实时信息
class Park
{
public:
        //对应时刻点信息以及直接能推理出的信息
        timepoint tp;
        //当前slot的站内车资源状况
        vector<Car> Cars;   

        //用以回溯。会记录回溯后发现本不应该发车的车辆。
        //如果回溯目标点向前移动，则在目标点直接记录应该睡眠的车两。
        //如果回溯目标点不变，则在目标点添加车辆。
        //如果回溯目标点往回移动，则清除回溯目标之后的所有记录。
        vector<int> ShouldSleep_Cars;
        //根据回溯记录判断车辆是否应该直接休眠
        int marked_to_sleep(int carx)
        {
                for (int i = 0; i < ShouldSleep_Cars.size(); i++)
                {
                        if (carx == ShouldSleep_Cars[i])
                        {
                                return 1;
                        }
                }
                return 0;
        }

        //此slot是否发了新车,0/1 //此slot是否有新司机上班
        int ifnewcar;
        int ifnewdriver;
        //此slot发的车的id，0开始,-1表缺车发.
        int carx;
        int driverx;
        //当时的chosen
        int chosen;
        //此slot未排班之前，用了多少新车
        int carsused;
        int driverused;

        int endt()
        {
                return tp.tot + tp.triptime;
        }

        Park(timepoint& intp)
        {
                tp = intp;

                ifnewcar = -1;
                ifnewdriver = -1;
                carx = -1;
                driverx = -1;
                chosen = -2;
                carsused = -1;
                driverused = -1;
                Cars.clear();
        }

        void clear()
        {
                ifnewcar = -1;
                ifnewdriver = -1;
                carx = -1;
                driverx = -1;
                chosen = -2;
                carsused = -1;
                driverused = -1;
                Cars.clear();
        }
};