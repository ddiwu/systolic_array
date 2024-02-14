#include "systolic_array.h"

template<typename T>
struct Process_Element
{
	T a, b, val;

	Process_Element()
	{
		reset();
	}

	void reset()
	{
		a = 0;
		b = 0;
		val = 0;
	}

	void process(T a_i, T b_i)
	{
		a = a_i;			// a left to right
		b = b_i;			// b top to bottom
		val += a_i*b_i;     // mul and add, most essecial
	}
};

template<typename T, int LEN>
struct Systolic_Array
{
	Process_Element<T> pe[LEN][LEN];

	void reset()
	{
		for (int r = 0; r < LEN; r++)
			for (int c = 0; c < LEN; c++)
				pe[r][c].reset();
	}

	void reset(int row, int col)
	{
		for (int r = 0; r < row; r++)
			for (int c = 0; c < col; c++)
				pe[r][c].reset();
	}

	void pulse(T a_vec[LEN], T b_vec[LEN])
	{
		systolic_array_outer_loop:
		for (int i = 2*LEN - 2; i >= 0; i--)
		{
#pragma HLS UNROLL
			// PE number
			int pe_num = (i > LEN - 1)?(2 * LEN - 1 - i):(i + 1);

			systolic_array_inner_loop:
			for (int j = 0; j < pe_num; j++)
			{
				// position
				int pos_y = (i >= LEN)?LEN-1-j:i-j;
				int pos_x = (i >= LEN)?i-LEN+j+1:j;
				// the data flows from the left and top
				T a_get = (pos_y == 0)?a_vec[pos_x]:pe[pos_x][pos_y-1].a;
				T b_get = (pos_x == 0)?b_vec[pos_y]:pe[pos_x-1][pos_y].b;
				pe[pos_x][pos_y].process(a_get, b_get);
			}
		}
	}

// 	void pulse(T a_vec[LEN], T b_vec[LEN])
// 	{
// 		#pragma HLS UNROLL
// 		for (int i = 0; i < LEN; i++)
// 		{
// 			#pragma HLS UNROLL
// 			for (int j = 0; j < LEN; j++)
// 			{
// 				int pos_y = (i >= LEN)?LEN-1-j:i-j;
// 				int pos_x = (i >= LEN)?i-LEN+j+1:j;
// 				// the data flows from the left and top
// 				T a_get = (pos_y == 0)?a_vec[pos_x]:pe[pos_x][pos_y-1].a;
// 				T b_get = (pos_x == 0)?b_vec[pos_y]:pe[pos_x-1][pos_y].b;
// 				pe[pos_x][pos_y].process(a_get, b_get);
// 			}
// 		}
// 	}
// };

Systolic_Array<DataType, SIDE_LEN> systolic_matrix;
void pe_cal(int piece_a_cell, int piece_b_cell, int row, int col1, int col, int ori_col1, DataType din_a[], DataType din_b[])
{
	systolic_matrix.reset(row, col);

	// the edge number which will be put into PE next
	DataType a_vec[SIDE_LEN], b_vec[SIDE_LEN];

	// total 256+4+4-2 times to finish a block cauculation
	int total_pulse = col1 + row + col -2;

	pe_outer_loop:
	for (int i = 0; i < total_pulse; i++)
	{
		pe_inner_loop:
		for (int j = 0; j < row; j++)
		{
			int a_index = piece_a_cell*col1*SIDE_LEN + j*col1 + i - j;
			int b_index = (i - j)*ori_col1 + j + piece_b_cell*SIDE_LEN;

			a_vec[j] = (i >= j && i < j+col1 && j < row) ? din_a[a_index] : (ap_int<16>)0;
			b_vec[j] = (i >= j && i < j+col1 && j < col) ? din_b[b_index] : (ap_int<16>)0;
		}
		systolic_matrix.pulse(a_vec, b_vec);
	}
}

void output(int piece_a_cell, int piece_b_cell, int row, int col1, int ori_col1, DataType res[])
{
	for(int i = 0; i < row; i++)
	{
#pragma HLS UNROLL
		for(int j = 0; j < col1; j++)
			res[piece_a_cell*SIDE_LEN*ori_col1+i*ori_col1+(piece_b_cell*SIDE_LEN+j)]= systolic_matrix.pe[i][j].val;
	}
}

void systolic_array(DataType din_a[], DataType din_b[], DataType res[])
{
	// important to regulate the interface
#pragma HLS INTERFACE s_axilite port=return//reg
#pragma HLS INTERFACE m_axi depth=65536 port=din_a offset=slave//ram
#pragma HLS INTERFACE m_axi depth=65536 port=din_b offset=slave
#pragma HLS INTERFACE m_axi depth=65536 port=res offset=slave
	
	int piece_a_row, piece_b_col;

#pragma HLS DATAFLOW//make tasks pipeline

	// matrix block
	s_outer_loop:
	for (int i = 0; i < 256/SIDE_LEN; i++)
	{
		//block row number
		piece_a_row = 4;
		s_inner_loop:
		for (int j = 0; j < 256/SIDE_LEN; j++)
		{
			//block col number
			piece_b_col = 4;
			pe_cal(i, j, 4, 256, 4, 256, din_a, din_b);
			output(i, j, 4, 4, 256, res);
		}
	}
}