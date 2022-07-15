#include "mmul_.hpp"
#include "hls_stream.h"

extern "C" {
	void mmul(hls::stream<axis_t> &in0, hls::stream<axis_t> &in1, hls::stream<axis_t> &out) {
	#pragma HLS INTERFACE s_axilite port = return bundle = control
	#pragma HLS INTERFACE axis port = in0
	#pragma HLS INTERFACE axis port = in1
	#pragma HLS INTERFACE axis port = out

		DataType l_A[N2];
		DataType l_B[N2];
		DataType l_C[N2];

	#pragma HLS ARRAY_PARTITION variable = l_A factor = 16 dim = 1 cyclic
	#pragma HLS ARRAY_PARTITION variable = l_B factor = 16 dim = 1 block
	#pragma HLS ARRAY_PARTITION variable = l_C factor = 16 dim = 1 cyclic

		int j_limit = 512 / DataTypeSize;
		int i_limit = N2 / j_limit;
		converter_t converter;

	load_A:
		for (int i = 0; i < i_limit; i++) {
			axis_t temp = in0.read();
			for (int j = 0; j < j_limit; j++) {
				int high = j * DataTypeSize + DataTypeSize - 1;
				int low = j * DataTypeSize;
				int index = i * 16 + j;

				converter.i = temp.data.range(high, low);
				l_A[index] = converter.d;
			}
		}

	load_B:
		for (int i = 0; i < i_limit; i++) {
			axis_t temp = in1.read();
			for (int j = 0; j < j_limit; j++) {
				int high = j * DataTypeSize + DataTypeSize - 1;
				int low = j * DataTypeSize;
				int index = i * 16 + j;

				converter.i = temp.data.range(high, low);
				l_B[index] = converter.d;
			}
		}

	MMUL1:
		for (int m = 0; m < N; ++m) {
			MMULL2:
				for (int n = 0; n < N; ++n) {
				#pragma HLS PIPELINE II = 1
					DataType sum = 0;
					MMULL3:
						for (int k = 0; k < N; ++k){
							sum += l_A[m * N + k] * l_B[k * N + n];
							l_C[m * N + n] = sum;
						}
				}
		}

	write_C:
		for (int i = 0; i < i_limit; i++) {
			axis_t temp;
			for (int j = 0; j < j_limit; j++) {
				int high = j * DataTypeSize + DataTypeSize - 1;
				int low = j * DataTypeSize;
				converter.d = l_C[i * 16 + j];
				temp.data.range(high, low) = converter.i;
			}
			ap_uint<1> last = 0;
			if (i == i_limit - 1) {
				last = 1;
			}
			temp.last = last;
			temp.keep = -1;
			out.write(temp);
		}
	}
}
