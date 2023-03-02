import random
import numpy as np
import torch
from torch.utils.data import Dataset

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')


# Generate problem instance data
class CustomizedBusDataset(Dataset):
    ''' ---------------------------------------
        (1)num_samples：int, the sample size of the instance.
        (2)input_size：int, The number of stations of the instance.
        (3)max_load：int, Maximum capacity per vehicle.
        (4)max_demand: int, .
        (5)seed：random number.
        (6)all_station：
        (7)travel_time_G:
        (8)Loc_x: List of X coordinates for each station.
        (9)Loc_y: List of X coordinates for each station.
        (10)up_down_index：int,
        (11)depot：i.e. CP
        (12)home_id:
        (13)work_id:'''
    def __init__(self, num_samples, input_size, max_load, max_demand, seed, all_station, travel_time_G, Loc_x, Loc_y, up_down_index, depot, home_id, work_id):
        super(CustomizedBusDataset, self).__init__()

        if max_load < max_demand:
            raise ValueError(':param max_load: must be > max_demand')

        if seed is None:
            seed = np.random.randint(1234567890)
        np.random.seed(seed)
        torch.manual_seed(seed)

        self.num_samples = num_samples
        self.max_load = max_load
        self.max_demand = max_demand
        self.max_time = 150  # Total service time of CB (minutes)
        self.wait_time = 10  # Maximum waiting time
        '''Space-time distribution of bus line website sites: 
        X coordinate, Y coordinate, expected alighting time, shortest travel time between stations'''
        space_time_demand = torch.rand((num_samples, len(all_station) + 8, input_size))
        '''Record the dynamic information of travel demand to calculate the reward function. 
        [get off id, expected time to get off, number of pre-booked passengers, number of real-time reserved passengers, 
        number of canceled passengers, actual arrival time, assigned agent number, counted number of people getting off]'''
        problem_constraint1 = torch.rand((num_samples, 8, input_size))
        problem_constraint2 = torch.rand((num_samples, 8, input_size))
        problem_constraint3 = torch.rand((num_samples, 8, input_size))

        '''Observation information of each agent: real-time location, real-time time, arrival time at the station,
         departure time, distribution of passengers, and travel time'''
        trip_time = torch.rand((num_samples, 1, input_size))
        real_time = torch.rand((num_samples, 1, input_size))
        arrival_time = torch.rand((num_samples, 1, input_size))
        departure_time = torch.rand((num_samples, 1, input_size))
        passenger_flow = torch.rand((num_samples, 1, input_size))
        travel_time = torch.rand((num_samples, 1, input_size))
        decision_state1 = torch.rand((num_samples, 1, input_size))
        decision_state2 = torch.rand((num_samples, 1, input_size))
        decision_state3 = torch.rand((num_samples, 1, input_size))
        decision_state4 = torch.rand((num_samples, 1, input_size))
        decision_state5 = torch.rand((num_samples, 1, input_size))
        decision_state6 = torch.rand((num_samples, 1, input_size))
        decision_state7 = torch.rand((num_samples, 1, input_size))
        decision_state8 = torch.rand((num_samples, 1, input_size))
        decision_state9 = torch.rand((num_samples, 1, input_size))
        decision_state10 = torch.rand((num_samples, 1, input_size))

        for i in range(num_samples):
            for j in range(len(space_time_demand[i][0])):
                space_time_demand[i][0][j] = Loc_x[j]
            for j in range(len(space_time_demand[i][1])):
                space_time_demand[i][1][j] = Loc_y[j]
            for j in range(len(problem_constraint1[i][0])):
                if 0 < j < up_down_index[0]:
                    problem_constraint1[i][0][j] = random.choice([i for i in range(1, up_down_index[1]+1)])
                else:
                    problem_constraint1[i][0][j] = 0
            for j in range(len(problem_constraint2[i][0])):
                if 0 < j < up_down_index[0]:
                    problem_constraint2[i][0][j] = random.choice([i for i in range(1, up_down_index[1]+1)])
                else:
                    problem_constraint2[i][0][j] = 0
            for j in range(len(problem_constraint3[i][0])):
                if 0 < j < up_down_index[0]:
                    problem_constraint3[i][0][j] = random.choice([i for i in range(1, up_down_index[1]+1)])
                else:
                    problem_constraint3[i][0][j] = 0
            for j in range(len(space_time_demand[i][2])):
                if 0 < j < up_down_index[0]:
                    arr_time = travel_time_G[home_id[j - 1]][work_id[int(problem_constraint1[i][0][j]) - 1]]
                    # space_time_demand[i][2][j] = (travel_time_G[depot[0]][home_id[j - 1]] * random.randint(1, max(int((20-arr_time)/travel_time_G[depot[0]][home_id[j - 1]]-1), 1)) + random.randint(0, 5)) / self.max_time
                    space_time_demand[i][2][j] = (travel_time_G[depot[0]][home_id[j - 1]] * random.randint(1, max(int((20 - arr_time) / travel_time_G[depot[0]][home_id[j - 1]] - 1), 1)) + random.randint(0, 5)) / self.max_time
                    problem_constraint1[i][1][j] = max(float(space_time_demand[i][2][j]) + (arr_time + random.randint(10, 15)) / self.max_time, 1)
                elif j == 0:
                    space_time_demand[i][2][j] = 0
                elif j >= up_down_index[0]:
                    space_time_demand[i][2][j] = 1
            for j in range(len(space_time_demand[i][3])):
                if 0 < j < up_down_index[0]:
                    arr_time = travel_time_G[home_id[j - 1]][work_id[int(problem_constraint2[i][0][j]) - 1]]
                    space_time_demand[i][3][j] = space_time_demand[i][2][j] + random.randint(1, 10) / self.max_time
                    problem_constraint2[i][1][j] = max(float(space_time_demand[i][3][j]) + (arr_time + random.randint(10, 15)) / self.max_time, 1)
                elif j == 0:
                    space_time_demand[i][3][j] = 0
                elif j >= up_down_index[0]:
                    space_time_demand[i][3][j] = 1
            for j in range(len(space_time_demand[i][4])):
                if 0 < j < up_down_index[0]:
                    arr_time = travel_time_G[home_id[j - 1]][work_id[int(problem_constraint3[i][0][j]) - 1]]
                    space_time_demand[i][4][j] = space_time_demand[i][3][j] + random.randint(1, 10) / self.max_time
                    problem_constraint3[i][1][j] = max(float(space_time_demand[i][4][j]) + (arr_time + random.randint(10, 15)) / self.max_time, 1)
                elif j == 0:
                    space_time_demand[i][4][j] = 0
                elif j >= up_down_index[0]:
                    space_time_demand[i][4][j] = 1
            for j in range(len(space_time_demand[i][5])):
                if 0 < j < up_down_index[0]:
                    space_time_demand[i][5][j] = float(space_time_demand[i][2][j]) + self.wait_time / self.max_time
                elif j == 0:
                    space_time_demand[i][5][j] = 0
                else:
                    space_time_demand[i][5][j] = 1
            for j in range(len(space_time_demand[i][6])):
                if 0 < j < up_down_index[0]:
                    space_time_demand[i][6][j] = float(space_time_demand[i][3][j]) + self.wait_time / self.max_time
                elif j == 0:
                    space_time_demand[i][6][j] = 0
                else:
                    space_time_demand[i][6][j] = 1
            for j in range(len(space_time_demand[i][7])):
                if 0 < j < up_down_index[0]:
                    space_time_demand[i][7][j] = float(space_time_demand[i][4][j]) + self.wait_time / self.max_time
                elif j == 0:
                    space_time_demand[i][7][j] = 0
                else:
                    space_time_demand[i][7][j] = 1
            for k in range(8, len(all_station)+8):
                for j in range(len(space_time_demand[i][k])):
                    space_time_demand[i][k][j] = travel_time_G[all_station[j]][all_station[k-8]] / self.max_time
            for j in range(len(trip_time[i][0])):
                trip_time[i][0][j] = travel_time_G[all_station[0]][all_station[j]] / self.max_time
                real_time[i][0][j] = 0
                arrival_time[i][0][j] = 0
                departure_time[i][0][j] = 0
                passenger_flow[i][0][j] = 0
                travel_time[i][0][j] = 0
                decision_state1[i][0][j] = 0
                decision_state2[i][0][j] = 2 / self.max_time
                decision_state3[i][0][j] = 4 / self.max_time
                decision_state4[i][0][j] = 6 / self.max_time
                decision_state5[i][0][j] = 8 / self.max_time
                decision_state6[i][0][j] = 10 / self.max_time
                decision_state7[i][0][j] = 12 / self.max_time
                decision_state8[i][0][j] = 14 / self.max_time
                decision_state9[i][0][j] = 16 / self.max_time
                decision_state10[i][0][j] = 18 / self.max_time
            for con in range(2, 8):
                for j in range(len(problem_constraint1[i][con])):
                    problem_constraint1[i][con][j] = 0
                for j in range(len(problem_constraint2[i][con])):
                    problem_constraint2[i][con][j] = 0
                for j in range(len(problem_constraint3[i][con])):
                    problem_constraint3[i][con][j] = 0
        # CP location will be the first station.
        # locations = torch.rand((num_samples, 2, input_size + 1))
        self.static = space_time_demand
        dynamic_shape = (num_samples, 1, input_size)
        loads = torch.full(dynamic_shape, 1.)
        demands1 = torch.randint(0, max_demand, dynamic_shape)
        demands1 = demands1 / float(max_load)
        demands1[:, 0, 0] = 0  # CP starts with a demand of 0
        demands1[:, 0, -len(work_id):] = 0
        demands2 = torch.randint(0, max_demand, dynamic_shape)
        demands2 = demands2 / float(max_load)
        demands2[:, 0, 0] = 0  # CP starts with a demand of 0
        demands2[:, 0, -len(work_id):] = 0
        demands3 = torch.randint(0, max_demand, dynamic_shape)
        demands3 = demands3 / float(max_load)
        demands3[:, 0, 0] = 0  # CP starts with a demand of 0
        demands3[:, 0, -len(work_id):] = 0
        self.problem_record1 = problem_constraint1
        self.problem_record2 = problem_constraint2
        self.problem_record3 = problem_constraint3
        self.problem_record1[:, 2] = demands1[:, 0]
        self.problem_record2[:, 2] = demands2[:, 0]
        self.problem_record3[:, 2] = demands3[:, 0]
        self.problem_record1[:, 1, 0] = 0
        self.problem_record2[:, 1, 0] = 0
        self.problem_record3[:, 1, 0] = 0
        self.problem_record1[:, 1, -1] = 0
        self.problem_record2[:, 1, -1] = 0
        self.problem_record3[:, 1, -1] = 0
        self.problem_record1[:, 1, -2] = 0
        self.problem_record2[:, 1, -2] = 0
        self.problem_record3[:, 1, -2] = 0
        departure_time[:, 0, 0] = 0
        self.dynamic1 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state1), axis=1))

        departure_time[:, 0, 0] = 2 / self.max_time
        self.dynamic2 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state2), axis=1))

        departure_time[:, 0, 0] = 4 / self.max_time
        self.dynamic3 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state3), axis=1))
        # 添加智能体信息
        departure_time[:, 0, 0] = 6 / self.max_time
        self.dynamic4 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state4), axis=1))

        departure_time[:, 0, 0] = 8 / self.max_time
        self.dynamic5 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state5), axis=1))

        departure_time[:, 0, 0] = 10 / self.max_time
        self.dynamic6 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state6), axis=1))

        departure_time[:, 0, 0] = 12 / self.max_time
        self.dynamic7 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state7), axis=1))

        departure_time[:, 0, 0] = 14 / self.max_time
        self.dynamic8 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state8), axis=1))

        departure_time[:, 0, 0] = 16 / self.max_time
        self.dynamic9 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state9), axis=1))

        departure_time[:, 0, 0] = 18 / self.max_time
        self.dynamic10 = torch.tensor(np.concatenate(
            (loads, demands1, demands2, demands3, trip_time, real_time, arrival_time, departure_time, passenger_flow,
             travel_time, decision_state10), axis=1))

    def __len__(self):
        return self.num_samples

    def __getitem__(self, idx):
        return (self.static[idx], self.dynamic1[idx], self.dynamic2[idx], self.dynamic3[idx], self.dynamic4[idx],
                self.dynamic5[idx], self.dynamic6[idx], self.dynamic7[idx], self.dynamic8[idx], self.dynamic9[idx],
                self.dynamic10[idx], self.static[idx, :, 0:1], self.problem_record1[idx], self.problem_record2[idx],
                self.problem_record3[idx])
