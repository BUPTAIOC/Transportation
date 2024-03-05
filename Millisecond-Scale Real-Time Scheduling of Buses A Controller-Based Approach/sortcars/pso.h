#pragma once
//#include "Timepoint.h"
#include <cstdlib>
#include <ctime>

#include <string>
#include <iostream>
#include <vector>
#include <map>

using namespace std;
// One dimension.
class d
{
public:

        double value;
        double speed;

        double bound_1;
        double bound_2;
        double bound_spd;

        d()
        {
                value = 0;
                speed = 0;
                bound_1 = 0; 
                bound_2 = 0;
                bound_spd = 0;
        }

        d(double inbound1,
          double inbound2,
          double inboundspd,
          double inval = 0,
          double inspeed = 0)
        {
                bound_1 = inbound1;
                bound_2 = inbound2;
                bound_spd = inboundspd;

                value = inval;
                speed = inspeed;
        }

        void init()
        {
            value = bound_1        + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (bound_2 - bound_1)));
            speed = -1 * bound_spd + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (2 * bound_spd)));
        }

        void value_bounder()
        {
                if (value < bound_1)
                {
                        value = bound_1;
                }
                else if (value > bound_2)
                {
                        value = bound_2;
                }
        }
        void speed_bounder()
        {
                if (speed < -1 * bound_spd)
                {
                        speed = -1 * bound_spd;
                }
                else if (speed > bound_spd)
                {
                        speed = bound_spd;
                }
        }

        double val()
        {
                if (value < bound_1)
                {
                        return bound_1;
                }
                if (value > bound_2)
                {
                        return bound_2;
                }
                return value;
        }
};

// A position composed of several dimensions.
class P
{
public:
        vector<d> ds;

        double punish;

        P()
        {
                punish = 9999999.99;
        }

        P(int dnum)
        {
                d td;
                for (int i = 0; i < dnum; i++)
                {
                        ds.push_back(td);
                }
                punish = 9999999;
        }

        void copyP(P& inP)
        {
                ds = inP.ds;
                punish = inP.punish;
        }

        // clarify which function you are referring to and provide more context or code so I can assist you better.
        void init()
        {
                for (int i = 0; i < ds.size(); i++)
                {
                        ds[i].init();
                }
        }

        void add_d(double inval,
                double inspeed,
                double inbound1,
                double inbound2,
                double inboundspd)
        {
                d td(inval, 
                     inspeed,
                     inbound1,
                     inbound2,
                     inboundspd);
                ds.push_back(td);
        }
        
        void next_value()
        {
                for (int i = 0; i < ds.size(); i++)
                {
                        ds[i].value += ds[i].speed;
                        ds[i].value_bounder();
                }
        }
};
//generation
class Gr
{
public:
        vector<P> ps;
        int i_of_best_p;

        P * gr_bestP()
        {
                return & ps[i_of_best_p];
        }
        double gr_bestPunish()
        {
                return ps[i_of_best_p].punish;
        }

        //Particles acquire a random initial position.
        void init(P& inP)
        {
                ps.push_back(inP);
                ps.back().init();
        }

        Gr()
        {
                i_of_best_p = 0;
        }
};
//swarm
class Swarm
{
public:
        int d_num;
        int gr_num;
        int Max_epoch;
        
        double oumiga;
        double c1;
        double c2;

        // Standard position vector
        // Contains dimension information, dimension boundaries
        P p_standard;
        // grs[i] represents bird i
        vector<Gr> grs;
        // Obtain the bird_i with the best historical score
        int i_of_best_gr;

        double sw_bestPunish()
        {
                return grs[i_of_best_gr].gr_bestPunish();
        }

        double sw_avgPunish(int epoch)
        {
                double total = 0;
                for (int i = 0; i < grs.size(); i++)
                {
                        total += grs[i].ps[epoch].punish;
                }
                return total / gr_num;
        }

        P* sw_bestP()
        {
                return grs[i_of_best_gr].gr_bestP();
        }

        Swarm()
        {
                d_num = 0;
                gr_num = 0;
                Max_epoch = 0;
                oumiga = 0;
                c1 = 0;
                c2 = 0;
        }
      
        Swarm(int in_grnum,int in_max_epoch,double in_oumiga, double in_c1, double in_c2)
        {
                d_num = 0;
                gr_num = in_grnum;
                Max_epoch = in_max_epoch;
                oumiga = in_oumiga;
                c1 = in_c1;
                c2 = in_c2;

                i_of_best_gr = -1;
                grs.clear();
        }

        void Swa_copy(Swarm inswa)
        {
                d_num = inswa.d_num;
                gr_num = inswa.gr_num;
                Max_epoch = inswa.Max_epoch;
                oumiga = inswa.oumiga;
                c1 = inswa.c1;
                c2 = inswa.c2;

                p_standard = inswa.p_standard;
                grs = inswa.grs; i_of_best_gr = inswa.i_of_best_gr;
        }

        // For each dimension, call this function once
        // Construct the standard vector
        void cin_new_d(double inbound1,double inbound2,double inboundspd, double inval=0, double inspeed=0)
        {
                if (inboundspd <= 1)inboundspd = (inbound2 - inbound1) / 5;
                d td(inbound1,inbound2,inboundspd,inval,inspeed);
                p_standard.ds.push_back(td);
                d_num++;
        }

