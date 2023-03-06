#include "float.h"
#include "MyAI.h"
#include <unistd.h>

MyAI::MyAI(void){}

MyAI::~MyAI(void){}

void MyAI::Name(const char* data[], char* response)
{
	strcpy(response, "MyAI");
}

void MyAI::Version(const char* data[], char* response)
{
	strcpy(response, "1.0.0");
}

void MyAI::Time_setting(const char* data[], char* response)
{
	this->red_time = stoi(data[1]);
	this->blue_time = stoi(data[1]);
	strcpy(response, "1");
}

void MyAI::Board_setting(const char* data[], char* response)
{
	this->board_size = stoi(data[1]);
	this->red_piece_num = stoi(data[2]);
	this->blue_piece_num = stoi(data[2]);
	strcpy(response, "1");
}

void MyAI::Ini(const char* data[], char* response)
{
	// set color
	if(!strcmp(data[1], "R"))
	{
		this->color = RED;
	}
	else if(!strcmp(data[1], "B"))
	{
		this->color = BLUE;
	}

	char position[15];
	this->Init_board_state(position);

	sprintf(response, "%c%c %c%c %c%c %c%c %c%c %c%c", position[0], position[1], position[2], position[3],
													   position[4], position[5], position[6], position[7],
													   position[8], position[9], position[10], position[11]);

}

void MyAI::Get(const char* data[], char* response)
{
	// set color
	if(!strcmp(data[1], "R"))
	{
		this->color = RED;
	}
	else if(!strcmp(data[1], "B"))
	{
		this->color = BLUE;
	}

	// set dice & board
	this->dice = stoi(data[2]);
	char position[25];
	sprintf(position, "%s%s%s%s%s%s%s%s%s%s%s%s", data[3], data[4], data[5], data[6], data[7], data[8], 
												  data[9], data[10], data[11], data[12], data[13], data[14]);
	this->Set_board(position);

	// generate move
	char move[4];
	this->Generate_move(move);
	sprintf(response, "%c%c %c%c", move[0], move[1], move[2], move[3]);
}

void MyAI::Exit(const char* data[], char* response)
{
	fprintf(stderr, "Bye~\n");
}

// *********************** AI FUNCTION *********************** //

void MyAI::Init_board_state(char* position)
{
	int order[PIECE_NUM] = {0, 1, 2, 3, 4, 5};
	string red_init_position = "A1B1C1A2B2A3";
	string blue_init_position = "E3D4E4C5D5E5";

	// assign the initial positions of your pieces in random order
	for(int i = 0; i < PIECE_NUM; i++)
	{
		int j = rand() % (PIECE_NUM - i) + i;
		int tmp = order[i];
		order[i] = order[j];
		order[j] = tmp;
	}

	for(int i = 0; i < PIECE_NUM; i++)
	{
		if(this->color == RED)
		{
			position[order[i] * 2] = red_init_position[i * 2];
			position[order[i] * 2 + 1] = red_init_position[i * 2 + 1];
		}
		else if(this->color == BLUE)
		{
			position[order[i] * 2] = blue_init_position[i * 2];
			position[order[i] * 2 + 1] = blue_init_position[i * 2 + 1];
		}
	}
}

void MyAI::Set_board(char* position)
{
	memset(this->board, 0, sizeof(this->board));
	memset(this->blue_exist, 1, sizeof(this->blue_exist));
	memset(this->red_exist, 1, sizeof(this->red_exist));
	this->blue_piece_num = PIECE_NUM;
	this->red_piece_num = PIECE_NUM;

	int lost_piece_num = 0;
	for(int i = 0; i < PIECE_NUM * 2; i++)
	{
		int index = i * 2 - lost_piece_num;

		// the piece does not exist
		while(position[index] == '0')
		{
			index = i * 2 - lost_piece_num + 1;
			lost_piece_num++;
			// blue
			if(i < PIECE_NUM) 
			{
				this->blue_piece_num --;
				this->blue_exist[i] = 0;
			}
			// red
			else 
			{
				this->red_piece_num --;
				this->red_exist[i - PIECE_NUM] = 0;
			}
			i += 1;
		}
		// 1~6: blue pieces; 7~12: red pieces
		this->board[position[index + 1] - '1'][position[index] - 'A'] = i + 1;
	}
	fprintf(stderr, "\nThe current board:\n");
	this->Print_chessboard();
}

