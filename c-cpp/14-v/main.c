#include <stdio.h>


typedef struct vt_s    VT;
typedef struct base_s  Base;
typedef struct child_1_s Child1;
typedef struct child_2_s Child2;


extern VT child_1_vt;
extern VT child_2_vt;


struct vt_s {
	void (*perform) (void *ctx);
};

struct base_s {
	char *name;

	void *w;
	VT *vt;
};

static void perform(Base *it) {
	if ((it->w) && (it->vt) && (it->vt->perform)) {
		it->vt->perform(it->w);
		return;
	}

	printf("[base @ %p]\n", it);
}

struct child_1_s {
	Base base;
};

static int child_1_init(Child1 *it) {
	it->base.name = "child-1";
	it->base.w = (void*)it;
	it->base.vt = &child_1_vt;
}

static void child_1_perform(void *ctx) {
	Child1 *it = (Child1*)ctx;
	printf("[%s @ %p]\n", it->base.name, it);
}

VT child_1_vt = {
	.perform = child_1_perform,
};

struct child_2_s {
	Base base;
};

static int child_2_init(Child2 *it) {
	it->base.name = "child-2";
	it->base.w = (void*)it;
	it->base.vt = &child_2_vt;
}

VT child_2_vt = {
	.perform = NULL,
};


int main(void) {
	Child1 c1 = { 0 };
	Child2 c2 = { 0 };

	child_1_init(&c1);
	child_2_init(&c2);

	perform((Base*)&c1);
	perform((Base*)&c2);
}
