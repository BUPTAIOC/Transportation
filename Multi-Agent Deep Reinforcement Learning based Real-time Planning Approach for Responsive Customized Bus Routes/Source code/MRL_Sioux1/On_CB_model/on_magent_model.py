import copy
import torch
import torch.nn as nn
import torch.nn.functional as F

device = torch.device('cuda' if torch.cuda.is_available() else 'cpu')
# device = torch.device('cpu')


class CB_Encoder(nn.Module):
    """Encodes the static & dynamic states using 1d convolution neural network."""

    def __init__(self, input_size, hidden_size):
        super(CB_Encoder, self).__init__()
        self.conv = nn.Conv1d(input_size, hidden_size, kernel_size=1)

    def forward(self, input):
        output = self.conv(input)
        return output  # (batch, hidden_size, seq_len)


class CB_Attention(nn.Module):
    """Calculates attention over the input nodes given the current state."""

    def __init__(self, hidden_size):
        super(CB_Attention, self).__init__()

        # W processes features from static decoder elements
        self.v = nn.Parameter(torch.zeros((1, 1, hidden_size), device=device, requires_grad=True))

        self.W = nn.Parameter(torch.zeros((1, hidden_size, 3 * hidden_size), device=device, requires_grad=True))

    def forward(self, static_hidden, dynamic_hidden, decoder_hidden):

        batch_size, hidden_size, _ = static_hidden.size()

        hidden = decoder_hidden.unsqueeze(2).expand_as(static_hidden)
        hidden = torch.cat((static_hidden, dynamic_hidden, hidden), 1)

        # Broadcast some dimensions so we can do batch-matrix-multiply
        v = self.v.expand(batch_size, 1, hidden_size)
        W = self.W.expand(batch_size, hidden_size, -1)

        attns = torch.bmm(v, torch.tanh(torch.bmm(W, hidden)))
        attns = F.softmax(attns, dim=2)  # (batch, seq_len)
        return attns


class CB_Agent(nn.Module):
    """Calculates the next state given the previous state and input embeddings."""

    def __init__(self, hidden_size, num_layers=1, dropout=0.2):
        super(CB_Agent, self).__init__()

        self.hidden_size = hidden_size
        self.num_layers = num_layers

        # Used to calculate probability of selecting next state
        self.v = nn.Parameter(torch.zeros((1, 1, hidden_size), device=device, requires_grad=True))

        self.W = nn.Parameter(torch.zeros((1, hidden_size, 2 * hidden_size), device=device, requires_grad=True))

        # Used to compute a representation of the current decoder output
        self.gru = nn.GRU(hidden_size, hidden_size, num_layers, batch_first=True,
                          dropout=dropout if num_layers > 1 else 0)
        self.encoder_attn = CB_Attention(hidden_size)

        self.drop_rnn = nn.Dropout(p=dropout)
        self.drop_hh = nn.Dropout(p=dropout)

    def forward(self, static_hidden, dynamic_hidden, decoder_hidden, last_hh):

        rnn_out, last_hh = self.gru(decoder_hidden.transpose(2, 1), last_hh)
        rnn_out = rnn_out.squeeze(1)

        # Always apply dropout on the gated recurrent units output
        rnn_out = self.drop_rnn(rnn_out)
        if self.num_layers == 1:
            # If > 1 layer dropout is already applied
            last_hh = self.drop_hh(last_hh) 

        # Given a summary of the output, find an  input context
        enc_attn = self.encoder_attn(static_hidden, dynamic_hidden, rnn_out)
        context = enc_attn.bmm(static_hidden.permute(0, 2, 1))  # (B, 1, num_feats)

        # Calculate the next output using batch-matrix-multiply
        context = context.transpose(1, 2).expand_as(static_hidden)
        energy = torch.cat((static_hidden, context), dim=1)  # (B, num_feats, seq_len)

        v = self.v.expand(static_hidden.size(0), -1, -1)
        W = self.W.expand(static_hidden.size(0), -1, -1)

        probs = torch.bmm(v, torch.tanh(torch.bmm(W, energy))).squeeze(1)

        return probs, last_hh


