"""Defines the main trainer model for combinatorial problems

Each task must define the following functions:
* mask_fn: can be None
* update_fn: can be None
* reward_fn: specifies the quality of found solutions
* render_fn: Specifies how to plot found solutions. Can be None
"""
import csv
import os
import random
import time
import argparse
import datetime
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
import torch.optim as optim
from torch.utils.data import DataLoader
import copy
from MRL_Sioux1.On_CB_model.on_magent_model import MA_CB_RP_Sioux1, CB_Encoder

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
# device = torch.device('cpu')

class StateCritic(nn.Module):
    """Estimates the problem complexity.
    This is a basic module that just looks at the log-probabilities predicted by
    the encoder + decoder, and returns an estimate of complexity
    """
    def __init__(self, static_size, dynamic_size, hidden_size):
        super(StateCritic, self).__init__()
        # Define the encoder & decoder models
        self.static_encoder = CB_Encoder(static_size, hidden_size)
        self.dynamic_encoder1 = CB_Encoder(dynamic_size, hidden_size)
        self.dynamic_encoder2 = CB_Encoder(dynamic_size, hidden_size)
        self.dynamic_encoder3 = CB_Encoder(dynamic_size, hidden_size)
        self.dynamic_encoder4 = CB_Encoder(dynamic_size, hidden_size)
        self.dynamic_encoder5 = CB_Encoder(dynamic_size, hidden_size)
        self.fc1 = nn.Conv1d(hidden_size * 6, 20, kernel_size=1)
        self.fc2 = nn.Conv1d(20, 20, kernel_size=1)
        self.fc3 = nn.Conv1d(20, 1, kernel_size=1)

        for p in self.parameters():
            if len(p.shape) > 1:
                nn.init.xavier_uniform_(p)

    def forward(self, static, dynamic1, dynamic2, dynamic3, dynamic4, dynamic5):
        # Use the probabilities of visiting each station
        static_hidden = self.static_encoder(static)
        dynamic_hidden1 = self.dynamic_encoder1(dynamic1)
        dynamic_hidden2 = self.dynamic_encoder2(dynamic2)
        dynamic_hidden3 = self.dynamic_encoder3(dynamic3)
        dynamic_hidden4 = self.dynamic_encoder4(dynamic4)
        dynamic_hidden5 = self.dynamic_encoder5(dynamic5)
        hidden = torch.cat((static_hidden, dynamic_hidden1, dynamic_hidden2, dynamic_hidden3, dynamic_hidden4, dynamic_hidden5), 1)
        output = F.relu(self.fc1(hidden))
        output = F.relu(self.fc2(output))
        output = self.fc3(output).sum(dim=2)
        return output


def validate(data_loader, actor, reward_fn, travel_time_G, seed_od, up_station, all_station, max_loc_x, max_loc_y, render_fn=None, save_dir='.', num_plot=5):
    """Used to monitor progress on a validation or test set."""
    actor.eval()
    if not os.path.exists(save_dir):
        os.makedirs(save_dir)
    rewards = []
    for batch_idx, batch in enumerate(data_loader):
        static, dynamic1, dynamic2, dynamic3, dynamic4, dynamic5, x0, record1, record2, record3 = batch
        static = static.to(device)
        dynamic1 = dynamic1.to(device)
        dynamic2 = dynamic2.to(device)
        dynamic3 = dynamic3.to(device)
        dynamic4 = dynamic4.to(device)
        dynamic5 = dynamic5.to(device)
        record1 = record1.to(device)
        record2 = record2.to(device)
        record3 = record3.to(device)
        x0 = x0.to(device) if len(x0) > 0 else None
        all_method_max_load = 40
        all_method_max_time = 150
        AGENT_SIZE = 6
        if save_dir == 'test':
            start = time.time()
            with torch.no_grad():
                tour_indices, _, update_constraint, update_dynamic = actor(static, dynamic1, dynamic2, dynamic3, dynamic4, dynamic5,
                                                                           record1, record2, record3,
                                                                           travel_time_G, up_station, all_station, x0)
            end = time.time()
            reward, value_list = reward_fn(static, tour_indices, travel_time_G, all_station, update_constraint, up_station, update_dynamic, all_method_max_load, all_method_max_time, AGENT_SIZE-1)  # 奖励
            rewards.append(reward.mean().item())
            value_list.append([end-start])
        # if render_fn is not None and batch_idx < num_plot:
        #     name = 'batch%d_%2.4f.png'%(batch_idx, reward.mean().item())
        #     path = os.path.join(save_dir, name)
        #     # print('render_fn', static, tour_indices, path)
        #     render_fn(static, tour_indices, path, all_station, max_loc_x, max_loc_y)
    actor.train()
    return np.mean(rewards)


