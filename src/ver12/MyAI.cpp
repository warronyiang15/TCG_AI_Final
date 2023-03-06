#include "float.h"
#include "MyAI.h"
#include "hashtable.h"

//node hashtable[2][HASH_SIZE][BUCKET_SIZE];
node hashtable[2][HASH_SIZE];
uint64_t debug_total = 0;
uint64_t debug_hit = 0;
uint64_t debug_collision = 0;
uint64_t debug_search = 0;
uint64_t debug_cut = 0;
uint64_t debug_search_chance = 0;
uint64_t debug_search_cut = 0;

/*
const double INF   = 393216;
const double v_max = 393216;
const double v_min = -393216;
const double red_eval[BOARD_SIZE][BOARD_SIZE] = {
	{ 1,  2,  2,  2,  2},
	{ 2,  4,  6,  8, 16},
	{ 2,  6, 32, 32, 64},
	{ 2,  8, 32, 256, 1024},
	{ 2, 16, 64, 1024, 65536}
};

const double blue_eval[BOARD_SIZE][BOARD_SIZE] = {
	{65536, 1024, 64, 16, 2},
	{ 1024,  256, 32, 8, 2},
	{   64,   32, 32, 6, 2},
	{   16,    8,  6, 4, 2},
	{2, 2, 2, 2, 1}
};
*/

const double INF   = 3072;
const double v_max = 3072;
const double v_min = -3072;
const double red_eval[BOARD_SIZE][BOARD_SIZE] = {
	{ 1,  2,  2,  2 ,   2},
	{ 2,  4,  6,  8 ,  16},
	{ 2,  6, 32, 32 ,  64},
	{ 2,  8, 32, 128, 256},
	{ 2, 16, 64, 256, 512}
};

const double blue_eval[BOARD_SIZE][BOARD_SIZE] = {
	{512, 256, 64, 16, 2},
	{ 256,  128, 32, 8, 2},
	{   64,   32, 32, 6, 2},
	{   16,    8,  6, 4, 2},
	{2, 2, 2, 2, 1}
};

bool isBluePieces(const int piece){
	if( piece >= 1 && piece <= 6 ) return true;
	else return false;
}

bool isRedPieces(const int piece ){
	if( piece >= 7 && piece <= 12 ) return true;
	else return false;
}

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

// TODO
void MyAI::Generate_move(char* move)
{
	this->setup_state();
	const int limit = 7;
	const int threshold = 10000;
	const int my_color = this->color;
	std::pair<double, int> res = this->F4( -INF, INF, 0, limit, my_color);

	/* Move */
	int result[100];
	int move_count = this->get_legal_move(result);
	int piece = result[res.second * 3];
	int start_point = result[res.second * 3 + 1];
	int end_point = result[res.second * 3 + 2];
	sprintf(move, "%c%c%c%c", 'A' + start_point % BOARD_SIZE, '1' + start_point / BOARD_SIZE, 'A' + end_point % BOARD_SIZE, '1' + end_point / BOARD_SIZE);
	this->Take_move(piece, start_point, end_point);

	fprintf(stderr, "Evaluation move = %lf\n", res.first);
	//! Debug
	if( this->isEnd() ){
		fprintf(stderr, "Total lookup = %llu\n", debug_total);
		fprintf(stderr, "Total hit = %llu\n", debug_hit);
		fprintf(stderr, "Total collision = %llu\n", debug_collision);
		fprintf(stderr, "Total search = %llu\n", debug_search);
		fprintf(stderr, "Total cut = %llu\n", debug_cut);
		fprintf(stderr, "Total search chance = %llu\n", debug_search_chance);
		fprintf(stderr, "Total search cut = %llu\n", debug_search_cut);
	}

	// print the result
	fprintf(stderr, "============================\nMy result:\n");
	if(piece <= PIECE_NUM) fprintf(stderr, "Blue piece %d: (%c%c) -> (%c%c)\n", piece, move[0], move[1], move[2], move[3]);
	else fprintf(stderr, "Red piece %d: (%c%c) -> (%c%c)\n", piece - PIECE_NUM, move[0], move[1], move[2], move[3]);
	this->Print_chessboard();
	fprintf(stderr, "============================\n");
}