class MA_CB_RP_Sioux1(nn.Module):
    ''' ---------------------------------------
    (1)static_size：int, defines how many features are in the static elements of the model.
    (2)dynamic_size：int > 1, defines how many features are in the dynamic elements of the model.
    (3)hidden_size：int, defines all static, dynamic, and decoder output units.
    (4)Agent_n: int, number of agents.
    (5)update_fn：this method is used to calculate the input dynamic element to be updated.
    (6)mask_fn：this method is used to help speed up network training by providing a sort of "rule" guideline for the algorithm.
    (7)mask_start: this method is used to process the action space of each agent through a masking mechanism.
    (8)update_tw: this method is used to process each travel demand for each station through the matching mechanism.
    (9)update_od: This method is used to update the travel demand distribution across the road network and generate new travel demand in the road network.
    (10)num_layers：int, specifies the number of hidden layers to use in the decoder
    (11)dropout：float, define the exit rate of the decoder to prevent overfitting'''
    def __init__(self, static_size, dynamic_size, hidden_size, Agent_n,
                 update_fn=None, mask_fn=None, mask_start=None, update_tw=None, update_od=None, num_layers=1, dropout=0.):
        super(MA_CB_RP_Sioux1, self).__init__()

        if dynamic_size < 1:
            raise ValueError(':param dynamic_size: must be > 0, even if the '
                             'problem has no dynamic elements')
        self.agent_number = Agent_n
        self.update_fn = update_fn
        self.mask_fn = mask_fn
        self.mask_start = mask_start
        self.update_tw = update_tw
        self.update_od = update_od

        # Define the static_encoder model. Environment state shared by all agents.
        self.static_encoder = CB_Encoder(static_size, hidden_size)
        # Define the agent_dynamic_encoder models. Each agent has a dynamic encoder model.
        # self.agent_dynamic_encoder = [CB_Encoder(dynamic_size, hidden_size) for i in range(self.agent_number)]
        self.agent_dynamic_encoder1 = CB_Encoder(dynamic_size, hidden_size)
        self.agent_dynamic_encoder2 = CB_Encoder(dynamic_size, hidden_size)
        self.agent_dynamic_encoder3 = CB_Encoder(dynamic_size, hidden_size)
        self.agent_dynamic_encoder4 = CB_Encoder(dynamic_size, hidden_size)
        self.agent_dynamic_encoder5 = CB_Encoder(dynamic_size, hidden_size)

        # Define the agent_dynamic_encoder models. Each agent has a decoder model.
        # self.agent_decoder = [CB_Encoder(dynamic_size, hidden_size) for i in range(self.agent_number)]
        self.agent_decoder1 = CB_Encoder(static_size, hidden_size)
        self.agent_decoder2 = CB_Encoder(static_size, hidden_size)
        self.agent_decoder3 = CB_Encoder(static_size, hidden_size)
        self.agent_decoder4 = CB_Encoder(static_size, hidden_size)
        self.agent_decoder5 = CB_Encoder(static_size, hidden_size)

        # Define the agent_CB_Agent models. Each agent has a CB_Agent model.
        # self.agent_CB_Agent = [CB_Agent(hidden_size, num_layers, dropout) for i in range(self.agent_number)]
        self.agent_pointer1 = CB_Agent(hidden_size, num_layers, dropout)
        self.agent_pointer2 = CB_Agent(hidden_size, num_layers, dropout)
        self.agent_pointer3 = CB_Agent(hidden_size, num_layers, dropout)
        self.agent_pointer4 = CB_Agent(hidden_size, num_layers, dropout)
        self.agent_pointer5 = CB_Agent(hidden_size, num_layers, dropout)

        for p in self.parameters():
            if len(p.shape) > 1:
                nn.init.xavier_uniform_(p)
        # Used as a proxy initial state in the decoder when not specified
        self.x0 = torch.zeros((1, static_size, 1), requires_grad=True, device=device)

    def forward(self, static, dynamic1, dynamic2, dynamic3, dynamic4, dynamic5, record1, record2, record3,
                travel_time_G, up_station, all_station, decoder_input=None, last_hh=None):
        '''
            (1)static：static information in the CB operating environment, (batch_size, features, num_stations).
            (2)dynamic：dynamic information observed by each agent in the CB operating environment, [(batch_size, features, num_stations), (batch_size, features, num_stations),...].
            (3)record：important information recorded in the CB operating environment, used to calculate the return function, [(batch_size, features, num_stations), (batch_size, features, num_stations),...].
            (4)travel_time_G: travel time matrix for road environment.
            (5)up_station：list of boarding stations in the origin area.
            (6)all_station：list of all stations in the road network.
            (7)decoder_input: The input of the agent decoder, (batch_size, features).
            (8)last_hh: Last hidden state of gated recurrent units, (batch_size, num_hidden).'''
        last_hh1 = last_hh
        last_hh2 = last_hh
        last_hh3 = last_hh
        last_hh4 = last_hh
        last_hh5 = last_hh
        # dynamic0 = [dyn.clone() for dyn in ori_dynamic]
        dynamic0 = [dynamic1.clone(), dynamic2.clone(), dynamic3.clone(), dynamic4.clone(), dynamic5.clone()]
        decision_list = [[] for i in range(len(static[:]))]  # Agents decision list
        # decision_point = torch.ones(len(static[:]), 3, len(all_station)) * np.inf
        for ns in range(len(decision_list)):  # According to the number of agents
            for a_idx in range(1, self.agent_number):
                decision_list[ns].append([dynamic0[a_idx-1][ns][7][0].item(), a_idx])
        for so in range(len(decision_list)):  # Decision ranking
            list.sort(decision_list[so], key=(lambda x: [x[0]]))
        batch_size, input_size, sequence_size = static.size()
        # decoder_input_list = [decoder_input for i in range(self.agent_number)]
        decoder_input1 = decoder_input
        decoder_input2 = decoder_input
        decoder_input3 = decoder_input
        decoder_input4 = decoder_input
        decoder_input5 = decoder_input
        if decoder_input is None:
            # decoder_input_list = [self.x0.expand(batch_size, -1, -1) for i in range(self.agent_number)]
            decoder_input1 = self.x0.expand(batch_size, -1, -1)
            decoder_input2 = self.x0.expand(batch_size, -1, -1)
            decoder_input3 = self.x0.expand(batch_size, -1, -1)
            decoder_input4 = self.x0.expand(batch_size, -1, -1)
            decoder_input5 = self.x0.expand(batch_size, -1, -1)
        #  Agents' mask list
        # mask_list = [torch.ones(batch_size, sequence_size, device=device) for i in range(self.agent_number)]
        mask1 = torch.ones(batch_size, sequence_size, device=device)
        mask2 = torch.ones(batch_size, sequence_size, device=device)
        mask3 = torch.ones(batch_size, sequence_size, device=device)
        mask4 = torch.ones(batch_size, sequence_size, device=device)
        mask5 = torch.ones(batch_size, sequence_size, device=device)
        # Agents' judgment list
        mask_judge = torch.ones(batch_size, sequence_size, device=device)
        # for ma, m_val in enumerate(mask_list):
        #     m_val[:, 0] = 0  # This means that all vehicles depart from CP
        #     m_val[:, len(up_station):] = 0
        mask1[:, 0] = 0  # This means that all vehicles depart from CP
        mask1[:, len(up_station):] = 0  # This means that all vehicles depart from CP
        mask2[:, 0] = 0  # This means that all vehicles depart from CP
        mask2[:, len(up_station):] = 0  # This means that all vehicles depart from CP
        mask3[:, 0] = 0  # This means that all vehicles depart from CP
        mask3[:, len(up_station):] = 0  # This means that all vehicles depart from CP
        mask4[:, 0] = 0  # This means that all vehicles depart from CP
        mask4[:, len(up_station):] = 0  # This means that all vehicles depart from CP
        mask5[:, 0] = 0  # This means that all vehicles depart from CP
        mask5[:, len(up_station):] = 0  # This means that all vehicles depart from CP

        # agent_mask_list = [torch.ones(batch_size, sequence_size, device=device) for i in range(self.agent_number)]
        agent_mask1 = torch.ones(batch_size, sequence_size, device=device)
        agent_mask2 = torch.ones(batch_size, sequence_size, device=device)
        agent_mask3 = torch.ones(batch_size, sequence_size, device=device)
        agent_mask4 = torch.ones(batch_size, sequence_size, device=device)
        agent_mask5 = torch.ones(batch_size, sequence_size, device=device)
        # for ag, a_val in enumerate(agent_mask_list):
        #     a_val[:, 0] = 0
        agent_mask1[:, 0] = 0
        agent_mask2[:, 0] = 0
        agent_mask3[:, 0] = 0
        agent_mask4[:, 0] = 0
        agent_mask5[:, 0] = 0
        decision_mask_dict = {}  # Decision mask dictionary
        tw_mask = torch.ones(batch_size, 3, len(all_station), device=device)  # Mask of all travel demands for all stations
        tw_mask[:, 0:, 0] = 0  # This means that CP has no travel demand
        tw_mask[:, 0:, len(up_station):] = 0  # All alighting stations have no travel demand
        # This dictionary is used to store all the routes constructed by the agent, i.e. the sequence of stations.
        tour_idx = {}
        tour_logp = {}
        tour_idx_dict = {}
        static_hidden = self.static_encoder(static)
        # dynamic_hidden_list = [self.agent_dynamic_encoder1(agent_dyn) for agent_dyn in range(self.agent_number)]
        dynamic_hidden1 = self.agent_dynamic_encoder1(dynamic1)
        dynamic_hidden2 = self.agent_dynamic_encoder2(dynamic2)
        dynamic_hidden3 = self.agent_dynamic_encoder3(dynamic3)
        dynamic_hidden4 = self.agent_dynamic_encoder4(dynamic4)
        dynamic_hidden5 = self.agent_dynamic_encoder5(dynamic5)
        for a_id in range(1, self.agent_number):
            if a_id not in tour_idx.keys():
                tour_idx[a_id] = []
                tour_idx_dict[a_id] = [[] for i in range(len(static[:]))]
            if a_id not in tour_logp.keys():
                tour_logp[a_id] = []
            if a_id not in decision_mask_dict.keys():
                decision_mask_dict[a_id] = [0 for i in range(batch_size)]
        max_steps = sequence_size if self.mask_fn is None else 1000  # Decision step
        # dynamic = [o_dyn.clone() for o_dyn in ori_dynamic]
        dynamic = [dynamic1.clone(), dynamic2.clone(), dynamic3.clone(), dynamic4.clone(), dynamic5.clone()]
        information_list = []  # Record updated Passenger data, Passenger flow data update for comparison algorithm during testing
        for _ in range(max_steps):
            '''Update decision time'''
            step_list = []
            for a_id in range(1, self.agent_number):
                decision_mask_dict[a_id] = [0 for i in range(batch_size)]
            cy_decision_list = copy.deepcopy(decision_list)
            # dynamic1_cl0_list = [dyn0.clone() for dyn0 in ori_dynamic]
            dynamic1_cl0 = dynamic1.clone()
            dynamic2_cl0 = dynamic2.clone()
            dynamic3_cl0 = dynamic3.clone()
            dynamic4_cl0 = dynamic4.clone()
            dynamic5_cl0 = dynamic5.clone()
            time_list = []
            for ns0, ns0a in enumerate(cy_decision_list):
                if ns0a:
                    decision_mask_dict[ns0a[0][1]][ns0] = 1
                    decision_list[ns0].remove(ns0a[0])
                    # for dyn_cl0, d_cl0_value in enumerate(dynamic1_cl0_list):
                    #     dynamic1_cl0_list[dyn_cl0][ns0][5] = dynamic[ns0a[0][1] - 1][ns0][10].clone()
                    dynamic1_cl0[ns0][5] = dynamic[ns0a[0][1] - 1][ns0][10].clone()
                    dynamic2_cl0[ns0][5] = dynamic[ns0a[0][1] - 1][ns0][10].clone()
                    dynamic3_cl0[ns0][5] = dynamic[ns0a[0][1] - 1][ns0][10].clone()
                    dynamic4_cl0[ns0][5] = dynamic[ns0a[0][1] - 1][ns0][10].clone()
                    dynamic5_cl0[ns0][5] = dynamic[ns0a[0][1] - 1][ns0][10].clone()
                    time_list.append([dynamic[ns0a[0][1] - 1][ns0][10].clone()][0][0].item())
                for n0 in range(1, len(ns0a)):
                    if ns0a[n0][0] == ns0a[0][0]:
                        decision_mask_dict[ns0a[n0][1]][ns0] = 1
                        decision_list[ns0].remove(ns0a[n0])
            step_list.append(time_list)
            dynamic1 = torch.as_tensor(dynamic1_cl0.clone().data, device=dynamic1.device)
            dynamic2 = torch.as_tensor(dynamic2_cl0.clone().data, device=dynamic2.device)
            dynamic3 = torch.as_tensor(dynamic3_cl0.clone().data, device=dynamic3.device)
            dynamic4 = torch.as_tensor(dynamic4_cl0.clone().data, device=dynamic4.device)
            dynamic5 = torch.as_tensor(dynamic5_cl0.clone().data, device=dynamic5.device)
            # dynamic_uod = [dyn_uod.clone() for dyn_uod in ori_dynamic]
            dynamic_uod = [dynamic1.clone(), dynamic2.clone(), dynamic3.clone(), dynamic4.clone(), dynamic5.clone()]
            # constraint_uod = [con_uod.clone() for con_uod in record]
            constraint_uod = [record1.clone(), record2.clone(), record3.clone()]
            dynamic, constraint, _list = self.update_od(dynamic_uod, constraint_uod, static, up_station, tw_mask)
            step_list.append(_list)
            information_list.append(step_list)
            dynamic1_cl1 = dynamic[0].clone()
            dynamic2_cl1 = dynamic[1].clone()
            dynamic3_cl1 = dynamic[2].clone()
            dynamic4_cl1 = dynamic[3].clone()
            dynamic5_cl1 = dynamic[4].clone()
            dynamic1 = torch.as_tensor(dynamic1_cl1.clone().data, device=dynamic1.device)
            dynamic2 = torch.as_tensor(dynamic2_cl1.clone().data, device=dynamic2.device)
            dynamic3 = torch.as_tensor(dynamic3_cl1.clone().data, device=dynamic3.device)
            dynamic4 = torch.as_tensor(dynamic4_cl1.clone().data, device=dynamic4.device)
            dynamic5 = torch.as_tensor(dynamic5_cl1.clone().data, device=dynamic5.device)
            record1_cl = constraint[0].clone()
            record2_cl = constraint[1].clone()
            record3_cl = constraint[2].clone()
            record1 = torch.as_tensor(record1_cl.clone().data, device=record1.device)
            record2 = torch.as_tensor(record2_cl.clone().data, device=record2.device)
            record3 = torch.as_tensor(record3_cl.clone().data, device=record3.device)
            decision_agent_id = []
            for a_nn in range(1, self.agent_number):
                decision_agent_id.append(a_nn)
            mask_judge_x = agent_mask1.clone() + agent_mask2.clone() + agent_mask3.clone() + agent_mask4.clone() + agent_mask5.clone()
            mask_judge = torch.as_tensor(mask_judge_x.clone().data, device=mask_judge.device)
            if not mask_judge.byte().any():
                break
            for ag_id in range(1, self.agent_number):
                if ag_id == 1:
                    mask1_start, agent_mask1_start = self.mask_start(mask1, dynamic1, up_station, tw_mask, agent_mask1)
                    mask1 = torch.as_tensor(mask1_start.clone().data, device=mask1.device)
                    agent_mask1 = torch.as_tensor(agent_mask1_start.clone().data, device=agent_mask1.device)
                    if not mask1.byte().any():
                        decision_agent_id.remove(ag_id)
                    visit_mask1 = (mask1.clone()).sum(1).eq(0)
                    if visit_mask1.any():
                        visit_idx_mask1 = visit_mask1.nonzero().squeeze()
                        mask1[visit_idx_mask1, 0] = 1
                        mask1[visit_idx_mask1, 1:] = 0
                if ag_id == 2:
                    mask2_start, agent_mask2_start = self.mask_start(mask2, dynamic2, up_station, tw_mask, agent_mask2)
                    mask2 = torch.as_tensor(mask2_start.clone().data, device=mask2.device)
                    agent_mask2 = torch.as_tensor(agent_mask2_start.clone().data, device=agent_mask2.device)
                    if not mask2.byte().any():
                        decision_agent_id.remove(ag_id)

                    visit_mask2 = (mask2.clone()).sum(1).eq(0)
                    if visit_mask2.any():
                        visit_idx_mask2 = visit_mask2.nonzero().squeeze()
                        mask2[visit_idx_mask2, 0] = 1
                        mask2[visit_idx_mask2, 1:] = 0
                if ag_id == 3:
                    mask3_start, agent_mask3_start = self.mask_start(mask3, dynamic3, up_station, tw_mask, agent_mask3)
                    mask3 = torch.as_tensor(mask3_start.clone().data, device=mask3.device)
                    agent_mask3 = torch.as_tensor(agent_mask3_start.clone().data, device=agent_mask3.device)
                    if not mask3.byte().any():
                        decision_agent_id.remove(ag_id)
                    visit_mask3 = (mask3.clone()).sum(1).eq(0)
                    if visit_mask3.any():
                        visit_idx_mask3 = visit_mask3.nonzero().squeeze()
                        mask3[visit_idx_mask3, 0] = 1
                        mask3[visit_idx_mask3, 1:] = 0
                if ag_id == 4:
                    mask4_start, agent_mask4_start = self.mask_start(mask4, dynamic4, up_station, tw_mask, agent_mask4)
                    mask4 = torch.as_tensor(mask4_start.clone().data, device=mask4.device)
                    agent_mask4 = torch.as_tensor(agent_mask4_start.clone().data, device=agent_mask4.device)
                    if not mask4.byte().any():
                        decision_agent_id.remove(ag_id)
                    visit_mask4 = (mask4.clone()).sum(1).eq(0)
                    if visit_mask4.any():
                        visit_idx_mask4 = visit_mask4.nonzero().squeeze()
                        mask4[visit_idx_mask4, 0] = 1
                        mask4[visit_idx_mask4, 1:] = 0
                if ag_id == 5:
                    mask5_start, agent_mask5_start = self.mask_start(mask5, dynamic5, up_station, tw_mask, agent_mask5)
                    mask5 = torch.as_tensor(mask5_start.clone().data, device=mask5.device)
                    agent_mask5 = torch.as_tensor(agent_mask5_start.clone().data, device=agent_mask5.device)
                    if not mask5.byte().any():
                        decision_agent_id.remove(ag_id)
                    visit_mask5 = (mask5.clone()).sum(1).eq(0)
                    if visit_mask5.any():
                        visit_idx_mask5 = visit_mask5.nonzero().squeeze()
                        mask5[visit_idx_mask5, 0] = 1
                        mask5[visit_idx_mask5, 1:] = 0
            '''Determine which agents have reached the decision point based on the arrival status of the vehicle.
            Update the mask according to the travel demand. Mask update rules: 
            # (1) when there is no passenger flow at a station, its mask is set to 0; 
            # (2) when there are passengers at the boarding station, the masks of all alighting stations are set to 0; 
            # (3) when all boarding stations When the passenger flow of the station is 0, the masks of all alighting stations are set to 1; 
            # (4) when there is no passenger flow at all boarding stations and all alighting stations, only the mask of CP is set to 1.'''
            for a_n in decision_agent_id:
                if a_n == 1:
                    decision_mask_tensor1 = torch.tensor(decision_mask_dict[a_n]).clone().to(device)
                    decoder_hidden1 = self.agent_decoder1(decoder_input1)
                    probs1, last_hh1 = self.agent_pointer1(static_hidden, dynamic_hidden1, decoder_hidden1, last_hh1)
                    probs1 = F.softmax(probs1 + mask1.log(), dim=1)
                    # During training, the action is sampled for the next step according to its probability;
                    # During testing, we can take a greedy approach and select the action with the highest probability
                    if self.training:
                        m1 = torch.distributions.Categorical(probs1)  # Sampling
                        ptr1 = m1.sample()
                        while not torch.gather(mask1, 1, ptr1.data.unsqueeze(1)).byte().all():
                            ptr1 = m1.sample()
                        logp1 = m1.log_prob(ptr1)
                        ptr1 = ptr1 * decision_mask_tensor1.clone()
                        logp1 = logp1 * decision_mask_tensor1.clone()
                    else:
                        prob1, ptr1 = torch.max(probs1, 1)  # Greedy
                        ptr1 = ptr1 * decision_mask_tensor1.clone()
                        logp1 = prob1.log()

                    constraint_utw1 = [record1.clone(), record2.clone(), record3.clone()]
                    constraint_1_1, line_stop_tw1, tw_mask_tw1 = self.update_tw(dynamic1, ptr1.data, constraint_utw1, a_n, tw_mask)
                    record1_cl1_1 = constraint_1_1[0].clone()
                    record2_cl1_1 = constraint_1_1[1].clone()
                    record3_cl1_1 = constraint_1_1[2].clone()
                    record1 = torch.as_tensor(record1_cl1_1.clone().data, device=record1.device)
                    record2 = torch.as_tensor(record2_cl1_1.clone().data, device=record2.device)
                    record3 = torch.as_tensor(record3_cl1_1.clone().data, device=record3.device)
                    tw_mask = torch.as_tensor(tw_mask_tw1.clone().data, device=tw_mask.device)

                    if self.update_fn is not None:
                        constraint_ufn1 = [record1.clone(), record2.clone(), record3.clone()]
                        dynamic1, constraint_1_2 = self.update_fn(dynamic1, ptr1.data, constraint_ufn1, static,
                                                                  up_station, travel_time_G, all_station,
                                                                  tour_idx_dict[a_n], line_stop_tw1)
                        record1_cl1_2 = constraint_1_2[0].clone()
                        record2_cl1_2 = constraint_1_2[1].clone()
                        record3_cl1_2 = constraint_1_2[2].clone()
                        record1 = torch.as_tensor(record1_cl1_2.clone().data, device=record1.device)
                        record2 = torch.as_tensor(record2_cl1_2.clone().data, device=record2.device)
                        record3 = torch.as_tensor(record3_cl1_2.clone().data, device=record3.device)
                        '''Update observation information for the agent'''
                        dynamic_hidden1 = self.agent_dynamic_encoder1(dynamic1)
                    dynamic2_wbl1 = dynamic2.clone()
                    dynamic3_wbl1 = dynamic3.clone()
                    dynamic4_wbl1 = dynamic4.clone()
                    dynamic5_wbl1 = dynamic5.clone()
                    dynamic2_wbl1[:, 1:4, 0:len(up_station)] = dynamic1[:, 1:4, 0:len(up_station)].clone()
                    dynamic3_wbl1[:, 1:4, 0:len(up_station)] = dynamic1[:, 1:4, 0:len(up_station)].clone()
                    dynamic4_wbl1[:, 1:4, 0:len(up_station)] = dynamic1[:, 1:4, 0:len(up_station)].clone()
                    dynamic5_wbl1[:, 1:4, 0:len(up_station)] = dynamic1[:, 1:4, 0:len(up_station)].clone()
                    dynamic2 = torch.as_tensor(dynamic2_wbl1.data, device=dynamic2.device)
                    dynamic3 = torch.as_tensor(dynamic3_wbl1.data, device=dynamic3.device)
                    dynamic4 = torch.as_tensor(dynamic4_wbl1.data, device=dynamic4.device)
                    dynamic5 = torch.as_tensor(dynamic5_wbl1.data, device=dynamic5.device)
                    for ns0 in range(len(decision_list)):
                        if ptr1.data[ns0].item() != 0:
                            decision_list[ns0].append([dynamic1[ns0][10][0].clone().item(), a_n])
                    tour_logp[a_n].append(logp1.unsqueeze(1))
                    tour_idx[a_n].append(ptr1.data.unsqueeze(1))
                    for ns1 in range(len(decision_list)):
                        if ptr1.data[ns1].item() != 0:
                            tour_idx_dict[a_n][ns1].append(ptr1.data[ns1].item())
                    if self.mask_fn is not None:
                        '''Update mask information for the agent'''
                        mask1_fn, agent_mask1_fn = self.mask_fn(mask1, dynamic1, agent_mask1, ptr1.data)
                        mask1 = torch.as_tensor(mask1_fn.clone().data, device=mask1.device)
                        agent_mask1 = torch.as_tensor(agent_mask1_fn.clone().data, device=agent_mask1.device)
                    # Update the decoder input for each agent
                    decoder_input1 = torch.gather(static, 2, ptr1.view(-1, 1, 1).expand(-1, input_size, 1)).detach()

                if a_n == 2:
                    decision_mask_tensor2 = torch.tensor(decision_mask_dict[a_n]).clone().to(device)
                    decoder_hidden2 = self.agent_decoder2(decoder_input2)
                    probs2, last_hh2 = self.agent_pointer2(static_hidden, dynamic_hidden2, decoder_hidden2, last_hh2)
                    probs2 = F.softmax(probs2 + mask2.log(), dim=1)
                    if self.training:
                        m2 = torch.distributions.Categorical(probs2)
                        ptr2 = m2.sample()
                        while not torch.gather(mask2, 1, ptr2.data.unsqueeze(1)).byte().all():
                            ptr2 = m2.sample()
                        logp2 = m2.log_prob(ptr2)
                        ptr2 = ptr2 * decision_mask_tensor2.clone()
                        logp2 = logp2 * decision_mask_tensor2.clone()
                    else:
                        prob2, ptr2 = torch.max(probs2, 1)
                        ptr2 = ptr2 * decision_mask_tensor2.clone()
                        logp2 = prob2.log()

                    constraint_utw2 = [record1.clone(), record2.clone(), record3.clone()]
                    constraint_2_1, line_stop_tw2, tw_mask_tw2 = self.update_tw(dynamic2, ptr2.data, constraint_utw2,
                                                                                a_n, tw_mask)
                    record1_cl2_1 = constraint_2_1[0].clone()
                    record2_cl2_1 = constraint_2_1[1].clone()
                    record3_cl2_1 = constraint_2_1[2].clone()
                    record1 = torch.as_tensor(record1_cl2_1.clone().data, device=record1.device)
                    record2 = torch.as_tensor(record2_cl2_1.clone().data, device=record2.device)
                    record3 = torch.as_tensor(record3_cl2_1.clone().data, device=record3.device)
                    tw_mask = torch.as_tensor(tw_mask_tw2.clone().data, device=tw_mask.device)
                    if self.update_fn is not None:
                        constraint_ufn2 = [record1.clone(), record2.clone(), record3.clone()]
                        dynamic2, constraint_2_2 = self.update_fn(dynamic2, ptr2.data, constraint_ufn2,
                                                                  static, up_station, travel_time_G,
                                                                  all_station, tour_idx_dict[a_n], line_stop_tw2)
                        record1_cl2_2 = constraint_2_2[0].clone()
                        record2_cl2_2 = constraint_2_2[1].clone()
                        record3_cl2_2 = constraint_2_2[2].clone()
                        record1 = torch.as_tensor(record1_cl2_2.clone().data, device=record1.device)
                        record2 = torch.as_tensor(record2_cl2_2.clone().data, device=record2.device)
                        record3 = torch.as_tensor(record3_cl2_2.clone().data, device=record3.device)
                        '''Update observation information for the agent'''
                        dynamic_hidden2 = self.agent_dynamic_encoder2(dynamic2)
                    dynamic1_wbl2 = dynamic1.clone()
                    dynamic3_wbl2 = dynamic3.clone()
                    dynamic4_wbl2 = dynamic4.clone()
                    dynamic5_wbl2 = dynamic5.clone()
                    dynamic1_wbl2[:, 1:4, 0:len(up_station)] = dynamic2[:, 1:4, 0:len(up_station)].clone()
                    dynamic3_wbl2[:, 1:4, 0:len(up_station)] = dynamic2[:, 1:4, 0:len(up_station)].clone()
                    dynamic4_wbl2[:, 1:4, 0:len(up_station)] = dynamic2[:, 1:4, 0:len(up_station)].clone()
                    dynamic5_wbl2[:, 1:4, 0:len(up_station)] = dynamic2[:, 1:4, 0:len(up_station)].clone()
                    dynamic1 = torch.as_tensor(dynamic1_wbl2.data, device=dynamic1.device)
                    dynamic3 = torch.as_tensor(dynamic3_wbl2.data, device=dynamic3.device)
                    dynamic4 = torch.as_tensor(dynamic4_wbl2.data, device=dynamic4.device)
                    dynamic5 = torch.as_tensor(dynamic5_wbl2.data, device=dynamic5.device)
                    for ns0 in range(len(decision_list)):
                        if ptr2.data[ns0].item() != 0:
                            decision_list[ns0].append([dynamic1[ns0][10][0].clone().item(), a_n])
                    tour_logp[a_n].append(logp2.unsqueeze(1))
                    tour_idx[a_n].append(ptr2.data.unsqueeze(1))
                    for ns1 in range(len(decision_list)):
                        if ptr2.data[ns1].item() != 0:
                            tour_idx_dict[a_n][ns1].append(ptr2.data[ns1].item())
                    if self.mask_fn is not None:
                        '''Update mask information for the agent'''
                        mask2_fn, agent_mask2_fn = self.mask_fn(mask2, dynamic2, agent_mask2, ptr2.data)
                        mask2 = torch.as_tensor(mask2_fn.clone().data, device=mask2.device)
                        agent_mask2 = torch.as_tensor(agent_mask2_fn.clone().data, device=agent_mask2.device)
                    decoder_input2 = torch.gather(static, 2, ptr2.view(-1, 1, 1).expand(-1, input_size, 1)).detach()

                if a_n == 3:
                    decision_mask_tensor3 = torch.tensor(decision_mask_dict[a_n]).clone().to(device)
                    decoder_hidden3 = self.agent_decoder3(decoder_input3)
                    probs3, last_hh3 = self.agent_pointer3(static_hidden, dynamic_hidden3, decoder_hidden3, last_hh3)
                    probs3 = F.softmax(probs3 + mask3.log(), dim=1)
                    if self.training:
                        m3 = torch.distributions.Categorical(probs3)
                        ptr3 = m3.sample()
                        while not torch.gather(mask3, 1, ptr3.data.unsqueeze(1)).byte().all():
                            ptr3 = m3.sample()
                        logp3 = m3.log_prob(ptr3)
                        ptr3 = ptr3 * decision_mask_tensor3.clone()
                        logp3 = logp3 * decision_mask_tensor3.clone()
                    else:
                        prob3, ptr3 = torch.max(probs3, 1)
                        ptr3 = ptr3 * decision_mask_tensor3.clone()
                        logp3 = prob3.log()

                    constraint_utw3 = [record1.clone(), record2.clone(), record3.clone()]
                    constraint_3_1, line_stop_tw3, tw_mask_tw3 = self.update_tw(dynamic3, ptr3.data, constraint_utw3,
                                                                                a_n, tw_mask)
                    record1_cl3_1 = constraint_3_1[0].clone()
                    record2_cl3_1 = constraint_3_1[1].clone()
                    record3_cl3_1 = constraint_3_1[2].clone()
                    record1 = torch.as_tensor(record1_cl3_1.clone().data, device=record1.device)
                    record2 = torch.as_tensor(record2_cl3_1.clone().data, device=record2.device)
                    record3 = torch.as_tensor(record3_cl3_1.clone().data, device=record3.device)
                    tw_mask = torch.as_tensor(tw_mask_tw3.clone().data, device=tw_mask.device)
                    if self.update_fn is not None:
                        constraint_ufn3 = [record1.clone(), record2.clone(), record3.clone()]
                        dynamic3, constraint_3_2 = self.update_fn(dynamic3, ptr3.data, constraint_ufn3, static, up_station,
                                                                  travel_time_G, all_station, tour_idx_dict[a_n], line_stop_tw3)
                        record1_cl3_2 = constraint_3_2[0].clone()
                        record2_cl3_2 = constraint_3_2[1].clone()
                        record3_cl3_2 = constraint_3_2[2].clone()
                        record1 = torch.as_tensor(record1_cl3_2.clone().data, device=record1.device)
                        record2 = torch.as_tensor(record2_cl3_2.clone().data, device=record2.device)
                        record3 = torch.as_tensor(record3_cl3_2.clone().data, device=record3.device)
                        '''Update observation information for the agent'''
                        dynamic_hidden3 = self.agent_dynamic_encoder3(dynamic3)
                    dynamic1_wbl3 = dynamic1.clone()
                    dynamic2_wbl3 = dynamic2.clone()
                    dynamic4_wbl3 = dynamic4.clone()
                    dynamic5_wbl3 = dynamic5.clone()
                    dynamic1_wbl3[:, 1:4, 0:len(up_station)] = dynamic3[:, 1:4, 0:len(up_station)].clone()
                    dynamic2_wbl3[:, 1:4, 0:len(up_station)] = dynamic3[:, 1:4, 0:len(up_station)].clone()
                    dynamic4_wbl3[:, 1:4, 0:len(up_station)] = dynamic3[:, 1:4, 0:len(up_station)].clone()
                    dynamic5_wbl3[:, 1:4, 0:len(up_station)] = dynamic3[:, 1:4, 0:len(up_station)].clone()
                    dynamic1 = torch.as_tensor(dynamic1_wbl3.data, device=dynamic1.device)
                    dynamic2 = torch.as_tensor(dynamic2_wbl3.data, device=dynamic2.device)
                    dynamic4 = torch.as_tensor(dynamic4_wbl3.data, device=dynamic4.device)
                    dynamic5 = torch.as_tensor(dynamic5_wbl3.data, device=dynamic5.device)
                    for ns0 in range(len(decision_list)):
                        if ptr3.data[ns0].item() != 0:
                            decision_list[ns0].append([dynamic3[ns0][10][0].clone().item(), a_n])
                    tour_logp[a_n].append(logp3.unsqueeze(1))
                    tour_idx[a_n].append(ptr3.data.unsqueeze(1))
                    for ns1 in range(len(decision_list)):
                        if ptr3.data[ns1].item() != 0:
                            tour_idx_dict[a_n][ns1].append(ptr3.data[ns1].item())
                    if self.mask_fn is not None:
                        '''Update mask information for the agent'''
                        mask3_fn, agent_mask3_fn = self.mask_fn(mask3, dynamic3, agent_mask3, ptr3.data)
                        mask3 = torch.as_tensor(mask3_fn.clone().data, device=mask3.device)
                        agent_mask3 = torch.as_tensor(agent_mask3_fn.clone().data, device=agent_mask3.device)
                    decoder_input3 = torch.gather(static, 2, ptr3.view(-1, 1, 1).expand(-1, input_size, 1)).detach()

                if a_n == 4:
                    decision_mask_tensor4 = torch.tensor(decision_mask_dict[a_n]).clone().to(device)
                    decoder_hidden4 = self.agent_decoder4(decoder_input4)
                    probs4, last_hh4 = self.agent_pointer4(static_hidden, dynamic_hidden4, decoder_hidden4, last_hh4)
                    probs4 = F.softmax(probs4 + mask4.log(), dim=1)
                    if self.training:
                        m4 = torch.distributions.Categorical(probs4)
                        ptr4 = m4.sample()
                        while not torch.gather(mask4, 1, ptr4.data.unsqueeze(1)).byte().all():
                            ptr4 = m4.sample()
                        logp4 = m4.log_prob(ptr4)
                        ptr4 = ptr4 * decision_mask_tensor4.clone()
                        logp4 = logp4 * decision_mask_tensor4.clone()
                    else:
                        prob4, ptr4 = torch.max(probs4, 1)
                        ptr4 = ptr4 * decision_mask_tensor4.clone()
                        logp4 = prob4.log()
                    constraint_utw4 = [record1.clone(), record2.clone(), record3.clone()]
                    constraint_4_1, line_stop_tw4, tw_mask_tw4 = self.update_tw(dynamic4, ptr4.data, constraint_utw4,
                                                                                a_n, tw_mask)
                    record1_cl4_1 = constraint_4_1[0].clone()
                    record2_cl4_1 = constraint_4_1[1].clone()
                    record3_cl4_1 = constraint_4_1[2].clone()
                    record1 = torch.as_tensor(record1_cl4_1.clone().data, device=record1.device)
                    record2 = torch.as_tensor(record2_cl4_1.clone().data, device=record2.device)
                    record3 = torch.as_tensor(record3_cl4_1.clone().data, device=record3.device)
                    tw_mask = torch.as_tensor(tw_mask_tw4.clone().data, device=tw_mask.device)
                    if self.update_fn is not None:
                        constraint_ufn4 = [record1.clone(), record2.clone(), record3.clone()]
                        dynamic4, constraint_4_2 = self.update_fn(dynamic4, ptr4.data, constraint_ufn4, static, up_station,
                                                                  travel_time_G, all_station, tour_idx_dict[a_n], line_stop_tw4)
                        record1_cl4_2 = constraint_4_2[0].clone()
                        record2_cl4_2 = constraint_4_2[1].clone()
                        record3_cl4_2 = constraint_4_2[2].clone()
                        record1 = torch.as_tensor(record1_cl4_2.clone().data, device=record1.device)
                        record2 = torch.as_tensor(record2_cl4_2.clone().data, device=record2.device)
                        record3 = torch.as_tensor(record3_cl4_2.clone().data, device=record3.device)
                        dynamic_hidden4 = self.agent_dynamic_encoder4(dynamic4)
                    dynamic1_wbl4 = dynamic1.clone()
                    dynamic2_wbl4 = dynamic2.clone()
                    dynamic3_wbl4 = dynamic3.clone()
                    dynamic5_wbl4 = dynamic5.clone()
                    dynamic1_wbl4[:, 1:4, 0:len(up_station)] = dynamic4[:, 1:4, 0:len(up_station)].clone()
                    dynamic2_wbl4[:, 1:4, 0:len(up_station)] = dynamic4[:, 1:4, 0:len(up_station)].clone()
                    dynamic3_wbl4[:, 1:4, 0:len(up_station)] = dynamic4[:, 1:4, 0:len(up_station)].clone()
                    dynamic5_wbl4[:, 1:4, 0:len(up_station)] = dynamic4[:, 1:4, 0:len(up_station)].clone()
                    dynamic1 = torch.as_tensor(dynamic1_wbl4.data, device=dynamic1.device)
                    dynamic2 = torch.as_tensor(dynamic2_wbl4.data, device=dynamic2.device)
                    dynamic3 = torch.as_tensor(dynamic3_wbl4.data, device=dynamic3.device)
                    dynamic5 = torch.as_tensor(dynamic5_wbl4.data, device=dynamic5.device)
                    for ns0 in range(len(decision_list)):
                        if ptr4.data[ns0].item() != 0:
                            decision_list[ns0].append([dynamic4[ns0][10][0].clone().item(), a_n])
                    tour_logp[a_n].append(logp4.unsqueeze(1))
                    tour_idx[a_n].append(ptr4.data.unsqueeze(1))
                    for ns1 in range(len(decision_list)):
                        if ptr4.data[ns1].item() != 0:
                            tour_idx_dict[a_n][ns1].append(ptr4.data[ns1].item())
                    if self.mask_fn is not None:
                        mask4_fn, agent_mask4_fn = self.mask_fn(mask4, dynamic4, agent_mask4, ptr4.data)
                        mask4 = torch.as_tensor(mask4_fn.clone().data, device=mask4.device)
                        agent_mask4 = torch.as_tensor(agent_mask4_fn.clone().data, device=agent_mask4.device)
                    decoder_input4 = torch.gather(static, 2, ptr4.view(-1, 1, 1).expand(-1, input_size, 1)).detach()

                if a_n == 5:
                    decision_mask_tensor5 = torch.tensor(decision_mask_dict[a_n]).clone().to(device)
                    decoder_hidden5 = self.agent_decoder5(decoder_input5)
                    probs5, last_hh5 = self.agent_pointer5(static_hidden, dynamic_hidden5, decoder_hidden5, last_hh5)
                    probs5 = F.softmax(probs5 + mask5.log(), dim=1)
                    if self.training:
                        m5 = torch.distributions.Categorical(probs5)
                        ptr5 = m5.sample()
                        while not torch.gather(mask5, 1, ptr5.data.unsqueeze(1)).byte().all():
                            ptr5 = m5.sample()
                        logp5 = m5.log_prob(ptr5)
                        ptr5 = ptr5 * decision_mask_tensor5.clone()
                        logp5 = logp5 * decision_mask_tensor5.clone()
                    else:
                        prob5, ptr5 = torch.max(probs5, 1)
                        ptr5 = ptr5 * decision_mask_tensor5.clone()
                        logp5 = prob5.log()
                    constraint_utw5 = [record1.clone(), record2.clone(), record3.clone()]
                    constraint_5_1, line_stop_tw5, tw_mask_tw5 = self.update_tw(dynamic5, ptr5.data, constraint_utw5,
                                                                                a_n, tw_mask)
                    record1_cl5_1 = constraint_5_1[0].clone()
                    record2_cl5_1 = constraint_5_1[1].clone()
                    record3_cl5_1 = constraint_5_1[2].clone()
                    record1 = torch.as_tensor(record1_cl5_1.clone().data, device=record1.device)
                    record2 = torch.as_tensor(record2_cl5_1.clone().data, device=record2.device)
                    record3 = torch.as_tensor(record3_cl5_1.clone().data, device=record3.device)
                    tw_mask = torch.as_tensor(tw_mask_tw5.clone().data, device=tw_mask.device)
                    if self.update_fn is not None:
                        constraint_ufn5 = [record1.clone(), record2.clone(), record3.clone()]
                        dynamic5, constraint_5_2 = self.update_fn(dynamic5, ptr5.data, constraint_ufn5, static, up_station,
                                                                  travel_time_G, all_station, tour_idx_dict[a_n], line_stop_tw5)
                        record1_cl5_2 = constraint_5_2[0].clone()
                        record2_cl5_2 = constraint_5_2[1].clone()
                        record3_cl5_2 = constraint_5_2[2].clone()
                        record1 = torch.as_tensor(record1_cl5_2.clone().data, device=record1.device)
                        record2 = torch.as_tensor(record2_cl5_2.clone().data, device=record2.device)
                        record3 = torch.as_tensor(record3_cl5_2.clone().data, device=record3.device)
                        dynamic_hidden5 = self.agent_dynamic_encoder5(dynamic5)
                    dynamic1_wbl5 = dynamic1.clone()
                    dynamic2_wbl5 = dynamic2.clone()
                    dynamic3_wbl5 = dynamic3.clone()
                    dynamic4_wbl5 = dynamic4.clone()
                    dynamic1_wbl5[:, 1:4, 0:len(up_station)] = dynamic5[:, 1:4, 0:len(up_station)].clone()
                    dynamic2_wbl5[:, 1:4, 0:len(up_station)] = dynamic5[:, 1:4, 0:len(up_station)].clone()
                    dynamic3_wbl5[:, 1:4, 0:len(up_station)] = dynamic5[:, 1:4, 0:len(up_station)].clone()
                    dynamic4_wbl5[:, 1:4, 0:len(up_station)] = dynamic5[:, 1:4, 0:len(up_station)].clone()
                    dynamic1 = torch.as_tensor(dynamic1_wbl5.data, device=dynamic1.device)
                    dynamic2 = torch.as_tensor(dynamic2_wbl5.data, device=dynamic2.device)
                    dynamic3 = torch.as_tensor(dynamic3_wbl5.data, device=dynamic3.device)
                    dynamic4 = torch.as_tensor(dynamic4_wbl5.data, device=dynamic4.device)
                    for ns0 in range(len(decision_list)):
                        if ptr5.data[ns0].item() != 0:
                            decision_list[ns0].append([dynamic5[ns0][10][0].clone().item(), a_n])
                    tour_logp[a_n].append(logp5.unsqueeze(1))
                    tour_idx[a_n].append(ptr5.data.unsqueeze(1))
                    for ns1 in range(len(decision_list)):
                        if ptr5.data[ns1].item() != 0:
                            tour_idx_dict[a_n][ns1].append(ptr5.data[ns1].item())
                    if self.mask_fn is not None:
                        mask5_fn, agent_mask5_fn = self.mask_fn(mask5, dynamic5, agent_mask5, ptr5.data)
                        mask5 = torch.as_tensor(mask5_fn.clone().data, device=mask5.device)
                        agent_mask5 = torch.as_tensor(agent_mask5_fn.clone().data, device=agent_mask5.device)
                    decoder_input5 = torch.gather(static, 2, ptr5.view(-1, 1, 1).expand(-1, input_size, 1)).detach()
                # Update the mask state of all agents
                for ag_id in range(a_n+1, self.agent_number):
                    if ag_id == 1:
                        mask1_start, agent_mask1_start = self.mask_start(mask1, dynamic1, up_station, tw_mask, agent_mask1)
                        mask1 = torch.as_tensor(mask1_start.clone().data, device=mask1.device)
                        agent_mask1 = torch.as_tensor(agent_mask1_start.clone().data, device=agent_mask1.device)
                        visit_mask1 = (mask1.clone()).sum(1).eq(0)
                        if visit_mask1.any():
                            visit_idx_mask1 = visit_mask1.nonzero().squeeze()
                            mask1[visit_idx_mask1, 0] = 1
                            mask1[visit_idx_mask1, 1:] = 0
                    if ag_id == 2:
                        mask2_start, agent_mask2_start = self.mask_start(mask2, dynamic2, up_station, tw_mask, agent_mask2)
                        mask2 = torch.as_tensor(mask2_start.clone().data, device=mask2.device)
                        agent_mask2 = torch.as_tensor(agent_mask2_start.clone().data, device=agent_mask2.device)
                        visit_mask2 = (mask2.clone()).sum(1).eq(0)
                        if visit_mask2.any():
                            visit_idx_mask2 = visit_mask2.nonzero().squeeze()
                            mask2[visit_idx_mask2, 0] = 1
                            mask2[visit_idx_mask2, 1:] = 0
                    if ag_id == 3:
                        mask3_start, agent_mask3_start = self.mask_start(mask3, dynamic3, up_station, tw_mask, agent_mask3)
                        mask3 = torch.as_tensor(mask3_start.clone().data, device=mask3.device)
                        agent_mask3 = torch.as_tensor(agent_mask3_start.clone().data, device=agent_mask3.device)
                        visit_mask3 = (mask3.clone()).sum(1).eq(0)
                        if visit_mask3.any():
                            visit_idx_mask3 = visit_mask3.nonzero().squeeze()
                            mask3[visit_idx_mask3, 0] = 1
                            mask3[visit_idx_mask3, 1:] = 0
                    if ag_id == 4:
                        mask4_start, agent_mask4_start = self.mask_start(mask4, dynamic4, up_station, tw_mask, agent_mask4)
                        mask4 = torch.as_tensor(mask4_start.clone().data, device=mask4.device)
                        agent_mask4 = torch.as_tensor(agent_mask4_start.clone().data, device=agent_mask4.device)
                        visit_mask4 = (mask4.clone()).sum(1).eq(0)
                        if visit_mask4.any():
                            visit_idx_mask4 = visit_mask4.nonzero().squeeze()
                            mask4[visit_idx_mask4, 0] = 1
                            mask4[visit_idx_mask4, 1:] = 0
                    if ag_id == 5:
                        mask5_start, agent_mask5_start = self.mask_start(mask5, dynamic5, up_station, tw_mask, agent_mask5)
                        mask5 = torch.as_tensor(mask5_start.clone().data, device=mask5.device)
                        agent_mask5 = torch.as_tensor(agent_mask5_start.clone().data, device=agent_mask5.device)
                        visit_mask5 = (mask5.clone()).sum(1).eq(0)
                        if visit_mask5.any():
                            visit_idx_mask5 = visit_mask5.nonzero().squeeze()
                            mask5[visit_idx_mask5, 0] = 1
                            mask5[visit_idx_mask5, 1:] = 0
            for so in range(len(decision_list)):
                list.sort(decision_list[so], key=(lambda x: [x[0]]))
        for a_id0 in range(1, self.agent_number):
            tour_idx[a_id0] = torch.cat(tour_idx[a_id0], dim=1)
            tour_logp[a_id0] = torch.cat(tour_logp[a_id0], dim=1)
        return tour_idx, tour_logp, [record1.clone(), record2.clone(), record3.clone()], [dynamic1.clone(), dynamic2.clone(), dynamic3.clone(), dynamic4.clone(), dynamic5.clone()]



if __name__ == '__main__':
    raise Exception('Cannot be called from main')
