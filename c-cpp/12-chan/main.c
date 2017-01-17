#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>    /* malloc */
#include <string.h>    /* memcpy */
#include <sysexits.h>  /* EX_OK, EX_SOFTWARE */

#include <x265.h>

#include "./buf.h"
#include "./bufs.h"
#include "./enc.h"
#include "./encs.h"
#include "./profile.h"
#include "./signal.h"


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
#define DEFAULT_PRESET "ultrafast"

// tune:
//   psnr
//   ssim
//   grain
//   fastdecode
//   zerolatency
#define DEFAULT_TUNE "zerolatency"

#define DEFAULT_W       1920
#define DEFAULT_H       1080
#define DEFAULT_BFRAMES 0


static const Profile profiles[] =
{
    // { DEFAULT_W, DEFAULT_H, 204,  DEFAULT_BFRAMES, DEFAULT_PRESET, DEFAULT_TUNE, },
    { DEFAULT_W, DEFAULT_H, 504,  DEFAULT_BFRAMES, DEFAULT_PRESET, DEFAULT_TUNE, },
    { DEFAULT_W, DEFAULT_H, 1272, DEFAULT_BFRAMES, DEFAULT_PRESET, DEFAULT_TUNE, },
    { 0,         0,         0,    0,               NULL,           NULL,         },
};


int main(int argc, char **argv) {
	int exit_status = EX_OK;
	const Profile *profile = NULL;
	ENCs *encs = NULL;
	ENC *enc = NULL;
	Bufs *bufs_yuv = NULL;
	Buf *buf_yuv = NULL;

	MsgI msg_i = { 0 };
	MsgO msg_o = { 0 };

	char out_path[255] = { 0 };
	FILE *out = NULL;
	FILE *outs[20] = { NULL };

	size_t i = 0;
	size_t yuv_size = DEFAULT_W*DEFAULT_H + (2*(DEFAULT_W*DEFAULT_H))/4;
	void *yuv_enc = NULL;
	void *yuv_in = NULL;

	if (signal_init()) {
		exit_status = EX_SOFTWARE; goto cleanup;
	}

	encs_new(&encs);
	bufs_new(&bufs_yuv);
	yuv_in = malloc(yuv_size);

	for (profile=profiles; !profile_is_empty(profile); profile++) {
		ENCParam enc_param = {
			.profile = *profile,

			.cap_chan_i = 1,
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

		enc_push(enc, encs);

		printf("[OK] [+] encoder "); profile_print(&enc->param.profile);

		if (buf_new(&buf_yuv, 10, enc->param.profile.yuv_sz)) {
			fprintf(stderr, "YUV buffer allocation failed;\n");
			exit_status = EX_SOFTWARE; goto cleanup;
		}

		bufs_push(bufs_yuv, buf_yuv);

		snprintf(out_path, sizeof(out_path)-1, "/tmp/x265/%zdx%zd@%zd.x265",
			profile->w, profile->h, profile->b);
		outs[(int)enc->index] = fopen(out_path, "wb");
	}

	for(;;) {
		if (signal_sigstop_received)
			goto cleanup;

		/* Read input frame */
		if (fread(yuv_in, 1, yuv_size, stdin) != yuv_size) break;

		for (i = 0; i < encs->len; i++) {
			yuv_enc = NULL;
			enc = encs->els[i];
			buf_yuv = bufs_yuv->els[i];

			buf_get_available(buf_yuv, &yuv_enc);
			memcpy(yuv_enc, yuv_in, yuv_size);

			msg_i.kind = MSG_I_KIND_YUV;
			msg_i.yuv = yuv_enc;

			chan_send_silent(enc->chan_i, &msg_i);
			chan_notify(enc->chan_i);
		}

		encs_wait(encs);

		for (i = 0; i < encs->len; i++) {
			enc = encs->els[i];

			out = outs[i];

			while(chan_got_msg(enc->chan_o)) {
				chan_recv_silent(enc->chan_o, &msg_o);

				switch (msg_o.status) {
				case ENC_STATUS_MORE:
					break;

				case ENC_STATUS_OK:
					fwrite(msg_o.nal_payload, msg_o.nal_sz, 1, out);

					break;

				case ENC_STATUS_ERR:
					fprintf(stderr, "encoding error!\n");
					exit_status = EX_SOFTWARE;
					goto cleanup;
					break;
				}
			}
		}
	}

	// signal_wait();

cleanup:
	encs_stop(encs);
	encs_del(&encs);

	if (yuv_in) free(yuv_in);

	FILE **out_cursor = NULL;
	for (out_cursor=outs; *out_cursor; out_cursor++) {
		out = *out_cursor;
		fclose(out);
	}

	return exit_status;
}
