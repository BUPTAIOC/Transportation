#pragma once
#include "park.h"

#define DELETE_OR_SLEEP_working_car if (car->canot_tolong(triptime)){park.Cars.erase(park.Cars.begin() + i);i--;}else{car->sleep_to_long(oldtime);}

//如果需要回溯，那也只是这个排班内部的事情，不影响其他排班
class Schedule
{
public:
        int ScheduleID;
        //发车点车库，用以获取实时车库信息
        vector<Park> parks;
        //最后结束行程的车辆的结束时间。可能不是末班车，而是先发后至的稍早车辆。
        int endtime_ofday;
        //车辆排班结果，下标代表车id，第几个trip，元素代表这个trip在哪个Slot出发的。
        //car[].trip[]->(slot)
        vector<vector<Park*>> tripque;

        //已经排好的Slot数
        int sorted;
       
        pb_before_yueshu yueshu;//初始化后，本类内不会改变。
        pb_result_shuxing result;

        //构造函数，建立时刻点之间的连接,主要是同CP相邻时刻点的车库链接
        Schedule(vector<timepoint>& TimeTable, pb_before_yueshu& inyueshu, int S_id)
        {
                ScheduleID = S_id;

                sorted = 0;

                endtime_ofday = 0;

                yueshu = inyueshu;
                result = pb_result_shuxing();

                //初始化parks
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        parks.push_back(Park(TimeTable[i]));
                        if (TimeTable[i].tot + TimeTable[i].triptime > endtime_ofday)
                        {
                                endtime_ofday = TimeTable[i].tot + TimeTable[i].triptime;
                        }
                }
        }    

        Schedule()
        {

        }

        void clear()
        {
                sorted = 0;
                //初始化slos
                for (int i = 0; i < parks.size(); i++)
                {
                        parks[i].clear();//不清除时刻点链接
                }
                tripque.clear();
        }

        int VehicleSelector(Park& park, int endtime_ofday)
        {               
                //当作一辆车参与权重计算。
                //这辆车的所有属性的分综合为这个，算入总分当中。
                //即使车库有车，也可能发新车。
                int newcar_score = 0;

                int weak_i = 0;//统计醒着的车的下标集合
                int sleep_type1 = 0;//统计睡着的车的下标集合
                int sleep_type0 = 0;

                for (int i = 0; i < park.Cars.size(); i++)
                {
                        if (park.Cars[i].state == 0)
                        {
                                weak_i++;
                        }
                        else
                        {
                                if (park.Cars[i].type == 1)
                                {
                                        sleep_type1++;
                                }
                                else if (park.Cars[i].type == 0)
                                {
                                        sleep_type0++;
                                }
                        }
                }
                /////////////////////////////////////////////////////////////////////////
                //总分数
                vector<double>scores;
                vector<int> weight;
                int weight_tot = 0;
                //下面是单步评价标准。不代表最终评价，只用于车库发车决策。先把各种属性的得分分别算好，后面想用哪个自己加。
                /////////////////////////////////////////////////////////////////////////
                double sco = -1;
                double maxsco = 0.0;
                double minsco = 999999999.0;
                for (int i = 0; i < park.Cars.size(); i++)
                {
                        sco = park.Cars[i].simple_score(
                                endtime_ofday,
                                park.tp.tot,park.tp.triptime,
                                weak_i,
                                sleep_type1,
                                sleep_type0,
                                yueshu.Wplus_backhome,
                                yueshu.P_used.ds[3].val(),
                                yueshu.P_used.ds[4].val(),
                                yueshu.P_used.ds[5].val());

                        //sco = park.Cars[i].rbf_score(
                        //    endtime_ofday,
                        //    park.tp.tot, park.tp.triptime,
                        //    weak_i,
                        //    sleep_type1,
                        //    sleep_type0,
                        //    yueshu.H,yueshu.P_used);

                        scores.push_back(sco);//找到最大最小分数，方便归一化
                        if (sco < minsco && sco > 0)
                        {
                                minsco = sco;
                        }
                        if (sco > maxsco)
                        {
                                maxsco = sco;
                        }
                }
                for (int i = 0; i < park.Cars.size(); i++)
                {
                        int intsco = int((100 * scores[i]) / minsco);
                        weight.push_back(intsco);
                        weight_tot += intsco;
                }
                if (weight_tot == 0)
                {
                        cout << "weight_tot == 0 error"<<endl;
                }
                /////////////////////////////////////////////////////////////////////////
                int chosen_i = 0;
                double choseni_weight = weight[chosen_i];
                int randnum = qrand() % weight_tot;
                int numleft = randnum;
                switch (yueshu.ifrand)
                {
                case 0:
                        //策略：不随机,挑分高的车发。不会发新车，会唤醒。    
                        for (int i = 0; i < park.Cars.size(); i++)
                        {
                                if (weight[i] > choseni_weight)
                                {
                                        choseni_weight = weight[i];
                                        chosen_i = i;
                                }
                        }
                        break;
                case 1:
                        //策略：得分权重随机。得分既是权重。可能发新车。
                        for (int i = 0; i < park.Cars.size(); i++)
                        {
                                numleft -= weight[i];
                                if (numleft <= 0)
                                {
                                        chosen_i = i;
                                        break;
                                }
                        }
                        if (numleft > 0)
                        {
                                chosen_i = -1;
                        }
                        break;

                case 2:
                        //策略：得分排名权重随机。重新为各个名次定义权重。
                        break;
                default:
                        return 0;
                        break;
                }
                return chosen_i;
        }

        //更新车库信息，清除完结的车辆
        //用selector决定给定的slot怎么发车：新车？旧车？唤醒车？车号？
        //转移车辆，构建后续时刻点车库信息
        //copychose 用来复制断点之前的旧选择
        //如果需要回溯，则返回目标parkX，否则返回-1
        int mover(Park& park,int copychose)
        {
                //更新已用车辆信息
                park.carsused = result.carsused;
                park.driverused = result.drivsused;
                ////////////////////////////////////////////////////////////////////////////
                //找到发车到对岸的对岸slot
                //这趟trip的发车时刻
                int oldtime = park.tp.tot;
                //trip耗时
                int triptime = park.tp.triptime;
                //预测车辆nextrip到站时刻
                int next_arrivetime = oldtime + triptime;
                //车辆下trip到哪边
                int next_TableX = park.tp.gotoCP;
                //到站后下一个空闲slot的sloty。没有下个slot了就-1
                int nextrip_firstSlotY = park.tp.nextrip_firstParkX;//find_nextrip_firstSlotY4(TimeTable[next_TableX], next_arrivetime);
                //选中的车辆跑完此trip后，在对岸slot到来之前休息了多久
                int partrest; if (nextrip_firstSlotY >= 0)partrest = parks[nextrip_firstSlotY].tp.tot - next_arrivetime;
                //代表选中的车在车库中的位置。-1表示发新车，不用车库里的旧车。
                int chosen;
                int chosencarx;
                /////////////////////////////////////////////////////////////////////////////
                //下面是清扫车库中不能用的车：
                //时长超过8，6.5, 1.5
                //int atlistrest = 2;//司机至少休息的分钟数
                //下面清理车库中不能再用的车，或者换司机、让司机休眠。顺便让高峰车开始sleep。
                for (int i = 0; i < park.Cars.size(); i++)
                {
                        Car* car = &park.Cars[i];

                        switch (car->state)
                        {
                        case 0://0状态代表车刚刚还在work.下面的约束条件中，可能违背多个约束，比如maxtrips和worktime 
                                if (park.marked_to_sleep(car->carx))
                                {//如果发现这辆车有回溯记号，说明应该在这个时刻点直接下班，而不是选中发车
                                        DELETE_OR_SLEEP_working_car
                                }
                                else if (car->break_maxtrip()||car->canot_fulltrip(triptime))//正常下班的车辆,或双数行程下班的车辆.
                                {
                                        DELETE_OR_SLEEP_working_car
                                }
                                else if (car->break_worktime(triptime)||//worktime超标，休息过若干slot后才可能超过work time。
                                         car->break_drivetime(triptime))//触发这个条件时，分为奇数偶数trip情况
                                {//这些车一定trip数不满。如果trip是偶数，下班就好。如果trip是奇数，就要回溯到上个偶数trip,在那里下班。
                                        if (car->type != 1 && car->tripnum % 2 == 1 )// car->type != 1
                                        {
                                                if (yueshu.ifbacktrack == 1)//回溯模式
                                                {
                                                        Park* backto_park = tripque[car->carx][car->tripnum - 1];
                                                        if (backto_park->tp.tot >= yueshu.breaktime)//此时回溯,否则回溯也解决不了问题，只能继续。
                                                        {
                                                                backto_park->ShouldSleep_Cars.push_back(car->carx);
                                                                return backto_park->tp.Number;//应该回溯
                                                        }
                                                        else//回溯就要到断点之前了，回溯也解决不了问题了，就算了，继续排版
                                                        {
                                                                DELETE_OR_SLEEP_working_car
                                                        }
                                                }
                                                else//非回溯模式
                                                {
                                                        DELETE_OR_SLEEP_working_car
                                                }                                                
                                        }
                                        else//一般来说，短班车很可能转为长班车，不必保证偶数行程
                                        {
                                                DELETE_OR_SLEEP_working_car
                                        }
                                }
                                else if (car->type == 1 &&
                                        car->resthis >= yueshu.to_peakcar_restime &&
                                        car->tripnum % 2 == 0)//转换成高峰车,进入睡眠
                                {
                                        result.after_togaofengrestime++;
                                        car->sleep_to_peak(oldtime);
                                        yueshu.gaofengused++;
                                }
                                break;

                        case 1://1状态代表车在sleep.
                                if (park.marked_to_sleep(car->carx))
                                {//如果发现这辆车有回溯记号，说明应该在这个时刻点直接下班，而不是选中发车
                                        park.Cars.erase(park.Cars.begin() + i); i--;
                                }
                                else if (car->type == 1 && car->break_worktime(triptime))//短班车第一个司机下班后，sleep导致第一个司机的工作时间耗完
                                {                                    
                                        if (car->worktime() + 2 * triptime >= car->workmax * 2 ||
                                                car->drivetime + 2 * triptime >= car->drivemax * 2)
                                        {
                                                park.Cars.erase(park.Cars.begin() + i); i--;
                                        } 
                                        ////////////////////////////////////////////////////////////////////////新的长班车会遇到的情况
                                }
                                break;

                        default:
                                cout << "car state error";
                                break;
                        }
                }

                ///////////////////////////////////////是否发车、发新车、发旧车、哪辆旧车
                if (copychose >= 0)
                {
                        chosen = copychose;
                }
                else if (yueshu.if_min_newcar == 1 && 
                        result.carsused < yueshu.min_newcar_num && 
                        park.tp.tot >= yueshu.latest_new_car_time)
                {//到点必须至少发多少车，一般先关闭,line70、line85 line803可用.
                        chosen = -1;
                }
                else if (park.Cars.size() == 0)
                {
                        chosen = -1;
                }
                else if (park.Cars.size() == 1)
                {
                        chosen = 0;
                }
                else
                {//车库有旧车，发车数量也达到要求后，要考虑发旧车还是唤醒
                        chosen = VehicleSelector(park,endtime_ofday);
                }
                //动态调度时，可以在yueshu.max_carnum里限制最大车辆数
                if (chosen == -1 && result.carsused >= yueshu.max_carnum)
                {
                    chosen = -3;
                }
                //到此，chosen已定
                //////////////////////////////////////////////////////////////////////////
                park.chosen = chosen;
                //////////////////////////////////////////////////////////////////////////
                if (chosen == -1)//决定发新车
                {
                        chosencarx = result.carsused;
                        result.carsused++;
                        result.drivsused++;
                        //记录新车更新,更新slot节点信息
                        park.ifnewcar = 1;
                        park.ifnewdriver = 1;
                        park.carx = chosencarx;
                        //这一块更新车的排班队列
                        vector<Park*>ca;
                        tripque.push_back(ca);
                        tripque[chosencarx].push_back(&park);

                        if (nextrip_firstSlotY >= 0)//车入对面的车库
                        {
                                parks[nextrip_firstSlotY].Cars.push_back(
                                        Car(chosencarx, 1, 0, park.tp.Number,
                                                yueshu.max_drivetime, yueshu.max_worktime, yueshu.max_tripnum,
                                                triptime, partrest, 1, -1));
                        }
                }
                else if (chosen == -3)
                {
                        result.nocar_slotnum++;
                }
                else//用旧车
                {
                        Car* chosencar = &park.Cars[chosen];
                        chosencarx = chosencar->carx;
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        //这一块更新车的排班队列tripque 
                        switch (chosencar->state)
                        {
                        case 0://选的车是在工作的车
                                park.ifnewdriver = 0;
                                break;
                        case 1://选的车是在睡眠的车
                                switch (chosencar->type)
                                {
                                case 2:
                                        park.ifnewdriver = 1;
                                        if (chosencar->type == 1)//如果车在睡眠且还能继续加新司机, 成为长班车                                     
                                        {

                                        }
                                        break;
                                case 1:
                                        park.ifnewdriver = 1;
                                        if (chosencar->type == 1)//如果车在睡眠且还能继续加新司机,                                      
                                        {

                                        }
                                        break;
                                case 0:
                                        park.ifnewdriver = 0;//选中高峰车并唤醒
                                        break;
                                default:
                                        break;
                                }
                                break;
                        default:
                                break;
                        }
                        tripque[chosencarx].push_back(&park);//记录tripque更新                      
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        //记录新车更新,更新slot节点信息
                        park.ifnewcar = 0;
                        park.carx = chosencarx;
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        //到站后，还有后续时刻点，旧车入下个slot库
                        if (nextrip_firstSlotY >= 0)
                        {
                                Park* aimslo = &parks[nextrip_firstSlotY];
                                aimslo->Cars.push_back(*chosencar);//先把车开过去，再创建新车初始信息
                                Car* car = &aimslo->Cars.back();

                                if (chosencar->state == 1)//如果选中了在sleep的车，就在下一站唤醒(并压入新司机)
                                {
                                        //早早地把new司机资源压入本slot的chosencar内，策略将会与car的入库策略不同步。newcar不会压入本slot。
                                        //因此，本slot内的chosencar-driver不变，只在车和司机到下一站后才压入新司机，更新状态。简称，先把车开过去，再找新司机、把车唤醒。
                                        aimslo->Cars.back().weakup(park.tp.Number, 1, triptime, partrest);
                                }
                                else//如果选中的是正在work的车，就直接更新
                                {
                                        aimslo->Cars.back().update_drive(park.tp.Number, 1, triptime, partrest);
                                }

                        }
                }
                ///////////////////////////////////////////////////////////////////////////////////////////////////
                //将车库里剩的车放到本岸下个slot。 sleep peak车不算rest,其他算.
                if (park.tp.Nextrest_pointX >= 0)
                {
                        Park* next_restpark = & parks[park.tp.Nextrest_pointX];
                        int whole_rest = next_restpark->tp.tot - park.tp.tot;
                        for (int i = 0; i < park.Cars.size(); i++)
                        {
                                if (i != chosen)
                                {
                                        next_restpark->Cars.push_back(park.Cars[i]);
                                        Car* car = &next_restpark->Cars.back();

                                        switch (next_restpark-> Cars.back().state)
                                        {
                                        case 0://对于车库内在work的车,增加休息
                                                car->update_rest(park.tp.Number, whole_rest);
                                                break;

                                        case 1://对于车库内在sleep的车，高峰车不加休息时间，其他加。
                                                if (car->type == 0)//高峰车
                                                {

                                                }
                                                else if (car->type == 1)//休眠普通车
                                                {
                                                        next_restpark->Cars.back().update_rest(park.tp.Number, whole_rest);
                                                }
                                                break;

                                        default:
                                                cout << "car state error";
                                                break;
                                        }
                                }
                        }
                }
                else//本CP最后一个time point
                {

                }
                sorted++;
                return -1;
        }

        //将这个排班现场回溯到X位置调度之前。
        //重新构建现场,清理X后面的回溯记录
        void backtrack_rebuild(int toparkX)
        {
                vector<int> oldchoses;//复制原时刻点车库
                for (int i = 0; i < parks.size(); i++)//清除时要清理干净
                {
                        oldchoses.push_back(parks[i].chosen);
                        parks[i].clear();
                }               
                sorted = 0;

                tripque.clear();

                int backnum = result.backtrack_num;
                result = pb_result_shuxing();
                result.backtrack_num = backnum + 1;
                
                //一个个复制原来的动作
                //回溯到输入目标的前一瞬，不重复目标park的动作.
                for (int i = 0; i <= toparkX-1; i++)
                {
                        mover(parks[i], oldchoses[i]);
                }
                //清理X后面的回溯记录,因为环境已经变了
                for (int i = toparkX + 1; i < parks.size(); i++)
                {
                        parks[i].ShouldSleep_Cars.clear();
                }
        }

        //将这个排班现场回溯到X位置调度之前.不负责处理更新回溯信号。
        //删除之后的改动
        void backtrack_delet(int toparkX)
        {
                
        }

        pb_result_shuxing Schedule_parks(Schedule& oldPB)
        {
                int backtrack_to_parkx = -1;//-1不回溯，对应回溯parkx
                for (int pki = 0; pki < parks.size();)
                {
                        //阻塞时段不包括区间边界。所以在阻塞时段左边界的原方案park也要复制过来。
                        if (parks[pki].tp.tot < yueshu.breaktime)
                        {
                                backtrack_to_parkx = mover(parks[pki], oldPB.parks[pki].chosen);                               
                        }
                        else    
                        {
                                backtrack_to_parkx = mover(parks[pki], -1);
                        }         
                        //加上断点机制后，只从断点往后排班。与breaktime重合的slot也要重新调度
                        if (backtrack_to_parkx >= 0)//此时回溯到park[shouldsleep_parkx]，发的车改为去休眠或下班。
                        {
                                //cout <<"S_ID-"<< ScheduleID << " tracks " << result.backtrack_num + 1 <<
                                //        ",Car["<< parks[backtrack_to_parkx].ShouldSleep_Cars.back() << "] backto CP:"
                                // << backtrack_to_parkx << " from:" << pki << "->" 
                                //        << parks[backtrack_to_parkx].ShouldSleep_Cars.back() << ",shoule sleep cars: ";                             
                                //for (int c = 0; c < parks[backtrack_to_parkx].ShouldSleep_Cars.size(); c++)
                                //{
                                //        cout << parks[backtrack_to_parkx].ShouldSleep_Cars[c] << ",";
                                //}
                                //cout << endl;
                                
                                backtrack_rebuild(backtrack_to_parkx);
                                
                                pki = backtrack_to_parkx;
                        }
                        else
                        {
                                pki++;
                        }
                }
                /////////////////////////////////////////////////////////////////////////////////
                //排完班后统计结果参数，得出punish
                for (int i = 0; i < tripque.size(); i++)//i是车号
                {
                        ////////////////////////////////////下面是工作统计
                        Park* last_trip = tripque[i].back();
                        int chosen = last_trip->chosen;
                        if (chosen >= 0)
                        {
                                Car* chocar = &last_trip->Cars[chosen];
                                //
                        }
                        if (tripque[i].size() % 2 == 1)
                        {
                                result.unback_drivernum++;
                        }
                        if (tripque[i].size() > yueshu.max_tripnum)
                        {
                                result.drivsused++;
                        }
                        ////////////////////////////////////////////////////////
                        //for (int k = 1; k < tripque[pki].size(); k++)//k是trip
                        //{
                        //        Slot* slo = tripque[pki][k];
                        //        Car* chocar = &slo->Cars[slo->chosen];
                        //        int restcha = 4 - chocar->resthis;//休息时间均衡性部分
                        //        if (restcha <= 0)
                        //        {
                        //                restcha = 0;
                        //        }
                        //        else
                        //        {
                        //                result.buisy_restnum++;
                        //        }
                        //}
                }
                result.scorer();
                return result;
        }
};