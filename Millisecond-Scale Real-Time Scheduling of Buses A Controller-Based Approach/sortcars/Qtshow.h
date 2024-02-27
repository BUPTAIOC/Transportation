#pragma once
#include "onecase.h"
#include <QLabel>
#include <QPushButton> 
#include <QTextBrowser>
#include <QMainWindow>
#include <qevent.h>

#include <QPen>
#include <QPainter>


class TripsBshow:public QWidget
{
        Q_OBJECT
public:
        int i;//车号
        int k;//第几个trip

        QTextBrowser* show2;
        QTextBrowser* show3;



        Schedule* pb;

        TripsBshow(int ini, int ink, QTextBrowser* inshow2, QTextBrowser* inshow3, Schedule*inpb)
        {
                i = ini;
                k = ink;

                show2 = inshow2;
                show3 = inshow3;

                pb = inpb;
        }


public slots:

        int printshow()//tripB槽函数
        {
                //cout << i << " " << j << " " << k << "\n";
                show2->clear();
                show3->clear();

                QFont ft;
                ft.setPixelSize(18);//字体大小
                ft.setFamily("Microsoft YaHei");
                show2->setFont(ft); 
                show3->setFont(ft);
            
                Park* park = pb->tripque[i][k];
                QString carx            = QString::number(park->carx);

                QString carsused        = QString::number(park->carsused);
                QString driverused      = QString::number(park->driverused);
                
                QString chosen          = QString::number(park->chosen);
                QString ifnewcar        = QString::number(park->ifnewcar);                          
                                                           
                QString Tablex          = QString::number(park->tp.fromCP);
                QString Sloty           = QString::number(park->tp.SlotY);
                QString nextsloty       = QString::number(park->tp.Nextrest_pointX);

                QString tot             = QString::number(park->tp.tot);
                QString triptime        = QString::number(park->tp.triptime);
                QString endtime         = QString::number(park->endt());
                
                show2->append("trip[" + QString::number(i) + "][" + QString::number(k) + "]:");
                show2->append("SLOT[" + Tablex + "][" + Sloty + "],nexty["+ nextsloty + "]");
                show2->append("T:[" + tot + "]+["+triptime+"]=["+endtime+"]\n=====================");

                show2->append("carx        = " + carx);
                show2->append("chosen      = " + chosen);
                if (park->ifnewcar == 0)
                {
                        Car* chosencar = &park->Cars[park->chosen];
                        show2->append("# chosen carx:" + QString::number(chosencar->carx));
                        show2->append("  |resthis    = " + QString::number(chosencar->resthis));
                        show2->append("  |re,dr,wo = " + QString::number(chosencar->restime) +
                                        "|" + QString::number(chosencar->drivetime) +
                                        "|" + QString::number(chosencar->worktime()));
                        show2->append("  |tripnum    = " + QString::number(chosencar->tripnum));
                }
                else
                {
                        show2->append("# chosen carx:" + QString::number(park->carx));
                        show2->append("  |resthis    = " + QString::number(0));
                        show2->append("  |re,dr,wo = " + QString::number(0) +
                                "|" + QString::number(0) +
                                "|" + QString::number(0));
                        show2->append("  |tripnum    = " + QString::number(0));
                }

                show2->append("cars.size() = " + QString::number(park->Cars.size()));
                show2->append("######################");
                for (int i = 0; i < park->Cars.size(); i++)
                {
                        show3->append("# cars[" + QString::number(i) + "],carx: " + QString::number(park->Cars[i].carx));
                        show3->append("# ++++++++++++++++++++");
                        show3->append("  |carx:" + QString::number(park->Cars[i].carx)
							         +",state:" + QString::number(park->Cars[i].state)
							         +",type:" + QString::number(park->Cars[i].type));
                                show3->append("  |resthis    = " + QString::number(park->Cars[i].resthis));
                                show3->append("  |re,dr,wo = " + QString::number(park->Cars[i].restime) +
                                        "|" + QString::number(park->Cars[i].drivetime) +
                                        "|" + QString::number(park->Cars[i].worktime()));
                                show3->append("  |tripnum    = " + QString::number(park->Cars[i].tripnum));
                                show3->append("# ++++++++++++++++++++");
                        show3->append("######################");
                }
                return 0;
        }

};

