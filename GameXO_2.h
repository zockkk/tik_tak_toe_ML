#pragma once
#include "AbstractAgent.h"
#include "AbstractEnvironment.h"
#include <random>
#include <chrono>


void correct_action(Matrix<double>& _Y, std::vector<double>& _prob, std::default_random_engine& _dre, Matrix<double> arr)
{
	for (size_t i = 0; i < 9; i++) {
		_Y[i][0] *= arr[i][0];
	}
	double summ = 0;
	for (size_t i = 0; i < 9; i++) {
		summ += _Y[i][0];
	}
	for (size_t i = 0; i < 9; i++) {
		_Y[i][0] /= summ;
	}

	for (size_t i = 0; i < 9; i++) {
		_prob[i] = _Y[i][0];
	}
	std::discrete_distribution<size_t> dd(begin(_prob), end(_prob));
	int a = dd(_dre);
	for (size_t i = 0; i < 9; i++) {
		_Y[i][0] = 0.0;
		if (a == i) {
			_Y[i][0] = 1.0;
		}
	}
}


void play_with_human(AgentXO& playerAI)
{
	std::default_random_engine dre(std::chrono::steady_clock::now().time_since_epoch().count());
	std::vector<double> probabilities(9);
	EnvironmentXO state;
	Matrix<double> Y = Matrix <double>::colVector(9);
	Matrix<double> check = Matrix<double>::colVector(9, 0);
	int color = -1;
	int a;
	while (state.nextState(check))
	{
		color *= (-1);
		if (color == 1)
		{
			playerAI.doAction(state.getBoard(), Y);
			correct_action(Y, probabilities, dre, state.getArr());
			state.nextState(Y);

		}
		else
		{
			state.inversion();
			state.print();
			std::cin >> a;
			for (size_t i = 0; i < 9; i++) {
				Y[i][0] = 0;
				if (i == (a - 1)) Y[i][0] = 1;
			}
			state.nextState(Y);
			state.inversion();
		}
		state.print();
	}
}

void play_with_random(AgentXO& playerAI) {
	int counter[3] = { 0,0,0 };
	std::default_random_engine dre(std::chrono::steady_clock::now().time_since_epoch().count());
	std::vector<double> probabilities(9);
	AgentXO playerRand(3, { 9, 81, 9 });
	EnvironmentXO state;
	Matrix<double> Y = Matrix <double>::colVector(9);
	Matrix<double> check = Matrix<double>::colVector(9, 0);
	for (int j = 0; j < 10000; j++)
	{
		int color = -1;
		bool flag = true;
		while (state.nextState(check) && flag)
		{
			color *= (-1);
			if (color == 1)
			{
				playerAI.doAction(state.getBoard(), Y);
				correct_action(Y, probabilities, dre, state.getArr());
				state.nextState(Y);

			}
			else
			{
				state.inversion();
				playerRand.doAction(state.getBoard(), Y);
				correct_action(Y, probabilities, dre, state.getArr());
				state.nextState(Y);
				state.inversion();
			}
			//state.print();
			if (!state.nextState(check))
			{
				if (state.win() == 1) counter[0] += 1;
				if (state.win() == 0) counter[1] += 1;
				if (state.win() == -1) counter[2] += 1;
				//std::cout << state.win() << std::endl;
			}
		}
		state.restart();
	}
	//std::cout << "win - " << counter[0] << "lose - " << counter[2] << "draw - " << counter[1] << std::endl;
	std::cout << counter[2] << std::endl;
}

void game2()
{
	std::default_random_engine dre(std::chrono::steady_clock::now().time_since_epoch().count());
	std::vector<double> probabilities(9);
	AgentXO player1(5, { 9, 9, 9, 9, 9 });
	AgentXO player2(5, { 9, 9, 9, 9, 9 });
	EnvironmentXO state;
	Matrix<double> Y = Matrix <double>::colVector(9);
	Matrix<double> check = Matrix<double>::colVector(9, 0);
	const int epoch = 400;
	const int games = 10000;
	bool flag = true;
	play_with_random(player1);
	for (int i = 0; i < epoch; i++) 
	{
		Matrix<double> errors_yEpoch = Matrix<double>::colVector(9, 0.0);
		int luse_epoch = 0;
		for (int j = 0; j < games; j++) 
		{
			Matrix<double> errors_yGame = Matrix<double>::colVector(9, 0.0);
			int color = -1;
			flag = true;
			while (state.nextState(check) && flag) 
			{
				color *= (-1);
				if (color == 1) 
				{
					player1.doAction(state.getBoard(), Y);
					errors_yGame += (-1) * Y;
					correct_action(Y, probabilities, dre, state.getArr());
					errors_yGame += Y;
					state.nextState(Y);

				}
				else 
				{
					state.inversion();
					player2.doAction(state.getBoard(), Y);
					correct_action(Y, probabilities, dre, state.getArr());
					state.nextState(Y);
					state.inversion();
				}
				//state.print();
			}
			if (!state.nextState(check))
			{
				player1.setRewardGame(state.win());
				if (state.win() == -1) luse_epoch++;
			}
			//std::cout << player1.getRewardGame() << std::endl;
			errors_yGame = errors_yGame * player1.getRewardGame();
			errors_yEpoch += errors_yGame;
			player1.learning(&errors_yGame);
			player1.setReward(player1.getRewardGame());
			player1.setRewardGame(-player1.getRewardGame());
			state.restart();
		}
		play_with_random(player1);
		//std::cout << "wins - " << player1.getReward() << std::endl << "lose - " << luse_epoch << std::endl;
		//std::cout << player1.getReward() << std::endl << fails_epoch << std::endl; //<< player2.getReward() << std::endl;
		errors_yEpoch *= (1.0 / games);//check
		player1.save('_' + std::to_string(i) + ".txt");
		//player1.learning(&errors_yEpoch);//check
		//if (i>(epoch/2)) player2.set_fromfile('_' + std::to_string(i) + ".txt");
		//errors_Y2 *= (1.0 / games) * player2.getReward();
		//player2.learning(&errors_Y2);
		player1.setReward(-player1.getReward());
		//player2.setReward(-player2.getReward());
	}
	for (int i = 0; i < 7; i++) 
	{
		//play_with_human(player1);
	}
}