# Train a multi-agent model
def train(actor, critic, task, num_nodes, train_data, valid_data, reward_fn,
          render_fn, batch_size, actor_lr, critic_lr, max_grad_norm, up_station, all_station, max_loc_x, max_loc_y,
          **kwargs):
    """Constructs the main actor & critic networks, and performs all training."""

    now = '%s' % datetime.datetime.now().time()
    now = now.replace(':', '_')
    print('Start training:', now)
    save_dir = os.path.join(task, '%d' % num_nodes, now)
    checkpoint_dir = os.path.join(save_dir, 'checkpoints')
    if not os.path.exists(checkpoint_dir):
        os.makedirs(checkpoint_dir)
    all_method_max_load = 40  # The maximum passenger capacity of the bus
    all_method_max_time = 150  # Total operating hours of CB (min).
    AGENT_SIZE = 6 # Number of agents=(AGENT_SIZE-1)
    actor_optim = optim.Adam(actor.parameters(), lr=actor_lr)
    critic_optim = optim.Adam(critic.parameters(), lr=critic_lr)
    train_loader = DataLoader(train_data, batch_size, True, num_workers=0)
    valid_loader = DataLoader(valid_data, batch_size, False, num_workers=0)
    best_params = None
    best_reward = np.inf
    Loss_list, Reward_list = [], []
    for epoch in range(20):
        actor.train()
        critic.train()
        times, losses, rewards, critic_rewards = [], [], [], []
        epoch_start = time.time()
        start = epoch_start
        for batch_idx, batch in enumerate(train_loader):
            static, dynamic1, dynamic2, dynamic3, dynamic4, dynamic5, x0, record1, record2, record3 = batch
            static = static.to(device)
            dynamic1 = dynamic1.to(device)
            dynamic2 = dynamic2.to(device)
            dynamic3 = dynamic3.to(device)
            dynamic4 = dynamic4.to(device)
            dynamic5 = dynamic5.to(device)
            record1 = record1.to(device)
            record2 = record2.to(device)
            record3 = record3.to(device)
            x0 = x0.to(device) if len(x0) > 0 else None
            # Full forward pass through the dataset
            now = '%s' % datetime.datetime.now().time()
            now = now.replace(':', '_')
            tour_indices, tour_logp, update_constraint, update_dynamic = actor(static, dynamic1, dynamic2, dynamic3,
                                                                               dynamic4, dynamic5, record1, record2,
                                                                               record3, travel_time_G, up_station,
                                                                               all_station, x0)
            # Sum the log probabilities for each sation in routes.
            reward, value_list = reward_fn(static, tour_indices, travel_time_G, all_station, update_constraint,
                                           up_station, update_dynamic, all_method_max_load, all_method_max_time,
                                           AGENT_SIZE-1)
            # Query the critic for an estimate of the reward.
            all_tour_logp = torch.full(tour_logp[1].size(), 0.).sum(dim=1).to(device)
            for t_log in tour_logp.keys():
                all_tour_logp = tour_logp[t_log].sum(dim=1) + all_tour_logp
            critic_est = critic(static, dynamic1, dynamic2, dynamic3, dynamic4, dynamic5).view(-1)
            advantage = (reward - critic_est)
            actor_loss = torch.mean((advantage.detach() * all_tour_logp))
            critic_loss = torch.mean(advantage ** 2)
            actor_optim.zero_grad()
            actor_loss.backward()
            torch.nn.utils.clip_grad_norm_(actor.parameters(), max_grad_norm)
            actor_optim.step()
            critic_optim.zero_grad()
            critic_loss.backward()
            torch.nn.utils.clip_grad_norm_(critic.parameters(), max_grad_norm)
            critic_optim.step()
            critic_rewards.append(torch.mean(critic_est.detach()).item())
            rewards.append(torch.mean(reward.detach()).item())
            losses.append(torch.mean(actor_loss.detach()).item())
            if (batch_idx + 1) % 100 == 0:
                end = time.time()
                times.append(end - start)
                start = end
                mean_loss = np.mean(losses[-100:])
                mean_reward = np.mean(rewards[-100:])
                print('Batch %d/%d, reward: %2.3f, loss: %2.4f, took: %2.4fs' %
                      (batch_idx, len(train_loader), mean_reward, mean_loss, times[-1]))
        mean_loss = np.mean(losses)
        mean_reward = np.mean(rewards)
        # Save the weights.
        epoch_dir = os.path.join(checkpoint_dir, '%s' % epoch)
        if not os.path.exists(epoch_dir):
            os.makedirs(epoch_dir)
        save_path = os.path.join(epoch_dir, 'actor.pt')
        torch.save(actor.state_dict(), save_path)
        save_path = os.path.join(epoch_dir, 'critic.pt')
        torch.save(critic.state_dict(), save_path)
        valid_dir = os.path.join(save_dir, '%s' % epoch)
        mean_valid = validate(valid_loader, actor, reward_fn, travel_time_G,
                                                  up_station, all_station, max_loc_x, max_loc_y,
                                                  render_fn, valid_dir, num_plot=5)
        # Save best multi-agent model parameters.
        if mean_valid < best_reward:
            best_reward = mean_valid
            save_path = os.path.join(save_dir, 'actor.pt')
            torch.save(actor.state_dict(), save_path)
            save_path = os.path.join(save_dir, 'critic.pt')
            torch.save(critic.state_dict(), save_path)
        print('Mean epoch loss/reward/valid: %2.4f, %2.4f, %2.4f, took: %2.4fs '\
              '(%2.4fs / 100 batches)\n' % \
              (mean_loss, mean_reward, mean_valid, time.time() - epoch_start, np.mean(times)))
        Loss_list.append(mean_loss)
        Reward_list.append(mean_reward)
    return Loss_list, Reward_list