class SlotsBshow :public QWidget
{
        Q_OBJECT
public:
        int tablex;//车号
        int sloty;//本车第几个司机

        QTextBrowser* show2;
        QTextBrowser* show3;

        SlotsBshow(int inx, int iny, QTextBrowser* inshow2, QTextBrowser* inshow3)
        {
                tablex = inx;
                sloty = iny;

                show2 = inshow2;
                show3 = inshow3;
        }


public slots:

        int printshow()//slotB槽函数
        {
                cout << tablex << " " << sloty << " " << "\n";
                show2->append(QString::number(tablex));
                show2->append(QString::number(sloty));
                return 0;
        }

};



class MainWindow : public QMainWindow
{
        Q_OBJECT

///////////////////////////////////////////////////////////////////////////////////////
public slots:


        void lastw()
        {
                pbi--;
                if (pbi < 0)
                {
                        pbi = pbs.size() - 1;
                }
                clearwindow();
                drawindow(pbs[pbi],scoreth_ofpbi[pbi]);
                //cout << pbi+1 << " / " <<pbs.size() << " , " << pb.carsused << " " << pb.drivsused << "\n";
        }
        void nextw()
        {
                pbi++;
                if (pbi >= pbs.size())
                {
                        pbi = 0;
                }                       
                clearwindow();
                drawindow(pbs[pbi], scoreth_ofpbi[pbi]);
                //cout << pbi+1 << " / " << pbs.size() << " , " << pb.carsused << " " << pb.drivsused << "\n";
        }

        void worsew()
        {
                ith--;
                if (ith < 0)
                {
                        ith = pbs.size() - 1;
                }
                clearwindow();
                drawindow(pbs[pbi_ofscoreth[ith]], ith);
        }
        void beterw()
        {
                ith++;
                if (ith >= pbs.size())
                {
                        ith = 0;
                }
                clearwindow();
                drawindow(pbs[pbi_ofscoreth[ith]], ith);
        }

//////////////////////////////////////////////////////////////////////////////////////
public:  
        vector<Schedule> pbs;
        //pbi的得分属于什么名次
        vector<int> scoreth_ofpbi;
        //第i名是pb几
        vector<int> pbi_ofscoreth;

        int pbi;
        int ith;

        int show0h;
        int show1h;
        int show2h;
        int show3h;
        int leftw;
        int leftlabelen;
        int leftstartx;//排班按钮队列的左起始坐标
        int topy1;
        float lenplus;//tripb按钮宽度增益倍数

        vector<vector<QPushButton*>> tripB;
        vector<vector<TripsBshow*>> tripCao;

        vector<QPushButton*> slotB;
        vector<SlotsBshow*> slotCao;

        vector<QPushButton*> carsB;


        vector<QPushButton*> stationsB;


        vector<QLabel*> timehL;

        vector<QLabel*> workL;//用以显示每个司机的工作区间,与tripsb对应

        QPushButton* lastB;
        QPushButton* nextB;
        QPushButton* worseB;
        QPushButton* beterB;
        QPushButton* timeB;

        QTextBrowser* show1;//展示整个排班情况
        QTextBrowser* show2;//展示点中按钮的按钮trip发的车的信息
        QTextBrowser* show3;//展示点中按钮trip的车库信息

        vector<QLine> lines;


        void paintEvent(QPaintEvent *)
        {
                QPainter painter(this);
                painter.setPen(QPen(Qt::black, 1));//设置画笔形式  或者Qpen pen; pen.setColor(QColor(40, 115, 216)); pen.setWidth(2);
                for (int i = 0; i < lines.size(); i++)//画所有直线
                {
                        painter.drawLine(lines[i]);
                }              
        }

