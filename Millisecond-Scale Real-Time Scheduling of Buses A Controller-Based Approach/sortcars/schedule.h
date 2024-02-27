#pragma once
#include "park.h"

#define DELETE_OR_SLEEP_working_car if (car->canot_tolong(triptime)){park.Cars.erase(park.Cars.begin() + i);i--;}else{car->sleep_to_long(oldtime);}

//�����Ҫ���ݣ���Ҳֻ������Ű��ڲ������飬��Ӱ�������Ű�
class Schedule
{
public:
        int ScheduleID;
        //�����㳵�⣬���Ի�ȡʵʱ������Ϣ
        vector<Park> parks;
        //�������г̵ĳ����Ľ���ʱ�䡣���ܲ���ĩ�೵�������ȷ����������糵����
        int endtime_ofday;
        //�����Ű������±����id���ڼ���trip��Ԫ�ش������trip���ĸ�Slot�����ġ�
        //car[].trip[]->(slot)
        vector<vector<Park*>> tripque;

        //�Ѿ��źõ�Slot��
        int sorted;
       
        pb_before_yueshu yueshu;//��ʼ���󣬱����ڲ���ı䡣
        pb_result_shuxing result;

        //���캯��������ʱ�̵�֮�������,��Ҫ��ͬCP����ʱ�̵�ĳ�������
        Schedule(vector<timepoint>& TimeTable, pb_before_yueshu& inyueshu, int S_id)
        {
                ScheduleID = S_id;

                sorted = 0;

                endtime_ofday = 0;

                yueshu = inyueshu;
                result = pb_result_shuxing();

                //��ʼ��parks
                for (int i = 0; i < TimeTable.size(); i++)
                {
                        parks.push_back(Park(TimeTable[i]));
                        if (TimeTable[i].tot + TimeTable[i].triptime > endtime_ofday)
                        {
                                endtime_ofday = TimeTable[i].tot + TimeTable[i].triptime;
                        }
                }
        }    

        Schedule()
        {

        }

        void clear()
        {
                sorted = 0;
                //��ʼ��slos
                for (int i = 0; i < parks.size(); i++)
                {
                        parks[i].clear();//�����ʱ�̵�����
                }
                tripque.clear();
        }

        int VehicleSelector(Park& park, int endtime_ofday)
        {               
                //����һ��������Ȩ�ؼ��㡣
                //���������������Եķ��ۺ�Ϊ����������ֵܷ��С�
                //��ʹ�����г���Ҳ���ܷ��³���
                int newcar_score = 0;

                int weak_i = 0;//ͳ�����ŵĳ����±꼯��
                int sleep_type1 = 0;//ͳ��˯�ŵĳ����±꼯��
                int sleep_type0 = 0;

                for (int i = 0; i < park.Cars.size(); i++)
                {
                        if (park.Cars[i].state == 0)
                        {
                                weak_i++;
                        }
                        else
                        {
                                if (park.Cars[i].type == 1)
                                {
                                        sleep_type1++;
                                }
                                else if (park.Cars[i].type == 0)
                                {
                                        sleep_type0++;
                                }
                        }
                }
                /////////////////////////////////////////////////////////////////////////
                //�ܷ���
                vector<double>scores;
                vector<int> weight;
                int weight_tot = 0;
                //�����ǵ������۱�׼���������������ۣ�ֻ���ڳ��ⷢ�����ߡ��ȰѸ������Եĵ÷ֱַ���ã����������ĸ��Լ��ӡ�
                /////////////////////////////////////////////////////////////////////////
                double sco = -1;
                double maxsco = 0.0;
                double minsco = 999999999.0;
                for (int i = 0; i < park.Cars.size(); i++)
                {
                        sco = park.Cars[i].simple_score(
                                endtime_ofday,
                                park.tp.tot,park.tp.triptime,
                                weak_i,
                                sleep_type1,
                                sleep_type0,
                                yueshu.Wplus_backhome,
                                yueshu.P_used.ds[3].val(),
                                yueshu.P_used.ds[4].val(),
                                yueshu.P_used.ds[5].val());

                        //sco = park.Cars[i].rbf_score(
                        //    endtime_ofday,
                        //    park.tp.tot, park.tp.triptime,
                        //    weak_i,
                        //    sleep_type1,
                        //    sleep_type0,
                        //    yueshu.H,yueshu.P_used);

                        scores.push_back(sco);//�ҵ������С�����������һ��
                        if (sco < minsco && sco > 0)
                        {
                                minsco = sco;
                        }
                        if (sco > maxsco)
                        {
                                maxsco = sco;
                        }
                }
                for (int i = 0; i < park.Cars.size(); i++)
                {
                        int intsco = int((100 * scores[i]) / minsco);
                        weight.push_back(intsco);
                        weight_tot += intsco;
                }
                if (weight_tot == 0)
                {
                        cout << "weight_tot == 0 error"<<endl;
                }
                /////////////////////////////////////////////////////////////////////////
                int chosen_i = 0;
                double choseni_weight = weight[chosen_i];
                int randnum = qrand() % weight_tot;
                int numleft = randnum;
                switch (yueshu.ifrand)
                {
                case 0:
                        //���ԣ������,���ָߵĳ��������ᷢ�³����ỽ�ѡ�    
                        for (int i = 0; i < park.Cars.size(); i++)
                        {
                                if (weight[i] > choseni_weight)
                                {
                                        choseni_weight = weight[i];
                                        chosen_i = i;
                                }
                        }
                        break;
                case 1:
                        //���ԣ��÷�Ȩ��������÷ּ���Ȩ�ء����ܷ��³���
                        for (int i = 0; i < park.Cars.size(); i++)
                        {
                                numleft -= weight[i];
                                if (numleft <= 0)
                                {
                                        chosen_i = i;
                                        break;
                                }
                        }
                        if (numleft > 0)
                        {
                                chosen_i = -1;
                        }
                        break;

                case 2:
                        //���ԣ��÷�����Ȩ�����������Ϊ�������ζ���Ȩ�ء�
                        break;
                default:
                        return 0;
                        break;
                }
                return chosen_i;
        }

