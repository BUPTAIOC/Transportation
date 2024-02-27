#pragma once
#include <string>
#include <vector>
#include <stdio.h>
#include <iostream>
using namespace std;

//�����࣬��ʾ��������ȡֵ����,����ȡ��жȡ��ضȵ�
class attribute_level
{
public:
	//�������
	string level_name;
	//����ֵ���κ�������,���κ����� �������� ���Ҷ˵��x���ꣻĬ�����θ߶�Ϊ1
	double x1;
	double x2;
	double x3;
	double x4;

	double out(double x_input)
	{
		double k;
		double y;
		if (x_input <= x1)
		{
			y = 0;
		}
		else if (x_input < x2)
		{
			y = (x_input - x1) / (x2 - x1);
		}
		else if (x_input < x3)
		{
			y = 1;
		}
		else if (x_input < x4)
		{
			y = 1 - (x_input - x3) / (x4 - x3);
		}
		else
		{
			y = 0;
		}

		return y;
	}

	attribute_level(string in_name, double in_x1 = 0, double in_x2 = 0, double in_x3 = 0, double in_x4 = 0)
	{
		level_name = in_name;
		x1 = in_x1;
		x2 = in_x2;
		x3 = in_x3;
		x4 = in_x4;
	}
};


//�����࣬��ʾ���ࡢ��֬����������������
class attribute
{
public:
	string attribute_name;
	//�м����������
	vector<attribute_level> levels;

	attribute(string in_name)
	{
		attribute_name = in_name;
	}

	void add_level(string in_levelname, double in_x1, double in_x2, double in_x3, double in_x4)
	{
		levels.push_back(attribute_level(in_levelname, in_x1, in_x2, in_x3, in_x4));
	}
};

//ģ�����ƹ���IF THEN
class rule
{
public:
	//IF������
	string rule_name;

	vector<int> attribute_id;
	vector<int> attribute_levelid;
	int result_id;
	int result_levelid;

	rule(string in_rule_name, 
		 vector<int> in_attribute,
		 vector<int> in_level,
		 int result_no,
		 int result_level)
	{
		rule_name = in_rule_name;

		attribute_id = in_attribute;
		attribute_levelid = in_level;

		result_id = result_no;
		result_levelid = result_level;
	}
};

class FZsystem
{
public:
	//ģ��ϵͳ����
	string system_name;
	//ģ�����Լ�
	vector<attribute> attributes_set;
	//ģ������
	vector<rule> rules_set;
	//i-th����Ŀ��,Ҳ�����Ա�ʾ����ϴʱ��� �����У��̵ȣ�
	vector<attribute> outs;

	FZsystem(string in_name)
	{
		system_name = in_name;
	}

	//ģ�����Լ������������
	void add_attribute(string attribute_name,
		int level_num, 
		vector<string> level_name,
		vector<double> f_left1,
		vector<double> f_left2,
		vector<double> f_right1, 
		vector<double> f_right2)
	{
		attributes_set.push_back(attribute(attribute_name));
		for (int i = 0; i < level_num; i++)
		{
			attributes_set.back().add_level(level_name[i], f_left1[i], f_left2[i], f_right1[i], f_right2[i]);
		}
	}

	//����½��
	void add_result(string result_name,int level_num,vector<string> level_name, vector<double> level_center)
	{
		attributes_set.push_back(attribute(result_name));
		for (int i = 0; i < level_num; i++)
		{
			attributes_set.back().add_level(level_name[i], level_center[i], 0,0,0);
		}
		outs.push_back(attribute(result_name));
		for (int i = 0; i < level_num; i++)
		{
			outs.back().add_level(level_name[i], level_center[i], 0,0,0);
		}
	}

	//�����ģ������
	void add_rule(string in_rule_name,
				  vector<string> in_attribute_name,
				  vector<string> in_attribute_levelname,
				  string in_result_name,
				  string in_result_levelname)
	{
		vector<int> attribute_id;
		vector<int> attribute_levelid;
		int result_id = -1;
		int result_levelid = -1;

		for (int att_th = 0; att_th < in_attribute_name.size(); att_th++)
		{
			for (int i = 0; i < attributes_set.size(); i++)
			{
				if (attributes_set[i].attribute_name == in_attribute_name[att_th])
				{
					attribute_id.push_back(i);
				}
			}
			if(attribute_id.size() <= att_th)cout << "] attribute_name error: there is no this attribute_name" << "[" << in_attribute_name[att_th] <<  endl;

			for (int i = 0; i < attributes_set[attribute_id[att_th]].levels.size(); i++)
			{
				if (attributes_set[attribute_id[att_th]].levels[i].level_name == in_attribute_levelname[att_th])
				{
					attribute_levelid.push_back(i);
				}
			}
			if (attribute_levelid.size() <= att_th)cout << "] attribute_levelname error: there is no this attribute_levelname" << "[" << in_attribute_levelname[att_th] << endl;

			for (int i = 0; i < outs.size(); i++)
			{
				if (outs[i].attribute_name == in_result_name)
				{
					result_id = i;
				}
			}
			if (result_id == -1)cout << "] result_name error: there is no this result_name" << "[" << in_result_name << endl;

			for (int i = 0; i < outs[result_id].levels.size(); i++)
			{
				if (outs[result_id].levels[i].level_name == in_result_levelname)
				{
					result_levelid = i;
				}
			}
			if (result_levelid == -1)cout << "] result_levelname error: there is no this result_levelname" << "[" << in_result_levelname << endl;
		}

		rules_set.push_back(rule(in_rule_name, attribute_id, attribute_levelid, result_id, result_levelid));
	}


