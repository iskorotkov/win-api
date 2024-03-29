﻿#pragma once
#include "Windows.h"

class GameSession;

class GameRules
{
public:
	explicit GameRules(GameSession* game, int sign);

	class Status
	{
	public:
		static Status Win();
		static Status Lose();
		static Status InProgress();
		static Status Draw();

		[[nodiscard]] bool HasWon() const { return won; }
		[[nodiscard]] bool HasLost() const { return lost; }
		[[nodiscard]] bool IsInProgress() const { return !won && !lost; }
		[[nodiscard]] bool IsDraw() const { return draw; }

	protected:
		Status(const bool won, const bool lost, const bool draw = false) : won(won), lost(lost), draw(draw)
		{
		}

	private:
		bool won;
		bool lost;
		bool draw;
	};

	void RespondToTurnMessage(WPARAM wParam, LPARAM lParam);
	void FinishTurn();
	void StartTurn();

	[[nodiscard]] Status GetStatus() const;
	[[nodiscard]] bool IsOurTurn() const;
	[[nodiscard]] unsigned GetTurnMessageCode() const;
	[[nodiscard]] int GetOurSign() const;

private:
	bool isOurTurn = true;
	unsigned turnMessageCode = 0;
	GameSession* game;
	int ourSign;

	void SetupGame();

	void NotifyOthersAboutTurn() const;

	[[nodiscard]] bool IsOurSign(int value) const;
	[[nodiscard]] bool IsOpponentSign(int value) const;
	[[nodiscard]] bool IsTaken(int value) const;
};