        //���³�����Ϣ��������ĳ���
        //��selector����������slot��ô�������³����ɳ������ѳ������ţ�
        //ת�Ƴ�������������ʱ�̵㳵����Ϣ
        //copychose �������ƶϵ�֮ǰ�ľ�ѡ��
        //�����Ҫ���ݣ��򷵻�Ŀ��parkX�����򷵻�-1
        int mover(Park& park,int copychose)
        {
                //�������ó�����Ϣ
                park.carsused = result.carsused;
                park.driverused = result.drivsused;
                ////////////////////////////////////////////////////////////////////////////
                //�ҵ��������԰��Ķ԰�slot
                //����trip�ķ���ʱ��
                int oldtime = park.tp.tot;
                //trip��ʱ
                int triptime = park.tp.triptime;
                //Ԥ�⳵��nextrip��վʱ��
                int next_arrivetime = oldtime + triptime;
                //������trip���ı�
                int next_TableX = park.tp.gotoCP;
                //��վ����һ������slot��sloty��û���¸�slot�˾�-1
                int nextrip_firstSlotY = park.tp.nextrip_firstParkX;//find_nextrip_firstSlotY4(TimeTable[next_TableX], next_arrivetime);
                //ѡ�еĳ��������trip���ڶ԰�slot����֮ǰ��Ϣ�˶��
                int partrest; if (nextrip_firstSlotY >= 0)partrest = parks[nextrip_firstSlotY].tp.tot - next_arrivetime;
                //����ѡ�еĳ��ڳ����е�λ�á�-1��ʾ���³������ó�����ľɳ���
                int chosen;
                int chosencarx;
                /////////////////////////////////////////////////////////////////////////////
                //��������ɨ�����в����õĳ���
                //ʱ������8��6.5, 1.5
                //int atlistrest = 2;//˾��������Ϣ�ķ�����
                //�����������в������õĳ������߻�˾������˾�����ߡ�˳���ø߷峵��ʼsleep��
                for (int i = 0; i < park.Cars.size(); i++)
                {
                        Car* car = &park.Cars[i];

                        switch (car->state)
                        {
                        case 0://0״̬�����ոջ���work.�����Լ�������У�����Υ�����Լ��������maxtrips��worktime 
                                if (park.marked_to_sleep(car->carx))
                                {//��������������л��ݼǺţ�˵��Ӧ�������ʱ�̵�ֱ���°࣬������ѡ�з���
                                        DELETE_OR_SLEEP_working_car
                                }
                                else if (car->break_maxtrip()||car->canot_fulltrip(triptime))//�����°�ĳ���,��˫���г��°�ĳ���.
                                {
                                        DELETE_OR_SLEEP_working_car
                                }
                                else if (car->break_worktime(triptime)||//worktime���꣬��Ϣ������slot��ſ��ܳ���work time��
                                         car->break_drivetime(triptime))//�����������ʱ����Ϊ����ż��trip���
                                {//��Щ��һ��trip�����������trip��ż�����°�ͺá����trip����������Ҫ���ݵ��ϸ�ż��trip,�������°ࡣ
                                        if (car->type != 1 && car->tripnum % 2 == 1 )// car->type != 1
                                        {
                                                if (yueshu.ifbacktrack == 1)//����ģʽ
                                                {
                                                        Park* backto_park = tripque[car->carx][car->tripnum - 1];
                                                        if (backto_park->tp.tot >= yueshu.breaktime)//��ʱ����,�������Ҳ����������⣬ֻ�ܼ�����
                                                        {
                                                                backto_park->ShouldSleep_Cars.push_back(car->carx);
                                                                return backto_park->tp.Number;//Ӧ�û���
                                                        }
                                                        else//���ݾ�Ҫ���ϵ�֮ǰ�ˣ�����Ҳ������������ˣ������ˣ������Ű�
                                                        {
                                                                DELETE_OR_SLEEP_working_car
                                                        }
                                                }
                                                else//�ǻ���ģʽ
                                                {
                                                        DELETE_OR_SLEEP_working_car
                                                }                                                
                                        }
                                        else//һ����˵���̰೵�ܿ���תΪ���೵�����ر�֤ż���г�
                                        {
                                                DELETE_OR_SLEEP_working_car
                                        }
                                }
                                else if (car->type == 1 &&
                                        car->resthis >= yueshu.to_peakcar_restime &&
                                        car->tripnum % 2 == 0)//ת���ɸ߷峵,����˯��
                                {
                                        result.after_togaofengrestime++;
                                        car->sleep_to_peak(oldtime);
                                        yueshu.gaofengused++;
                                }
                                break;

                        case 1://1״̬������sleep.
                                if (park.marked_to_sleep(car->carx))
                                {//��������������л��ݼǺţ�˵��Ӧ�������ʱ�̵�ֱ���°࣬������ѡ�з���
                                        park.Cars.erase(park.Cars.begin() + i); i--;
                                }
                                else if (car->type == 1 && car->break_worktime(triptime))//�̰೵��һ��˾���°��sleep���µ�һ��˾���Ĺ���ʱ�����
                                {                                    
                                        if (car->worktime() + 2 * triptime >= car->workmax * 2 ||
                                                car->drivetime + 2 * triptime >= car->drivemax * 2)
                                        {
                                                park.Cars.erase(park.Cars.begin() + i); i--;
                                        } 
                                        ////////////////////////////////////////////////////////////////////////�µĳ��೵�����������
                                }
                                break;

                        default:
                                cout << "car state error";
                                break;
                        }
                }

                ///////////////////////////////////////�Ƿ񷢳������³������ɳ��������ɳ�
                if (copychose >= 0)
                {
                        chosen = copychose;
                }
                else if (yueshu.if_min_newcar == 1 && 
                        result.carsused < yueshu.min_newcar_num && 
                        park.tp.tot >= yueshu.latest_new_car_time)
                {//����������ٷ����ٳ���һ���ȹر�,line70��line85 line803����.
                        chosen = -1;
                }
                else if (park.Cars.size() == 0)
                {
                        chosen = -1;
                }
                else if (park.Cars.size() == 1)
                {
                        chosen = 0;
                }
                else
                {//�����оɳ�����������Ҳ�ﵽҪ���Ҫ���Ƿ��ɳ����ǻ���
                        chosen = VehicleSelector(park,endtime_ofday);
                }
                //��̬����ʱ��������yueshu.max_carnum�������������
                if (chosen == -1 && result.carsused >= yueshu.max_carnum)
                {
                    chosen = -3;
                }
                //���ˣ�chosen�Ѷ�
                //////////////////////////////////////////////////////////////////////////
                park.chosen = chosen;
                //////////////////////////////////////////////////////////////////////////
                if (chosen == -1)//�������³�
                {
                        chosencarx = result.carsused;
                        result.carsused++;
                        result.drivsused++;
                        //��¼�³�����,����slot�ڵ���Ϣ
                        park.ifnewcar = 1;
                        park.ifnewdriver = 1;
                        park.carx = chosencarx;
                        //��һ����³����Ű����
                        vector<Park*>ca;
                        tripque.push_back(ca);
                        tripque[chosencarx].push_back(&park);

                        if (nextrip_firstSlotY >= 0)//�������ĳ���
                        {
                                parks[nextrip_firstSlotY].Cars.push_back(
                                        Car(chosencarx, 1, 0, park.tp.Number,
                                                yueshu.max_drivetime, yueshu.max_worktime, yueshu.max_tripnum,
                                                triptime, partrest, 1, -1));
                        }
                }
                else if (chosen == -3)
                {
                        result.nocar_slotnum++;
                }
                else//�þɳ�
                {
                        Car* chosencar = &park.Cars[chosen];
                        chosencarx = chosencar->carx;
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        //��һ����³����Ű����tripque 
                        switch (chosencar->state)
                        {
                        case 0://ѡ�ĳ����ڹ����ĳ�
                                park.ifnewdriver = 0;
                                break;
                        case 1://ѡ�ĳ�����˯�ߵĳ�
                                switch (chosencar->type)
                                {
                                case 2:
                                        park.ifnewdriver = 1;
                                        if (chosencar->type == 1)//�������˯���һ��ܼ�������˾��, ��Ϊ���೵                                     
                                        {

                                        }
                                        break;
                                case 1:
                                        park.ifnewdriver = 1;
                                        if (chosencar->type == 1)//�������˯���һ��ܼ�������˾��,                                      
                                        {

                                        }
                                        break;
                                case 0:
                                        park.ifnewdriver = 0;//ѡ�и߷峵������
                                        break;
                                default:
                                        break;
                                }
                                break;
                        default:
                                break;
                        }
                        tripque[chosencarx].push_back(&park);//��¼tripque����                      
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        //��¼�³�����,����slot�ڵ���Ϣ
                        park.ifnewcar = 0;
                        park.carx = chosencarx;
                        //////////////////////////////////////////////////////////////////////////////////////////////
                        //��վ�󣬻��к���ʱ�̵㣬�ɳ����¸�slot��
                        if (nextrip_firstSlotY >= 0)
                        {
                                Park* aimslo = &parks[nextrip_firstSlotY];
                                aimslo->Cars.push_back(*chosencar);//�Ȱѳ�����ȥ���ٴ����³���ʼ��Ϣ
                                Car* car = &aimslo->Cars.back();

                                if (chosencar->state == 1)//���ѡ������sleep�ĳ���������һվ����(��ѹ����˾��)
                                {
                                        //����ذ�new˾����Դѹ�뱾slot��chosencar�ڣ����Խ�����car�������Բ�ͬ����newcar����ѹ�뱾slot��
                                        //��ˣ���slot�ڵ�chosencar-driver���䣬ֻ�ڳ���˾������һվ���ѹ����˾��������״̬����ƣ��Ȱѳ�����ȥ��������˾�����ѳ����ѡ�
                                        aimslo->Cars.back().weakup(park.tp.Number, 1, triptime, partrest);
                                }
                                else//���ѡ�е�������work�ĳ�����ֱ�Ӹ���
                                {
                                        aimslo->Cars.back().update_drive(park.tp.Number, 1, triptime, partrest);
                                }

                        }
                }
                ///////////////////////////////////////////////////////////////////////////////////////////////////
                //��������ʣ�ĳ��ŵ������¸�slot�� sleep peak������rest,������.
                if (park.tp.Nextrest_pointX >= 0)
                {
                        Park* next_restpark = & parks[park.tp.Nextrest_pointX];
                        int whole_rest = next_restpark->tp.tot - park.tp.tot;
                        for (int i = 0; i < park.Cars.size(); i++)
                        {
                                if (i != chosen)
                                {
                                        next_restpark->Cars.push_back(park.Cars[i]);
                                        Car* car = &next_restpark->Cars.back();

                                        switch (next_restpark-> Cars.back().state)
                                        {
                                        case 0://���ڳ�������work�ĳ�,������Ϣ
                                                car->update_rest(park.tp.Number, whole_rest);
                                                break;

                                        case 1://���ڳ�������sleep�ĳ����߷峵������Ϣʱ�䣬�����ӡ�
                                                if (car->type == 0)//�߷峵
                                                {

                                                }
                                                else if (car->type == 1)//������ͨ��
                                                {
                                                        next_restpark->Cars.back().update_rest(park.tp.Number, whole_rest);
                                                }
                                                break;

                                        default:
                                                cout << "car state error";
                                                break;
                                        }
                                }
                        }
                }
                else//��CP���һ��time point
                {

                }
                sorted++;
                return -1;
        }

