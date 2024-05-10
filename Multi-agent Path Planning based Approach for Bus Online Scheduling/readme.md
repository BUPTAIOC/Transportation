### Bus Scheduling Problem (BSP) instances

This repository contains 3 real-world BSP instances in Qingdao city, China, which is used in our paper:

> **《Multi-agent Path Planning based Approach for Bus Online Scheduling》**

There are two main parts：
>
> 1. **Information of three real-world BSP instances**
> 2. **Timetable of 3 real-world and 40 generated BSP instances**

#### 1. Information of three real-world BSP instances

| Bus lines                                  | 60    | 85   | 70    |
| ------------------------------------------ | ----- | ----- | ----- |
| Earliest departure time                    | 5:50  | 5:30  | 5:30  |
| Latest departure time                      | 22:00 | 23:00 | 23:01 |
| Number of departure times in the timetable | 120   | 170   | 306   |
| Maximum working time of short bus (h)      | 8     | 8     | 8     |
| Maximum driving time of short bus (h)      | 6.5   | 6.5   | 6.5   |
| Maximum number of trips of short bus       | 6     | 8     | 8     |
| Maximum number of trips of long bus        | 12    | 16    | 16    |
| Minimum rest time (min)                    | 3     | 3     | 3     |
| Large interval threshold (min)             | 180   | 180   | 180   |

The file `tm.json` contains the information above.

#### 2. Timetable of 3 real-world and 40 generated BSP instances

##### 2.1 Real-world BSP instances

In folder `real-world`, each line(`L`) has 3 files for its data:
| Filename        | Content                                                      |
| --------------- | ------------------------------------------------------------ |
| timetable{`L`}_1.txt | Timetable of control point 1, each row corresponds to a departure time |
| timetable{`L`}_2.txt | Timetable of control point 2, each row corresponds to a departure time |
| duration{`L`}.txt    | The mean travel time of vehicles of 24 hours in a day             |

##### 2.2 Generated BSP instances

In folder `generated`:
- The folder `line60` contains 10 instances generated from the real-world line60, 21 files in total.
- The folder `line85` contains 10 instances generated from the real-world line85, 21 files in total.
- The folder `line70` contains 20 instances generated from the real-world line70, 41 files in total.

Each foldler `line{L}` contains 3 kinds of files:
| Filename        | Content                                                      |
| --------------- | ------------------------------------------------------------ |
| timetable{`L`}-{`I`}_1.txt   | Timetable of control point 1 in instance `I`, each row corresponds to a departure time |
| timetable{`L`}-{`I`}_2.txt | Timetable of control point 2 in instance `I`, each row corresponds to a departure time |
| duration{`L`}.txt    | The mean travel time of vehicles of 24 hours in a day             |
