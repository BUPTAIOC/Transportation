#pragma once
#include"pso.h"
#include"rbf.h"
#include <iomanip>
#include <cmath>
#include <algorithm>



/////////////////////////////////////////////////////////////////////////////////////////////////
//�Ű�ǰ�����¸��Ű������Լ����������ϸ��Ű�����̬����
class pb_before_yueshu
{
public:
        //���ĸ�ʱ�俪ʼ�Űࡣ���Ա��졣���ʱ��֮��ĳ����������鳵��Ҫ���µı��췽���������
        int breaktime;
        //��һ����chosecar�����Ĳ���
        int ifrand;
        int ifbacktrack;
        int stationum;
        //��಻�ܳ������ٸ�trip
        int max_tripnum;//////////////////ʹ��PSO�Ż�
        //trip��������Ϣ���
        int atlist_one_rest;
        //�Ƿ��й���ʱ��Լ��
        int max_worktime;
        //�Ƿ��м�ʻʱ��Լ��
        int max_drivetime;
        //�߷峵��
        int if_gaofeng;
        int gaofengused;
        int to_peakcar_restime;/////ʹ��PSO�Ż�

        //��latest_new_car_time֮ǰ�����뷢��min_newcar_num���³�
        int if_min_newcar;
        int latest_new_car_time;
        int min_newcar_num;
        //��̬����ʱ���õ��������
        int max_carnum;
        //���ٰ������trip�ļ���ϵ��
        int Wplus_backhome;///////////////ʹ��PSO�Ż�

        P P_used;
        hiden_layer H;

        pb_before_yueshu()
        {
                breaktime = 0;
                ifrand = 0;
                ifbacktrack = 0;

                stationum = 0;

                max_tripnum = 0;

                atlist_one_rest = 1;

                max_worktime = 0;

                max_drivetime = 0;

                to_peakcar_restime = 0;

                if_gaofeng = 0;

                gaofengused = 0;

                this->max_carnum = 999999;

                if_min_newcar = 0;
                latest_new_car_time = 0;
                min_newcar_num = 0;

                Wplus_backhome = 0;
        }
        pb_before_yueshu(
                int inbreaktime,
                int inifrand,
                int inifbacktrack,
                int inifmincar,
                int instationum,
                int ifgaofeng,
                int max_carnum,

                int inatlist_one_rest,
                int inmax_work_time,
                int inmax_drive_time,
                hiden_layer & inH
        )
        {
                breaktime = inbreaktime;
                ifrand = inifrand;
                ifbacktrack = inifbacktrack;

                stationum = instationum;

                atlist_one_rest = inatlist_one_rest;

                max_worktime = inmax_work_time;

                max_drivetime = inmax_drive_time;

                if_gaofeng = ifgaofeng;

                this->max_carnum = max_carnum;

                gaofengused = 0;

                if_min_newcar = inifmincar;
                latest_new_car_time = 0;
                min_newcar_num = 0;

                Wplus_backhome = 0;

                H = inH;
        }

        void change_Particle(P* inP)
        {
                this->max_tripnum = int(inP->ds[0].val()); if (max_tripnum % 2 == 1) { max_tripnum--; }
                this->to_peakcar_restime = int(inP->ds[1].val());
                this->Wplus_backhome = int(inP->ds[2].val());
                this->latest_new_car_time = int(inP->ds[6].val());
                this->min_newcar_num = int(inP->ds[7].val());
                P_used = *inP;
        }
};

//���Ա�ʾ�Ѿ������pb�Ľ�����ԡ����������߷�����Ϣ�����յ÷ֵȡ�
class pb_result_shuxing
{
public:
        int carsused;
        int drivsused;

        //����������³�������ʱ����·����������������Ż�buisylevel
        //int after_finalnewcartime;
        //������ͼ�����߷�ת���ߵ�˾���������Ե���togaofengrestime
        int after_togaofengrestime;

        //û�г����ǵ�ʱ�̵���
        int nocar_slotnum;
        //��Ϣʱ��������ֵ����Ŀ��
        int buisy_restnum;
        //��Ϣʱ�䲻�㵼�µĳͷ�ֵ
        int buisy_punish;

        //trip������˫����˾����
        int unback_drivernum;

        //�����˼��βŵõ����Ž�
        int backtrack_num;

        int punish;//��ֵԽ�ߣ�Խ����

        pb_result_shuxing(
                int incarnum = 0,
                int indrivernum = 0,
                int inafter_togaofengrestime = 0,
                int inocar_slotnum = 0,
                int inbadrestnum = 0,
                int inbadrestpunish = 0,
                int inunback_drivernum = 0)
        {
                carsused = incarnum;
                drivsused = indrivernum;

                after_togaofengrestime = inafter_togaofengrestime;

                nocar_slotnum = inocar_slotnum;
                buisy_restnum = inbadrestnum;
                buisy_punish = inbadrestpunish;

                unback_drivernum = inunback_drivernum;

                backtrack_num = 0;

                punish = 0;
        }

        int scorer()
        {
                punish = 0;
                punish += carsused * 1000;
                punish += drivsused *100;

                punish += nocar_slotnum * 1000;
                //punish += buisy_punish / 10;
                punish += unback_drivernum * 2000;

                //punish += backtrack_num * 1;

                return punish;
        }
};