        void clearwindow()
        {
                delete timeB;
                show1->clear();
                show2->clear();
                show3->clear();

                for (int i = 0; i < tripB.size(); i++)
                {
                        for (int k = 0; k < tripB[i].size(); k++)
                        {
                                delete tripB[i][k];
                                delete tripCao[i][k];
                        }
                }
                tripB.clear();
                tripCao.clear();

                for (int i = 0; i < slotB.size(); i++)
                {
                                delete slotB[i];
                                delete slotCao[i];
                }
                slotB.clear();
                slotCao.clear();

                for (int i = 0; i < carsB.size(); i++)
                {
                        delete carsB[i];
                }
                carsB.clear();

                for (int i = 0; i < stationsB.size(); i++)
                {
                        delete stationsB[i];
                }
                stationsB.clear();

                for (int i = 0; i < timehL.size(); i++)
                {
                        delete timehL[i];
                }
                timehL.clear();

                for (int i = 0; i < workL.size(); i++)
                {
                        delete workL[i];
                }
                workL.clear();
                lines.clear();
        }
        
        void showindow()//在drawindow后会调用一下这个函数，用以重新显示界面，并重新连接槽函数
        {
                timeB->show();
                show1->show();
                show2->show();
                show3->show();

                for (int i = 0; i < tripB.size(); i++)
                {
                        for (int k = 0; k < tripB[i].size(); k++)
                        {         
                                connect(tripB[i][k], SIGNAL(clicked(bool)), tripCao[i][k], SLOT(printshow()));
                                tripB[i][k]->show(); 
                        }
                }

                for (int i = 0; i < slotB.size(); i++)
                {
                                connect(slotB[i], SIGNAL(clicked(bool)), slotCao[i], SLOT(printshow()));
                                slotB[i]->show();
                }
                for (int i = 0; i < carsB.size(); i++)
                {                       
                        carsB[i]->show();
                }
                for (int i = 0; i < stationsB.size(); i++)
                {
                        stationsB[i]->show();
                }
                for (int i = 0; i < timehL.size(); i++)
                {
                        timehL[i]->show();
                }
                for (int i = 0; i < workL.size(); i++)
                {
                        for (int j = 0; j < workL.size(); j++)
                        {
                                workL[i]->show();
                        }
                }
        }

