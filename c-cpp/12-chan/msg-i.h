#ifndef __MSG_I_H__
#define __MSG_I_H__


typedef struct msg_i_s         MsgI;
typedef enum   msg_i_kind_enum MsgIKind;


enum msg_i_kind_enum {
	MSG_I_KIND_YUV  = 0,
	MSG_I_KIND_STOP = 1,
};

struct msg_i_s {
	MsgIKind kind;
	void *yuv;
};


#endif /* __MSG_I_H__ */
