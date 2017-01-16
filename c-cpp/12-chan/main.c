#include <stdio.h>
#include <errno.h>     /* errno */
#include <stdint.h>
#include <stdlib.h>    /* malloc */
#include <string.h>    /* memcpy */
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */
#include <signal.h>    /* SIGINT, SIGTERM */
#include <semaphore.h> /* sem_t, sem_init, sem_wait, sem_post, sem_destroy */

#include <x265.h>

#include "./buf.h"
#include "./bufs.h"
#include "./enc.h"
#include "./encs.h"
#include "./profile.h"


// presets:
//   ultrafast
//   superfast
//   veryfast
//   faster
//   fast
//   medium
//   slow
//   slower
//   veryslow
//   placebo
#define DEFAULT_PRESET "placebo"

// tune:
//   psnr
//   ssim
//   grain
//   fastdecode
//   zerolatency
#define DEFAULT_TUNE "grain"


static const Profile profiles[] =
{
    { 1920, 1080, 204,  DEFAULT_PRESET, DEFAULT_TUNE, },
    { 1920, 1080, 504,  DEFAULT_PRESET, DEFAULT_TUNE, },
    { 1920, 1080, 1272, DEFAULT_PRESET, DEFAULT_TUNE, },
    { 1920, 1080, 2072, DEFAULT_PRESET, DEFAULT_TUNE, },
    { 0,    0,   0,    NULL,           NULL,         },
};


static sem_t sig_sem = { 0 };


static void signal_handler(int sig) {
	switch (sig) {
	case SIGINT:
	case SIGTERM:
		printf("SIGINT/SIGTERM => gracefully shutdown\n");
		sem_post(&sig_sem);
		break;
	default:
		fprintf(stderr, "caught wrong signal: %d\n", sig);
		return;
	}
}

static int signal_init(void) {
	struct sigaction signal_action;
	signal_action.sa_handler = &signal_handler;
	sem_init(&sig_sem, 0, 0);

	if (sigaction(SIGINT, &signal_action, NULL) == -1) {
		fprintf(stderr, "cannot handle SIGINT: \"%s\"", strerror(errno));
		return EX_OSERR;
	}
	if (sigaction(SIGTERM, &signal_action, NULL) == -1) {
		fprintf(stderr, "cannot handle SIGTERM: \"%s\"", strerror(errno));
		return EX_OSERR;
	}

	return 0;
}

static int signal_wait(void) {
	sem_wait(&sig_sem);
	return 0;
}

