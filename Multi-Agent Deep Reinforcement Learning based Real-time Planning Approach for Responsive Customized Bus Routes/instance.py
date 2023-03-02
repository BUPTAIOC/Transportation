"""Defines the main trainer model for combinatorial problems

Each task must define the following functions:
* mask_fn: can be None
* update_fn: can be None
* reward_fn: specifies the quality of found solutions
* render_fn: Specifies how to plot found solutions. Can be None
"""
import csv
import argparse
import datetime
import torch
import copy

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
# device = torch.device('cpu')


def train_pdptw(args, up_station, all_station, travel_time_G, Loc_x, Loc_y, up_down_index, depot, home_id, work_id,
                max_loc_x, max_loc_y, CB_task):
    Loss_list, Reward_list = [], []
    # Determine the type of scenario and CB network.
    from Offline_scenario import Off_CB_Sioux0_mpdptw, Off_CB_Sioux1_mpdptw, Off_CB_Major_mpdptw
    from Online_scenario import On_CB_Sioux0_mpdptw, On_CB_Sioux1_mpdptw, On_CB_Major_mpdptw
    if CB_task == 'Sioux_0':
        from CB_instances.sioux0_magent_pdptw import CustomizedBusDataset
        MAX_DEMAND = 5  # The maximum number of passengers for each travel demand
        AGENT_SIZE = 4  # Number of agents + 1
    elif CB_task == 'Sioux_1':
        from CB_instances.sioux1_magent_pdptw import CustomizedBusDataset
        MAX_DEMAND = 5  # The maximum number of passengers for each travel demand
        AGENT_SIZE = 6  # Number of agents + 1
    elif CB_task == 'Major':
        from CB_instances.major_magent_pdptw import CustomizedBusDataset
        MAX_DEMAND = 4  # The maximum number of passengers for each travel demand
        AGENT_SIZE = 11  # Number of agents + 1
    from mrl_model_simulation import MRL_CB_Simulation
    LOAD_DICT = {args.num_nodes: 40}  # The maximum passenger capacity of the bus
    STATIC_SIZE = len(all_station) + 8  # Static information dimension
    DYNAMIC_SIZE = 11  # Dynamic information dimension
    all_max_time = 150  # Total operating hours of CB (min).
    now = '%s' % datetime.datetime.now().time()
    now = now.replace(':', '_')
    print('Instance data start preparation:', now)
    each_max_load = LOAD_DICT[args.num_nodes] # The maximum passenger capacity of any vehicle

    # Trainning data
    train_data = CustomizedBusDataset(args.train_size, args.num_nodes, each_max_load, MAX_DEMAND, args.seed,
                                      all_station, travel_time_G, Loc_x, Loc_y, up_down_index,
                                      depot, home_id, work_id)
    # Verified data
    valid_data = CustomizedBusDataset(args.valid_size, args.num_nodes, each_max_load, MAX_DEMAND, args.seed + 1,
                                      all_station, travel_time_G, Loc_x, Loc_y, up_down_index,
                                      depot, home_id, work_id)
    # Test data
    test_data = CustomizedBusDataset(args.testing_size, args.num_nodes, each_max_load, MAX_DEMAND, args.seed + 2,
                                     all_station, travel_time_G, Loc_x, Loc_y, up_down_index, depot, home_id,
                                     work_id)
    return train_data, valid_data, test_data