        //������Ű��ֳ����ݵ�Xλ�õ���֮ǰ��
        //���¹����ֳ�,����X����Ļ��ݼ�¼
        void backtrack_rebuild(int toparkX)
        {
                vector<int> oldchoses;//����ԭʱ�̵㳵��
                for (int i = 0; i < parks.size(); i++)//���ʱҪ����ɾ�
                {
                        oldchoses.push_back(parks[i].chosen);
                        parks[i].clear();
                }               
                sorted = 0;

                tripque.clear();

                int backnum = result.backtrack_num;
                result = pb_result_shuxing();
                result.backtrack_num = backnum + 1;
                
                //һ��������ԭ���Ķ���
                //���ݵ�����Ŀ���ǰһ˲�����ظ�Ŀ��park�Ķ���.
                for (int i = 0; i <= toparkX-1; i++)
                {
                        mover(parks[i], oldchoses[i]);
                }
                //����X����Ļ��ݼ�¼,��Ϊ�����Ѿ�����
                for (int i = toparkX + 1; i < parks.size(); i++)
                {
                        parks[i].ShouldSleep_Cars.clear();
                }
        }

        //������Ű��ֳ����ݵ�Xλ�õ���֮ǰ.����������»����źš�
        //ɾ��֮��ĸĶ�
        void backtrack_delet(int toparkX)
        {
                
        }