// get all legal moves
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

bool MyAI::isEnd(){
	if( this->red_piece_num <= 0 || this->blue_piece_num <= 0 ) return true;
	if( isBluePieces(this->board[0][0]) ) return true;
	if( isRedPieces(this->board[4][4]) ) return true;
	return false;
}

double MyAI::eval(const int my_color){
	// 1. One of the side loses all the pieces, lose the game
	if( this->red_piece_num <= 0 ) {
		if( my_color == BLUE ) return v_max;
		else return v_min;
	}
	if( this->blue_piece_num <= 0 ){
		if( my_color == RED ) return v_max;
		else return v_min;
	}

	// 2. One of the side reach the end
	if( isBluePieces(this->board[0][0]) ){
		if( my_color == BLUE ) return v_max;
		else return v_min;
	}
	if( isRedPieces(this->board[4][4]) ){
		if( my_color == RED ) return v_max;
		else return v_min;
	}

	
	// Evaluate the board
	double red_exp[6], blue_exp[6], red_threat[6], blue_threat[6];
	for(int i = 0;i < 6;i++){
		red_exp[i] = blue_exp[i] = -INF;
		red_threat[i] = blue_threat[i] = 0;
	}

	for(int cube_index = 0; cube_index < 6; cube_index++){
		if( this->red_exist[cube_index] ){
			assert(this->red_cube[cube_index] != -1);
			int position = this->red_cube[cube_index];
			int i = position / BOARD_SIZE;
			int j = position % BOARD_SIZE;
			red_exp[cube_index] = red_eval[i][j];
			if( i + 1 < BOARD_SIZE && isBluePieces(this->board[i + 1][j]) ){
				red_threat[cube_index] = std::max(red_threat[cube_index], blue_eval[i + 1][j]);
			}
			if( j + 1 < BOARD_SIZE && isBluePieces(this->board[i][j + 1]) ){
				red_threat[cube_index] = std::max(red_threat[cube_index], blue_eval[i][j + 1]);
			}
			if( i + 1 < BOARD_SIZE && j + 1 < BOARD_SIZE && isBluePieces(this->board[i + 1][j + 1]) ){
				red_threat[cube_index] = std::max(red_threat[cube_index], blue_eval[i + 1][j + 1]);
			}
		}
		if( this->blue_exist[cube_index] ){
			assert(this->blue_cube[cube_index] != -1);
			int position = this->blue_cube[cube_index];
			int i = position / BOARD_SIZE;
			int j = position % BOARD_SIZE;
			blue_exp[cube_index] = blue_eval[i][j];
			if( i - 1 >= 0 && isRedPieces(this->board[i - 1][j]) ){
				blue_threat[cube_index] = std::max(blue_threat[cube_index], red_eval[i - 1][j] );
			}
			if( j - 1 >= 0 && isRedPieces(this->board[i][j - 1]) ){
				blue_threat[cube_index] = std::max(blue_threat[cube_index], red_eval[i][j - 1]);
			}
			if( i - 1 >= 0 && j - 1 >= 0 && isRedPieces(this->board[i - 1][j - 1]) ){
				blue_threat[cube_index] = std::max(blue_threat[cube_index], red_eval[i - 1][j - 1]);
			}
		}
	}

	double red_total_exp = 0;
	double blue_total_exp = 0;
	double red_total_threat = 0;
	double blue_total_threat = 0;

	for(int cube_index = 0; cube_index < 6; cube_index++){
		if( !this->red_exist[cube_index] ){
			assert(this->red_cube[cube_index] == -1);
			// Find the nearest pieces
			for(int l = cube_index; l >= 0;l--){
				if( this->red_exist[l] ){
					red_exp[cube_index] = red_exp[l];
					red_threat[cube_index] = std::max(red_threat[cube_index], red_threat[l]);
					break;
				}
			}
			for(int r = cube_index; r < PIECE_NUM; r++){
				if( this->red_exist[r] ){
					red_exp[cube_index] = std::max(red_exp[cube_index], red_exp[r]);
					red_threat[cube_index] = std::max(red_threat[cube_index], red_threat[r]);
					break;
				}
			}
		}

		if( !this->blue_exist[cube_index] ){
			assert( this->blue_cube[cube_index] == -1 );
			// find the nearest pieces
			for(int l = cube_index; l >= 0; l--){
				if( this->blue_exist[l] ){
					blue_exp[cube_index] = blue_exp[l];
					blue_threat[cube_index] = std::max(blue_threat[cube_index], blue_threat[l]);
					break;
				}
			}
			for(int r = cube_index; r < PIECE_NUM; r++){
				if( this->blue_exist[r] ){
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
		assert( red_exp[cube_index] >= 0 );
		assert( blue_exp[cube_index] >= 0 );
	}

	if( my_color == RED ){
		return (red_total_exp - blue_total_exp) + ((red_total_threat - blue_total_threat) * 0.5);
	}
	else{
		return (blue_total_exp - red_total_exp) + ((blue_total_threat - red_total_threat) * 0.5);
	}
	
}

int MyAI::Take_move(const int piece, const int start_point, const int end_point ){
	int start_row = start_point / BOARD_SIZE;
	int start_col = start_point % BOARD_SIZE;
	int end_row = end_point / BOARD_SIZE;
	int end_col = end_point % BOARD_SIZE;

	int enemy_piece = -1;

	// 1. Take the pieces away 
	this->board[start_row][start_col] = 0;
	if( isBluePieces( piece ) )
		this->key1 ^= pr_cube[BLUE][ piece - 1 ][ start_point ];
	else 
		this->key2 ^= pr_cube[RED][ piece - 7 ][ start_point ];
	
	// 2. If there has another pieces on the target square
	if( this->board[end_row][end_col] > 0 ){
		enemy_piece = this->board[end_row][end_col];

		if(this->board[end_row][end_col] <= PIECE_NUM)
		{
			this->blue_exist[this->board[end_row][end_col] - 1] = 0;
			this->blue_cube[this->board[end_row][end_col] - 1 ] = -1;
			this->blue_piece_num--;
			this->key1 ^= pr_cube[BLUE][this->board[end_row][end_col] - 1][end_point];
		}
		else
		{
			this->red_exist[this->board[end_row][end_col] - 7] = 0;
			this->red_cube[this->board[end_row][end_col] - 7] = -1;
			this->red_piece_num--;	
			this->key2 ^= pr_cube[RED][this->board[end_row][end_col] - 7][end_point];		
		}
	}

	// 3. Put the piece to end_point
	this->board[end_row][end_col] = piece;
	if( isBluePieces(piece) )
		this->key1 ^= pr_cube[BLUE][piece - 1][ end_point ];
	else 
		this->key2 ^= pr_cube[RED][piece - 7][end_point];
	
	return enemy_piece;
}

void MyAI::Undo_move(const int piece, const int start_point, const int end_point, int enemy_piece){
	int start_row = start_point / BOARD_SIZE;
	int start_col = start_point % BOARD_SIZE;
	int end_row = end_point / BOARD_SIZE;
	int end_col = end_point % BOARD_SIZE;

	// 1. Take the piece on end_point
	this->board[end_row][end_col] = 0;
	if( isBluePieces(piece) )
		this->key1 ^= pr_cube[BLUE][piece - 1][ end_point ];
	else 
		this->key2 ^= pr_cube[RED][piece - 7][end_point];
	
	// 2. If there has another pieces on the target square originally
	if( enemy_piece != -1 ){
		this->board[end_row][end_col] = enemy_piece;
		
		if(this->board[end_row][end_col] <= PIECE_NUM)
		{
			this->blue_exist[this->board[end_row][end_col] - 1] = true;
			this->blue_cube[this->board[end_row][end_col] - 1 ] = end_point;
			this->blue_piece_num++;
			this->key1 ^= pr_cube[BLUE][this->board[end_row][end_col] - 1][end_point];
		}
		else
		{
			this->red_exist[this->board[end_row][end_col] - 7] = true;
			this->red_cube[this->board[end_row][end_col] - 7] = end_point;
			this->red_piece_num++;	
			this->key2 ^= pr_cube[RED][this->board[end_row][end_col] - 7][end_point];		
		}
	}

	// 3. Put the piece back to start_point
	this->board[start_row][start_col] = piece;
	if( isBluePieces( piece ) )
		this->key1 ^= pr_cube[BLUE][ piece - 1 ][ start_point ];
	else 
		this->key2 ^= pr_cube[RED][ piece - 7 ][ start_point ];

}

void MyAI::setup_state(){
	this->key1 = 0;
	this->key2 = 0;
	this->blue_piece_num = 0;
	this->red_piece_num = 0;

	for(int i = 0;i < PIECE_NUM;i++){
		this->red_cube[i] = -1;
		this->blue_cube[i] = -1;
		this->blue_exist[i] = false;
		this->red_exist[i] = false;
	}
	
	for(int i = 0;i < BOARD_SIZE;i++){
		for(int j = 0;j < BOARD_SIZE;j++){
			if( this->board[i][j] == 0 ) continue;
			int position = i * BOARD_SIZE + j;
			if( isBluePieces(this->board[i][j]) ){
				this->blue_exist[this->board[i][j] - 1] = true;
				this->blue_cube[this->board[i][j] - 1] = position;
				this->blue_piece_num++;
				this->key1 ^= pr_cube[BLUE][this->board[i][j] - 1][position];
			}
			else{
				assert( isRedPieces(this->board[i][j]) );
				this->red_exist[this->board[i][j] - 7] = true;
				this->red_cube[this->board[i][j] - 7] = position;
				this->red_piece_num++;
				this->key2 ^= pr_cube[RED][this->board[i][j] - 7][position];
			}
		}
	}
	return;
}

std::pair<double, int> MyAI::F4( double alpha, double beta, int depth, const int limit_depth, const int my_color ){
	
	// Look up table here
	const uint64_t fixed_key1 = this->key1;
	const uint64_t fixed_key2 = this->key2;
	const int fixed_dice = this->dice;
	const int fixed_color = this->color;
	const int idx = lookup_entry(fixed_key1, fixed_key2, fixed_dice, fixed_color);
	if( idx != -1 && hashtable[fixed_color][idx].depth >= depth ){
		if( hashtable[fixed_color][idx].flag == EXACT ){
			return hashtable[fixed_color][idx].value;
		}
		else if( hashtable[fixed_color][idx].flag == LOWER ){
			alpha = std::max( alpha, hashtable[fixed_color][idx].value.first );
		}
		else if( hashtable[fixed_color][idx].flag == UPPER ){
			beta = std::min( beta, hashtable[fixed_color][idx].value.first );
		}

		if( alpha >= beta ) {
			debug_cut++;
			return hashtable[fixed_color][idx].value;
		}
	}
	


	int result[100];
	// determine the successor positions
	int move_count = this->get_legal_move(result);
	if( move_count == 0 || depth >= limit_depth || this->isEnd() )
		return std::pair<double, int>(this->eval(my_color), -1);

	double m = -INF;
	double t = -INF;

	debug_search++;

	// Move
	//int hist_color = this->color;
	//int hist_dice = this->dice;
	int enemy_piece = this->Take_move(result[0], result[1], result[2]);
	this->color = this->color == RED ? BLUE : RED;

	m = std::max(m, this->Star1_F4( alpha, beta, depth + 1, limit_depth, my_color).first );

	// Undo
	this->color = fixed_color;
	this->dice = fixed_dice;
	this->Undo_move( result[0], result[1], result[2], enemy_piece );

	std::pair<double, int>res(m, 0);
	// Beta cut off
	if( m >= beta ) {
		debug_cut++;
		update_entry(idx, fixed_key1, fixed_key2, fixed_dice, depth, LOWER, res, fixed_color );
		return res;
	}

	for(int i = 1 ; i < move_count; i++){

		// Move
		enemy_piece = this->Take_move(result[i * 3], result[i * 3 + 1], result[i * 3 + 2]);
		this->color = this->color == RED ? BLUE : RED;

		t = this->Star1_F4( m, m + 1, depth + 1, limit_depth, my_color).first;

		// Undo
		this->color = fixed_color;
		this->dice = fixed_dice;
		this->Undo_move( result[i * 3], result[i * 3 + 1], result[i * 3 + 2], enemy_piece);

		if( t > m ){
			if( limit_depth - depth < 3 || t >= beta ){
				m = t;
				res.first = m;
				res.second = i;
			}
			else{
				// Move
				enemy_piece = this->Take_move(result[i * 3], result[i * 3 + 1], result[i * 3 + 2]);
				this->color = this->color == RED ? BLUE : RED;

				m = this->Star1_F4( t, beta, depth + 1, limit_depth, my_color ).first;

				// Undo
				this->color = fixed_color;
				this->dice = fixed_dice;
				this->Undo_move( result[i * 3], result[i * 3 + 1], result[i * 3 + 2], enemy_piece);
				
				res.first = m;
				res.second = i;
			}
		}

		if( m >= v_max || m >= beta ) { 
			debug_cut++;
			update_entry( idx, fixed_key1, fixed_key2, fixed_dice, depth, LOWER, res, fixed_color );
			return res;
		}
	}
	
	if( m > beta )
		update_entry( idx, fixed_key1, fixed_key2, fixed_dice, depth, EXACT, res, fixed_color );
	else 
		update_entry( idx, fixed_key1, fixed_key2, fixed_dice, depth, LOWER, res, fixed_color );
	return res;
}

std::pair<double, int> MyAI::G4( double alpha, double beta, int depth, const int limit_depth, const int my_color ){

	// Look up table here
	const uint64_t fixed_key1 = this->key1;
	const uint64_t fixed_key2 = this->key2;
	const int fixed_dice = this->dice;
	const int fixed_color = this->color;
	const int idx = lookup_entry( fixed_key1, fixed_key2, fixed_dice, fixed_color );
	
	if( idx != -1 && hashtable[fixed_color][idx].depth >= depth ){
		if( hashtable[fixed_color][idx].flag == EXACT )
			return hashtable[fixed_color][idx].value;
		else if( hashtable[fixed_color][idx].flag == LOWER )
			alpha = std::max( alpha, hashtable[fixed_color][idx].value.first );
		else if( hashtable[fixed_color][idx].flag == UPPER )
			beta = std::min( beta, hashtable[fixed_color][idx].value.first );
		
		if( alpha >= beta ){
			debug_cut++;
			return hashtable[fixed_color][idx].value;
		}
	}

	int result[100];
	// determine the successor positions
	int move_count = this->get_legal_move(result);
	if( move_count == 0 || depth >= limit_depth || this->isEnd() ){
		return std::pair<double, int>(this->eval(my_color), -1);
	}
	
	double m = INF;
	double t = INF;
	debug_search++;

	// Move
	int enemy_piece = this->Take_move(result[0], result[1], result[2]);
	this->color = this->color == RED ? BLUE : RED;
	
	m = std::min(m, this->Star1_G4( alpha, beta, depth + 1, limit_depth, my_color).first );

	// Undo
	this->color = fixed_color;
	this->dice = fixed_dice;
	this->Undo_move(result[0], result[1], result[2], enemy_piece);

	std::pair<double, int>res(m, 0);
	// alpha cut off
	if( m <= alpha ){
		debug_cut++;
		update_entry( idx, fixed_key1, fixed_key2, fixed_dice, depth, UPPER, res, fixed_color );
		return res;
	}

	for(int i = 1;i < move_count;i++){
		// Move
		enemy_piece = this->Take_move(result[i * 3], result[i * 3 + 1], result[i * 3 + 2]);
		this->color = this->color == RED ? BLUE : RED;

		t = this->Star1_G4(m - 1, m, depth + 1, limit_depth, my_color).first;

		// Undo
		this->color = fixed_color;
		this->dice = fixed_dice;
		this->Undo_move( result[i * 3], result[i * 3 + 1], result[i * 3 + 2], enemy_piece);

		if( t < m ){
			if( limit_depth - depth < 3 || t <= alpha ){
				m = t;
				res.first = m;
				res.second = i;
			}
			else{
				// Move
				enemy_piece = this->Take_move(result[i * 3], result[i * 3 + 1], result[i * 3 + 2]);
				this->color = this->color == RED ? BLUE : RED;

				m = this->Star1_G4( alpha, t, depth + 1, limit_depth, my_color).first;
				
				// Undo
				this->color = fixed_color;
				this->dice = fixed_dice;
				this->Undo_move( result[i * 3], result[i * 3 + 1], result[i * 3 + 2], enemy_piece);

				res.first = m;
				res.second = i;
			}
		}
		if( m <= v_min || m <= alpha ) { 
			debug_cut++;
			update_entry( idx, fixed_key1, fixed_key2, fixed_dice, depth, UPPER, res, fixed_color );
			return res;
		}
	}

	if( m < alpha ) {
		update_entry( idx, fixed_key1, fixed_key2, fixed_dice, depth, EXACT , res, fixed_color );
	}
	else{
		update_entry( idx, fixed_key1, fixed_key2, fixed_dice, depth, UPPER , res, fixed_color );
	}
	return res;
}

std::pair<double, int> MyAI::Star1_F4( double alpha, double beta, int depth, const int limit_depth, const int my_color ){
	debug_search_chance++;
	double vsum = 0;
	const double c = 6;
	double A = c * (alpha - v_max) + v_max;
	double B = c * (beta - v_min) + v_min;
	double m = v_min;
	double M = v_max;
	int hist_dice = this->dice;
	for(int dice = 1; dice <= 6; dice++){
		this->dice = dice;
		double t = this->G4( std::max(A, v_min), std::min(B, v_max), depth, limit_depth, my_color).first;
		this->dice = hist_dice;

		m = m + ((t - v_min) / c);
		M = M + ((t - v_max) / c);
		if( t >= B ) { 
			debug_search_cut++;
			return std::pair<double, int>(m, -1);
		}
		if( t <= A ) { 
			debug_search_cut++;
			return std::pair<double, int>(M, -1);
		}
		vsum += t;
		A = A + v_max - t;
		B = B + v_min - t;
	}
	return std::pair<double, int>( vsum / c, -1 );
}

std::pair<double, int> MyAI:: Star1_G4( double alpha, double beta, int depth, const int limit_depth, const int my_color ){
	debug_search_chance++;
	double vsum = 0;
	const double c = 6;
	double A = c * ( alpha - v_max ) + v_max;
	double B = c * ( beta - v_min ) + v_min;
	double m = v_min;
	double M = v_max;
	int hist_dice = this->dice;

	for(int dice = 1; dice <= 6; dice++){

		this->dice = dice;
		double t = this->F4( std::max(A, v_min), std::min(B, v_max), depth, limit_depth, my_color ).first;
		this->dice = hist_dice;

		m = m + ((t - v_min) / c);
		M = M + ((t - v_max) / c);
		if( t >= B ) { 
			debug_search_cut++;
			return std::pair<double, int>(m, -1);
		}
		if( t <= A ) {
			debug_search_cut++;
			return std::pair<double, int>(M, - 1);
		} 
		vsum += t;
		A = A + v_max - t;
		B = B + v_min - t;
	}
	return std::pair<double, int>( vsum / c, - 1);
}