if __name__ == '__main__':
    '''Call the files in folders "Sioux" and "Major" 
    to generate training, validation and test data sets for the three road netwoks, respectively.'''
    # Codes for customized bus route network instances, such as Sioux_0, Sioux_1, and Major.
    # Load the files and parameters of the road network
    all_stations = []
    depot = []
    home_id = []
    work_id = []
    file_adress = './Data/'
    transit_network = ''  # a customized bus route network
    network_trip_time = ''  # travel time file
    network_station = ''  # station location file
    CB_task = ''  # input a name for the load network
    '''the CB network: Sioux_0'''
    if CB_task == 'Sioux_0':
        transit_network = 'Sioux'
        network_trip_time = 'sioux_time'
        network_station = 'sioux_stops'
        # a CP
        depot = [19]
        # boarding stations
        home_id = [13, 14, 18, 20, 21, 22, 23]
        # alighting stations
        work_id = [3, 4]

    '''the CB network: Sioux_1'''
    if CB_task == 'Sioux_1':
        transit_network = 'Sioux'
        network_trip_time = 'sioux_time'
        network_station = 'sioux_stops'
        # a CP
        depot = [19]
        # boarding stations
        home_id = [9, 10, 13, 14, 15, 16, 17, 18, 20, 21, 22, 23]
        # alighting stations
        work_id = [3, 4]

    '''the CB network: Major'''
    if CB_task == 'Major':
        transit_network = 'Major'
        network_trip_time = 'major_time'
        network_station = 'major_stops'
        # a CP
        depot = [105]
        # boarding stations
        home_id = [2, 3, 4, 11, 27, 28, 29, 30, 40, 68, 122, 124, 126, 154, 155, 156, 157, 198, 199, 200, 203, 204,
                   225, 249, 250, 251, 252, 253, 254, 282, 283, 286]
        # alighting stations
        work_id = [24, 149, 151, 229]

        '''Generate more new customized bus network problems.'''
        # home_id = random.sample(home_id, 31)
        # work_id = random.sample(work_id, 3)
        # print('home_id:', home_id)
        # print('work_id:', work_id)

    up_station = copy.deepcopy(depot)
    up_station.extend(copy.deepcopy(home_id))
    all_station = copy.deepcopy(depot)
    all_station.extend(copy.deepcopy(home_id))
    all_station.extend(copy.deepcopy(work_id))
    # Travel time distribution
    travel_time_file = file_adress + transit_network+ '/' + network_trip_time +'.csv'
    # Road network structure, i.e. station location distribution
    stop_file = file_adress + transit_network + '/' + network_station +'.csv'
    Loc = []
    Loc_x = []
    Loc_y = []
    down_Loc_x = []
    down_Loc_y = []
    with open(stop_file, 'r', encoding='utf-8') as f0:
        for row0 in csv.reader(f0):
            Loc.append([float(row0[1]), float(row0[2])])
    Loc_x.append(Loc[depot[0]][0])
    Loc_y.append(Loc[depot[0]][1])
    for i in range(len(Loc)):
        if i in home_id:
            Loc_x.append(Loc[i][0])
            Loc_y.append(Loc[i][1])
    for i in range(len(Loc)):
        if i in work_id:
            Loc_x.append(Loc[i][0])
            Loc_y.append(Loc[i][1])
    max_loc_x = max(Loc_x)
    max_loc_y = max(Loc_y)
    Loc_x = [i / max(Loc_x) for i in Loc_x]
    Loc_y = [i / max(Loc_y) for i in Loc_y]
    up_down_index = [len(depot)+len(home_id), len(work_id)]
    travel_time_G = [[0 for j in range(len(Loc))] for i in range(len(Loc))]
    with open(travel_time_file, 'r', encoding='utf-8') as f1:
        for row1 in csv.reader(f1):
            travel_time_G[int(row1[0])][int(row1[1])] = float(row1[2])
    #  Parameters
    #  Parameters
    parser = argparse.ArgumentParser(description='CTRNDP')
    parser.add_argument('--seed', default=12345, type=int)
    parser.add_argument('--checkpoint', default=None)
    parser.add_argument('--test', action='store_true', default=False)
    parser.add_argument('--task', default='offline')  # offline scenario
    # parser.add_argument('--task', default='online')   # online scenario
    parser.add_argument('--nodes', dest='num_nodes', default=up_down_index[0] + up_down_index[1], type=int)
    parser.add_argument('--upnodes', dest='up_num_nodes', default=up_down_index[0], type=int)
    parser.add_argument('--downnodes', dest='down_num_nodes', default=up_down_index[1], type=int)
    parser.add_argument('--actor_lr', default=5e-4, type=float)
    parser.add_argument('--critic_lr', default=5e-4, type=float)
    parser.add_argument('--max_grad_norm', default=2., type=float)
    parser.add_argument('--batch_size', default=128, type=int)
    parser.add_argument('--hidden', dest='hidden_size', default=128, type=int)
    parser.add_argument('--dropout', default=0.1, type=float)
    parser.add_argument('--layers', dest='num_layers', default=1, type=int)
    parser.add_argument('--train-size', default=64000, type=int)
    parser.add_argument('--valid-size', default=1280, type=int)
    parser.add_argument('--testing-size', default=128, type=int)

    args = parser.parse_args()
    if args.task == 'offline':
        train_data, valid_data, test_data = train_pdptw(args, up_station, all_station, travel_time_G, Loc_x, Loc_y,
                                             up_down_index, depot, home_id, work_id, max_loc_x, max_loc_y, CB_task)
    elif args.task == 'online':
        train_data, valid_data, test_data = train_pdptw(args, up_station, all_station, travel_time_G, Loc_x, Loc_y,
                                             up_down_index, depot, home_id, work_id, max_loc_x, max_loc_y, CB_task)
    else:
        raise ValueError('Task <%s> not understood' % args.task)