        pb_result_shuxing Schedule_parks(Schedule& oldPB)
        {
                int backtrack_to_parkx = -1;//-1�����ݣ���Ӧ����parkx
                for (int pki = 0; pki < parks.size();)
                {
                        //����ʱ�β���������߽硣����������ʱ����߽��ԭ����parkҲҪ���ƹ�����
                        if (parks[pki].tp.tot < yueshu.breaktime)
                        {
                                backtrack_to_parkx = mover(parks[pki], oldPB.parks[pki].chosen);                               
                        }
                        else    
                        {
                                backtrack_to_parkx = mover(parks[pki], -1);
                        }         
                        //���϶ϵ���ƺ�ֻ�Ӷϵ������Űࡣ��breaktime�غϵ�slotҲҪ���µ���
                        if (backtrack_to_parkx >= 0)//��ʱ���ݵ�park[shouldsleep_parkx]�����ĳ���Ϊȥ���߻��°ࡣ
                        {
                                //cout <<"S_ID-"<< ScheduleID << " tracks " << result.backtrack_num + 1 <<
                                //        ",Car["<< parks[backtrack_to_parkx].ShouldSleep_Cars.back() << "] backto CP:"
                                // << backtrack_to_parkx << " from:" << pki << "->" 
                                //        << parks[backtrack_to_parkx].ShouldSleep_Cars.back() << ",shoule sleep cars: ";                             
                                //for (int c = 0; c < parks[backtrack_to_parkx].ShouldSleep_Cars.size(); c++)
                                //{
                                //        cout << parks[backtrack_to_parkx].ShouldSleep_Cars[c] << ",";
                                //}
                                //cout << endl;
                                
                                backtrack_rebuild(backtrack_to_parkx);
                                
                                pki = backtrack_to_parkx;
                        }
                        else
                        {
                                pki++;
                        }
                }
                /////////////////////////////////////////////////////////////////////////////////
                //������ͳ�ƽ���������ó�punish
                for (int i = 0; i < tripque.size(); i++)//i�ǳ���
                {
                        ////////////////////////////////////�����ǹ���ͳ��
                        Park* last_trip = tripque[i].back();
                        int chosen = last_trip->chosen;
                        if (chosen >= 0)
                        {
                                Car* chocar = &last_trip->Cars[chosen];
                                //
                        }
                        if (tripque[i].size() % 2 == 1)
                        {
                                result.unback_drivernum++;
                        }
                        if (tripque[i].size() > yueshu.max_tripnum)
                        {
                                result.drivsused++;
                        }
                        ////////////////////////////////////////////////////////
                        //for (int k = 1; k < tripque[pki].size(); k++)//k��trip
                        //{
                        //        Slot* slo = tripque[pki][k];
                        //        Car* chocar = &slo->Cars[slo->chosen];
                        //        int restcha = 4 - chocar->resthis;//��Ϣʱ������Բ���
                        //        if (restcha <= 0)
                        //        {
                        //                restcha = 0;
                        //        }
                        //        else
                        //        {
                        //                result.buisy_restnum++;
                        //        }
                        //}
                }
                result.scorer();
                return result;
        }
};