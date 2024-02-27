#pragma once
#include "pso.h"
#include <math.h>
#include <fstream>
//��������������pso�Ż����������Ŀ�����
//�������ǻ���pso�ģ��������pso���������һ��rbf����ʽ����Ϊ���
//��ģ�鲻��Ӱ��psoģ�������ʹ�ã�����pso��һ����չ


//������ڵ�,��¼����rbf������pso�е��±�
class inode
{
public:
	
	//����Ϊ���Ż�������pso�ڵ�ָ��
	vector<int> centers_ptr;//����������,sizeΪ�����ά��
	int miu_ptr;//exp����ϵ��
	int w_ptr;//��ԪȨֵ

	//����Ϊ���Ż�����
	vector<double> centers;//����������,sizeΪ�����ά��
	double miu;//exp����ϵ��
	double w;//��ԪȨֵ
	//
	inode()
	{
		centers_ptr.clear();
		miu_ptr = 0;
		w_ptr = 0;
	}
	inode(vector<int>& in_ptr1, int in_ptr2, int in_ptr3)
	{
		centers_ptr = in_ptr1;
		miu_ptr = in_ptr2;
		w_ptr = in_ptr3;
	}

	double out(P& p_used, vector<double>& inputs)
	{
		centers.clear();
		for (int i = 0; i < centers_ptr.size(); i++)
		{
			centers.push_back(p_used.ds[centers_ptr[i]].val());
		}
		miu = p_used.ds[miu_ptr].val();
		//miu = miu * miu;
		w = p_used.ds[w_ptr].val();

		double distance_squ = 0;
		for (int i = 0; i < inputs.size(); i++)
		{
			double minus = inputs[i] - centers[i];
			distance_squ += (minus) * (minus);
		}

		double result = w / exp(miu * distance_squ);
		return result;
	}
};


//������
class hiden_layer
{
public:
	int input_size;//������ģ
	int inode_numb;//�������ģ

	double b_miu1;
	double b_miu2;
	double b_w1;
	double b_w2;

	vector<inode> inodes;

	hiden_layer(double inb_miu1 = 0.0001, double inb_miu2 = 1, double inb_w1 = -100, double inb_w2 = 100)
	{
		input_size = 0;
		inode_numb = 0;

		b_miu1 = inb_miu1;
		b_miu2 = inb_miu2;
		b_w1 = inb_w1;
		b_w2 = inb_w2;
	}

	vector<double> inb1;
	vector<double> inb2;
	vector<double> inbspd;
	//����һά����,��Ҫ����ÿһά�����pso��Ӧλ���ٶȷ�Χ������
	void add_input_size(double inbound1, double inbound2, double inboundspd)
	{
		input_size++;

		inb1.push_back(inbound1);
		inb2.push_back(inbound2);
		inbspd.push_back(inboundspd);
	}

    //����������������˳���ʼ���ṹ��
	void set_inode_numb(int numb,Swa& S)
	{
		qsrand(QTime(0, 0, 0).secsTo(QTime::currentTime()));

		if (numb <= 0)numb = 1;

		for (int i = 0; i < numb; i++)
		{
			inodes.push_back(inode());//��ʼ��inodes

			for (int j = 0; j < input_size; j++)
			{
				inodes.back().centers_ptr.push_back(S.d_num);//��ʼ��inodes
				S.cin_new_d(inb1[j], inb2[j], inbspd[j]);//����������
			}
			inodes.back().miu_ptr = S.d_num;//��ʼ��inodes
			S.cin_new_d(b_miu1, b_miu2, 0.2*(b_miu2 - b_miu1));//������miuϵ��
			inodes.back().w_ptr = S.d_num;//��ʼ��inodes
			S.cin_new_d(b_w1, b_w2, 0.2 * (b_w2 - b_w1));//�ڵ�Ȩֵw
		}
	}

	double out_tot(P& p_used, vector<double>& inputs)
	{
		double output = 0;
		for (int i = 0; i < inodes.size(); i++)
		{
			output += inodes[i].out(p_used, inputs);
		}
		return output;
	}

	double best_out_tot(P& p_used, vector<double>& inputs)
	{
		double output = 0;
		for (int i = 0; i < inodes.size(); i++)
		{
			output += inodes[i].out(p_used, inputs);
		}
		return output;
	}

	
	void print_to_excel(string fname, Swa& S)
	{
		ofstream oFile;

		oFile.open(fname + ".csv", ios::out | ios::trunc);//

		for (int i = 0; i < S.Max_epoch; i++)////////////////////////////////////////�Ű���ѭ��
		{
			for (int j = 0; j < S.gr_num; j++)
			{
				oFile << "No epo p:" << "," << i * S.Max_epoch + j + 1 << "," << i + 1 << "," << j + 1 << ",";//����š�epoch���������
				oFile << "punish:" << "," << S.grs[j].ps[i].punish << ",";
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
				    oFile << "," << S.grs[j].ps[i].ds[k].value;
					//if (k % 3 == 2)oFile << ",";
				}

				oFile << endl;
			}
		}
		oFile.close();
	}
};