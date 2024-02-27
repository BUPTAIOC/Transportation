#pragma once
#include "timepoint.h"
#include "onecase.h"
//int printshow4(
//        vector<vector<timeslot>>& TimeTable,
//        vector<int>& trip_time_map, 
//        paiban4* a,
//        int p,
//        int num
//)
//{
//        printf("\n arrange%d, carsused=%d,arrange: \n", p, num);
//
//
//
//        for (int i = 0; i < a.tripqueSlotY.size(); i++)
//        {
//                int xa = a.tripqueTableX[i][0];//xaya是前slot的xy
//                int ya = a.tripqueSlotY[i][0];
//                int oldtime = TimeTable[xa][ya].tot;
//                int nowtriptime = trip_time_map[oldtime];
//                printf("car%d from%d:\t ", i, oldtime);
//
//                int xb;//xbyb是后slot的xy
//                int yb;
//                int newtime;
//                int restime;
//
//                int lastchosen;
//                int resthis;
//                float restall;
//                float driveall1 = nowtriptime;
//                float driveall2;
//
//                for (int k = 1; k < a.tripqueSlotY[i].size(); k++)
//                {
//                        xb = a.tripqueTableX[i][k];//xbyb是后slot的xy
//                        yb = a.tripqueSlotY[i][k];
//                        newtime = TimeTable[xb][yb].tot;                   
//                        restime  = newtime - oldtime - nowtriptime;
//
//                        lastchosen = a.slos[xb][yb].lastchosen;
//                        resthis = a.slos[xb][yb].cars[lastchosen].resthis;
//                        restall = a.slos[xb][yb].cars[lastchosen].restime;
//
//
//                        printf("[%d] ", restime);
//
//                        xa = xb;
//                        ya = yb;
//                        oldtime = TimeTable[xa][ya].tot;
//                        nowtriptime = trip_time_map[oldtime];
//
//                        driveall2 = a.slos[xb][yb].cars[lastchosen].drivetime + nowtriptime;
//                        driveall1 += nowtriptime;
//                }
//                float workall = TimeTable[xb][yb].tot + trip_time_map[TimeTable[xb][yb].tot] - TimeTable[a.tripqueTableX[i][0]][a.tripqueSlotY[i][0]].tot;
//                printf("----------------tripnum[%d]rest[%.1f]drive1[%.1f]drive2[%.1f]work[%.1f]\n", a.tripqueSlotY[i].size(), restall/60,driveall1/60, driveall2 / 60, workall/60);
//        }
//        return 0;
//}
//
//int printshow5(
//        vector<vector<timeslot>>& TimeTable,
//        vector<int>& trip_time_map,
//        paiban5& a,
//        int p,
//        int num
//)
//{
//        printf("\n arrange%d, carsused=%d|%d, drivsused=%d: \n", p, num, a.carsused, a.drivsused);
//
//        int carx1;//1指的从tripque[car][who][trip]->(slot) 中得到排班数据.
//        int driver1;//注意，上式指向的slot是车和司机的出发slot。这个slot的cars内并没有保存此车和司机的信息。
//        int drivall1;//而到对岸后可能直接下班，因而对岸cars内也没有保存车和司机的信息。
//        int fromwhen1;
//        int restall1;
//        int workall1;
//
//
//        int carx2;//2指的从car[lastchosen]->driver 中得到排班数据
//        int driver2;
//        int drivetime2;
//        int fromwhen2;
//        int restime2;
//        int worktime2;
//
//        int linenum = 1;
//        for (int i = 0; i < a.tripque.size(); i++)//第i辆车
//        {
//                for (int j = 0; j < a.tripque[i].size(); j++)//第j个司机
//                {
//                        int oldtime = a.tripque[i][j][0]->tot;
//                        int oldtriptime = trip_time_map[oldtime];
//
//                        carx1 = i;//1指的从tripque[car][who][trip]->(slot) 中得到排班数据
//                        driver1 = j;
//                        drivall1 = oldtriptime;
//                        fromwhen1 = oldtime;
//                        restall1 = 0;
//                        workall1 = oldtriptime;
//
//                        printf("car[%d]dri[%d]:\t[%d]", i, j, oldtriptime);//第一个nowtriptime
//                        for (int k = 1; k < a.tripque[i][j].size(); k++)//第k个trip
//                        {
//                                //int nowchosen = a.tripque[i][j][k]->chosen;
//                                //Car* chosenCar = &a.tripque[i][j][k]->Cars[nowchosen];
//                                //driver* nowdriver = &chosenCar->drivers[j];
//
//
//                                int nowtime = a.tripque[i][j][k]->tot;
//                                int resthis1 = nowtime - oldtime - oldtriptime;  restall1 += resthis1; workall1 += resthis1;
//                                //int resthis2 = nowdriver->resthis;//符合restime2                         
//                                //printf(" (%d|%d) ", resthis1, 0);// resthis2);
//
//
//                                int nowtriptime = a.tripque[i][j][k]->triptime; drivall1 += nowtriptime; workall1 += nowtriptime;
//                                //printf("[%d]", nowtriptime);
//
//
//                                oldtime = nowtime;
//                                oldtriptime = nowtriptime;
//                        }
//                        /////////////////////////////////////////////////////////////////////////////////////
//                        //2指的从car[lastchosen]->driver 中得到排班数据
//                        int lastchosen = a.tripque[i][j].back()->chosen;
//                        int lastriptime = a.tripque[i][j].back()->triptime;
//
//                        if (lastchosen == -1)//始发车不在cars库里
//                        {
//                                carx2 = i;
//                                driver2 = -1;
//                                drivetime2 = lastriptime;
//                                fromwhen2 = a.tripque[i][j].back()->tot;
//                                restime2 = 0;
//                                worktime2 = lastriptime;
//                        }
//                        else
//                        {
//                                carx2 = a.tripque[i][j].back()->Cars[lastchosen].driv()->carx;
//                                driver2 = a.tripque[i][j].back()->Cars[lastchosen].driv()->id;
//                                drivetime2 = a.tripque[i][j].back()->Cars[lastchosen].driv()->drivetime + lastriptime;
//                                fromwhen2 = a.tripque[i][j].back()->Cars[lastchosen].driv()->fromwhen;
//                                restime2 = a.tripque[i][j].back()->Cars[lastchosen].driv()->restime;
//                                worktime2 = a.tripque[i][j].back()->Cars[lastchosen].driv()->worktime + lastriptime;
//                        }
//                        //////////////////////////////////////////////////////////////////////////////////////
//
//                        printf("line%d\t>>> ", linenum);
//                        printf("car[%d]\t", carx2);
//                        printf("who[%d]\t", driver2);
//                        printf("tripnum[%d]\t", int(a.tripque[i][j].size()));
//                        printf("rest[%.1f|%.1f]\t", float(restall1) / 60, float(restime2) / 60);
//                        printf("drive[%.1f|%.1f]\t", float(drivall1) / 60, float(drivetime2) / 60);//drive12相同
//                        printf("work[%.1f|%.1f]\n", float(workall1) / 60, float(worktime2) / 60);
//
//                        linenum++;
//                }
//
//        }
//        return 0;
//}