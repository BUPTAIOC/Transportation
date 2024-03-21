### Bus Scheduling Problem (BSP) instances

This repository contains 3 real-world BSP instances in Qingdao city, China, which is used in our paper:

> **《Multi-agent path Planning based Scheduling Approach for bus lines》**

> There are two main parts：
>
> 1. **Information of three real-world BSP instances**
> 2. **Timetable of 40 generated instances**

1. ##### Information of three real-world BSP instances

| Bus lines                                  | 60    | 85   | 70    |
| ------------------------------------------ | ----- | ----- | ----- |
| Earliest departure time                    | 5:50  | 5:30  | 5:30  |
| Latest departure time                      | 22:00 | 23:00 | 23:01 |
| Number of departure times in the timetable | 120   | 170   | 306   |
| Maximum working time of short vehicle (h)  | 8     | 8     | 8     |
| Maximum driving time of short vehicle (h)  | 6.5   | 6.5   | 6.5   |
| Maximum number of trips of long vehicle    | 12    | 16    | 16    |
| Minimum rest time (min)                    | 3     | 3     | 3     |
| Maximum rest time (min)                    | 180   | 180   | 180   |

2. ##### Timetable of 40 generated instances

Each line has a folder for its data, there are three files in the folder:

| Filename        | Content                                                      |
| --------------- | ------------------------------------------------------------ |
| timetable.txt   | Timetable of control point 1, each row corresponds to a departure time |
| timetable_2.txt | Timetable of control point 2, each row corresponds to a departure time |
| tmessage.txt    | The travel time of vehicles of 24 hours in a day             |