def train_pdptw(args, up_station, all_station, travel_time_G, Loc_x, Loc_y, up_down_index, depot, home_id, work_id, max_loc_x, max_loc_y):
    Loss_list, Reward_list = [], []
    from MRL_Sioux1.On_CB_tasks import on_magent_pdptw
    from MRL_Sioux1.On_CB_tasks.on_magent_pdptw import CustomizedBusDataset
    LOAD_DICT = {args.num_nodes: 40}  # The maximum passenger capacity of the bus
    MAX_DEMAND = 5  # The maximum number of passengers for each travel demand
    STATIC_SIZE = len(all_station) + 8  # Static information dimension
    DYNAMIC_SIZE = 11  # Dynamic information dimension
    AGENT_SIZE = 6  # Number of agents=(AGENT_SIZE-1)
    now = '%s' % datetime.datetime.now().time()
    now = now.replace(':', '_')
    print('Instance data start preparation:', now)
    max_load = LOAD_DICT[args.num_nodes]
    # Trainning data
    train_data = CustomizedBusDataset(args.train_size, args.num_nodes, max_load, MAX_DEMAND, args.seed,
                                      all_station, travel_time_G, Loc_x, Loc_y, up_down_index,
                                      depot, home_id, work_id)
    #  verified data
    valid_data = CustomizedBusDataset(args.valid_size, args.num_nodes, max_load, MAX_DEMAND, args.seed + 1,
                                      all_station, travel_time_G, Loc_x, Loc_y, up_down_index,
                                      depot, home_id, work_id)
    actor = MA_CB_RP_Sioux1(STATIC_SIZE,
                    DYNAMIC_SIZE,
                    args.hidden_size,
                    AGENT_SIZE,
                    train_data.update_dynamic,
                    train_data.update_mask,
                    train_data.update_mask_start,
                    train_data.update_time_window_state,
                    train_data.update_station_tw_od,
                    args.num_layers,
                    args.dropout).to(device)

    critic = StateCritic(STATIC_SIZE, DYNAMIC_SIZE, args.hidden_size).to(device)
    kwargs = vars(args)
    kwargs['train_data'] = train_data
    kwargs['valid_data'] = valid_data
    kwargs['reward_fn'] = on_magent_pdptw.reward
    kwargs['render_fn'] = on_magent_pdptw.render
    kwargs['down_num_nodes'] = args.down_num_nodes
    kwargs['up_station'] = up_station
    kwargs['all_station'] = all_station
    kwargs['max_loc_x'] = max_loc_x
    kwargs['max_loc_y'] = max_loc_y

    if args.checkpoint:
        path = os.path.join(args.checkpoint, 'actor.pt')
        actor.load_state_dict(torch.load(path, device))
        path = os.path.join(args.checkpoint, 'critic.pt')
        critic.load_state_dict(torch.load(path, device))
    now = '%s' % datetime.datetime.now().time()
    now = now.replace(':', '_')
    print('Data preparation finished:', now)
    # Tests for comparing algorithms
    seed_od = []
    for j in range(150):
        seed = random.randint(0, 1234567890)
        seed_od.append(seed)
    # Train a multi-agent model
    if not args.test:
        Loss_list, Reward_list = train(actor, critic, **kwargs)
    # Call the trained multi-agent model to test(only one GPU can be used).
    # actor_state_dict = torch.load('./actor.pt')
    # actor.load_state_dict(actor_state_dict)

    test_data = CustomizedBusDataset(args.testing_size, args.num_nodes, max_load, MAX_DEMAND, args.seed + 2,
                                     all_station, travel_time_G, Loc_x, Loc_y, up_down_index,
                                     depot, home_id, work_id)
    test_dir = 'test'
    test_loader = DataLoader(test_data, args.batch_size, False, num_workers=0)
    # Call actor_test when comparing experiments
    # actor_test = MA_CB_RP_Sioux1(STATIC_SIZE,
    #                         DYNAMIC_SIZE,
    #                         args.hidden_size,
    #                         AGENT_SIZE,
    #                         train_data.update_dynamic,
    #                         train_data.update_mask,
    #                         train_data.update_mask_start,
    #                         train_data.update_time_window_state,
    #                         train_data.update_station_tw_od,
    #                         args.num_layers,
    #                         args.dropout).to(device)
    on_madrl_out, si_out, gni_out, hr_out, rga_out, rmga_out = validate(test_loader, actor, on_magent_pdptw.reward,
                                                                        travel_time_G, up_station, all_station,
                                                                        max_loc_x, max_loc_y, on_magent_pdptw.render,
                                                                        test_dir, num_plot=5)
    print('MADRL-The average reward of CB routes: ', on_madrl_out)
    return Loss_list, Reward_list


