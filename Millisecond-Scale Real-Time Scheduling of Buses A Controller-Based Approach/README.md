# visual studio 2022, open "sortcars.sln"

set the bus line and constrains in "setting.h"

run the code and the results are shown in:
sortcars/lineX/best_scheme.csv   (original scheduling scheme)
sortcars/lineX/best_reschedulingMOD-1_scheme.csv   (rescheduling scheme after traffic jam based on best_scheme.csv, traffic jam settings are in main.cpp)
sortcars/lineX/best_reschedulingMOD-2_scheme.csv   (rescheduling scheme after vehicle failure based on best_scheme.csv, settings are in main.cpp)

if you want to add a new bus line, add a floder: sortcars/lineNew, and add TimeTable_merged.csv, tmessage.txt, then set linename in setting.h to lineNew

Timetable for lineNew is in file sortcars/lineX/TimeTable_merged.csv
one line in TimeTable_merged.csv like "5:50       0      1     70" means: a timepoint start 5:50, from CP0 to CP1, trip time is 70 min

constrains for lineNew is in file sortcars/lineX/tmessage.txt