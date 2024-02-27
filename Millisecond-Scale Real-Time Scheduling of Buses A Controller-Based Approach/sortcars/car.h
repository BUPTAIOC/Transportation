#pragma once
#include "yueshu_result.h"
using namespace std;

//ʹ��Car.times���ı��뷽����
class Car
{
public:
        //����ţ���0��ʼ
        int carx;
        //Ŀǰ������˾�����ϰ���ȣ�0 work;1 sleep��
        int state;
        //�����ͣ����೵2 �̰೵1 �߷峵0. ��ʼȫ��Ϊ1����;��ǰ˯�߱�0������˯�ߺ󱻻��ѱ�2
        int type;

        //�����ĸ�ʱ�̵���µģ����ڻ��ݻ�ԭ�ֳ���
        int fromparkx;
        //��ʱ��ʼ˯�������Ӽơ�-1����û˯����
        int whenslep;

        int tripnum;
        int tripmax;
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
        //�Ѿ���·�Ͽ�����ʱ��������
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
        //�Ѿ���Ϣ����ʱ��
        int restime;
        //��ǰ��Ϣʱ����Ϣ�˶��
        int resthis;
        //�ӷ�������ǰ����ʱ��������
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
        //���working���޷�ת��Ϊ���೵��.���Բ���sleep_to_long��
        int canot_tolong(int triptime)
        {
                switch (type)
                {
                case 0://��
                        return 1;
                        break;
                case 1://��s
                        return 0;
                        break;
                case 2://��
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
        void update_rest(int infromx, int restplus)//����Ϣʱ��ֻ��drive����¡�
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

                restime += partrest;//����һ��trip��ſ�ʼ��������Ϣʱ����

                drivetime += driveplus;

                //worktime += driveplus + partrest;  
                //worktime = restime + drivetime;

                resthis = partrest;//�����ۼӵ�����Ϣʱ��
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////
        //ʹ��������˯�߸߷�
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
        //ʹ��������˯�߶̰�
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
        //���ѳ���,����ʱΪslotcar����ռ�,ת������state��
        void weakup(int infromx, int intriplus, int indrive_plus, int inrest_plus)
        {
                state = 0;

                if (type == 2)//���೵�����ٱ�������
                {
                        cout << carx << "can not wake up" << endl;
                }

                if (type == 1)//������������˵����Ϊ���೵��
                {
                        type = 2;
                        //�����������ߡ�
                        tripmax *= 2;
                        drivemax *= 2;
                        workmax *= 2;
                        update_drive(infromx, intriplus, indrive_plus, inrest_plus);
                }

                if (type == 0)//���Ѹ߷泵
                {
                        update_drive(infromx, intriplus, indrive_plus, inrest_plus);
                }
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////
        /////////////////////////////////////////////////////////////////////////////////////////////////
        //chosecarʱ���������ֺ��������ش�ʱ������Ϊ��һʱ�̵㷢���ĵ÷֣�Ȩ�أ���       
        //ʹ�ó��� ʣ�๤��ʱ��/ʣ��trip�� ���������ֺ��������ش�ʱ������ѡ�е�Ȩ�ء�+ pso
        double simple_score(
                int endtime_ofday,//һ�����ǰ���һ�������ĵ�վʱ��
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
                if (state == 0)//����״̬�ĳ�
                {
                        int tripleft = Tripleft();
                        int workleft = Workleft();//ʣ�๤��ʱ��

                        if (workmax - worktime() > endtime_ofday - nowtime)//ʵ���ϣ�һ������ʱ����¼��worktime���ܲ�ȷ�С�
                        {
                                workleft = endtime_ofday - nowtime;
                        }
                        int wateleft = 1 + workleft - triptime;//ʣ�����Ϣʱ�䡣��һ�ַ�ʽ�ǲ�����triptime
                        if (wateleft <= 0)
                        {
                                cout << "wateleft <=0 error" << endl;
                                wateleft = 1;
                        }

                        double trips_pear_min = double(tripleft) / double(wateleft);

                        //trips_pear_min^2*(resthis^2 + 100*IFlastrip) ��һ������Ч��Ȩ�ع�ʽ
                        score = 100 * pow(trips_pear_min, pow1)*(pow(resthis, pow2) + Wplus_backhome * Y + 1) ;
                        //score = 100 * (trips_pear_min + Wplus_backhome * Y + 1) * pow(resthis, pow2);(�������Ĺ�ʽ)

                        //cout << score << endl;
                        if (score <= 0)
                        {
                                cout << "score <=0 error"<<endl;
                        }
                }
                else//����״̬�ĳ�
                {
                        int emptime = nowtime - whenslep;

                        if (aweak_carnum > 0)//���������ڹ�����
                        {
                                score = 0;
                        }
                        else if (sleeptype1_carnum > 0)//ֻʣsleep car,��sleep ��ͨ��+sleep peak
                        {
                                if (type == 2)//sleep��
                                {
                                        score = 0;
                                        cout << "Wrong! should not have sleeping long car" << endl;
                                }
                                else if (type == 1)//sleep��
                                {
                                        score = 2 + 2 * pow(emptime, 2);
                                }
                                else if (type == 0)
                                {
                                        score = 0; //�߷峵�����ȼ��ģ����ײ���     
                                }
                        }
                        else if (sleeptype2_carnum > 0)//ֻʣsleep peak car
                        {
                                if (type == 0)
                                {
                                        score = 1 + 1 * pow(emptime, 2); //�߷峵�����ȼ��ģ����ײ���     
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

        //chosecarʱ���������ֺ��������ش�ʱ������Ϊ��һʱ�̵㷢���ĵ÷֣�Ȩ�أ���       
        //ʹ�ó��� ʣ�๤��ʱ��/ʣ��trip�� ���������ֺ��������ش�ʱ������ѡ�е�Ȩ�ء�+ pso + rbf
        double rbf_score(
            int endtime_ofday,//һ�����ǰ���һ�������ĵ�վʱ��
            int nowtime, int triptime,
            int aweak_carnum, int sleeptype1_carnum, int sleeptype2_carnum, hiden_layer & H, P & p_used)
        {
            int Y = 0;
            if (tripmax - 1 == tripnum)
            {
                Y = 1;
            }

            double score = 0;
            if (state == 0)//����״̬�ĳ�
            {
                int tripleft = Tripleft();
                int workleft = Workleft();//ʣ�๤��ʱ��

                if (workmax - worktime() > endtime_ofday - nowtime)//ʵ���ϣ�һ������ʱ����¼��worktime���ܲ�ȷ�С�
                {
                    workleft = endtime_ofday - nowtime;
                }
                int wateleft = 1 + workleft - triptime;//ʣ�����Ϣʱ�䡣��һ�ַ�ʽ�ǲ�����triptime
                if (wateleft <= 0)
                {
                    cout << "wateleft <=0 error" << endl;
                    wateleft = 1;
                }

                double trips_pear_min = double(tripleft) / double(wateleft);

                vector<double> inputs;
                //inputs.push_back(double(tripleft));
                //inputs.push_back(double(wateleft));
                //inputs.push_back(trips_pear_min * 2500);
                inputs.push_back(resthis);
                //inputs.push_back(double(tripleft));
                //inputs.push_back(double(wateleft));
                //inputs.push_back(double(Y));

                //trips_pear_min^2*(resthis^2 + 100*IFlastrip) ��һ������Ч��Ȩ�ع�ʽ

                score = H.out_tot(p_used, inputs);
                if (score <= 1)score = 1;

                //cout << score << endl;
                if (score <= 0)
                {
                    cout << "score <=0 error" << endl;
                }
            }
            else//����״̬�ĳ�
            {
                int emptime = nowtime - whenslep;

                if (aweak_carnum > 0)//���������ڹ�����
                {
                    score = 0;
                }
                else if (sleeptype1_carnum > 0)//ֻʣsleep car,��sleep ��ͨ��+sleep peak
                {
                    if (type == 2)//sleep��
                    {
                        score = 0;
                        cout << "Wrong! should not have sleeping long car" << endl;
                    }
                    else if (type == 1)//sleep��
                    {
                        score = 2 + 2 * pow(emptime, 2);
                    }
                    else if (type == 0)
                    {
                        score = 0; //�߷峵�����ȼ��ģ����ײ���     
                    }
                }
                else if (sleeptype2_carnum > 0)//ֻʣsleep peak car
                {
                    if (type == 0)
                    {
                        score = 1 + 1 * pow(emptime, 2); //�߷峵�����ȼ��ģ����ײ���     
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