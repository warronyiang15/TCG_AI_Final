#ifndef MYAI_INCLUDED
#define MYAI_INCLUDED 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <string>

using std::stoi;
using std::string;

#define RED 0
#define BLUE 1
#define BOARD_SIZE 5
#define PIECE_NUM 6
#define COMMAND_NUM 7

class MyAI  
{
	const char* commands_name[COMMAND_NUM] = {
		"name",
		"version",
		"time_setting",
		"board_setting",
		"ini",
		"get",
		"exit"
	};
public:
	MyAI(void);
	~MyAI(void);

	// commands
	void Name(const char* data[], char* response);
	void Version(const char* data[], char* response);
	void Time_setting(const char* data[], char* response);
	void Board_setting(const char* data[], char* response);
	void Ini(const char* data[], char* response);
	void Get(const char* data[], char* response);
	void Exit(const char* data[], char* response);

private:
	bool red_exist[PIECE_NUM], blue_exist[PIECE_NUM];	// move effect (only when eat)
	int color;											// move effect
	int red_time, blue_time;
	int board_size;
	int dice;											// move effect
	int board[BOARD_SIZE][BOARD_SIZE];					// move effect (only one)
	int red_piece_num, blue_piece_num;					// move effect (only when eat)

	// Board
	void Init_board_state(char* position);
	void Set_board(char* position);
	void Print_chessboard();
	void Generate_move(char* move);
	void Make_move(const int piece, const int start_point, const int end_point);
	int get_legal_move(int* result); 
	int referee(int piece, int* src, int* dst);
	double eval(const int& my_color);
	bool isEnd();
	std::pair<double, int> F4( double alpha, double beta, int depth, const int& my_color);
	std::pair<double, int> G4( double alpha, double beta, int depth, const int& my_color);
	std::pair<double, int> Star1_F4( double alpha, double beta, int depth, const int& my_color );
	std::pair<double, int> Star1_G4( double alpha, double beta, int depth, const int& my_color );
	std::pair<double, int> Star0_F4( double alpha, double beta, int depth, const int& my_color );
	std::pair<double, int> Star0_G4( double alpha, double beta, int depth, const int& my_color );
	
	// Additional Function
	std::pair<int, int> Take_move(const int piece, const int start_point, const int end_point);
	void Undo_move(const int piece, const int start_point, const int end_point, std::pair<int, int>history);
};

#endif