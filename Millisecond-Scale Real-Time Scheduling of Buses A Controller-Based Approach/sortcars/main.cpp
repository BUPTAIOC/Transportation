// �Ű�2.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
#pragma once
#include "setting.h"
#include <QtWidgets/QApplication>

#include <stdlib.h>
#include <time.h> 

using namespace std;

//��ӡ�ص��Ƚ������ͳ��
void print_result_sts(string fname, Schedule pb)
{
    ofstream oFile;

    oFile.open(fname, ios::out | ios::app);//

    oFile << "," << "cars" << "," << pb.result.carsused
        << "," << "dirs" << "," << pb.result.drivsused
        << "," << "emps" << "," << pb.result.nocar_slotnum
        << "," << "evens" << "," << pb.result.unback_drivernum;

    oFile << endl;
}

int main(int argc, char *argv[])
{       
        QApplication app(argc, argv);
        /////////////////////////////////////////////////////////////////////////////////////////��̬����
        Setting setting;
        onecase case0(
                    setting.line_name,
                    setting.TimeTable,
                    setting.yueshu,
                    setting.S);
        MainWindow w0(case0);
        w0.show();
        /// //////////////////////////////////////////////////////////////////////////////////
        int res_mod = -1;
        int MaxCar_num = 0;//���ó�����
        int orig_cars = case0.pbs[case0.pbi_ofscoreth[0]].result.carsused;
        for (MaxCar_num = orig_cars + 0; MaxCar_num <= orig_cars + 10; MaxCar_num++)
        {
            string fname = case0.line_name + "/RES_result_congestion_MAXcars[" + to_string(MaxCar_num) + "].csv";
            auto data = readDataFromFile("random_congestions.txt");//ģ�����ӵ�²����ͳ�ƽ��
            ofstream oFile; oFile.open(fname, ios::out | ios::trunc); oFile.close();//�������ļ�
            for (int i = 0; i < data.size(); i++)
            {
                /////////////////////////////////////////////////////////////////////////////////////////////ģ��ͻ�������1���Ű�
                onecase case1(case0);
                if (data[i].size() >= 1) {
                    case1.change_triptime_blocked(std::get<0>(data[i][0]), std::get<1>(data[i][0]), std::get<2>(data[i][0]));
                    case1.resort_oldcase(res_mod, MaxCar_num);
                    //MainWindow w1(case1);
                    //w1.show();
                }
                if (data[i].size() == 1) {
                    print_result_sts(fname, case1.pbs[case1.pbi_ofscoreth[0]]); //case0.pbs.push_back(case1.pbs[case1.pbi_ofscoreth[0]]); 
                    continue;
                }
                ///////////////////////////////////////////////////////////////////////////////////////////////ģ��ͻ�������2���Ű�
                onecase case2(case1);
                if (data[i].size() >= 2) {
                    case2.change_triptime_blocked(std::get<0>(data[i][1]), std::get<1>(data[i][1]), std::get<2>(data[i][1]));
                    case2.resort_oldcase(res_mod, MaxCar_num);
                    //MainWindow w2(case2);
                    //w2.show();
                }
                if (data[i].size() == 2) {
                    print_result_sts(fname, case2.pbs[case2.pbi_ofscoreth[0]]); //case0.pbs.push_back(case2.pbs[case2.pbi_ofscoreth[0]]);
                    continue;
                }
                ///////////////////////////////////////////////////////////////////////////////////////////////ģ��ͻ�������3���Ű�
                onecase case3(case2);
                if (data[i].size() >= 3) {
                    case3.change_triptime_blocked(std::get<0>(data[i][2]), std::get<1>(data[i][2]), std::get<2>(data[i][2]));
                    case3.resort_oldcase(res_mod, MaxCar_num);
                    //MainWindow w3(case3);
                    //w3.show();
                }
                if (data[i].size() == 3) {
                    print_result_sts(fname, case3.pbs[case3.pbi_ofscoreth[0]]); //case0.pbs.push_back(case3.pbs[case3.pbi_ofscoreth[0]]);
                    continue;
                }
            }
        }

         MaxCar_num = 100;//���ó�����
         //����ӵ�¹۲� /////////////////////////////////////////
        //onecase case1(case0);
        //case1.change_triptime_blocked(8.9666, 1.86563, 20);
        //case1.resort_oldcase(res_mod, MaxCar_num);
        //MainWindow w1(case1);
        //w1.show();
        ////// ����ӵ�¹۲�
        //onecase case2(case1);
        //case2.change_triptime_blocked(11.0726, 1.832744, 19);
        //case2.resort_oldcase(res_mod, MaxCar_num);
        //MainWindow w2(case2);
        //w2.show();
        //// //����ӵ�¹۲�
        //onecase case3(case2);
        //case3.change_triptime_blocked(18.6245, 1.623745, 14);
        //case3.resort_oldcase(res_mod, MaxCar_num);
        //MainWindow w3(case3);
        //w3.show();
        // /////////////////////////////////////////////////////


        res_mod = -2;
        MaxCar_num = 100;//���ó�����
        //auto data = readDataFromFile("random_vehicle_failure.txt");//ģ������������ϲ����ͳ�ƽ��
        //ofstream oFile; oFile.open(case0.line_name + "/RES_result_Vfailure.csv", ios::out | ios::trunc); oFile.close();//�������ļ�
        //for (int i = 0; i < data.size(); i++)
        //{
        //    /////////////////////////////////////////////////////////////////////////////////////////////ģ��ͻ�������1���Ű�
        //    onecase case1(case0);
        //    if (data[i].size() >= 1) {
        //        case1.change_triptime_vehicle_failure(std::get<0>(data[i][0]), std::get<1>(data[i][0]), std::get<2>(data[i][0]));
        //        case1.resort_oldcase(res_mod, MaxCar_num);
        //        //MainWindow w1(case1);
        //        //w1.show();
        //    }
        //    if (data[i].size() == 1) {
        //        print_result_sts(case1.line_name + "/RES_result_Vfailure.csv", case1.pbs[case1.pbi_ofscoreth[0]]); //case0.pbs.push_back(case1.pbs[case1.pbi_ofscoreth[0]]); 
        //        continue;
        //    }
        //    ///////////////////////////////////////////////////////////////////////////////////////////////ģ��ͻ�������2���Ű�
        //    onecase case2(case1);
        //    if (data[i].size() >= 2) {
        //        case2.change_triptime_vehicle_failure(std::get<0>(data[i][1]), std::get<1>(data[i][1]), std::get<2>(data[i][1]));
        //        case2.resort_oldcase(res_mod, MaxCar_num);
        //        //MainWindow w2(case2);
        //        //w2.show();
        //    }
        //    if (data[i].size() == 2) {
        //        print_result_sts(case2.line_name + "/RES_result_Vfailure.csv", case2.pbs[case2.pbi_ofscoreth[0]]); //case0.pbs.push_back(case2.pbs[case2.pbi_ofscoreth[0]]);
        //        continue;
        //    }
        //    ///////////////////////////////////////////////////////////////////////////////////////////////ģ��ͻ�������3���Ű�
        //    onecase case3(case2);
        //    if (data[i].size() >= 3) {
        //        case3.change_triptime_vehicle_failure(std::get<0>(data[i][2]), std::get<1>(data[i][2]), std::get<2>(data[i][2]));
        //        case3.resort_oldcase(res_mod, MaxCar_num);
        //        //MainWindow w3(case3);
        //        //w3.show();
        //    }
        //    if (data[i].size() == 3) {
        //        print_result_sts(case3.line_name + "/RES_result_Vfailure.csv", case3.pbs[case3.pbi_ofscoreth[0]]); //case0.pbs.push_back(case3.pbs[case3.pbi_ofscoreth[0]]);
        //        continue;
        //    }
        //}
        //MaxCar_num = 2;//���ó�����
        //// ���ι��Ϲ۲� /////////////////////////////////////////
        //onecase case1(case0);
        //case1.change_triptime_vehicle_failure(8.4625, 0, 453);
        //case1.resort_oldcase(res_mod, MaxCar_num);
        //MainWindow w1(case1);
        //w1.show();
        //// ���ι��Ϲ۲�
        //onecase case2(case1);
        //case2.change_triptime_vehicle_failure(13.6485, 0, 279);
        //case2.resort_oldcase(res_mod, MaxCar_num);
        //MainWindow w2(case2);
        //w2.show();
        //// ���ι��Ϲ۲�
        //onecase case3(case2);
        //case3.change_triptime_vehicle_failure(20.6245, 1.623745, 260);
        //case3.resort_oldcase(res_mod, MaxCar_num);
        //MainWindow w3(case3);
        //w3.show();
         /////////////////////////////////////////////////////
 

        ///////////////////////////////////////////////////////////////////////////////////////////////ģ������г�ʱ��
        //for (int i = 0; i < 10; i++)
        //{
        //        case3.change_triptime_simprand_triptime(0.1);
        //        case3.resort_oldcase(0);
        //        case3.pbi_ofscoreth.push_back(i + 1);
        //        case3.scoreth_ofpbi.push_back(i + 1);
        //}
        //MainWindow w4(case3);      
        //w4.show();
        /////////////////////////////////////////////////////////////////////////////////////////////

        /*
        rbf��Ϻ���
        Swa S1 = Swa(100, 200, 0.8, 0.3, 0.3);//��ʼ��������epoch��oumiga�����������������Ⱥ�������
        hiden_layer H;
        int x_range = 10;
        H.add_input_size(0, x_range, x_range / 10);
        H.add_input_size(0, x_range, x_range / 10);
        H.set_inode_numb(10, S1);
        //S1.cin_new_d(-10, 10, 2);//yueshu.P_used.ds[0]//tripnum
        S1.init_swa();
        double a = 1.0;//����Ŀ�꺯����z = ax+by+c
        double b = -1.5;
        double c = -5;
        //int point_num = 10;
        for (int i = 0; i < S1.Max_epoch; i++)////////////////////////////////////////�Ű���ѭ��
        {
            for (int j = 0; j < S1.grs.size(); j++)
            {
                S1.grs[j].ps[i].punish = 0;

                for (int point = 0; point < x_range * x_range; point++)
                {
                    double x = double(point)/ x_range;
                    double y = double(point % x_range);

                    vector <double> inputs; inputs.push_back(x); inputs.push_back(y);

                    //P* p_used = &S1.grs[j].ps[i];
                    //double pso_out =  ((p_used->ds[0].val() * x*x) + p_used->ds[1].val() * x + p_used->ds[2].val()); 

                    double rbf_out = H.out_tot(S1.grs[j].ps[i], inputs);
                    double real_out = ((a * x) + b * y + c);
                    double squ_pun = abs(rbf_out - real_out);// *abs(rbf_out - real_out) / point_num;
                    S1.grs[j].ps[i].punish += squ_pun * squ_pun / 100;// point_num;

                    cout << "";
                }
            }

            S1.get_next_ps(i);
            printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f\n", i, S1.sw_avgPunish(i), S1.sw_bestPunish());
        }*/
        //ofstream oFile;
        //oFile.open("rbffunc_line.csv", ios::out | ios::trunc);//
        //for (int point = 0; point <= point_num; point++)
        //{
        //    double x = double(point * x_range) / point_num;
        //    vector<double> inputs;
        //    inputs.push_back(x);
        //    double rbfy = H.best_out_tot(S1, inputs);
        //    oFile << "rbf line:" << "," << x << "," << rbfy << endl;//����š�epoch���������
        //}
        //oFile.close();
        //H.print_to_excel("pso",S1);
        return app.exec();
}