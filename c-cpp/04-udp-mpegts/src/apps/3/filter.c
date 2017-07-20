#include "filter.h"


int filter_new(Filter **out) {
	int ret = 0;
	Filter *it = NULL;

	it = calloc(1, sizeof(Filter));
	if (!it) return 1;

	*out = it;

	return ret;
}

int filter_init(Filter *it) {
	slice_init(&it->consumers, sizeof(char*));
	it->name = NULL;
	it->w = NULL;
	it->vt = NULL;
}

int filter_append_consumer(Filter *it, Filter *cnsmr) {
	printf("[%s @ %p] -> [%s @ %p]\n", it->name, (void*)it, cnsmr->name, (void*)cnsmr);
	slice_append(&it->consumers, &cnsmr);
	return 0;
}

int filter_consume_strm(Filter *it, Stream *strm) {
	return it->vt->consume_strm(it->w, strm);
}

int filter_consume_trk(Filter *it, Track *trk) {
	if ((!it->vt) || (!it->vt->consume_trk))
		return filter_produce_trk(it, trk);

	return it->vt->consume_trk(it->w, trk);
}

int filter_consume_pkt(Filter *it, Packet *pkt) {
	if ((!it->vt) || (!it->vt->consume_pkt))
		return filter_produce_pkt(it, pkt);

	return it->vt->consume_pkt(it->w, pkt);
}

int filter_consume_pkt_raw(Filter *it, uint8_t *buf, size_t bufsz) {
	if ((!it->vt) || (!it->vt->consume_pkt_raw))
		return filter_produce_pkt_raw(it, buf, bufsz);

	return it->vt->consume_pkt_raw(it->w, buf, bufsz);
}

int filter_consume_frm(Filter *it, Frame *frm) {
	if ((!it->vt) || (!it->vt->consume_frm))
		return filter_produce_frm(it, frm);

	return it->vt->consume_frm(it->w, frm);
}

int filter_produce_strm(Filter *it, Stream *strm) {
	int ret = 0;

	if (!it->consumers.len) {
		ret = 0;
		goto cleanup;
	}

	{int i = 0; for (i = 0; i < (int)it->consumers.len; i++) {
		Filter **pfltr = slice_get(&it->consumers, (size_t)i);
		filter_consume_strm(*pfltr, strm);
	}}

cleanup:
	return ret;
}

int filter_produce_trk(Filter *it, Track *trk) {
	int ret = 0;

	if (!it->consumers.len) {
		ret = 0;
		goto cleanup;
	}

	{int i = 0; for (i = 0; i < (int)it->consumers.len; i++) {
		Filter **pfltr = slice_get(&it->consumers, (size_t)i);
		filter_consume_trk(*pfltr, trk);
	}}

cleanup:
	return ret;
}

int filter_produce_pkt(Filter *it, Packet *pkt) {
	int ret = 0;

	if (!it->consumers.len) {
		ret = 0;
		goto cleanup;
	}

	{int i = 0; for (i = 0; i < (int)it->consumers.len; i++) {
		Filter **pfltr = slice_get(&it->consumers, (size_t)i);
		filter_consume_pkt(*pfltr, pkt);
	}}

cleanup:
	return ret;
}

int filter_produce_pkt_raw(Filter *it, uint8_t *buf, size_t bufsz) {
	int ret = 0;

	if (!it->consumers.len) {
		ret = 0;
		goto cleanup;
	}

	{int i = 0; for (i = 0; i < (int)it->consumers.len; i++) {
		Filter **pfltr = slice_get(&it->consumers, (size_t)i);
		filter_consume_pkt_raw(*pfltr, buf, bufsz);
	}}

cleanup:
	return ret;
}

int filter_produce_frm(Filter *it, Frame *frm) {
	int ret = 0;

	if (!it->consumers.len) {
		ret = 0;
		goto cleanup;
	}

	{int i = 0; for (i = 0; i < (int)it->consumers.len; i++) {
		Filter **pfltr = slice_get(&it->consumers, (size_t)i);
		filter_consume_frm(*pfltr, frm);
	}}

cleanup:
	return ret;
}

int filter_fin(Filter *it) {
	int ret = 0;

	if (!it) return ret;

	if (it->consumers.len)
		slice_fin(&it->consumers);

	return ret;
}

int filter_del(Filter **out) {
	int ret = 0;
	Filter *it = NULL;

	if (!out) return ret;

	it = *out;

	if (!it) return ret;

	filter_fin(it);

	free(it);
	*out = NULL;

	return ret;
}
