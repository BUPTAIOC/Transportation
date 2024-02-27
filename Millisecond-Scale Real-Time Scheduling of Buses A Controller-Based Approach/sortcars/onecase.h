#pragma once
//#include "excel.h"
#include "schedule.h"
#include <fstream>
//ָ���ĳ�ֿͻ�Ҫ���£�����ĳ���Ż������õ����Ű෽�����ϡ�����ӵ��ĳЩ���ʣ����ٵĳ������ȡ�
class onecase
{
public:
        string line_name;

        pb_before_yueshu yueshu;

        vector<timepoint> TimeTable;

        Swa S;

        Schedule oldPB;//����з����ϵļ̳У���ô�ѷ��������洢������
        vector<Schedule> pbs;

        //pbi�ĵ÷�����ʲô����
        vector<int> scoreth_ofpbi;
        //��i����pb��
        vector<int> pbi_ofscoreth;
        //pbi�ĵ÷�����ʲô����
        vector<int> scoreth_of_broken_pbi;
        //��i����pb��
        vector<int> pbi_of_broken_scoreth;

        /////////////////////////////////////////////////////
        void TimeTable_make_link()
        {
                //�ҵ�����վ֮����׸�park,����������arrivetime��park
                //�պ��ڷ���slotʱ�̵�վʱ�������ش�slot
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        TimeTable[i].nextrip_firstParkX = -1;

                        int arrivetime = TimeTable[i].tot + TimeTable[i].triptime + yueshu.atlist_one_rest;

                        for (int j = i + 1; j < TimeTable.size(); j++)
                        {
                                if (TimeTable[j].tot > arrivetime &&
                                        TimeTable[j].fromCP == TimeTable[i].gotoCP)
                                {
                                        TimeTable[i].nextrip_firstParkX = j;
                                        break;
                                }
                        }
                }
                //ͬCP�ĺ�һ��slotX
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        TimeTable[i].Nextrest_pointX = -1;

