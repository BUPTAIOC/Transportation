#pragma once
#include "onecase.h"

using namespace std;


/**/
//dynamic vehicle scheduling based on case0
int main(int argc, char* argv[])
{
    //////////////////////////////////////////////////////////////////////////////////////
    Setting settings;
    onecase case0(settings);//case0 is the original scheduling scheme

    int MaxCar_num = 999;//when you want to limit the car num used not exceed case0 cars used, int MaxCar_num = 25 for example;
    
    onecase case1(case0);
    //trafic jam start 10:00, during 1.5 hours, trips start at [10:00-11:30] add 18 minutes triptime
    //in file "random_congestions.txt", each line represent a period of traffic jam.
    int reschduling_mod = -1;
    case1.change_triptime_blocked(10.00, 1.5, 18);
    case1.rescheduling_oldcase(reschduling_mod, MaxCar_num);

    onecase case2(case0);
    //vehicle failure start at 13:00, one trip colsest after 13:00 add 200 minutes triptime
    //in file "random_vehicle_failure.txt", each line represent one vehicle failure.
    reschduling_mod = -2;
    case2.change_triptime_vehicle_failure(13.0, 0, 200);
    case2.rescheduling_oldcase(reschduling_mod, MaxCar_num);
    /// //////////////////////////////////////////////////////////////////////////////////
    return 0;
}