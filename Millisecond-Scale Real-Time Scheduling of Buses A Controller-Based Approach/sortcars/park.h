#pragma once
#include "timepoint.h"
#include "car.h"
//ʱ�̵㳵�⣬ʵʱ��Ϣ
class Park
{
public:
        //��Ӧʱ�̵���Ϣ�Լ�ֱ�������������Ϣ
        timepoint tp;
        //��ǰslot��վ�ڳ���Դ״��
        vector<Car> Cars;   

        //���Ի��ݡ����¼���ݺ��ֱ���Ӧ�÷����ĳ�����
        //�������Ŀ�����ǰ�ƶ�������Ŀ���ֱ�Ӽ�¼Ӧ��˯�ߵĳ�����
        //�������Ŀ��㲻�䣬����Ŀ�����ӳ�����
        //�������Ŀ��������ƶ������������Ŀ��֮������м�¼��
        vector<int> ShouldSleep_Cars;
        //���ݻ��ݼ�¼�жϳ����Ƿ�Ӧ��ֱ������
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

        //��slot�Ƿ����³�,0/1 //��slot�Ƿ�����˾���ϰ�
        int ifnewcar;
        int ifnewdriver;
        //��slot���ĳ���id��0��ʼ,-1��ȱ����.
        int carx;
        int driverx;
        //��ʱ��chosen
        int chosen;
        //��slotδ�Ű�֮ǰ�����˶����³�
        int carsused;
        int driverused;

        int endt()
        {
                return tp.tot + tp.triptime;
        }

        Park(timepoint& intp)
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