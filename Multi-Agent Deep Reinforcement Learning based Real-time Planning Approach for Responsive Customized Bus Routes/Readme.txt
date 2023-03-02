1. Each of the two road networks (Sioux and Major) has a folder, which contains two files. Take the folder "Sioux" as an example. It has two files, i.e. sioux_stops.csv and sioux_time.csv. The sioux_stops.csv stores station locations of the sioux_0 (sioux_1) CB network, and the sioux_time.csv file stores the travel time distribution of the sioux_0 (sioux_1) CB network.

2. Python main code "instance.py" is a demo to call the files in folders "Sioux" and "Major" to generate training, validation and test data sets for the three CB netwoks, respectively.

3. Detailed notes on how to generate data sets are given in "instances.py". In addition to the above Sioux_0, Sioux_1 and Major CB problem instances, researchers can generate more new CB problem instances according to their needs.

4. The main code with comments will be provided later. The guidance of the selection of coefficients , i.e Guidance_coefficients.txt, details how to define the coefficients for the return function.