int main(int argc, char **argv) {
	int exit_status = EX_OK;
	const Profile *profile = NULL;
	ENCs *encs = NULL;
	ENC *enc = NULL;
	Bufs *bufs_yuv = NULL;
	Buf *buf_yuv = NULL;

	if (signal_init()) {
		exit_status = EX_SOFTWARE; goto cleanup;
	}

	encs_new(&encs);
	bufs_new(&bufs_yuv);

	for (profile=profiles; !profile_is_empty(profile); profile++) {
		ENCParam enc_param = {
			.profile = *profile,

			.cap_chan_i = 10,
			.cap_chan_o = 10,
		};

		if (enc_new(&enc, &enc_param)) {
			fprintf(stderr, "encoder allocation failed;\n");
			exit_status = EX_SOFTWARE; goto cleanup;
		}

		if (enc_go(enc)) {
			fprintf(stderr, "encoder initialization/open failed;\n");
			exit_status = EX_SOFTWARE; goto cleanup;
		}

		encs_push(encs, enc);

		printf("[OK] [+] encoder "); profile_print(&enc->param.profile);

		if (buf_new(&buf_yuv, 10, enc->param.profile.yuv_sz)) {
			fprintf(stderr, "YUV buffer allocation failed;\n");
			exit_status = EX_SOFTWARE; goto cleanup;
		}

		bufs_push(bufs_yuv, buf_yuv);
	}

	size_t i = 0;
	size_t yuv_size = 1920*1080 + (2*(1920*1080))/4;
	void *yuv_enc = NULL;
	void *yuv_in = malloc(yuv_size);
	for(;;) {
		/* Read input frame */
		if (fread(yuv_in, 1, yuv_size, stdin) != yuv_size) break;

		for (i = 0; i < encs->len; i++) {
			yuv_enc = NULL;
			enc = encs->els[i];
			buf_yuv = bufs_yuv->els[i];

			buf_get_available(buf_yuv, &yuv_enc);
			memcpy(yuv_enc, yuv_in, yuv_size);

			MsgI msg_i = {
				.yuv = yuv_enc,
			};

			chan_send(enc->chan_i, &msg_i);
		}

		usleep(40 * 1000); // 25 fps
	}

	signal_wait();

cleanup:
	return exit_status;

// 	// ~~~~~~~~~~~~~~~~~~~~~~
// 	int exit_status = 0,
// 	    ret = 0;
// 	int i = 0;
// 	int width = 0,
// 	    height = 0;
// 	size_t luma_size = 0,
// 	       chroma_size = 0,
// 	       yuv_size = 0;
// 	int input_frame_count = 0;

// 	x265_encoder *encoder = NULL;
// 	x265_param *param = NULL;
// 	x265_picture *pic_in = NULL,
// 	             *pic_out = NULL;
// 	x265_nal *out_nals = NULL;
// 	uint32_t  out_nal_count = 0;
// 	char *path_out = NULL;
// 	FILE *out = NULL;

// 	void *input_yuv = NULL;

// 	if (argc <= 2) {
// 		fprintf(stderr, "Example usage: example 352x288 output.h265 <input.yuv\n");
// 		exit_status = 1;
// 		goto cleanup;
// 	}

// 	if (sscanf(argv[1], "%dx%d", &width, &height) != 2) {
// 		fprintf(stderr, "resolution not specified or incorrect\n");
// 		exit_status = 1;
// 		goto cleanup;
// 	} else {
// 		luma_size = width * height;
// 		chroma_size = luma_size / 4;
// 		yuv_size = luma_size + 2*chroma_size;
// 	}

// 	path_out = argv[2];
// 	out = fopen(path_out, "wb");

// 	if (!(param = x265_param_alloc())) {
// 		exit_status = 1; goto cleanup;
// 	}

// 	if (ret = x265_param_default_preset(param, DEFAULT_PRESET, DEFAULT_TUNE)) {
// 		exit_status = 1; goto cleanup;
// 	} else {
// 		param->internalCsp = X265_CSP_I420;  /* color-space */
// 		param->fpsNum = 25;
// 		param->fpsDenom = 1;
// 		param->sourceWidth = width;
// 		param->sourceHeight = height;
// 		param->bframes = 3;
// 		param->bRepeatHeaders = 1;
// 		param->bAnnexB = 1;
// 		// param->analysisMode = X265_ANALYSIS_LOAD;
// 		// param->analysisFileName = "/tmp/analysis-x265.bin";
// 		param->rc.bStrictCbr = 1;
// 		param->rc.bitrate = 204;
// 		param->rc.vbvBufferSize = 1.5 * param->rc.bitrate;
// 		param->rc.vbvMaxBitrate = 1.5 * param->rc.bitrate;
// 	}

// 	if (!(encoder = x265_encoder_open(param))) {
// 		exit_status = 1; goto cleanup;
// 	}

// 	if (!(pic_in = x265_picture_alloc())) {
// 		exit_status = 1; goto cleanup;
// 	} else {
// 		x265_picture_init(param, pic_in);
// 		pic_in->colorSpace = X265_CSP_I420;
// 	}

// 	if (!(pic_out = x265_picture_alloc())) {
// 		exit_status = 1; goto cleanup;
// 	} /* else {
// 		x265_picture_init(param, pic_out);
// 	} */

// 	input_yuv = malloc(yuv_size);

// 	for( ;; input_frame_count++ ) {
// 		/* Read input frame */
// 		if (fread(input_yuv, 1, yuv_size, stdin) != yuv_size) break;

// 		pic_in->stride[0] = width;
// 		pic_in->stride[1] = pic_in->stride[0] >> x265_cli_csps[pic_in->colorSpace].width[1];
// 		pic_in->stride[2] = pic_in->stride[0] >> x265_cli_csps[pic_in->colorSpace].width[2];
// 		pic_in->planes[0] = input_yuv;
// 		pic_in->planes[1] = (char*)pic_in->planes[0] + pic_in->stride[0] * height;
// 		pic_in->planes[2] = (char*)pic_in->planes[1] + pic_in->stride[1] * (height >> x265_cli_csps[pic_in->colorSpace].height[1]);

// 		ret = x265_encoder_encode(encoder, &out_nals, &out_nal_count, pic_in, pic_out);
// 		if (ret < 0) {
// 			exit_status = 1; goto cleanup;
// 		} else if (ret == 1) {
// 			for (i = 0; i < out_nal_count; i++) {
// 				x265_nal nal = out_nals[i];
// 				fprintf(stderr, "[+] %5d %5d %5d %5d %5d\n", out_nal_count, input_frame_count, nal.sizeBytes,
// 					pic_out->sliceType, pic_out->poc);
// 				if(!fwrite(nal.payload, nal.sizeBytes, 1, out)) {
// 					exit_status = 1; goto cleanup;
// 				}
// 			}

// 			// printf("%d %d %d\n", pic_out->stride[0], pic_out->stride[1], pic_out->stride[2]);
// 		}
// 	}

// 	for (;;) {
// 		ret = x265_encoder_encode(encoder, &out_nals, &out_nal_count, NULL, pic_out);
// 		if (ret == 0) {
// 			break;
// 		} else if (ret < 0) {
// 			exit_status = 1; goto cleanup;
// 		}

// 		for (i = 0; i < out_nal_count; i++) {
// 			x265_nal nal = out_nals[i];
// 			fprintf(stderr, "[+] %5d %5d %5d %5d %5d\n", out_nal_count, input_frame_count, nal.sizeBytes,
// 				pic_out->sliceType, pic_out->poc);
// 			if(!fwrite(nal.payload, nal.sizeBytes, 1, out)) {
// 				exit_status = 1; goto cleanup;
// 			}
// 		}
// 	}

// cleanup:
// 	if (param) {
// 		x265_param_free(param); param = NULL;
// 	}
// 	if (encoder) {
// 		x265_encoder_close(encoder); encoder = NULL;
// 	}
// 	if (pic_in) {
// 		x265_picture_free(pic_in); pic_in = NULL;
// 	}
// 	if (pic_out) {
// 		x265_picture_free(pic_out); pic_out = NULL;
// 	}
// 	x265_cleanup();

// 	return exit_status;
}
