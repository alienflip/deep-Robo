#include "ap_axi_sdata.h"
#include "hls_stream.h"

typedef ap_axiu<32, 1, 1, 1> pkt;

void ssmul(hls::stream< pkt > &INPUT0, hls::stream< pkt > &INPUT1, hls::stream< pkt > &OUTPUT, ap_int<32> constant0, ap_int<32> constant1)
{
#pragma HLS interface s_axilite port=constant0
#pragma HLS interface s_axilite port=constant1
#pragma HLS INTERFACE axis port=INPUT0
#pragma HLS INTERFACE axis port=INPUT1
#pragma HLS INTERFACE axis port=OUTPUT

	pkt data_p;
	pkt data_q;
	pkt data_r;

	while(1){
		INPUT0.read(data_p);
		INPUT1.read(data_q);
        
        data_r.dest = data_p.dest;
        data_r.id = data_p.id;
        data_r.keep = data_p.keep;
        data_r.last = data_p.last;
        data_r.strb = data_p.strb;
        data_r.user = data_p.user;

		data_r.data = data_p.data + data_q.data;
		OUTPUT.write(data_r);

		if(data_r.last){
			break;
		}
	}
}