void MyAI::Print_chessboard()
{
	fprintf(stderr, "\n");
	// 1~6 represent blue pieces; A~F represent red pieces
	for(int i = 0;i < BOARD_SIZE; i++)
	{
		fprintf(stderr, "<%d>   ", i + 1);
		for(int j = 0;j < BOARD_SIZE; j++)
		{
			if(this->board[i][j] <= PIECE_NUM) fprintf(stderr, "%d  ", this->board[i][j]);
			else fprintf(stderr, "%c  ", 'A' + (this->board[i][j] - 7)); 
		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "\n     ");
	for (int i = 0;i < BOARD_SIZE; i++)
	{
		fprintf(stderr, "<%c>", 'A' + i);
	}
	fprintf(stderr, "\n\n");
	fprintf(stderr, "The number of blue pieces: %d\nThe number of red pieces: %d\n", this->blue_piece_num, this->red_piece_num);
}

#define MAX_INF 1000000
#define MIN_INF -1000000
#define MAX_EVAL 393216
#define MIN_EVAL -393216

const int red_eval[BOARD_SIZE][BOARD_SIZE] = {
	{ 0,  1,  1,  1,  1},
	{ 1,  2,  2,  2,  3},
	{ 1,  2,  4,  4,  5},
	{ 1,  2,  4,  8, 10},
	{ 1,  3,  5, 10, 16}
};

const int blue_eval[BOARD_SIZE][BOARD_SIZE] = {
	{16, 10,  5,  3,  1},
	{10,  8,  4,  2,  1},
	{ 5,  4,  4,  2,  1},
	{ 3,  2,  2,  2,  1},
	{ 1,  1,  1,  1,  0}
};

const int POW2[17] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536};

bool MyAI::isEnd(){
	// one sides loses all the pieces
	if( this->red_piece_num <= 0 || this->blue_piece_num <= 0 ) return true;

	// Blue reached the top left end
	if( this->board[0][0] >= 1 && this->board[0][0] <= 6 ) return true;

	// Red reached the bottom right end
	if( this->board[4][4] >= 7 && this->board[4][4] <= 12) return true;
	return false;
}