        // After defining all dimensions, call this function to construct the initial population
        int init_swa()
        {
                //qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
                srand(static_cast <unsigned> (time(0)));

                if (d_num == 0)
                {
                        return 0;
                }

                grs.clear();
                Gr tgr = Gr();
                for (int i = 0; i < gr_num; i++)
                {                        
                        grs.push_back(tgr);
                        grs[i].init(p_standard);
                }

                return d_num;
        }

        // Initialize this population with the final positions and velocities of another population's last generation.
        int init_swa(Swarm inS)
        {
                //qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));
                srand(static_cast <unsigned> (time(0)));

                if (d_num == 0)
                {
                        return 0;
                }

                grs.clear();
                Gr tgr = Gr();
                for (int i = 0; i < gr_num; i++)
                {
                        grs.push_back(tgr);
                        grs[i].init(p_standard);                      
                        grs[i].ps[0] = inS.grs[i].ps.back();
                }

                return d_num;
        }

        // Update the particle's best position (p), 
        // the swarm's best position (p), velocity, 
        // and next position (nextp) after i steps.
        void get_next_ps(int epoch_i)
        {
                //////////////////////////////////////////
                // Update the best position and fitness.
                if (epoch_i == 0)// Update for the first time.
                {
                        i_of_best_gr = 0;
                        int bestPunish = grs[i_of_best_gr].gr_bestPunish(); // This is the pointer to the global best.
                        for (int i = 0; i < grs.size(); i++)
                        {
                                grs[i].i_of_best_p = 0;// Pointing to best_x.

                                if (grs[i_of_best_gr].gr_bestPunish() > grs[i].gr_bestPunish())
                                {
                                        i_of_best_gr = i;
                                }
                        }
                }
                else
                {
                        for (int i = 0; i < grs.size(); i++)
                        {
                                if (grs[i].gr_bestPunish() > grs[i].ps[epoch_i].punish)
                                {
                                        grs[i].i_of_best_p = epoch_i;

                                        if (grs[i_of_best_gr].gr_bestPunish() > grs[i].gr_bestPunish())
                                        {
                                                i_of_best_gr = i;
                                        }
                                }
                        }
                }

                // Update the next velocity and position of the particles.
                // Update velocity first, then update position.
                for (int i = 0; i < grs.size(); i++)
                {
                        grs[i].ps.push_back(grs[i].ps.back());

                        for (int k = 0; k < p_standard.ds.size(); k++)
                        {
                                d*aim_d = &grs[i].ps.back().ds[k];

                                //V = w*V + C1*rand(0,1)(P_bird-X) + C2*rand(0,1)(P_swam-X)
                                double rand1 = double(rand() % 1000) / 1000;
                                double rand2 = double(rand() % 1000) / 1000;
                                double oldp = aim_d->value;
                              
                                aim_d->speed = 
                                oumiga * aim_d->speed +
                                c1 * rand1*(grs[i].ps[grs[i].i_of_best_p].ds[k].value - oldp) +
                                c2 * rand2*(grs[i_of_best_gr].ps[grs[i_of_best_gr].i_of_best_p].ds[k].value - oldp);// Update velocity.

                                aim_d->speed_bounder();
                        }
                        // Update position.
                        grs[i].ps.back().next_value();
////////////////////////////////////////////////////////////////////////////////////////////////////////////
                }

                return ;
        }
};

/*
                        Swarm S;
                        S = Swarm(swam_size, evaluation_times, 0.8, 0.3, 0.3);//swarm size£¬epoch£¬oumiga£¬Personal best attraction, swarm best attraction.
                        S.cin_new_d(6, 7, 1);//constrains.P_used.ds[0]//tripnum
                        S.cin_new_d(46, 48, 1);//constrains.P_used.ds[1]//to peakcar restime
                        S.cin_new_d(1386, 1388, 1);//constrains.P_used.ds[2]// b plus
                        S.cin_new_d(19, 20, 5);//constrains.P_used.ds[3]//sleep minus
                        S.cin_new_d(3.2, 3.4, 1);//constrains.P_used.ds[4]//pow1
                        S.cin_new_d(3.84, 3.86, 1);//constrains.P_used.ds[5]//pow2
                        S.cin_new_d(300, 660, 60);//constrains.P_used.ds[6]//last new car time
                        S.cin_new_d(10, 30, 2);//constrains.P_used.ds[7]//min newcar num
                        S.init_swa();
                        for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////Scheduling main loop.
                        {
                                for (int j = 0; j < S.grs.size(); j++) {
                                    f.change_P(&S.grs[j].ps[i]); // Update particle group parameters to the constraint structure
                                    S.grs[j].ps[i].punish = evaluate(); // Write back the fitness evaluation value
                                }
                                S.get_next_ps(i); // Update the population to the next generation
                                printf("add pso epoch:%d, avg_punish:%f, bestPunish:%f \n", i, S.sw_avgPunish(i), S.sw_bestPunish());
                        }
                        print_to_excel(line_name + "/ofline_test_mod1", S, schedules);// Input particle trajectories into a file.
*/