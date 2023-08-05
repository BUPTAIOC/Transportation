// Randomly_generate_congestions.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <random>
#include <fstream>

using namespace std;

int main()
{
    int instance_num = 100;

    //int cong_num_min = 1;//times
    //int cong_num_max = 3;//times

    //double congestion_start_min = 8.0;//hour
    //double congestion_start_max = 20.0;//hour

    //double cong_dur_min = 0.5;//hour
    //double cong_dur_max = 2;//hour

    //int trip_delay_min = 5; //minute
    //int trip_delay_max = 20;//minute

    //std::random_device rand;
    //std::mt19937 gen(rand());
    //ofstream oFile;
    //oFile.open("random_congestions.txt", ios::out | ios::trunc);//
    // /////////////////////////////////////////
    int cong_num_min = 1;//times
    int cong_num_max = 3;//times

    double congestion_start_min = 8.0;//hour
    double congestion_start_max = 20.0;//hour

    double cong_dur_min = 0;//hour
    double cong_dur_max = 0;//hour

    int trip_delay_min = 60; //minute
    int trip_delay_max = 600;//minute

    // /////////////////////////////////////////
    std::random_device rand;
    std::mt19937 gen(rand());
    ofstream oFile;
    oFile.open("random_vehicle_failure.txt", ios::out | ios::trunc);//

    for (int i = 0; i < instance_num; i++)
    {
        // Uniform distribution between 1 congestions and 3 congestions
        std::uniform_int_distribution<> cong_num(cong_num_min, cong_num_max);
        int num = cong_num(gen);
        std::cout << num <<endl;
        oFile << num << endl;

        std::vector<double> vec = {};
        for (int j = 0; j < num; j++)
        {
            // Uniform distribution between 8:00 and 20:00 
            std::uniform_real_distribution<> cong_start(congestion_start_min, congestion_start_max);
            double startTime = cong_start(gen);
            vec.push_back(startTime);
        }
        std::sort(vec.begin(), vec.end());

        for (int j = 0;j<num;j++)
        {
            std::cout << vec[j] << ",";
            oFile << vec[j] << ",";

            // Uniform distribution between 30 minutes and 120 minutes
            std::uniform_real_distribution<> cong_during_minute(cong_dur_min, cong_dur_max);
            double duration = cong_during_minute(gen);
            std::cout << duration << ",";
            oFile << duration << ",";

            // Uniform distribution between 5 minutes and 20 minutes
            std::uniform_int_distribution<> dis3(trip_delay_min, trip_delay_max);
            int travelTimeIncrease = dis3(gen);
            std::cout << travelTimeIncrease;
            std::cout << std::endl;
            oFile << travelTimeIncrease << endl;
        }
    }

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