int MyAI::eval(const int& my_color){
	// if one of the side doesn't have pieces
	// 393216, -393216
	if( this->red_piece_num <= 0 ){
		if( my_color == RED ) return MIN_EVAL;
		else return MAX_EVAL;
	}
	if( this->blue_piece_num <= 0 ){
		if( my_color == RED ) return MAX_EVAL;
		else return MIN_EVAL;
	}
	// Blue reached the top left end
	if( this->board[0][0] >= 1 && this->board[0][0] <= 6 ) {
		if( my_color == BLUE ) return MAX_EVAL;
		else return MIN_EVAL;
	}
	// Red reached the bottom right end
	if( this->board[4][4] >= 7 && this->board[4][4] <= 12 ) {
		if( my_color == RED ) return MAX_EVAL;
		else return MIN_EVAL;
	}

	int red_exp[6], blue_exp[6], red_threat[6], blue_threat[6];
	memset(red_exp, -1, sizeof(red_exp));
	memset(blue_exp, -1, sizeof(blue_exp));
	memset(red_threat, 0, sizeof(red_threat));
	memset(blue_threat, 0, sizeof(blue_threat));

	for(int i = 0;i < BOARD_SIZE;i++){
		for(int j = 0;j < BOARD_SIZE;j++){
			// Empty pieces
			if( this->board[i][j] == 0 ) continue;
			// Blue Pieces
			if( this->board[i][j] >= 1 && this->board[i][j] <= 6 ) {
				int cube_index = this->board[i][j] - 1;
				int pos_score  = blue_eval[i][j];
				blue_exp[cube_index] = POW2[pos_score];
				// Blue_threat
				// top, left, top-left
				if( i - 1 >= 0 ){
					if( this->board[i - 1][j] >= 7 && this->board[i - 1][j] <= 12 ){
						int o_pos_score  = red_eval[i - 1][j];
						blue_threat[cube_index] = std::max(blue_threat[cube_index], POW2[o_pos_score]);
					}
				}
				if( j - 1 >= 0 ){
					if( this->board[i][j - 1] >= 7 && this->board[i][j - 1] <= 12 ){
						int o_pos_score  = red_eval[i][j - 1];
						blue_threat[cube_index] = std::max(blue_threat[cube_index], POW2[o_pos_score]);
					}
				}
				if( i - 1 >= 0 && j - 1 >= 0 ){
					if( this->board[i - 1][j - 1] >= 7 && this->board[i - 1][j - 1] <= 12 ){
						int o_pos_score  = red_eval[i - 1][j - 1];
						blue_threat[cube_index] = std::max(blue_threat[cube_index], POW2[o_pos_score]);
					}
				}
			}
			// Red Pieces
			else{
				int cube_index = this->board[i][j] - PIECE_NUM - 1;
				int pos_score  = red_eval[i][j];
				red_exp[cube_index] = POW2[pos_score];
				// Red_threat
				// bottom, right, bottom-right
				if( i + 1 < BOARD_SIZE ){
					if( this->board[i + 1][j] >= 1 && this->board[i + 1][j] <= 6 ){
						int o_pos_score = blue_eval[i + 1][j];
						red_threat[cube_index] = std::max(red_threat[cube_index], POW2[o_pos_score]);
					}
				}
				if( j + 1 < BOARD_SIZE ){
					if( this->board[i][j + 1] >= 1 && this->board[i][j + 1] <= 6 ){
						int o_pos_score = blue_eval[i][j + 1];
						red_threat[cube_index] = std::max(red_threat[cube_index], POW2[o_pos_score]);
					}
				}
				if( i + 1 < BOARD_SIZE && j + 1 < BOARD_SIZE ){
					if( this->board[i + 1][j + 1] >= 1 && this->board[i + 1][j + 1] <= 6 ){
						int o_pos_score = blue_eval[i + 1][j + 1];
						red_threat[cube_index] = std::max(red_threat[cube_index], POW2[o_pos_score] );
					}
				}
			}
		}
	}

	int red_total_exp  = 0;
	int blue_total_exp = 0;
	int red_total_threat  = 0;
	int blue_total_threat = 0;

	for(int cube_index = 0; cube_index < 6;cube_index++){
		// Not exists
		if( red_exp[cube_index] == -1 ){
			// search for lower
			for(int l = cube_index; l >= 0;l--){
				if( red_exp[l] != -1 ){
					red_exp[cube_index] = std::max(red_exp[cube_index], red_exp[l]);
					red_threat[cube_index] = std::max(red_threat[cube_index], red_threat[l]);
					break;
				}
			}
			// search for higher
			for(int r = cube_index; r < PIECE_NUM; r++){
				if( red_exp[r] != -1 ){
					red_exp[cube_index] = std::max(red_exp[cube_index], red_exp[r]);
					red_threat[cube_index] = std::max(red_threat[cube_index], red_threat[r]);
					break;
				}
			}
		}
		
		if( blue_exp[cube_index] == -1 ){
			for(int l = cube_index; l >= 0; l--){
				if( blue_exp[l] != -1 ){
					blue_exp[cube_index] = std::max(blue_exp[cube_index], blue_exp[l]);
					blue_threat[cube_index] = std::max(blue_threat[cube_index], blue_threat[l]);
					break;
				}
			}
			for(int r = cube_index; r < PIECE_NUM; r++){
				if( blue_exp[r] != -1 ){
					blue_exp[cube_index] = std::max(blue_exp[cube_index], blue_exp[r]);
					blue_threat[cube_index] = std::max(blue_threat[cube_index], blue_threat[r]);
					break;
				}
			}
		}

		red_total_exp += red_exp[cube_index];
		blue_total_exp += blue_exp[cube_index];
		red_total_threat += red_threat[cube_index];
		blue_total_threat += blue_threat[cube_index];
	}
	if( my_color == RED )
		return (red_total_exp - blue_total_exp) + (red_total_threat - blue_total_threat) / 3;
	else
		return (blue_total_exp - red_total_exp) + (blue_total_threat - red_total_threat) / 3;
}

// Define pair<int,int> as [score, best_move]
std::pair<int, int> MyAI::F3(int alpha, int beta, int depth, const int& my_color){
	int result[100];
	// determine the successor positions p1, ..., pb
	int move_count = this->get_legal_move(result);
	if( move_count == 0 || depth == 0 || this->isEnd() )
		return std::pair<int,int>(this->eval(my_color), -1);
	
	int m = MIN_INF;
	int t;
	std::pair<int, int>res(m, 0);
	for(int i = 0;i < move_count;i++){
		MyAI next_AI = *this;
		next_AI.Make_move(result[i * 3], result[i * 3 + 1], result[i * 3 + 2]);
		next_AI.color = !this->color;
		t = next_AI.Star0_F3( std::max(alpha, m), beta, depth - 1, my_color ).first;
		if( t > m ){
			m = t;
			res.first = m;
			res.second = i;
		}
		// beta cut off
		if( m >= beta ) return res;
	}
	return res;
}

