#pragma once
#include "timepoint.h"
#include "car.h"
// we call the Control point information at each moment as 'park', recording real-time information for each vehicle.
class Park
{
public:
        // Corresponding moment information and directly inferable information:
        Timepoint tp;
        // Current tp's in-station vehicle resource status:
        vector<Car> Cars;
        // For backtracking. It records vehicles that, upon backtracking, are found not to have been supposed to depart.
        // If the backtracking target moves forward, record directly at the target point the vehicles that should sleep.
        // If the backtracking target remains unchanged, add vehicles at the target point.
        // If the backtracking target moves backward, clear all records after the backtracking target.
        vector<int> ShouldSleep_Cars;
        // Determine whether the vehicle should go directly to sleep based on backtracking records.
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
        // Whether a new car was dispatched in this timepoint, 0/1
        int ifnewcar;
        // Whether a new driver started work in this timepoint
        int ifnewdriver;
        // The id of the car dispatched in this timepoint, starting from 0, -1 indicates no car was dispatched.
        int carx;
        int driverx;
        // The chosen one at that time
        int chosen;
        // How many new cars were used before scheduling for this timepoint
        int carsused;
        int driverused;

        int endt()
        {
                return tp.time + tp.triptime;
        }

        Park(Timepoint & intp)
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