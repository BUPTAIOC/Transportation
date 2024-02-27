#pragma once
#include "yueshu_result.h"
using namespace std;

//使用Car.times表达的编码方法。
class Car
{
public:
        //车编号，从0开始
        int carx;
        //目前车辆和司机的上班进度：0 work;1 sleep。
        int state;
        //车类型：长班车2 短班车1 高峰车0. 初始全都为1，中途提前睡眠变0，正常睡眠后被唤醒变2
        int type;

        //是由哪个时刻点更新的，用于回溯还原现场。
        int fromparkx;
        //何时开始睡觉，分钟计。-1代表没睡过。
        int whenslep;

        int tripnum;
        int tripmax;
        int Tripleft()
        {
                return tripmax - tripnum;
        }
        int break_maxtrip()
        {
                if (tripnum >= tripmax)
                {
                        return 1;
                }
                return 0;
        }
        //已经在路上开的总时间与上限
        int drivetime;
        int drivemax;
        int Driveleft()
        {
                return drivemax - drivetime;
        }
        int break_drivetime(int triptime)
        {
                if (drivetime + triptime > drivemax)
                {
                        return 1;
                }
                return 0;
        }
        //已经休息的总时间
        int restime;
        //当前休息时段休息了多久
        int resthis;
        //从发车到当前的总时间与上限
        int worktime()
        {
                return restime + drivetime;
        }
        int workmax;
        int Workleft()
        {
                return workmax - worktime();
        }
        int break_worktime(int triptime)
        {
                if (Workleft() < triptime)
                {
                        return 1;
                }
                return 0;
        }
        int canot_fulltrip(int triptime)
        {
                if (Workleft() <= triptime * 2 + 1  && tripnum % 2 == 0)//nowtripnum == max_tripnum -2
                {
                        return 1;
                }
                return 0;
        }
        //这个working车无法转换为长班车了.所以不能sleep_to_long。
        int canot_tolong(int triptime)
        {
                switch (type)
                {
                case 0://高
                        return 1;
                        break;
                case 1://短s
                        return 0;
                        break;
                case 2://长
                        return 1;
                        break;
                default:
                        return 1;
                        break;
                }
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////
        Car(int incarx, int intype, int instate,int infromx,
                int indrivmax, int inworkmax, int intripmax,
                int indrivetime, int inrestime, int intripnum,
                int inwhenslep)
        {
                carx = incarx;
                type = intype;
                state = instate;

                fromparkx = infromx;

                drivemax = indrivmax;
                workmax = inworkmax;
                tripmax = intripmax;

                tripnum = intripnum;
                drivetime = indrivetime;
                restime = inrestime;
                resthis = inrestime;

                whenslep = inwhenslep;
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////
        void update_rest(int infromx, int restplus)//总休息时间只在drive后更新。
        {
                fromparkx = infromx;
                resthis += restplus;
                restime += restplus;
                //worktime += restplus;
                //worktime = restime + drivetime;
        }

        void update_drive(int infromx, int triplus, int driveplus, int partrest)
        {
                fromparkx = infromx;

                tripnum += triplus;

                restime += partrest;//有下一个trip后才开始更新总休息时长。

                drivetime += driveplus;

                //worktime += driveplus + partrest;  
                //worktime = restime + drivetime;

                resthis = partrest;//重新累加单次休息时长
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //使车辆进入睡眠高峰
        void sleep_to_peak(int intosleep_slotot)
        {
                if (state == 1)
                {
                        cout << "state1car sleep error" << endl;
                        return;
                }
                if (type != 1)
                {
                        cout << "can not sleep error" << endl;
                }
                state = 1;
                type = 0;

                whenslep = intosleep_slotot - resthis;

                restime -= resthis;
                resthis = 0;
        }
        //使车辆进入睡眠短班
        void sleep_to_long(int intosleep_slotot)
        {
                if (state == 1)
                {
                        cout << "state 1 car sleep error" << endl;
                        return;
                }
                if (type != 1)
                {
                        cout << "can not sleep error" << endl;
                }
                state = 1;
                type = 1;

                whenslep = intosleep_slotot - resthis;
        }
        //唤醒车辆,唤醒时为slotcar分配空间,转换车辆state。
        void weakup(int infromx, int intriplus, int indrive_plus, int inrest_plus)
        {
                state = 0;

                if (type == 2)//长班车不能再被唤醒了
                {
                        cout << carx << "can not wake up" << endl;
                }

                if (type == 1)//唤醒正常车，说明变为长班车。
                {
                        type = 2;
                        //提升运力上线。
                        tripmax *= 2;
                        drivemax *= 2;
                        workmax *= 2;
                        update_drive(infromx, intriplus, indrive_plus, inrest_plus);
                }

                if (type == 0)//唤醒高锋车
                {
                        update_drive(infromx, intriplus, indrive_plus, inrest_plus);
                }
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////////////////
        //chosecar时的引导评分函数。返回此时车辆作为下一时刻点发车的得分（权重）。       
        //使用车辆 剩余工作时间/剩余trip数 的引导评分函数。返回此时车辆被选中的权重。+ pso
        double simple_score(
                int endtime_ofday,//一天结束前最后一个车辆的到站时间
                int nowtime, int triptime,
                int aweak_carnum, int sleeptype1_carnum, int sleeptype2_carnum,
                int Wplus_backhome = 100, double sleep_minu = 0.1, double pow1 = 2, double pow2 = 2)
        {
                int Y = 0;
                if (tripmax - 1 == tripnum)
                {
                        Y = 1;
                }

                double score = 0;
                if (state == 0)//工作状态的车
                {
                        int tripleft = Tripleft();
                        int workleft = Workleft();//剩余工作时间

                        if (workmax - worktime() > endtime_ofday - nowtime)//实际上，一天快结束时，记录的worktime可能不确切。
                        {
                                workleft = endtime_ofday - nowtime;
                        }
                        int wateleft = 1 + workleft - triptime;//剩余可休息时间。另一种方式是不考虑triptime
                        if (wateleft <= 0)
                        {
                                cout << "wateleft <=0 error" << endl;
                                wateleft = 1;
                        }

                        double trips_pear_min = double(tripleft) / double(wateleft);

                        //trips_pear_min^2*(resthis^2 + 100*IFlastrip) 是一个很有效的权重公式
                        score = 100 * pow(trips_pear_min, pow1)*(pow(resthis, pow2) + Wplus_backhome * Y + 1) ;
                        //score = 100 * (trips_pear_min + Wplus_backhome * Y + 1) * pow(resthis, pow2);(审稿意见的公式)

                        //cout << score << endl;
                        if (score <= 0)
                        {
                                cout << "score <=0 error"<<endl;
                        }
                }
                else//休眠状态的车
                {
                        int emptime = nowtime - whenslep;

                        if (aweak_carnum > 0)//还有其他在工作车
                        {
                                score = 0;
                        }
                        else if (sleeptype1_carnum > 0)//只剩sleep car,有sleep 普通车+sleep peak
                        {
                                if (type == 2)//sleep长
                                {
                                        score = 0;
                                        cout << "Wrong! should not have sleeping long car" << endl;
                                }
                                else if (type == 1)//sleep短
                                {
                                        score = 2 + 2 * pow(emptime, 2);
                                }
                                else if (type == 0)
                                {
                                        score = 0; //高峰车是来救急的，轻易不用     
                                }
                        }
                        else if (sleeptype2_carnum > 0)//只剩sleep peak car
                        {
                                if (type == 0)
                                {
                                        score = 1 + 1 * pow(emptime, 2); //高峰车是来救急的，轻易不用     
                                }
                                else
                                {
                                        score = 0;
                                        cout << "Wrong! should not have other car" << endl;
                                }
                        }

                }
                return (score);
        }

        //chosecar时的引导评分函数。返回此时车辆作为下一时刻点发车的得分（权重）。       
        //使用车辆 剩余工作时间/剩余trip数 的引导评分函数。返回此时车辆被选中的权重。+ pso + rbf
        double rbf_score(
            int endtime_ofday,//一天结束前最后一个车辆的到站时间
            int nowtime, int triptime,
            int aweak_carnum, int sleeptype1_carnum, int sleeptype2_carnum, hiden_layer & H, P & p_used)
        {
            int Y = 0;
            if (tripmax - 1 == tripnum)
            {
                Y = 1;
            }

            double score = 0;
            if (state == 0)//工作状态的车
            {
                int tripleft = Tripleft();
                int workleft = Workleft();//剩余工作时间

                if (workmax - worktime() > endtime_ofday - nowtime)//实际上，一天快结束时，记录的worktime可能不确切。
                {
                    workleft = endtime_ofday - nowtime;
                }
                int wateleft = 1 + workleft - triptime;//剩余可休息时间。另一种方式是不考虑triptime
                if (wateleft <= 0)
                {
                    cout << "wateleft <=0 error" << endl;
                    wateleft = 1;
                }

                double trips_pear_min = double(tripleft) / double(wateleft);

                vector<double> inputs;
                //inputs.push_back(double(tripleft));
                //inputs.push_back(double(wateleft));
                //inputs.push_back(trips_pear_min * 2500);
                inputs.push_back(resthis);
                //inputs.push_back(double(tripleft));
                //inputs.push_back(double(wateleft));
                //inputs.push_back(double(Y));

                //trips_pear_min^2*(resthis^2 + 100*IFlastrip) 是一个很有效的权重公式

                score = H.out_tot(p_used, inputs);
                if (score <= 1)score = 1;

                //cout << score << endl;
                if (score <= 0)
                {
                    cout << "score <=0 error" << endl;
                }
            }
            else//休眠状态的车
            {
                int emptime = nowtime - whenslep;

                if (aweak_carnum > 0)//还有其他在工作车
                {
                    score = 0;
                }
                else if (sleeptype1_carnum > 0)//只剩sleep car,有sleep 普通车+sleep peak
                {
                    if (type == 2)//sleep长
                    {
                        score = 0;
                        cout << "Wrong! should not have sleeping long car" << endl;
                    }
                    else if (type == 1)//sleep短
                    {
                        score = 2 + 2 * pow(emptime, 2);
                    }
                    else if (type == 0)
                    {
                        score = 0; //高峰车是来救急的，轻易不用     
                    }
                }
                else if (sleeptype2_carnum > 0)//只剩sleep peak car
                {
                    if (type == 0)
                    {
                        score = 1 + 1 * pow(emptime, 2); //高峰车是来救急的，轻易不用     
                    }
                    else
                    {
                        score = 0;
                        cout << "Wrong! should not have other car" << endl;
                    }
                }

            }
            return (score);
        }
};

/////////////////////////////////////////////////////////////////////////////////////////////////