std::pair<int, int> MyAI::G3(int alpha, int beta, int depth, const int& my_color){
	int result[100];
	// determine the successor position p1, ... , pb
	int move_count = this->get_legal_move(result);
	if( move_count == 0 || depth == 0 || this->isEnd() )
		return std::pair<int,int>(this->eval(my_color), -1);
	
	int m = MAX_INF;
	int t;
	std::pair<int, int>res(m, 0);
	for(int i = 0;i < move_count;i++){
		MyAI next_AI = *this;
		next_AI.Make_move(result[i * 3], result[i * 3 + 1], result[i * 3 + 2]);
		next_AI.color = !this->color;
		t = next_AI.Star0_G3( alpha, std::min(beta, m), depth - 1, my_color ).first;
		if( t < m ) {
			m = t;
			res.first = m;
			res.second = i;
		}
		// alpha cur off
		if( m <= alpha ) return res;
	}
	return res;
}

std::pair<int ,int> MyAI::Star0_F3(int alpha, int beta, int depth, const int& my_color){
	int vsum = 0;
	for(int dice = 1 ; dice <= 6; dice++){
		MyAI next_AI = *this;
		next_AI.dice = dice;
		vsum += next_AI.G3( MIN_INF, MAX_INF, depth, my_color ).first;
	}
	return std::pair<int,int>( vsum / 6, -1 );
}
std::pair<int ,int> MyAI::Star0_G3(int alpha, int beta, int depth, const int& my_color){
	int vsum = 0;
	for(int dice = 1; dice <= 6; dice++){
		MyAI next_AI = *this;
		next_AI.dice = dice;
		vsum += next_AI.F3( MIN_INF, MAX_INF, depth, my_color ).first;
	}
	return std::pair<int,int>( vsum / 6, -1 );
}

// TODO: here the main part
void MyAI::Generate_move(char* move)
{
	fprintf(stderr, " ---------- Before ----------\n");
	this->Print_chessboard();
	std::pair<int, int> res = this->F3(MIN_INF, MAX_INF, 5, this->color);
	fprintf(stderr, " ---------- After ----------\n");
	this->Print_chessboard();

	fprintf(stderr, " Evaluation Score -> %d\n", res.first);
	fprintf(stderr, " Best_move found -> %d\n", res.second);
	int result[100];
	int move_count = this->get_legal_move(result);
	int piece = result[res.second * 3];
	int start_point = result[res.second * 3 + 1];
	int end_point = result[res.second * 3 + 2];
	sprintf(move, "%c%c%c%c", 'A' + start_point % BOARD_SIZE, '1' + start_point / BOARD_SIZE, 'A' + end_point % BOARD_SIZE, '1' + end_point / BOARD_SIZE);
	this->Make_move(piece, start_point, end_point);
	// print the result
	fprintf(stderr, "============================\nMy result:\n");
	if(piece <= PIECE_NUM) fprintf(stderr, "Blue piece %d: (%c%c) -> (%c%c)\n", piece, move[0], move[1], move[2], move[3]);
	else fprintf(stderr, "Red piece %d: (%c%c) -> (%c%c)\n", piece - PIECE_NUM, move[0], move[1], move[2], move[3]);
	this->Print_chessboard();
	fprintf(stderr, "============================\n");
}