if __name__ == '__main__':
    # Load the files and parameters of the road network
    all_stations = []
    file_adress = './Data/'
    transit_network = 'Sioux'
    network_trip_time = 'sioux_time'
    network_station = 'sioux_stops'
    depot = [19]
    home_id = [9, 10, 13, 14, 15, 16, 17, 18, 20, 21, 22, 23]
    work_id = [3, 4]
    up_station = copy.deepcopy(depot)
    up_station.extend(copy.deepcopy(home_id))
    all_station = copy.deepcopy(depot)
    all_station.extend(copy.deepcopy(home_id))
    all_station.extend(copy.deepcopy(work_id))
    # Travel time distribution
    travel_time_file = file_adress +transit_network+'/'+network_trip_time+'.csv'
    # Road network structure, i.e. station location distribution
    stop_file = file_adress +transit_network+'/'+network_station+'.csv'
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
    travel_time_G = [[0 for i in range(len(Loc))] for i in range(len(Loc))]
    with open(travel_time_file, 'r', encoding='utf-8') as f1:
        for row1 in csv.reader(f1):
            travel_time_G[int(row1[0])][int(row1[1])] = float(row1[2])
    #  Parameters
    parser = argparse.ArgumentParser(description='CTRNDP')
    parser.add_argument('--seed', default=12345, type=int)
    parser.add_argument('--checkpoint', default=None)
    parser.add_argument('--test', action='store_true', default=False)
    parser.add_argument('--task', default='on_mapdptw')
    parser.add_argument('--nodes', dest='num_nodes', default=up_down_index[0]+up_down_index[1], type=int)
    parser.add_argument('--upnodes', dest='up_num_nodes', default=up_down_index[0], type=int)
    parser.add_argument('--downnodes', dest='down_num_nodes', default=up_down_index[1], type=int)
    parser.add_argument('--actor_lr', default=5e-4, type=float)
    parser.add_argument('--critic_lr', default=5e-4, type=float)
    parser.add_argument('--max_grad_norm', default=2., type=float)
    parser.add_argument('--batch_size', default=128, type=int)
    parser.add_argument('--hidden', dest='hidden_size', default=128, type=int)
    parser.add_argument('--dropout', default=0.1, type=float)
    parser.add_argument('--layers', dest='num_layers', default=1, type=int)
    parser.add_argument('--train-size',default=64000, type=int)
    parser.add_argument('--valid-size', default=1280, type=int)
    parser.add_argument('--testing-size', default=128, type=int)

    args = parser.parse_args()
    if args.task == 'on_mapdptw':
        Loss_list, Reward_list = train_pdptw(args, up_station, all_station, travel_time_G, Loc_x, Loc_y,
                                             up_down_index, depot, home_id, work_id, max_loc_x, max_loc_y)
    else:
        raise ValueError('Task <%s> not understood'%args.task)
    print(len(Loss_list))
    print('Loss_list:', Loss_list)
    print('Reward_list:', Reward_list)