                        for (int j = i + 1; j < TimeTable.size(); j++)
                        {
                                if (TimeTable[i].fromCP == TimeTable[j].fromCP)
                                {
                                        TimeTable[i].Nextrest_pointX = j;
                                        break;
                                }
                        }
                }
        }

        //��ӡ��������
        void print_to_excel(string fname, Swa& S, vector<Schedule>& pbs)
        {
                ofstream oFile;

                oFile.open(fname + ".csv", ios::out | ios::trunc);//

                for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////�Ű���ѭ��
                {
                        int total_back = 0;
                        for (int j = 0; j < S.gr_num; j++)
                        {
                                oFile << "No epo p:" <<","<<i* S.Max_epoch + j + 1 <<"," << i + 1 << "," << j + 1<< ",";//����š�epoch���������
                                oFile << S.grs[j].ps[i].punish << ",";
                                if (j == S.grs.size() - 1)
                                {
                                        //printf("i:%d %d\t", i*S.grs.size() + j, S.grs[j].ps[i].punish);
                                        oFile << S.sw_avgPunish(i);
                                }
                                else
                                {
                                        //printf("i:%d %d\t", i*S.grs.size() + j, S.grs[j].ps[i].punish);
                                        oFile << "";
                                }

                                for (int k = 0; k < S.p_standard.ds.size(); k++)
                                {
                                        //printf("%.2f\t", S.grs[j].ps[i].ds[k].value);
                                        if (S.grs[j].ps[i].punish == 12000)
                                        {
                                                oFile << "," << S.grs[j].ps[i].ds[k].value << "," << S.grs[j].ps[i].ds[k].value;
                                        }
                                        else
                                        {
                                                oFile << "," << S.grs[j].ps[i].ds[k].value << "," << "";
                                        }                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               
                                }

                                total_back += pbs[i * S.gr_num + j].result.backtrack_num;
                                if (j == S.grs.size() - 1)
                                {
                                        //printf("avg punish: %d\t", totpunish / S.grs.size());
                                        oFile << "," << "," << pbs[i * S.gr_num + j].result.backtrack_num << "," << double(total_back) / S.gr_num;
                                }
                                else
                                {
                                        oFile << "," << "," << pbs[i * S.gr_num + j].result.backtrack_num << "," << "";
                                }
                                //printf("\n");
                                oFile << endl;
                        }
                }
                oFile.close();
        }
        //��ӡ���ŷ���
        void print_scheme(string fname, Schedule pb)
        {
            ofstream SchedulFile;//����Ű���
            SchedulFile.open(fname, ios::out | ios::trunc);//
            for (int car_i = 0; car_i < pb.tripque.size(); car_i++)//i�ǳ���
            {
                SchedulFile << pb.tripque[car_i][0]->tp.tot - pb.tripque[0][0]->tp.tot + 1;
                SchedulFile << " ";
                SchedulFile << pb.tripque[car_i][0]->tp.tot;
                SchedulFile << " ";
                SchedulFile << pb.tripque[car_i][0]->tp.fromCP;
                SchedulFile << " ";
                for (int k = 0; k < pb.tripque[car_i].size(); k++)//k��trip
                {
                    SchedulFile << pb.tripque[car_i][k]->tp.tot;
                    SchedulFile << " ";
                    SchedulFile << pb.tripque[car_i][k]->tp.triptime;
                    if(k < pb.tripque[car_i].size()-1)SchedulFile << " ";
                }
                SchedulFile << endl;
            }
            SchedulFile.close();
        }
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
        /////////////////////////////////////////////////////
        //��ʼ�Ű�����ѭ��
        onecase(string line,
                vector<timepoint>& inTimeTable,
                pb_before_yueshu inyueshu,
                Swa& inS
        )
        {
                line_name = line;
                TimeTable = inTimeTable;
                yueshu = inyueshu;
                S = inS;

                TimeTable_make_link();

                Schedule pb(TimeTable,yueshu,-1);//��ʼ���Ŀ��Ű෽��
                for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////�Ű���ѭ��
                {
                        for (int j = 0; j < S.grs.size(); j++)
                        {
                                pbs.push_back(pb); pbs.back().clear(); pbs.back().ScheduleID = j;
                                pbs.back().yueshu.change_Particle(&S.grs[j].ps[i]);//������Ⱥ�������µ�Լ���ṹ��
                                S.grs[j].ps[i].punish = pbs.back().Schedule_parks(oldPB).punish;                             
                        }
                        S.get_next_ps(i);

                        printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f \n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                }

                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                /////////////////////////////////////////////////////������������
                for (int i = 0; i < pbs.size(); i++)
                {
                        int num = 0;
                        for (int j = 0; j < pbs.size(); j++)
                        {
                                if (pbs[i].result.punish > pbs[j].result.punish)
                                {
                                        num++;
                                }
                        }
                        scoreth_ofpbi.push_back(num);
                }
                ///////////////////////////////////////////////////////
                for (int i = 0; i < pbs.size(); i++)
                {
                        for (int j = 0; j < pbs.size(); j++)
                        {
                                if (scoreth_ofpbi[j] == i)
                                {
                                        pbi_ofscoreth.push_back(j);
                                }
                        }
                        //cout << i << " th:car " << pbi_ofscoreth[i] << endl;
                }
                print_scheme(line_name + "/best_scheme.txt", pbs[pbi_ofscoreth[0]]);//��ӡ�����Ű�
                print_to_excel(line_name + "/ofline_train", S, pbs);//�����ӹ켣�����ļ�
                cout << endl;
        }
        //�����ص����Ű໷��
        onecase(onecase& oldcase)
        {
                line_name = oldcase.line_name;
                yueshu = oldcase.yueshu;
                TimeTable = oldcase.TimeTable;
                S = oldcase.S;
                ///////////////////////////////////////////////////////////////////////
                //��ʼ���Ŀ��Ű෽��,�̳��ϴε��ȵ����ŷ���
                oldPB = oldcase.pbs[oldcase.pbi_ofscoreth[0]];
        }

        //����ǰ�г�ʱ��������²��� r% 
        void change_triptime_simprand_triptime(float r)
        {
                //����ʱ�̱�
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        int radis = int(r * TimeTable[i].triptime);
                        int rplus = rand() % (2*radis + 1);

                        TimeTable[i].triptime += rplus - radis;
                }
        }
        //���������г�ʱ����trip���ӷ��ӣ������ص���
        void change_triptime_blocked(double breakbeginH, double breaklenH, int triplusM)
        {
                //����ʱ�̱�
            int break_t = -1;
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        //����ʱ�β���������߽硣����������ʱ����߽��ԭ����parkҲҪ���ƹ�����
                        if (TimeTable[i].tot > 60 * breakbeginH && TimeTable[i].tot < 60 * (breakbeginH + breaklenH))
                        {
                                TimeTable[i].triptime += triplusM;
                                if(break_t == -1)break_t = TimeTable[i].tot;
                        }
                }
                yueshu.breaktime = break_t;
        }
        //���³������Ͽ�ʼʱ����ʱ����ͨ���ӳ�����ʱ�̵�triptime��ʵ��
        void change_triptime_vehicle_failure(double failbeginH, double len, double faillenM)
        {
            //����ʱ�̱�
            int break_t = -1;
            for (int i = 0; i < TimeTable.size(); i++)
            {
                //failʱ�̺��׸�ʱ�̵���Ϊ�ǳ������ϵ�
                if (TimeTable[i].tot > 60 * failbeginH)
                {
                    TimeTable[i].triptime += faillenM;
                    if (break_t == -1)break_t = TimeTable[i].tot;
                    break;
                }
            }
            yueshu.breaktime = break_t;
        }

        //������ʵ�ʵ�·ÿСʱ���г�ʱ����Χ����������г�ʱ������ģ����һ��ĵ�·���
        void change_triptime_realrand_triptime()
        {
                //ͳ�ƣ�����mod3
                vector<int> max_triptime; max_triptime.resize(24, -9999);
                vector<int> avg_triptime; avg_triptime.resize(24, 0);
                vector<int> count; count.resize(24, 0);
                vector<int> min_triptime; min_triptime.resize(24, 99999);
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        int time = TimeTable[i].tot;
                        int hour = time / 60;
                        avg_triptime[hour] += TimeTable[i].triptime;
                        count[hour]++;
                        if (max_triptime[hour] < TimeTable[i].triptime)
                        {
                                max_triptime[hour] = TimeTable[i].triptime;
                        }
                        if (min_triptime[hour] > TimeTable[i].triptime)
                        {
                                min_triptime[hour] = TimeTable[i].triptime;
                        }
                }
                for (int i = 0; i < avg_triptime.size(); i++)
                {
                        if (count[i] > 0)avg_triptime[i] /= count[i];
                }

                for (int i = 0; i < TimeTable.size(); i++)
                {
                        int time = TimeTable[i].tot;
                        int hour = time / 60;
                        TimeTable[i].triptime = min_triptime[hour] + rand() % (1 + max_triptime[hour] - min_triptime[hour]);
                }
        }
        //modle��-2 �̶�����ģʽ,����������ΪMaxCarNum��ģ�⳵�����ϣ�����δ����
        //modle:-1 �̶�����ģʽ������������ΪMaxCarNum��ģ�����ӵ�£�����δ����
        //modle��0 �̶�����ģʽ������ʹ���Ż���Ĳ�������֮��̶���
        //modle��1 �����Ż�ģʽ������ʹ���Ż���Ĳ�������֮������Ż���
        //modle��2 �����Ż�ģʽ������ʹ�������������֮������Ż���
        void resort_oldcase(int mod, int MaxCarNum)
        {
                vector<int> empint;
               
                //��������
                TimeTable_make_link();               

                Schedule PBempty(TimeTable, yueshu, -1);//��ʼ���Ŀ��Ű෽��
                pb_result_shuxing result;//�����Ű�
                switch (mod)
                {
                case -2://modle��-2 �̶�����ģʽ��ģ�⳵�����ϣ�����δ����
                    pbs.push_back(PBempty); pbs.back().clear();
                    pbs.back().yueshu.change_Particle(S.sw_bestP());//������Ⱥ�������µ�Լ���ṹ��
                    pbs.back().yueshu.max_carnum = MaxCarNum;//�����Լ��
                    result = pbs.back().Schedule_parks(oldPB);//�����Ű�
                    result.scorer();//��дĿ�꺯��ֵ
                    break;

                case -1://modle��-1 �̶�����ģʽ������������Ϊԭ����������δ����
                    pbs.push_back(PBempty); pbs.back().clear();
                    pbs.back().yueshu.change_Particle(S.sw_bestP());//������Ⱥ�������µ�Լ���ṹ��
                    pbs.back().yueshu.max_carnum = MaxCarNum;//�����Լ��
                    result = pbs.back().Schedule_parks(oldPB);//�����Ű�
                    result.scorer();//��дĿ�꺯��ֵ
                    break;

                case 0://modle��0 �̶�����ģʽ������ʹ���Ż���Ĳ�������֮��̶���
                        pbs.push_back(PBempty); pbs.back().clear();
                        pbs.back().yueshu.change_Particle(S.sw_bestP());//������Ⱥ�������µ�Լ���ṹ��
                        result = pbs.back().Schedule_parks(oldPB);//�����Ű�
                        result.scorer();//��дĿ�꺯��ֵ
                        break;


                case 1://modle��1 �����Ż�ģʽ������ʹ���Ż���Ĳ�������֮������Ż���                      
                        S.init_swa(S);
                        for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////�Ű���ѭ��
                        {
                                for (int j = 0; j < S.grs.size(); j++)
                                {
                                        pbs.push_back(PBempty); pbs.back().clear(); pbs.back().ScheduleID = j;
                                        pbs.back().yueshu.change_Particle(&S.grs[j].ps[i]);//������Ⱥ�������µ�Լ���ṹ��
                                        result = pbs.back().Schedule_parks(oldPB);//�����Ű�
                                        S.grs[j].ps[i].punish = result.scorer();//��дĿ�꺯��ֵ                                       
                                }
                                S.get_next_ps(i);
                                printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f \n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                        }
                        //print_to_excel(line_name + "/ofline_test_mod1", S, pbs);//�����ӹ켣�����ļ�
                        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                        break;


                case 2://modle��2 �����Ż�ģʽ������ʹ�������������֮������Ż���
                        S.init_swa();
                        for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////�Ű���ѭ��
                        {
                                for (int j = 0; j < S.grs.size(); j++)
                                {
                                        pbs.push_back(PBempty); pbs.back().clear(); pbs.back().ScheduleID = j;
                                        pbs.back().yueshu.change_Particle(&S.grs[j].ps[i]);//������Ⱥ�������µ�Լ���ṹ��
                                        result = pbs.back().Schedule_parks(oldPB);//�����Ű�
                                        S.grs[j].ps[i].punish = result.scorer();//��дĿ�꺯��ֵ
                                }
                                S.get_next_ps(i);
                                printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f\n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                        }
                        //print_to_excel(line_name + "/ofline_test_mod2", S, pbs);//�����ӹ켣�����ļ�
                        break;

                default:
                        break;
                }

                /////////////////////////////////////////////////////������������
                for (int i = 0; i < pbs.size(); i++)
                {
                        int num = 0;
                        for (int j = 0; j < pbs.size(); j++)
                        {
                                if (pbs[i].result.punish > pbs[j].result.punish)
                                {
                                        num++;
                                }
                        }
                        scoreth_ofpbi.push_back(num);
                }
                ///////////////////////////////////////////////////////
                for (int i = 0; i < pbs.size(); i++)
                {
                        for (int j = 0; j < pbs.size(); j++)
                        {
                                if (scoreth_ofpbi[j] == i)
                                {
                                        pbi_ofscoreth.push_back(j);
                                }
                        }
                }
                print_scheme(line_name + "/best_RE_scheme.txt", pbs[pbi_ofscoreth[0]]);//��ӡ�ص��ȵ������Ű�
                //print_result_sts(line_name + "/RE_result.csv", pbs[pbi_ofscoreth[0]]);//��ӡͳ������
                cout << endl;
        }
};