// get all legal moves
//! Note that get_legal_move() will return ALL AVAILABLE MOVE!
// Return value : int [move_count], (int* [all possible move])
int MyAI::get_legal_move(int* result)
{
	int src, dst[3];
	int movable_piece;
	int move_count = 0;
	int result_count = 0;

	if(this->color == BLUE)
	{
		// the corresponding piece is alive
		if(this->blue_exist[this->dice - 1])
		{
			movable_piece = this->dice;
			move_count = this->referee(movable_piece, &src, dst);
			for(int i = result_count; i < result_count + move_count; i++) 
			{
				result[i * 3] = movable_piece;
				result[i * 3 + 1] = src;
				result[i * 3 + 2] = dst[i];
			}
			result_count += move_count;
		}
		// the corresponding piece does not exist
		else
		{
			// seeking for the next-higher piece
			for(int i = this->dice; i <= PIECE_NUM; ++i)
			{
				if(this->blue_exist[i - 1])
				{
					movable_piece = i;
					move_count = this->referee(movable_piece, &src, dst);
					int index = 0;
					for(int j = result_count; j < result_count + move_count; j++, index++) 
					{
						result[j * 3] = movable_piece;
						result[j * 3 + 1] = src;
						result[j * 3 + 2] = dst[index];
					}
					result_count += move_count;
					break;
				}
			}
			// seeking for the next-lower piece
			for(int i = this->dice; i >= 1; --i)
			{
				if(this->blue_exist[i - 1])
				{
					movable_piece = i;
					move_count = this->referee(movable_piece, &src, dst);
					int index = 0;
					for(int j = result_count; j < result_count + move_count; j++, index++) 
					{
						result[j * 3] = movable_piece;
						result[j * 3 + 1] = src;
						result[j * 3 + 2] = dst[index];
					}
					result_count += move_count;
					break;
				}
			}
		}
	}

	else if(this->color == RED)
	{
		// the corresponding piece is alive
		if(this->red_exist[this->dice - 1])
		{
			movable_piece = this->dice + PIECE_NUM;
			move_count = this->referee(movable_piece, &src, dst);
			for(int i = result_count; i < result_count + move_count; i++) 
			{
				result[i * 3] = movable_piece;
				result[i * 3 + 1] = src;
				result[i * 3 + 2] = dst[i];
			}
			result_count += move_count;
		}
		// the corresponding piece does not exist
		else
		{
			// seeking for the next-higher piece
			for(int i = this->dice; i <= PIECE_NUM; ++i)
			{
				if(this->red_exist[i - 1])
				{
					movable_piece = i + PIECE_NUM; 
					move_count = this->referee(movable_piece, &src, dst);
					int index = 0;
					for(int j = result_count; j < result_count + move_count; j++, index++) 
					{
						result[j * 3] = movable_piece;
						result[j * 3 + 1] = src;
						result[j * 3 + 2] = dst[index];
					}
					result_count += move_count;
					break;
				}
			}
			// seeking for the next-lower piece
			for(int i = this->dice; i >= 1; --i)
			{
				if(this->red_exist[i - 1])
				{
					movable_piece = i + PIECE_NUM; 
					move_count = this->referee(movable_piece, &src, dst);
					int index = 0;
					for(int j = result_count; j < result_count + move_count; j++, index++) 
					{
						result[j * 3] = movable_piece;
						result[j * 3 + 1] = src;
						result[j * 3 + 2] = dst[index];
					}
					result_count += move_count;
					break;
				}
			}			
		}		
	}
	return result_count;
}

// get possible moves of the piece
int MyAI::referee(int piece, int* src, int* dst)
{
	for(int i = 0; i < BOARD_SIZE; i++)
	{
		for(int j = 0; j < BOARD_SIZE; j++)
		{
			if(this->board[i][j] == piece)
			{
				*src = i * BOARD_SIZE + j;
			}
		}
	}
	// blue piece
	if(piece <= PIECE_NUM) 
	{
		// the piece is on the leftmost column
		if(*src % BOARD_SIZE == 0)
		{
			dst[0] = *src - BOARD_SIZE; // up
			return 1;
		}
		// the piece is on the uppermost row
		else if(*src < BOARD_SIZE)
		{
			dst[0] = *src - 1; // left
			return 1;
		}
		else
		{
			dst[0] = *src - 1; // left
			dst[1] = *src - BOARD_SIZE; // up
			dst[2] = *src - BOARD_SIZE - 1; // upper left
			return 3;
		}
	}

	// red piece
	else
	{
		// the piece is on the rightmost column
		if(*src % BOARD_SIZE == 4)
		{
			dst[0] = *src + BOARD_SIZE; // down
			return 1;
		}
		// the piece is on the downmost row
		else if(*src >= BOARD_SIZE * (BOARD_SIZE - 1))
		{
			dst[0] = *src + 1; // right
			return 1;
		}
		else
		{
			dst[0] = *src + 1; // right
			dst[1] = *src + BOARD_SIZE; // down
			dst[2] = *src + BOARD_SIZE + 1; // bottom right
			return 3;
		}
	}	
}

// Move the pieces from *start_point to *end_point
// TODO: Undo_move
void MyAI::Make_move(const int piece, const int start_point, const int end_point)
{
	int start_row = start_point / BOARD_SIZE;
	int start_col = start_point % BOARD_SIZE;
	int end_row = end_point / BOARD_SIZE;
	int end_col = end_point % BOARD_SIZE;

	this->board[start_row][start_col] = 0;

	// there has another piece on the target sqaure
	if(this->board[end_row][end_col] > 0)
	{
		if(this->board[end_row][end_col] <= PIECE_NUM)
		{
			this->blue_exist[this->board[end_row][end_col] - 1] = 0;
			this->blue_piece_num--;
		}
		else
		{
			this->red_exist[this->board[end_row][end_col] - 7] = 0;
			this->red_piece_num--;			
		}
	}
	this->board[end_row][end_col] = piece;
}
