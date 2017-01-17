#ifndef __ENC_STATUS__
#define __ENC_STATUS__


typedef enum   enc_status_enum ENCStatus;


enum enc_status_enum {
	ENC_STATUS_MORE = 0,  /* need more YUV for encoder */
	ENC_STATUS_OK   = 1,  /* OK, NALs are ready */
	ENC_STATUS_ERR  = 2,  /* error while encoding */
};


#endif /* __ENC_STATUS__ */