        void drawindow(Schedule& pb,int scoreth, MainWindow* parent = 0)//paiban5& pb, 负责展示传进来的某一个排班
        {
                vector<QPushButton*> bt2;
                QPushButton* bt1;
                vector<TripsBshow*> bts2;
                SlotsBshow* sbw1;
                vector<TripsBshow*> bts1;
                QLabel* lt1;
               
                /////////////////////////////////////////////////////////////////////////////////////////
                int daystart = pb.tripque[0][0]->tp.tot;//一天开始时间
                int dayend = pb.endtime_ofday;


                int shigh = topy1 / 2;//dtation num
                //topy1 = shigh * pb.Slos.size();
                ////////////////////////////////////////////////////////////////////////////////////                          
                int showh = show1h + show2h + show3h;
                int bhigh = 35;//tripb按钮高度
                if (bhigh*pb.result.carsused > (show2h + show3h))
                {
                        bhigh = (show2h + show3h) / pb.result.carsused;
                }

                int timelen = (dayend - daystart) * lenplus;
                int rigendx = leftw + leftlabelen + timelen;

                lines.push_back(QLine(leftstartx, show1h, rigendx, show1h));//timebar与carB中间的线
                lines.push_back(QLine(leftstartx, topy1, rigendx, topy1));//timebar与slotB中间的线
                lines.push_back(QLine(0, showh, rigendx, showh));//slotB的底线

                QFont ft2;
                ft2.setPixelSize(18);//字体大小
                ft2.setFamily("Microsoft YaHei");
                for (int m = daystart; m <= dayend; m++)
                {                        
                        if (m % 60 == 0)
                        {
                                int xnow = leftstartx + lenplus * (m-daystart);
                                lines.push_back(QLine(xnow, topy1, xnow, showh));//时间刻度线

                                //timehL.push_back(new QLabel(QString::number(m/60) + ":00" + "\n " + QString::number(m), this));//每隔1h的时间刻度
                                timehL.push_back(new QLabel(QString::number(m / 60) + ":00", this));//每隔1h的时间刻度
                                timehL.back()->setStyleSheet("background-color: rgba(255,255,255,100%)");
                                timehL.back()->setWordWrap(1);
                                timehL.back()->setGeometry(xnow - 10, topy1 + 5, 50, 20);
                                timehL.back()->setFont(ft2);
                        }
                }
                lines.push_back(QLine(rigendx, 0, rigendx, showh));//右边边界线
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                timeB = new QPushButton("Time:", this);
                timeB->setStyleSheet("background-color: rgba(220,200,220,100%)");
                timeB->setGeometry(leftw, topy1, leftlabelen, show1h - topy1);
                timeB->setFont(ft2);
                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                for (int i = 0; i < 2; i++)
                {
                        stationsB.push_back(new QPushButton("CP " + QString::number(i), this));
                        stationsB[i]->setStyleSheet("background-color: rgba(210,210,200,100%)");
                        stationsB[i]->setGeometry(leftw, i*shigh, leftlabelen, shigh);
                        stationsB[i]->setFont(ft2);
                }
                for (int i = 0; i < pb.parks.size(); i++)
                {
                        int startime = pb.parks[i].tp.tot;
                        int len = pb.parks[i].tp.triptime;
                        int j = pb.parks[i].tp.fromCP;
                        int plus1 = 0;
                        if (j == 1)
                        {
                                plus1 = -shigh * 1 / 2;
                        }

                        slotB.push_back(new QPushButton("", this));
                        slotCao.push_back(new SlotsBshow(i, j, show2, show3));
                        slotB[i]->setGeometry(leftstartx + lenplus * (startime - daystart), j * shigh + plus1, lenplus * len, shigh + shigh * 1 / 2);

                        int color = 230 - (startime - daystart) * 180 / dayend;
                        QString col = QString::number(color);
                        QString tttt = "background-color: rgba(" + col + "," + col + "," + col + ",100%)";
                        slotB[i]->setStyleSheet(tttt);//"background-color: rgba(200,200,200,100%)"
                }
                //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                for (int i = 0; i < pb.tripque.size(); i++)//i是车号
                {
                        int startime = pb.tripque[i][0]->tp.tot;
                        int endtime = pb.tripque[i].back()->endt();
                        int cartype = 1;
                        if (pb.tripque[i].back()->chosen >= 0)
                        {
                                Car* car = &pb.tripque[i].back()->Cars[pb.tripque[i].back()->chosen];
                                cartype = car->type;
                        }
                        tripB.push_back(bt2); tripCao.push_back(bts2); 

                        int liney = show1h + 1 + i * bhigh;//(show2h+show3h) / pb.carsused;
                        lines.push_back(QLine(leftstartx, liney, rigendx, liney));//timebar与slotB中间的线

                        QString strc = "Veh " + QString::number(i+1);//表示车辆的按钮
                        carsB.push_back(new QPushButton(strc, this));
                        carsB[i]->setStyleSheet("background-color: rgba(200,200,200,100%)");
                        carsB[i]->setGeometry(leftw, liney, leftlabelen, bhigh);
                        carsB[i]->setFont(ft2);
                        ///////////////////////////worklabel：
                        workL.push_back(new QLabel("", this));//每隔1h的时间刻度
                        workL[i]->setGeometry(leftstartx + lenplus * (startime - daystart), liney, lenplus * (endtime - startime), bhigh);
                        workL[i]->setStyleSheet("background-color: rgba(255,255,200,0%)");
                        for (int k = 0; k < pb.tripque[i].size(); k++)//k是trip
                        {
                                startime = pb.tripque[i][k]->tp.tot;
                                int len = pb.tripque[i][k]->tp.triptime;

                                QString str = QString::number(k + 1);//pb.tripque[i][j][k]->Cars.size() +'/' + QString::number(pb.tripque[i][j][k]->Cars.size());// 
                                tripB[i].push_back(new QPushButton(str, this));//表示trip的按钮
                                tripCao[i].push_back(new TripsBshow(i,k,show2,show3,&pb));//表示trip的按钮槽函数结构
                                if(cartype == 0)
                                {
                                        tripB[i][k]->setStyleSheet("background-color: rgba(150,200,240,70%)");
                                }
                                else if (cartype == 1)
                                {
                                        tripB[i][k]->setStyleSheet("background-color: rgba(150,240,240,70%)");
                                }
                                else if (cartype == 2)
                                {
                                        tripB[i][k]->setStyleSheet("background-color: rgba(150,240,150,70%)");
                                }
                                tripB[i][k]->setGeometry(leftstartx + lenplus * (startime - daystart), liney, lenplus * len, bhigh);
                                tripB[i][k]->setFont(ft2);
                        }                       
                }
                ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                int carsused = pb.result.carsused;
                int drinum = pb.result.drivsused;

                QFont ft;
                ft.setPixelSize(15);//字体大小
                ft.setFamily("Microsoft YaHei");
                show1->setFont(ft);

                show1->append("pbi:" + QString::number(pbi + 1) + "/" + QString::number(pbs.size())
                );

                show1->append("max_trip:" + QString::number(pb.yueshu.max_tripnum)
                        + ",peaks:" + QString::number(pb.yueshu.gaofengused)
                );

                show1->append("");
                show1->append("topeak| Wplus |  p1  |  p2  |:");
                show1->append(
                          "  " + QString::number(pb.yueshu.to_peakcar_restime)
                        + "  |  " + QString::number(pb.yueshu.Wplus_backhome,'g',4)
                        + " | " + QString::number(pb.yueshu.P_used.ds[4].val(), 'g', 4)
                        + " | " + QString::number(pb.yueshu.P_used.ds[5].val(), 'g', 4));
                show1->append("");
                
                show1->append("cars: " + QString::number(carsused)
                        + " | dris: " + QString::number(drinum)
                        + " | even: " + QString::number(pb.result.unback_drivernum)
                        + " | track: " + QString::number(pb.result.backtrack_num)
                        + " | empts: " + QString::number(pb.result.nocar_slotnum)
                );

                show1->append("punish: " + QString::number(pb.result.punish)
                        + ",th: " + QString::number(scoreth));

                ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                this->resize(rigendx + 20, showh + 20);
                showindow();
                update();
        }
        //window初始化时，先定义好不变部分的按钮与label：lastb，nextb，
        MainWindow(onecase& incase, MainWindow* parent = 0)
        {
                pbs = incase.pbs;
                scoreth_ofpbi = incase.scoreth_ofpbi;
                pbi_ofscoreth = incase.pbi_ofscoreth;

                pbi = 0;
                ith = 0;

                show0h = 60;
                show1h = 250;
                show2h = 400;
                show3h = 200;
                leftw = 300;
                leftlabelen = 70;
                leftstartx = leftw + leftlabelen;//排班按钮队列的左起始坐标
                topy1 = 150;              
                ////////////////////////////////////////////////////////////////////////////////////                          

                //show2h = bhigh * pb.carsused;
                lenplus = 1.3;//tripb按钮宽度增益倍数

                lastB = new QPushButton("<-Last", this);
                lastB->setStyleSheet("background-color: rgba(200,220,220,100%)");
                lastB->setGeometry(0, 0, leftw / 2, show0h/2);               
                nextB = new QPushButton("Next->", this);
                nextB->setStyleSheet("background-color: rgba(220,200,200,100%)");
                nextB->setGeometry(leftw / 2, 0, leftw / 2, show0h/2);
                worseB = new QPushButton("<-worse", this);
                worseB->setStyleSheet("background-color: rgba(180,180,150,100%)");
                worseB->setGeometry(0, show0h/2, leftw / 2, show0h/2);
                beterB = new QPushButton("better->", this);
                beterB->setStyleSheet("background-color: rgba(150,150,180,100%)");
                beterB->setGeometry(leftw / 2, show0h / 2, leftw / 2, show0h / 2);

                show1 = new QTextBrowser(this);//展示整个排班情况
                show1->setGeometry(0, show0h, leftw, show1h);               
                show2 = new QTextBrowser(this);//展示点中按钮的信息
                show2->setGeometry(0, show1h, leftw, show2h);
                show3 = new QTextBrowser(this);//展示点中按钮的信息
                show3->setGeometry(0, show1h + show2h, leftw, show3h);


                connect(lastB, SIGNAL(clicked(bool)), this, SLOT(lastw()));
                connect(nextB, SIGNAL(clicked(bool)), this, SLOT(nextw()));
                connect(worseB, SIGNAL(clicked(bool)), this, SLOT(worsew()));
                connect(beterB, SIGNAL(clicked(bool)), this, SLOT(beterw()));

                drawindow(pbs[pbi_ofscoreth[ith]], ith);//pb
        }

        MainWindow(MainWindow* parent = 0)
        {

        }
};
