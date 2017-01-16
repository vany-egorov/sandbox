#include "./enc.h"


int enc_new(ENC **out, ENCParam *param) {
	int ret = 0;

	ENC *it = NULL;

	it = (ENC*)calloc(1, sizeof(ENC));
	if (!it) { ret = 1; goto cleanup; }
	*out = it;

	it->thread = 0;

	it->chan_i = NULL;
	it->chan_o = NULL;

	it->h265_encoder = NULL;

	if (chan_new(&it->chan_i, param->cap_chan_i, sizeof(MsgI))) {
		ret = 1; goto cleanup;
	}
	if (chan_new(&it->chan_o, param->cap_chan_o, sizeof(MsgO))) {
		ret = 1; goto cleanup;
	}
	it->param = *param;
	profile_init(&it->param.profile);

cleanup:
	return ret;
}

static int enc_open(ENC *it) {
	int ret = 0;

	x265_param *h265_param = NULL;

	if (!(h265_param = x265_param_alloc())) {
		ret = 1; goto cleanup;
	}

	if (ret = x265_param_default_preset(h265_param, it->param.profile.preset, it->param.profile.tune)) {
		ret = 1; goto cleanup;
	} else {
		h265_param->internalCsp = X265_CSP_I420;  /* color-space */
		h265_param->fpsNum = 25;
		h265_param->fpsDenom = 1;
		h265_param->sourceWidth = (int)it->param.profile.w;
		h265_param->sourceHeight = (int)it->param.profile.h;
		h265_param->bframes = 3;
		h265_param->bRepeatHeaders = 1;
		h265_param->bAnnexB = 1;
		// h265_param->analysisMode = X265_ANALYSIS_LOAD;
		// h265_param->analysisFileName = "/tmp/analysis-x265.bin";
		h265_param->rc.bStrictCbr = 1;
		h265_param->rc.bitrate = (int)it->param.profile.b;
		h265_param->rc.vbvBufferSize = 1.5 * h265_param->rc.bitrate;
		h265_param->rc.vbvMaxBitrate = 1.5 * h265_param->rc.bitrate;
	}

	if (!(it->h265_encoder = x265_encoder_open(h265_param))) {
		ret = 1; goto cleanup;
	}

	if (!(it->h265_pic_in = x265_picture_alloc())) {
		ret = 1; goto cleanup;
	} else {
		x265_picture_init(h265_param, it->h265_pic_in);

		it->h265_pic_in->colorSpace = X265_CSP_I420;

		it->h265_pic_in->stride[0] = (int)it->param.profile.w;
		it->h265_pic_in->stride[1] = it->h265_pic_in->stride[0] >> x265_cli_csps[it->h265_pic_in->colorSpace].width[1];
		it->h265_pic_in->stride[2] = it->h265_pic_in->stride[0] >> x265_cli_csps[it->h265_pic_in->colorSpace].width[2];
	}

	if (!(it->h265_pic_out = x265_picture_alloc())) {
		ret = 1; goto cleanup;
	}

cleanup:
	if (h265_param) {
		x265_param_free(h265_param);
		h265_param = NULL;
	}
	return ret;
}

static void* enc_worker(void *args) {
	int ret = 0;
	ENC *it = NULL;
	it = (ENC*)args;
	MsgI msg_i = { 0 };

	int       h265_ret = 0;
	x265_nal *h265_nals = NULL;
	uint32_t  h265_nal_count = 0;

	char path_out[255] = { 0 };
	snprintf(path_out, sizeof(path_out)-1, "/tmp/egorov/%zdx%zd@%zd.x265",
		it->param.profile.w, it->param.profile.h, it->param.profile.b);
	FILE *out = fopen(path_out, "wb");
	int i = 0;

	enc_open(it);

	int plane_1_offset = it->h265_pic_in->stride[0] * (int)it->param.profile.h;
	int plane_2_offset = it->h265_pic_in->stride[1] * ((int)it->param.profile.h >> x265_cli_csps[it->h265_pic_in->colorSpace].height[1]);

	for (;;) {
		chan_wait(it->chan_i);
		chan_recv(it->chan_i, &msg_i);  /* TODO: read multiple messages at once */

		it->h265_pic_in->planes[0] = (char*)msg_i.yuv;
		it->h265_pic_in->planes[1] = (char*)msg_i.yuv + plane_1_offset;
		it->h265_pic_in->planes[2] = it->h265_pic_in->planes[1] + plane_2_offset;

		h265_ret = x265_encoder_encode(it->h265_encoder, &h265_nals, &h265_nal_count, it->h265_pic_in, it->h265_pic_out);
		if (h265_ret == 1) {
			for (i = 0; i < h265_nal_count; i++) {
				x265_nal nal = h265_nals[i];
				fprintf(stderr, "[+] %5d %5d %5d %5d\n", h265_nal_count, nal.sizeBytes,
					it->h265_pic_out->sliceType, it->h265_pic_out->poc);
				fwrite(nal.payload, nal.sizeBytes, 1, out);
			}
		}
	}
}

int enc_go(ENC *it) {
	int ret = 0;

	if (pthread_create(&it->thread, NULL, enc_worker, (void*)it)) {
		ret = 1; goto cleanup;
	}

cleanup:
	return ret;
}

int enc_del(ENC **out) {
	int ret = 0;
	ENC *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	chan_del(&it->chan_i);
	chan_del(&it->chan_o);

	free(it);

	*out = NULL;

	return ret;
}
