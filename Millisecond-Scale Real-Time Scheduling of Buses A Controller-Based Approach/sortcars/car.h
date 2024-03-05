#pragma once
#include "setting.h"
using namespace std;

//
class Car
{
public:
        //Vehicle number, starting from 0.
        int carx;
        //Current work state of vehicles and drivers: 0 work; 1 sleep.
        int state;
        // Vehicle type: long type 2, short type 1, peak type 0. 
        // Initially all are 1, change to 0 if they sleep, 
        // change to 2 after normal sleep and being awakened.
        int type;

        // It is updated by which timepoint, used for backtracking to restore the scene.
        int fromparkx;
        // When to start sleeping, measured in minutes. -1 represents never slept.
        int whenslep;
        // current trip num
        int tripnum;
        // maximum trip num
        int tripmax;
        // remaining trip num
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
        // driving time and the upper limit.
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
        // The total rest time that has been taken.
        int restime;
        // How long the current rest period has lasted.
        int resthis;
        // The total time from departure to the present and the upper limit.
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
        // Determine that this working vehicle can no longer be converted to a long-route bus. 
        // Therefore, it cannot sleep to a long type.
        int cannot_convert_to_long_vehicle(int triptime)
        {
                switch (type)
                {
                case 0://peak type
                        return 1;
                        break; 
                case 1://short type
                        return 0;
                        break;
                case 2://long type
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
        void update_rest(int infromx, int restplus)//The total rest time is only updated after a trip.
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

                restime += partrest;

                drivetime += driveplus;

                //worktime += driveplus + partrest;  
                //worktime = restime + drivetime;

                resthis = partrest;// Recalculate the duration of a single rest.
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////
        // Put the vehicle into sleep state peak type.
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
        // Put the vehicle into sleep long type.
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
        // Wake up the vehicle, convert the vehicle state.
        void weakup(int infromx, int intriplus, int indrive_plus, int inrest_plus)
        {
                state = 0;

                if (type == 2)// The long type bus can no longer be awakened.
                {
                        cout << carx << "can not wake up" << endl;
                }

                if (type == 1)// Wake up the normal vehicle, indicating it becomes a long type bus.
                {
                        type = 2;
                        // change informations as a long type
                        tripmax *= 2;
                        drivemax *= 2;
                        workmax *= 2;
                        update_drive(infromx, intriplus, indrive_plus, inrest_plus);
                }

                if (type == 0) // Wake up the peak vehicle.
                {
                        update_drive(infromx, intriplus, indrive_plus, inrest_plus);
                }
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////////////////
        // The guiding scoring function when choosing a car.
        // It returns the score (weight) of the vehicle as the departing vehicle for the next moment.
        // Use the guiding scoring function of the vehicle's remaining working time / remaining number of trips. 
        // Return the weight of the vehicle being selected at this moment.
        double simple_scorer(
                int endtime_ofday,// The arrival time of the last vehicle before the end of the day.
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
                if (state == 0)// The vehicle in working state.
                {
                        int tripleft = Tripleft();
                        int workleft = Workleft();// Remaining working time.

                        if (workmax - worktime() > endtime_ofday - nowtime)//near the end of the day, the recorded worktime may not be accurate.
                        {
                                workleft = endtime_ofday - nowtime;
                        }
                        int wateleft = 1 + workleft - triptime;//Remaining rest time. Another approach is not to consider the triptime.
                        if (wateleft <= 0)
                        {
                                cout << "wateleft <=0 error" << endl;
                                wateleft = 1;
                        }

                        double trips_pear_min = double(tripleft) / double(wateleft);

                        //trips_pear_min^2*(resthis^2 + 100*IFlastrip) This is a very effective weight formula.
                        score = 100 * pow(trips_pear_min, pow1)*(pow(resthis, pow2) + Wplus_backhome * Y + 1) ;
                        //score = 100 * (trips_pear_min + Wplus_backhome * Y + 1) * pow(resthis, pow2);//(The formula in the review comments)

                        //cout << score << endl;
                        if (score <= 0)
                        {
                                cout << "score <=0 error"<<endl;
                        }
                }
                else// The vehicle in sleep mode.
                {
                        int emptime = nowtime - whenslep;

                        if (aweak_carnum > 0)// There are other vehicles in operation (working).
                        {
                                score = 0;
                        }
                        else if (sleeptype1_carnum > 0)// Only sleep cars left, including sleep short cars + sleep peak cars.
                        {
                                if (type == 2)//sleep long
                                {
                                        score = 0;
                                        cout << "Wrong! should not have sleeping long car" << endl;
                                }
                                else if (type == 1)//sleep short
                                {
                                        score = 2 + 2 * pow(emptime, 2);
                                }
                                else if (type == 0)
                                {
                                        score = 0; // Peak-time vehicles are for emergency use and should not be used lightly.   
                                }
                        }
                        else if (sleeptype2_carnum > 0)//ֻʣsleep peak car
                        {
                                if (type == 0)
                                {
                                        score = 1 + 1 * pow(emptime, 2); // Peak-time vehicles are for emergency use.     
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