	//�������Ľ��ֵת��Ϊ����ֵ���������룩
	vector<vector<int>> result_to_int(vector<vector<double>> result)
	{
		vector<vector<int>> out;
		out.resize(result.size());

		for (int i = 0; i < result.size(); i++)
		{
			for (int j = 0; j < result[i].size(); j++)
			{
				if (result[i][j] - int(result[i][j]) < 0.5)
				{
					out[i].push_back(int(result[i][j]));
				}
				else
				{
					out[i].push_back(int(result[i][j]) + 1);
				}
			}
		}
		return out;
	}

	//���ݸ����Եľ�ȷ����x��ģ�������ó�����y; ���ֵΪi-th���۵�j-model ���۽��; modle 0:max,1:avg
	vector<vector<double>> FZreasoning(vector<double> x)
	{
		vector<double> t;

		vector<vector<double>> membership;//��������x�������i���Ե�j���������ֵ����	

		vector<vector<vector<double>>> belongs;//i-out��j-level�ĵ�k������ľ���ֵ

		vector<vector<double>> y;//���ֵout, ���ֵΪi-th���۵�j-model ���۽��

		belongs.resize(outs.size());
		y.resize(outs.size());
		for (int i = 0; i < belongs.size(); i++)
		{
			belongs[i].resize(outs[i].levels.size());
		}
		//�ȼ���������Ե������Ⱦ���
		for (int i = 0; i < x.size(); i++)
		{
			membership.push_back(t);
			membership[i].resize(attributes_set[i].levels.size());

			for (int j = 0; j < attributes_set[i].levels.size(); j++)
			{
				membership[i][j] = attributes_set[i].levels[j].out(x[i]);//������ֵ
			}
		}
		//�ټ����������ľ���ֵ
		for (int i = 0; i < rules_set.size(); i++)
		{
			double AND_membership = 1;//�������������ȳ˻�
			for (int att_th = 0; att_th < rules_set[i].attribute_id.size(); att_th++)
			{
				AND_membership *= 
				membership[rules_set[i].attribute_id[att_th]][rules_set[i].attribute_levelid[att_th]];
			}

			belongs[rules_set[i].result_id][rules_set[i].result_levelid].push_back
			(
				AND_membership
			);
		}

		//�ȼ���max
		vector<vector<double>> tot_belongs; tot_belongs.resize(belongs.size());
		for (int i = 0; i < belongs.size(); i++)//i-job
		{
			tot_belongs[i].resize(belongs[i].size());
			for (int j = 0; j < belongs[i].size(); j++)//j-level
			{
				for (int k = 0; k < belongs[i][j].size(); k++)//k-rule
				{
					tot_belongs[i][j] += belongs[i][j][k];
				}
			}
		} 
		for (int i = 0; i < tot_belongs.size(); i++)//i-job
		{
			double out_level = -1;
			double max_sum = -1;
			for (int j = 0; j < tot_belongs[i].size(); j++)//j-level
			{
				if (max_sum < tot_belongs[i][j])
				{
					max_sum = tot_belongs[i][j];
					out_level = outs[i].levels[j].x1;
				}
			}
			y[i].push_back(out_level);
		}
		
		//�ټ���avg
		for (int i = 0; i < belongs.size(); i++)//i-job
		{
			double tot_belongs = 0;
			double tot_out = 0;
			for (int j = 0; j < belongs[i].size(); j++)//j-level
			{
				for (int k = 0; k < belongs[i][j].size(); k++)
				{
					tot_belongs += belongs[i][j][k];
					tot_out += outs[i].levels[j].x1 * belongs[i][j][k];
				}
			}
			if (tot_out == 0)
			{
				y[i].push_back(0);
			}
			else
			{
				y[i].push_back(tot_out / tot_belongs);
			}
		}

		return y;
	}

};