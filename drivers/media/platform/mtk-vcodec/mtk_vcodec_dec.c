/*
 * Copyright (c) 2016 MediaTek Inc.
 * Author: PC Chen <pc.chen@mediatek.com>
 *         Tiffany Lin <tiffany.lin@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <media/v4l2-event.h>
#include <media/v4l2-mem2mem.h>
#include <media/videobuf2-dma-contig.h>
#include <linux/delay.h>

#include "mtk_vcodec_drv.h"
#include "mtk_vcodec_dec.h"
#include "mtk_vcodec_intr.h"
#include "mtk_vcodec_util.h"
#include "vdec_drv_if.h"
#include "mtk_vcodec_dec_pm.h"

#define DFT_CFG_WIDTH	MTK_VDEC_MIN_W
#define DFT_CFG_HEIGHT	MTK_VDEC_MIN_H

static struct mtk_video_fmt
	mtk_video_formats[MTK_MAX_DEC_CODECS_SUPPORT] = { {0} };
static struct mtk_codec_framesizes
	mtk_vdec_framesizes[MTK_MAX_DEC_CODECS_SUPPORT] = { {0} };
static struct mtk_video_fmt *default_out_fmt;
static struct mtk_video_fmt *default_cap_fmt;

#define NUM_SUPPORTED_FRAMESIZE ARRAY_SIZE(mtk_vdec_framesizes)
#define NUM_FORMATS ARRAY_SIZE(mtk_video_formats)

static void get_supported_format(struct mtk_vcodec_ctx *ctx)
{
	unsigned int i;
	const struct mtk_video_fmt *fmt = NULL;
	const struct mtk_vcodec_dec_pdata *dec_pdata = ctx->dev->vdec_pdata;

	memcpy(mtk_video_formats, dec_pdata->vdec_formats,
		sizeof(struct mtk_video_fmt) * dec_pdata->num_formats);
	for (i = 0; i < dec_pdata->num_formats; i++) {
		fmt = &dec_pdata->vdec_formats[i];
		if (dec_pdata->default_out_fmt == fmt)
			default_out_fmt = &mtk_video_formats[i];

		if (dec_pdata->default_cap_fmt == fmt)
			default_cap_fmt = &mtk_video_formats[i];
	}

	if (vdec_if_get_param(ctx,
		GET_PARAM_CAPABILITY_SUPPORTED_FORMATS,
		&mtk_video_formats)  != 0) {
		mtk_v4l2_debug(1, "warning!! Cannot get supported format");
		return;
	}

	for (i = 0; i < MTK_MAX_DEC_CODECS_SUPPORT; i++) {
		if (mtk_video_formats[i].fourcc != 0)
			mtk_v4l2_debug(1,
			  "fmt[%d] fourcc %d type %d planes %d",
			  i, mtk_video_formats[i].fourcc,
			  mtk_video_formats[i].type,
			  mtk_video_formats[i].num_planes);
	}
	for (i = 0; i < MTK_MAX_DEC_CODECS_SUPPORT; i++) {
		if (mtk_video_formats[i].fourcc != 0 &&
			mtk_video_formats[i].type == MTK_FMT_DEC) {
			default_out_fmt = &mtk_video_formats[i];
			break;
		}
	}
	for (i = 0; i < MTK_MAX_DEC_CODECS_SUPPORT; i++) {
		if (mtk_video_formats[i].fourcc != 0 &&
			mtk_video_formats[i].type == MTK_FMT_FRAME) {
			default_cap_fmt = &mtk_video_formats[i];
			break;
		}
	}
}

static struct mtk_video_fmt *mtk_vdec_find_format(struct mtk_vcodec_ctx *ctx,
	struct v4l2_format *f, unsigned int t)
{
	struct mtk_video_fmt *fmt;
	unsigned int k;

	mtk_v4l2_debug(3, "[%d] fourcc %d", ctx->id, f->fmt.pix_mp.pixelformat);
	for (k = 0; k < MTK_MAX_DEC_CODECS_SUPPORT &&
		 mtk_video_formats[k].fourcc != 0; k++) {
		fmt = &mtk_video_formats[k];
		if (fmt->fourcc == f->fmt.pix_mp.pixelformat &&
			mtk_video_formats[k].type == t)
			return fmt;
	}

	return NULL;
}

void mtk_vdec_update_fmt(struct mtk_vcodec_ctx *ctx,
				unsigned int pixelformat)
{
	const struct mtk_video_fmt *fmt;
	struct mtk_q_data *dst_q_data;
	unsigned int k;

	dst_q_data = &ctx->q_data[MTK_Q_DATA_DST];
	for (k = 0; k < MTK_MAX_DEC_CODECS_SUPPORT; k++) {
		fmt = &mtk_video_formats[k];
		if (fmt->fourcc == pixelformat) {
			mtk_v4l2_debug(1, "Update cap fourcc(%d -> %d)",
				dst_q_data->fmt->fourcc, pixelformat);
			dst_q_data->fmt = fmt;
			return;
		}
	}

	mtk_v4l2_err("Cannot get fourcc(%d), using init value", pixelformat);
}

struct mtk_video_fmt *mtk_find_fmt_by_pixel(unsigned int pixelformat)
{
	struct mtk_video_fmt *fmt;
	unsigned int k;

	for (k = 0; k < MTK_MAX_DEC_CODECS_SUPPORT; k++) {
		fmt = &mtk_video_formats[k];
		if (fmt->fourcc == pixelformat)
			return fmt;
	}
	mtk_v4l2_err("Error!! Cannot find fourcc: %d use default", pixelformat);

	return default_out_fmt;
}


static struct mtk_q_data *mtk_vdec_get_q_data(struct mtk_vcodec_ctx *ctx,
					      enum v4l2_buf_type type)
{
	if (V4L2_TYPE_IS_OUTPUT(type))
		return &ctx->q_data[MTK_Q_DATA_SRC];

	return &ctx->q_data[MTK_Q_DATA_DST];
}

static int vidioc_try_decoder_cmd(struct file *file, void *priv,
				struct v4l2_decoder_cmd *cmd)
{
	switch (cmd->cmd) {
	case V4L2_DEC_CMD_STOP:
	case V4L2_DEC_CMD_START:
		if (cmd->flags != 0) {
			mtk_v4l2_debug(1, "cmd->flags=%u", cmd->flags);
		}
		break;
	default:
		return -EINVAL;
	}
	return 0;
}


static int vidioc_decoder_cmd(struct file *file, void *priv,
				struct v4l2_decoder_cmd *cmd)
{
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);
	struct vb2_queue *src_vq, *dst_vq;
	struct mtk_video_dec_buf *empty_buf;
	int ret;
	const struct mtk_vcodec_dec_pdata *dec_pdata = ctx->dev->vdec_pdata;

	ret = vidioc_try_decoder_cmd(file, priv, cmd);
	if (ret)
		return ret;

	empty_buf = ctx->empty_flush_buf;
	mtk_v4l2_debug(1, "decoder cmd=%u, flag=%u, used:%d",
		cmd->cmd, cmd->flags, empty_buf->used);
	dst_vq = v4l2_m2m_get_vq(ctx->m2m_ctx,
				V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
	switch (cmd->cmd) {
	case V4L2_DEC_CMD_STOP:
		src_vq = v4l2_m2m_get_vq(ctx->m2m_ctx,
				V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE);
		if (!vb2_is_streaming(src_vq)) {
			mtk_v4l2_debug(1, "Output stream is off. No need to flush.");
			return 0;
		}
		if (empty_buf->used == false) {
			empty_buf->used = true;
			if (!dec_pdata->uses_stateless_api) {
				empty_buf->lastframe = true;
				if (cmd->flags & V4L2_BUF_FLAG_EARLY_EOS)
					empty_buf->isEarlyEos = true;
				else
					empty_buf->isEarlyEos = false;
			}
			v4l2_m2m_buf_queue(ctx->m2m_ctx,
				&empty_buf->vb);
			v4l2_m2m_try_schedule(ctx->m2m_ctx);
		}
		break;

	case V4L2_DEC_CMD_START:
		vb2_clear_last_buffer_dequeued(dst_vq);
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

void mtk_vdec_unlock(struct mtk_vcodec_ctx *ctx)
{
	mtk_v4l2_debug(4, "ctx %p [%d]", ctx, ctx->id);
	mutex_unlock(&ctx->dev->dec_mutex);
}

void mtk_vdec_lock(struct mtk_vcodec_ctx *ctx)
{
	unsigned int suspend_block_cnt = 0;

	while (ctx->dev->is_codec_suspending == 1) {
		suspend_block_cnt++;
		if (suspend_block_cnt > SUSPEND_TIMEOUT_CNT) {
			mtk_v4l2_debug(4, "VDEC blocked by suspend\n");
			suspend_block_cnt = 0;
		}
		usleep_range(10000, 20000);
	}

	mtk_v4l2_debug(4, "ctx %p [%d]", ctx, ctx->id);
	mutex_lock(&ctx->dev->dec_mutex);
}

void mtk_vcodec_dec_empty_queues(struct mtk_vcodec_ctx *ctx)
{
	struct vb2_v4l2_buffer *src_buf = NULL, *dst_buf = NULL;
	struct vb2_v4l2_buffer *last_buf = NULL;
	int i = 0;

	last_buf = &ctx->empty_flush_buf->vb;
	while ((src_buf = v4l2_m2m_src_buf_remove(ctx->m2m_ctx))) {
		if (last_buf != src_buf)
			v4l2_m2m_buf_done(src_buf, VB2_BUF_STATE_ERROR);
	}

	while ((dst_buf = v4l2_m2m_dst_buf_remove(ctx->m2m_ctx))) {
		for (i = 0; i < dst_buf->vb2_buf.num_planes; i++)
			vb2_set_plane_payload(&dst_buf->vb2_buf, i, 0);
		v4l2_m2m_buf_done(dst_buf, VB2_BUF_STATE_ERROR);
	}

	ctx->state = MTK_STATE_FREE;
}

void mtk_vcodec_dec_release(struct mtk_vcodec_ctx *ctx)
{
	vdec_if_deinit(ctx);
	memset(mtk_vdec_framesizes, 0, sizeof(mtk_vdec_framesizes));
	ctx->state = MTK_STATE_FREE;
}

void mtk_vcodec_dec_set_default_params(struct mtk_vcodec_ctx *ctx)
{
	struct mtk_q_data *q_data;

	ctx->dev->vdec_pdata->init_vdec_params(ctx);

	ctx->m2m_ctx->q_lock = &ctx->dev->dev_mutex;
	ctx->fh.m2m_ctx = ctx->m2m_ctx;
	ctx->fh.ctrl_handler = &ctx->ctrl_hdl;
	INIT_WORK(&ctx->decode_work, ctx->dev->vdec_pdata->worker);
	ctx->colorspace = V4L2_COLORSPACE_REC709;
	ctx->ycbcr_enc = V4L2_YCBCR_ENC_DEFAULT;
	ctx->quantization = V4L2_QUANTIZATION_DEFAULT;
	ctx->xfer_func = V4L2_XFER_FUNC_DEFAULT;
	get_supported_format(ctx);

	q_data = &ctx->q_data[MTK_Q_DATA_SRC];
	memset(q_data, 0, sizeof(struct mtk_q_data));
	q_data->visible_width = DFT_CFG_WIDTH;
	q_data->visible_height = DFT_CFG_HEIGHT;
	q_data->fmt = default_out_fmt;
	q_data->field = V4L2_FIELD_NONE;

	q_data->sizeimage[0] = DFT_CFG_WIDTH * DFT_CFG_HEIGHT;
	q_data->bytesperline[0] = 0;

	q_data = &ctx->q_data[MTK_Q_DATA_DST];
	memset(q_data, 0, sizeof(struct mtk_q_data));
	q_data->visible_width = DFT_CFG_WIDTH;
	q_data->visible_height = DFT_CFG_HEIGHT;
	q_data->coded_width = DFT_CFG_WIDTH;
	q_data->coded_height = DFT_CFG_HEIGHT;
	q_data->fmt = default_cap_fmt;
	q_data->field = V4L2_FIELD_NONE;

	v4l_bound_align_image(&q_data->coded_width,
				MTK_VDEC_MIN_W,
				MTK_VDEC_MAX_W, 4,
				&q_data->coded_height,
				MTK_VDEC_MIN_H,
				MTK_VDEC_MAX_H, 5, 6);

	q_data->sizeimage[0] = q_data->coded_width * q_data->coded_height;
	q_data->bytesperline[0] = q_data->coded_width;
	q_data->sizeimage[1] = q_data->sizeimage[0] / 2;
	q_data->bytesperline[1] = q_data->coded_width;
}

int mtk_vdec_set_param(struct mtk_vcodec_ctx *ctx)
{
	unsigned long in[8] = {0};

	mtk_v4l2_debug(4,
		"[%d] param change %d decode mode %d frame width %d frame height %d max width %d max height %d",
		ctx->id, ctx->dec_param_change,
		ctx->dec_params.decode_mode,
		ctx->dec_params.frame_size_width,
		ctx->dec_params.frame_size_height,
		ctx->dec_params.fixed_max_frame_size_width,
		ctx->dec_params.fixed_max_frame_size_height);

	if (ctx->dec_param_change & MTK_DEC_PARAM_DECODE_MODE) {
		in[0] = ctx->dec_params.decode_mode;
		if (vdec_if_set_param(ctx, SET_PARAM_DECODE_MODE, in) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot set param", ctx->id);
			return -EINVAL;
		}
		ctx->dec_param_change &= (~MTK_DEC_PARAM_DECODE_MODE);
	}

	if (ctx->dec_param_change & MTK_DEC_PARAM_FRAME_SIZE) {
		in[0] = ctx->dec_params.frame_size_width;
		in[1] = ctx->dec_params.frame_size_height;
		if (in[0] != 0 && in[1] != 0) {
			if (vdec_if_set_param(ctx,
				SET_PARAM_FRAME_SIZE, in) != 0) {
				mtk_v4l2_err("[%d] Error!! Cannot set param",
					ctx->id);
				return -EINVAL;
			}
		}
		ctx->dec_param_change &= (~MTK_DEC_PARAM_FRAME_SIZE);
	}

	if (ctx->dec_param_change &
		MTK_DEC_PARAM_FIXED_MAX_FRAME_SIZE) {
		in[0] = ctx->dec_params.fixed_max_frame_size_width;
		in[1] = ctx->dec_params.fixed_max_frame_size_height;
		if (in[0] != 0 && in[1] != 0) {
			if (vdec_if_set_param(ctx,
				SET_PARAM_SET_FIXED_MAX_OUTPUT_BUFFER,
				in) != 0) {
				mtk_v4l2_err("[%d] Error!! Cannot set param",
					ctx->id);
				return -EINVAL;
			}
		}
		ctx->dec_param_change &= (~MTK_DEC_PARAM_FIXED_MAX_FRAME_SIZE);
	}

	if (ctx->dec_param_change & MTK_DEC_PARAM_CRC_PATH) {
		in[0] = (unsigned long)ctx->dec_params.crc_path;
		if (vdec_if_set_param(ctx, SET_PARAM_CRC_PATH, in) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot set param", ctx->id);
			return -EINVAL;
		}
		ctx->dec_param_change &= (~MTK_DEC_PARAM_CRC_PATH);
	}

	if (ctx->dec_param_change & MTK_DEC_PARAM_GOLDEN_PATH) {
		in[0] = (unsigned long)ctx->dec_params.golden_path;
		if (vdec_if_set_param(ctx, SET_PARAM_GOLDEN_PATH, in) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot set param", ctx->id);
			return -EINVAL;
		}
		ctx->dec_param_change &= (~MTK_DEC_PARAM_GOLDEN_PATH);
	}

	if (ctx->dec_param_change & MTK_DEC_PARAM_WAIT_KEY_FRAME) {
		in[0] = (unsigned long)ctx->dec_params.wait_key_frame;
		if (vdec_if_set_param(ctx, SET_PARAM_WAIT_KEY_FRAME, in) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot set param", ctx->id);
			return -EINVAL;
		}
		ctx->dec_param_change &= (~MTK_DEC_PARAM_WAIT_KEY_FRAME);
	}

	if (ctx->dec_param_change & MTK_DEC_PARAM_NAL_SIZE_LENGTH) {
		in[0] = (unsigned long)ctx->dec_params.wait_key_frame;
		if (vdec_if_set_param(ctx, SET_PARAM_NAL_SIZE_LENGTH,
					in) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot set param", ctx->id);
			return -EINVAL;
		}
		ctx->dec_param_change &= (~MTK_DEC_PARAM_NAL_SIZE_LENGTH);
	}

	if (ctx->dec_param_change & MTK_DEC_PARAM_OPERATING_RATE) {
		in[0] = (unsigned long)ctx->dec_params.operating_rate;
		if (vdec_if_set_param(ctx, SET_PARAM_OPERATING_RATE, in) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot set param", ctx->id);
			return -EINVAL;
		}
		ctx->dec_param_change &= (~MTK_DEC_PARAM_OPERATING_RATE);
	}

	if (ctx->dec_param_change & MTK_DEC_PARAM_TOTAL_FRAME_BUFQ_COUNT) {
		in[0] = (unsigned long)ctx->dec_params.total_frame_bufq_count;
		if (vdec_if_set_param
			(ctx, SET_PARAM_TOTAL_FRAME_BUFQ_COUNT, in) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot set param", ctx->id);
			return -EINVAL;
		}
		ctx->dec_param_change &=
			(~MTK_DEC_PARAM_TOTAL_FRAME_BUFQ_COUNT);
	}

	return 0;
}

static int vidioc_vdec_qbuf(struct file *file, void *priv,
			    struct v4l2_buffer *buf)
{
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);
	struct vb2_queue *vq;
	struct vb2_buffer *vb;
	struct mtk_video_dec_buf *mtkbuf;
	struct vb2_v4l2_buffer  *vb2_v4l2;

	if (ctx->state == MTK_STATE_ABORT) {
		mtk_v4l2_err("[%d] Call on QBUF after unrecoverable error",
				ctx->id);
		return -EIO;
	}

	vq = v4l2_m2m_get_vq(ctx->m2m_ctx, buf->type);
	vb = vq->bufs[buf->index];
	vb2_v4l2 = container_of(vb, struct vb2_v4l2_buffer, vb2_buf);
	mtkbuf = container_of(vb2_v4l2, struct mtk_video_dec_buf, vb);

	if (buf->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		ctx->input_max_ts =
			(timeval_to_ns(&buf->timestamp) > ctx->input_max_ts) ?
			timeval_to_ns(&buf->timestamp) : ctx->input_max_ts;

		mtkbuf->lastframe = false;
		mtkbuf->isEarlyEos = false;
		if (buf->flags & V4L2_BUF_FLAG_LAST)
			mtkbuf->isEarlyEos = true;
		mtk_v4l2_debug(1, "[%d] id=%d getdata BS(%d,%d) vb=%p pts=%llu %llu %d",
			ctx->id, buf->index,
			buf->m.planes[0].bytesused,
			buf->length, vb,
			timeval_to_ns(&buf->timestamp),
			ctx->input_max_ts,
			mtkbuf->isEarlyEos);
	} else
		mtk_v4l2_debug(1, "[%d] id=%d FB (%d) vb=%p",
				ctx->id, buf->index,
				buf->length, mtkbuf);

	if (buf->flags & V4L2_BUF_FLAG_NO_CACHE_CLEAN) {
		mtk_v4l2_debug(4, "[%d] No need for Cache clean, buf->index:%d. mtkbuf:%p",
			ctx->id, buf->index, mtkbuf);
		mtkbuf->flags |= NO_CAHCE_CLEAN;
	}

	if (buf->flags & V4L2_BUF_FLAG_NO_CACHE_INVALIDATE) {
		mtk_v4l2_debug(4, "[%d] No need for Cache invalidate, buf->index:%d. mtkbuf:%p",
			ctx->id, buf->index, mtkbuf);
		mtkbuf->flags |= NO_CAHCE_INVALIDATE;
	}

	return v4l2_m2m_qbuf(file, ctx->m2m_ctx, buf);
}

static int vidioc_vdec_dqbuf(struct file *file, void *priv,
			     struct v4l2_buffer *buf)
{
	int ret = 0;
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);
	struct vb2_queue *vq;
	struct vb2_buffer *vb;
	struct mtk_video_dec_buf *mtkbuf;
	struct vb2_v4l2_buffer  *vb2_v4l2;

	if (ctx->state == MTK_STATE_ABORT) {
		mtk_v4l2_err("[%d] Call on DQBUF after unrecoverable error",
				ctx->id);
		return -EIO;
	}

	ret = v4l2_m2m_dqbuf(file, ctx->m2m_ctx, buf);
	buf->reserved = ctx->errormap_info[buf->index % VB2_MAX_FRAME];

	if (buf->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE &&
		ret == 0) {
		vq = v4l2_m2m_get_vq(ctx->m2m_ctx, buf->type);
		vb = vq->bufs[buf->index];
		vb2_v4l2 = container_of(vb, struct vb2_v4l2_buffer, vb2_buf);
		mtkbuf = container_of(vb2_v4l2, struct mtk_video_dec_buf, vb);

		if (mtkbuf->flags & CROP_CHANGED)
			buf->flags |= V4L2_BUF_FLAG_CROP_CHANGED;
		if (mtkbuf->flags & REF_FREED)
			buf->flags |= V4L2_BUF_FLAG_REF_FREED;
	}

	return ret;
}

static int vidioc_vdec_querycap(struct file *file, void *priv,
				struct v4l2_capability *cap)
{
	strscpy(cap->driver, MTK_VCODEC_DEC_NAME, sizeof(cap->driver));
	strscpy(cap->bus_info, MTK_PLATFORM_STR, sizeof(cap->bus_info));
	strscpy(cap->card, MTK_PLATFORM_STR, sizeof(cap->card));

	cap->device_caps  = V4L2_CAP_VIDEO_M2M_MPLANE | V4L2_CAP_STREAMING;
	cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;
	return 0;
}

static int vidioc_vdec_subscribe_evt(struct v4l2_fh *fh,
				     const struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_EOS:
		return v4l2_event_subscribe(fh, sub, 2, NULL);
	case V4L2_EVENT_SOURCE_CHANGE:
		return v4l2_src_change_event_subscribe(fh, sub);
	case V4L2_EVENT_VDEC_ERROR:
		return v4l2_event_subscribe(fh, sub, 0, NULL);
	case V4L2_EVENT_MTK_VDEC_NOHEADER:
		return v4l2_event_subscribe(fh, sub, 0, NULL);
	default:
		return v4l2_ctrl_subscribe_event(fh, sub);
	}
}

static int vidioc_try_fmt(struct v4l2_format *f,
			  const struct mtk_video_fmt *fmt)
{
	struct v4l2_pix_format_mplane *pix_fmt_mp = &f->fmt.pix_mp;
	int i;

	pix_fmt_mp->field = V4L2_FIELD_NONE;

	if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		pix_fmt_mp->num_planes = 1;
		pix_fmt_mp->plane_fmt[0].bytesperline = 0;
	} else if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		int tmp_w, tmp_h;

		pix_fmt_mp->height = clamp(pix_fmt_mp->height,
					MTK_VDEC_MIN_H,
					MTK_VDEC_MAX_H);
		pix_fmt_mp->width = clamp(pix_fmt_mp->width,
					MTK_VDEC_MIN_W,
					MTK_VDEC_MAX_W);

		/*
		 * Find next closer width align 64, heign align 64, size align
		 * 64 rectangle
		 * Note: This only get default value, the real HW needed value
		 *       only available when ctx in MTK_STATE_HEADER state
		 */
		tmp_w = pix_fmt_mp->width;
		tmp_h = pix_fmt_mp->height;
		v4l_bound_align_image(&pix_fmt_mp->width,
					MTK_VDEC_MIN_W,
					MTK_VDEC_MAX_W, 6,
					&pix_fmt_mp->height,
					MTK_VDEC_MIN_H,
					MTK_VDEC_MAX_H, 6, 9);

		if (pix_fmt_mp->width < tmp_w &&
			(pix_fmt_mp->width + 64) <= MTK_VDEC_MAX_W)
			pix_fmt_mp->width += 64;
		if (pix_fmt_mp->height < tmp_h &&
			(pix_fmt_mp->height + 64) <= MTK_VDEC_MAX_H)
			pix_fmt_mp->height += 64;

		mtk_v4l2_debug(0,
			"before resize width=%d, height=%d, after resize width=%d, height=%d, sizeimage=%d",
			tmp_w, tmp_h, pix_fmt_mp->width,
			pix_fmt_mp->height,
			pix_fmt_mp->width * pix_fmt_mp->height);

		pix_fmt_mp->num_planes = fmt->num_planes;
		pix_fmt_mp->plane_fmt[0].sizeimage =
				pix_fmt_mp->width * pix_fmt_mp->height;
		pix_fmt_mp->plane_fmt[0].bytesperline = pix_fmt_mp->width;

		if (pix_fmt_mp->num_planes == 2) {
			pix_fmt_mp->plane_fmt[1].sizeimage =
				(pix_fmt_mp->width * pix_fmt_mp->height) / 2;
			pix_fmt_mp->plane_fmt[1].bytesperline =
				pix_fmt_mp->width;
		}
	}

	for (i = 0; i < pix_fmt_mp->num_planes; i++)
		memset(&(pix_fmt_mp->plane_fmt[i].reserved[0]), 0x0,
			   sizeof(pix_fmt_mp->plane_fmt[0].reserved));

	pix_fmt_mp->flags = 0;
	memset(&pix_fmt_mp->reserved, 0x0, sizeof(pix_fmt_mp->reserved));
	return 0;
}

static int vidioc_try_fmt_vid_cap_mplane(struct file *file, void *priv,
				struct v4l2_format *f)
{
	const struct mtk_video_fmt *fmt;
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);

	fmt = mtk_vdec_find_format(ctx, f, MTK_FMT_FRAME);
	if (!fmt) {
		f->fmt.pix.pixelformat =
			ctx->q_data[MTK_Q_DATA_DST].fmt->fourcc;
		fmt = mtk_vdec_find_format(ctx, f, MTK_FMT_FRAME);
	}

	return vidioc_try_fmt(f, fmt);
}

static int vidioc_try_fmt_vid_out_mplane(struct file *file, void *priv,
				struct v4l2_format *f)
{
	struct v4l2_pix_format_mplane *pix_fmt_mp = &f->fmt.pix_mp;
	const struct mtk_video_fmt *fmt;
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);

	fmt = mtk_vdec_find_format(ctx, f, MTK_FMT_DEC);
	if (!fmt) {
		f->fmt.pix.pixelformat =
			ctx->q_data[MTK_Q_DATA_SRC].fmt->fourcc;
		fmt = mtk_vdec_find_format(ctx, f, MTK_FMT_DEC);
	}

	if (pix_fmt_mp->plane_fmt[0].sizeimage == 0) {
		mtk_v4l2_err("sizeimage of output format must be given");
		return -EINVAL;
	}

	return vidioc_try_fmt(f, fmt);
}

static int vidioc_vdec_g_selection(struct file *file, void *priv,
			struct v4l2_selection *s)
{
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);
	struct mtk_q_data *q_data;

	if (s->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	q_data = &ctx->q_data[MTK_Q_DATA_DST];

	switch (s->target) {
	case V4L2_SEL_TGT_COMPOSE_DEFAULT:
	case V4L2_SEL_TGT_CROP_DEFAULT:
		s->r.left = 0;
		s->r.top = 0;
		s->r.width = ctx->picinfo.pic_w;
		s->r.height = ctx->picinfo.pic_h;
		break;
	case V4L2_SEL_TGT_COMPOSE_BOUNDS:
	case V4L2_SEL_TGT_CROP_BOUNDS:
		s->r.left = 0;
		s->r.top = 0;
		s->r.width = ctx->picinfo.buf_w;
		s->r.height = ctx->picinfo.buf_h;
		break;
	case V4L2_SEL_TGT_COMPOSE:
	case V4L2_SEL_TGT_CROP:
		if (vdec_if_get_param(ctx, GET_PARAM_CROP_INFO, &(s->r))) {
			/* set to default value if header info not ready yet*/
			s->r.left = 0;
			s->r.top = 0;
			s->r.width = q_data->visible_width;
			s->r.height = q_data->visible_height;
		}
		break;
	default:
		return -EINVAL;
	}

	if (ctx->state < MTK_STATE_HEADER) {
		/* set to default value if header info not ready yet*/
		s->r.left = 0;
		s->r.top = 0;
		s->r.width = q_data->visible_width;
		s->r.height = q_data->visible_height;
		return 0;
	}

	return 0;
}

static int vidioc_vdec_s_selection(struct file *file, void *priv,
				struct v4l2_selection *s)
{
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);

	if (s->type != V4L2_BUF_TYPE_VIDEO_CAPTURE)
		return -EINVAL;

	switch (s->target) {
	case V4L2_SEL_TGT_COMPOSE:
	case V4L2_SEL_TGT_CROP:
		s->r.left = 0;
		s->r.top = 0;
		s->r.width = ctx->picinfo.pic_w;
		s->r.height = ctx->picinfo.pic_h;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int vidioc_vdec_s_fmt(struct file *file, void *priv,
			     struct v4l2_format *f)
{
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);
	struct v4l2_pix_format_mplane *pix_mp;
	struct mtk_q_data *q_data, *out_q_data;
	int ret = 0, width = 0, height = 0;
	struct mtk_video_fmt *fmt;

	const struct mtk_vcodec_dec_pdata *dec_pdata = ctx->dev->vdec_pdata;
	uint64_t size[2];

	mtk_v4l2_debug(3, "[%d]", ctx->id);

	q_data = mtk_vdec_get_q_data(ctx, f->type);
	if (!q_data)
		return -EINVAL;

	pix_mp = &f->fmt.pix_mp;
	if (!dec_pdata->uses_stateless_api &&
	    (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) &&
	    vb2_is_busy(&ctx->m2m_ctx->out_q_ctx.q)) {
		mtk_v4l2_err("out_q_ctx buffers already requested");
		ret = -EBUSY;
	}

	if ((f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) &&
	    vb2_is_busy(&ctx->m2m_ctx->cap_q_ctx.q)) {
		mtk_v4l2_err("cap_q_ctx buffers already requested");
		ret = -EBUSY;
	}

	fmt = mtk_vdec_find_format(ctx, f,
		(f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) ?
		MTK_FMT_DEC : MTK_FMT_FRAME);
	if (fmt == NULL) {
		if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
			f->fmt.pix.pixelformat =
				default_out_fmt->fourcc;
			fmt = mtk_vdec_find_format(ctx, f, MTK_FMT_DEC);
		} else if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
			f->fmt.pix.pixelformat =
				default_cap_fmt->fourcc;
			fmt = mtk_vdec_find_format(ctx, f, MTK_FMT_FRAME);
		}
	}
	if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		width = pix_mp->width;
		height = pix_mp->height;
		out_q_data = mtk_vdec_get_q_data(ctx, f->type);
		if (!out_q_data)
			return -EINVAL;
	}


	q_data->fmt = fmt;
	vidioc_try_fmt(f, q_data->fmt);
	if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		q_data->sizeimage[0] = pix_mp->plane_fmt[0].sizeimage;
		q_data->coded_width = pix_mp->width;
		q_data->coded_height = pix_mp->height;
		size[0] = pix_mp->width;
		size[1] = pix_mp->height;

		ctx->colorspace = f->fmt.pix_mp.colorspace;
		ctx->ycbcr_enc = f->fmt.pix_mp.ycbcr_enc;
		ctx->quantization = f->fmt.pix_mp.quantization;
		ctx->xfer_func = f->fmt.pix_mp.xfer_func;

		ctx->current_codec = fmt->fourcc;
		if (ctx->state == MTK_STATE_FREE) {
			ret = vdec_if_init(ctx, q_data->fmt->fourcc);
			if (ret) {
				mtk_v4l2_err("[%d]: vdec_if_init() fail ret=%d",
					ctx->id, ret);
				return -EINVAL;
			}
			vdec_if_set_param(ctx, SET_PARAM_FRAME_SIZE,
					(void *)size);

			ctx->state = MTK_STATE_INIT;
		}
	}
	if (f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		vdec_if_set_param(ctx, SET_PARAM_FB_NUM_PLANES,
			(void *) &q_data->fmt->num_planes);

	/* Tolerate both OUTPUT and CAPTURE queues for compatibility reasons */
	if (dec_pdata->uses_stateless_api) {
		ctx->picinfo.pic_w = pix_mp->width;
		ctx->picinfo.pic_h = pix_mp->height;

		ret = vdec_if_get_param(ctx, GET_PARAM_PIC_INFO, &ctx->picinfo);
		if (ret) {
			mtk_v4l2_err("[%d]Error!! Get GET_PARAM_PICTURE_INFO Fail",
				ctx->id);
			return -EINVAL;
		}

		ctx->last_decoded_picinfo = ctx->picinfo;
		if (pix_mp->num_planes == 1) {
			ctx->q_data[MTK_Q_DATA_DST].sizeimage[0] =
				ctx->picinfo.fb_sz[0] +
				ctx->picinfo.fb_sz[1];
			ctx->q_data[MTK_Q_DATA_DST].bytesperline[0] =
				ctx->picinfo.buf_w;
		} else {
			ctx->q_data[MTK_Q_DATA_DST].sizeimage[0] =
				ctx->picinfo.fb_sz[0];
			ctx->q_data[MTK_Q_DATA_DST].bytesperline[0] =
				ctx->picinfo.buf_w;
			ctx->q_data[MTK_Q_DATA_DST].sizeimage[1] =
				ctx->picinfo.fb_sz[1];
			ctx->q_data[MTK_Q_DATA_DST].bytesperline[1] =
				ctx->picinfo.buf_w;
		}

		ctx->q_data[MTK_Q_DATA_DST].coded_width = ctx->picinfo.buf_w;
		ctx->q_data[MTK_Q_DATA_DST].coded_height = ctx->picinfo.buf_h;
		mtk_v4l2_debug(2, "[%d] vdec_if_init() num_plane = %d wxh=%dx%d pic wxh=%dx%d sz[0]=0x%x sz[1]=0x%x",
			ctx->id, pix_mp->num_planes,
			ctx->picinfo.buf_w, ctx->picinfo.buf_h,
			ctx->picinfo.pic_w, ctx->picinfo.pic_h,
			ctx->q_data[MTK_Q_DATA_DST].sizeimage[0],
			ctx->q_data[MTK_Q_DATA_DST].sizeimage[1]);
	}
	return 0;
}

static int vidioc_enum_framesizes(struct file *file, void *priv,
				struct v4l2_frmsizeenum *fsize)
{
	int i = 0;
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);
	const struct mtk_vcodec_dec_pdata *dec_pdata = ctx->dev->vdec_pdata;

	if (fsize->index != 0)
		return -EINVAL;
	if (mtk_vdec_framesizes[0].fourcc == 0) {
		memcpy(mtk_vdec_framesizes, dec_pdata->vdec_framesizes,
			sizeof(struct mtk_codec_framesizes) *
			dec_pdata->num_framesizes);

		if (vdec_if_get_param(ctx, GET_PARAM_CAPABILITY_FRAME_SIZES,
			&mtk_vdec_framesizes) != 0) {
			mtk_v4l2_debug(1, "[%d] Error!! Cannot get frame size",
				ctx->id);
			return -EINVAL;
		}

		for (i = 0; i < MTK_MAX_DEC_CODECS_SUPPORT; i++) {
			if (mtk_vdec_framesizes[i].fourcc != 0) {
				mtk_v4l2_debug(1,
				"vdec_fs[%d] fourcc %d, profile %d, level %d, s %d %d %d %d %d %d\n",
				i, mtk_vdec_framesizes[i].fourcc,
				mtk_vdec_framesizes[i].profile,
				mtk_vdec_framesizes[i].level,
				mtk_vdec_framesizes[i].stepwise.min_width,
				mtk_vdec_framesizes[i].stepwise.max_width,
				mtk_vdec_framesizes[i].stepwise.step_width,
				mtk_vdec_framesizes[i].stepwise.min_height,
				mtk_vdec_framesizes[i].stepwise.max_height,
				mtk_vdec_framesizes[i].stepwise.step_height);
			}
		}
	}

	for (i = 0; i < MTK_MAX_DEC_CODECS_SUPPORT &&
		 mtk_vdec_framesizes[i].fourcc != 0; ++i) {
		if (fsize->pixel_format != mtk_vdec_framesizes[i].fourcc)
			continue;

		fsize->type = V4L2_FRMSIZE_TYPE_STEPWISE;
		fsize->reserved[0] = mtk_vdec_framesizes[i].profile;
		fsize->reserved[1] = mtk_vdec_framesizes[i].level;
		fsize->stepwise = mtk_vdec_framesizes[i].stepwise;
		if (!(ctx->dev->dec_capability &
			  VCODEC_CAPABILITY_4K_DISABLED)) {
			mtk_v4l2_debug(1, "4K is enabled");
			fsize->stepwise.max_width =
					VCODEC_DEC_4K_CODED_WIDTH;
			fsize->stepwise.max_height =
					VCODEC_DEC_4K_CODED_HEIGHT;
		}
		mtk_v4l2_debug(4, "%x, %d %d %d %d %d %d %d %d %d %d",
					   ctx->dev->dec_capability,
					   fsize->reserved[0],
					   fsize->reserved[1],
					   fsize->stepwise.min_width,
					   fsize->stepwise.max_width,
					   fsize->stepwise.step_width,
					   fsize->stepwise.min_height,
					   fsize->stepwise.max_height,
					   fsize->stepwise.step_height,
					   fsize->reserved[0],
					   fsize->reserved[1]);
		return 0;
	}

	return -EINVAL;
}

static int vidioc_enum_fmt(struct v4l2_fmtdesc *f, void *priv,
				bool output_queue)
{
	struct mtk_video_fmt *fmt;
	int i, j = 0;

	for (i = 0; i < MTK_MAX_DEC_CODECS_SUPPORT; i++) {
		if (output_queue &&
			(mtk_video_formats[i].type != MTK_FMT_DEC))
			continue;
		if (!output_queue &&
			(mtk_video_formats[i].type != MTK_FMT_FRAME))
			continue;

		if (j == f->index)
			break;
		++j;
	}

	if (i == MTK_MAX_DEC_CODECS_SUPPORT ||
		mtk_video_formats[i].fourcc == 0)
		return -EINVAL;

	fmt = &mtk_video_formats[i];
	f->pixelformat = fmt->fourcc;
	f->flags = 0;
	memset(f->reserved, 0, sizeof(f->reserved));

	return 0;
}

static int vidioc_vdec_enum_fmt_vid_cap_mplane(struct file *file, void *priv,
					       struct v4l2_fmtdesc *f)
{
	return vidioc_enum_fmt(f, priv, false);
}

static int vidioc_vdec_enum_fmt_vid_out_mplane(struct file *file, void *priv,
					       struct v4l2_fmtdesc *f)
{
	return vidioc_enum_fmt(f, priv, true);
}

static int vidioc_vdec_g_fmt(struct file *file, void *priv,
			     struct v4l2_format *f)
{
	struct mtk_vcodec_ctx *ctx = fh_to_ctx(priv);
	struct v4l2_pix_format_mplane *pix_mp = &f->fmt.pix_mp;
	struct vb2_queue *vq;
	struct mtk_q_data *q_data;
	unsigned int fourcc;
	unsigned int i = 0;

	vq = v4l2_m2m_get_vq(ctx->m2m_ctx, f->type);
	if (!vq) {
		mtk_v4l2_err("no vb2 queue for type=%d", f->type);
		return -EINVAL;
	}

	q_data = mtk_vdec_get_q_data(ctx, f->type);

	pix_mp->field = V4L2_FIELD_NONE;
	pix_mp->colorspace = ctx->colorspace;
	pix_mp->ycbcr_enc = ctx->ycbcr_enc;
	pix_mp->quantization = ctx->quantization;
	pix_mp->xfer_func = ctx->xfer_func;

	if ((f->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) &&
	    (ctx->state >= MTK_STATE_HEADER)) {
		/* Until STREAMOFF is called on the CAPTURE queue
		 * (acknowledging the event), the driver operates as if
		 * the resolution hasn't changed yet.
		 * So we just return picinfo yet, and update picinfo in
		 * stop_streaming hook function
		 */
		q_data->sizeimage[0] = ctx->picinfo.fb_sz[0];
		q_data->sizeimage[1] = ctx->picinfo.fb_sz[1];
		q_data->bytesperline[0] = ctx->last_decoded_picinfo.buf_w;
		q_data->bytesperline[1] = ctx->last_decoded_picinfo.buf_w;
		q_data->coded_width = ctx->picinfo.buf_w;
		q_data->coded_height = ctx->picinfo.buf_h;
		ctx->last_decoded_picinfo.cap_fourcc = q_data->fmt->fourcc;

		/*
		 * Width and height are set to the dimensions
		 * of the movie, the buffer is bigger and
		 * further processing stages should crop to this
		 * rectangle.
		 */
		fourcc = ctx->q_data[MTK_Q_DATA_SRC].fmt->fourcc;
		if (fourcc == V4L2_PIX_FMT_RV30 ||
			fourcc == V4L2_PIX_FMT_RV40) {
			pix_mp->width = 1920;
			pix_mp->height = 1088;
		} else {
			pix_mp->width = q_data->coded_width;
			pix_mp->height = q_data->coded_height;
		}

		/*
		 * Set pixelformat to the format in which mt vcodec
		 * outputs the decoded frame
		 */
		pix_mp->num_planes = q_data->fmt->num_planes;
		pix_mp->pixelformat = q_data->fmt->fourcc;
		if (fourcc == V4L2_PIX_FMT_RV30 ||
			fourcc == V4L2_PIX_FMT_RV40) {
			for (i = 0; i < pix_mp->num_planes; i++) {
				pix_mp->plane_fmt[i].bytesperline = 1920;
				pix_mp->plane_fmt[i].sizeimage =
					q_data->sizeimage[i];
			}
		} else {
			for (i = 0; i < pix_mp->num_planes; i++) {
				pix_mp->plane_fmt[i].bytesperline =
					q_data->bytesperline[i];
				pix_mp->plane_fmt[i].sizeimage =
					q_data->sizeimage[i];
			}
		}
		mtk_v4l2_debug(1, "fourcc:(%d %d),bytesperline:%d,sizeimage:%d,%d,%d\n",
			ctx->q_data[MTK_Q_DATA_SRC].fmt->fourcc,
			q_data->fmt->fourcc,
			pix_mp->plane_fmt[0].bytesperline,
			pix_mp->plane_fmt[0].sizeimage,
			pix_mp->plane_fmt[1].bytesperline,
			pix_mp->plane_fmt[1].sizeimage);
	} else if (f->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		/*
		 * This is run on OUTPUT
		 * The buffer contains compressed image
		 * so width and height have no meaning.
		 * Assign value here to pass v4l2-compliance test
		 */
		pix_mp->width = q_data->visible_width;
		pix_mp->height = q_data->visible_height;
		pix_mp->plane_fmt[0].bytesperline = q_data->bytesperline[0];
		pix_mp->plane_fmt[0].sizeimage = q_data->sizeimage[0];
		pix_mp->pixelformat = q_data->fmt->fourcc;
		pix_mp->num_planes = q_data->fmt->num_planes;
	} else {
		pix_mp->num_planes = q_data->fmt->num_planes;
		pix_mp->pixelformat = q_data->fmt->fourcc;
		fourcc = ctx->q_data[MTK_Q_DATA_SRC].fmt->fourcc;

		if (fourcc == V4L2_PIX_FMT_RV30 ||
			fourcc == V4L2_PIX_FMT_RV40) {
			for (i = 0; i < pix_mp->num_planes; i++) {
				pix_mp->width = 1920;
				pix_mp->height = 1088;
				pix_mp->plane_fmt[i].bytesperline = 1920;
				pix_mp->plane_fmt[i].sizeimage =
					q_data->sizeimage[i];
			}
		} else {
			pix_mp->width = q_data->coded_width;
			pix_mp->height = q_data->coded_height;
			for (i = 0; i < pix_mp->num_planes; i++) {
				pix_mp->plane_fmt[i].bytesperline =
					q_data->bytesperline[i];
				pix_mp->plane_fmt[i].sizeimage =
					q_data->sizeimage[i];
			}
		}

		mtk_v4l2_debug(1, "[%d] type=%d state=%d Format information could not be read, not ready yet!",
				ctx->id, f->type, ctx->state);
	}

	return 0;
}

int vb2ops_vdec_queue_setup(struct vb2_queue *vq,
				unsigned int *nbuffers,
				unsigned int *nplanes,
				unsigned int sizes[],
				struct device *alloc_devs[])
{
	struct mtk_vcodec_ctx *ctx = vb2_get_drv_priv(vq);
	struct mtk_q_data *q_data;
	unsigned int i;

	q_data = mtk_vdec_get_q_data(ctx, vq->type);

	if (q_data == NULL) {
		mtk_v4l2_err("vq->type=%d err\n", vq->type);
		return -EINVAL;
	}

	if (*nplanes) {
		for (i = 0; i < *nplanes; i++) {
			if (sizes[i] < q_data->sizeimage[i])
				return -EINVAL;
		}
	} else {
		if (vq->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
			*nplanes = q_data->fmt->num_planes;
		else
			*nplanes = 1;

		for (i = 0; i < *nplanes; i++)
			sizes[i] = q_data->sizeimage[i];
	}

	mtk_v4l2_debug(1,
			"[%d]\t type = %d, get %d plane(s), %d buffer(s) of size 0x%x 0x%x ",
			ctx->id, vq->type, *nplanes, *nbuffers,
			sizes[0], sizes[1]);

	return 0;
}

int vb2ops_vdec_buf_prepare(struct vb2_buffer *vb)
{
	struct mtk_vcodec_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);
	struct mtk_q_data *q_data;
	int i;
	struct mtk_video_dec_buf *mtkbuf;
	struct vb2_v4l2_buffer *vb2_v4l2;
	unsigned int plane = 0;

	mtk_v4l2_debug(3, "[%d] (%d) id=%d",
			ctx->id, vb->vb2_queue->type, vb->index);

	q_data = mtk_vdec_get_q_data(ctx, vb->vb2_queue->type);

	for (i = 0; i < q_data->fmt->num_planes; i++) {
		if (vb2_plane_size(vb, i) < q_data->sizeimage[i]) {
			mtk_v4l2_err("data will not fit into plane %d (%lu < %d)",
				i, vb2_plane_size(vb, i),
				q_data->sizeimage[i]);
		}
	}

	// Check if need to proceed cache operations
	vb2_v4l2 = container_of(vb, struct vb2_v4l2_buffer, vb2_buf);
	mtkbuf = container_of(vb2_v4l2, struct mtk_video_dec_buf, vb);

	if (!(mtkbuf->flags & NO_CAHCE_CLEAN) &&
		!(ctx->dec_params.svp_mode) &&
		(vb->vb2_queue->memory == VB2_MEMORY_DMABUF)) {
		struct vdec_fb fb_param;

		for (plane = 0; plane < vb->num_planes; plane++) {
			struct vb2_dc_buf *dc_buf = vb->planes[plane].mem_priv;

			mtk_v4l2_debug(4, "[%d] Cache sync+", ctx->id);
			dma_sync_sg_for_device(&ctx->dev->plat_dev->dev,
				dc_buf->dma_sgt->sgl,
				dc_buf->dma_sgt->orig_nents,
				DMA_TO_DEVICE);
			fb_param.fb_base[plane].dma_addr =
				vb2_dma_contig_plane_dma_addr(vb,
				plane);
			if (vb->vb2_queue->type ==
					V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE)
				fb_param.fb_base[plane].size =
					(size_t)vb->planes[0].bytesused;
			else
				fb_param.fb_base[plane].size =
					ctx->picinfo.fb_sz[plane];

			mtk_v4l2_debug(4,
			  "[%d] [%d] [%d]Cache sync- TD for %p sz=%d dev %p",
			  ctx->id,
			  vb->vb2_queue->type,
			  plane,
			  (void *)fb_param.fb_base[plane].dma_addr,
			  (unsigned int)fb_param.fb_base[plane].size,
			  &ctx->dev->plat_dev->dev);
		}
	}

	return 0;
}

void vb2ops_vdec_buf_finish(struct vb2_buffer *vb)
{
	struct mtk_vcodec_ctx *ctx = vb2_get_drv_priv(vb->vb2_queue);
	struct vb2_v4l2_buffer *vb2_v4l2;
	struct mtk_video_dec_buf *buf;
	bool buf_error;
	unsigned int plane = 0;
	struct mtk_video_dec_buf *mtkbuf;

	vb2_v4l2 = container_of(vb, struct vb2_v4l2_buffer, vb2_buf);
	buf = container_of(vb2_v4l2, struct mtk_video_dec_buf, vb);
	mutex_lock(&ctx->lock);
	if (vb->vb2_queue->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		buf->queued_in_v4l2 = false;
		buf->queued_in_vb2 = false;
	}
	buf_error = buf->error;
	mutex_unlock(&ctx->lock);

	if (buf_error) {
		mtk_v4l2_err("Unrecoverable error on buffer.");
		ctx->state = MTK_STATE_ABORT;
	}

	if (vb->vb2_queue->type != V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
		return;

	// Check if need to proceed cache operations for Capture Queue
	vb2_v4l2 = container_of(vb, struct vb2_v4l2_buffer, vb2_buf);
	mtkbuf = container_of(vb2_v4l2, struct mtk_video_dec_buf, vb);

	if (!(mtkbuf->flags & NO_CAHCE_INVALIDATE) &&
		!(ctx->dec_params.svp_mode) &&
		(vb->vb2_queue->memory == VB2_MEMORY_DMABUF)) {
		for (plane = 0; plane < buf->frame_buffer.num_planes; plane++) {
			struct vdec_fb dst_mem;
			struct vb2_dc_buf *dc_buf = vb->planes[plane].mem_priv;

			mtk_v4l2_debug(4, "[%d] Cache sync+", ctx->id);
			dma_sync_sg_for_cpu(&ctx->dev->plat_dev->dev,
				dc_buf->dma_sgt->sgl,
				dc_buf->dma_sgt->orig_nents, DMA_FROM_DEVICE);

			dst_mem.fb_base[plane].dma_addr =
				vb2_dma_contig_plane_dma_addr(vb, plane);
			dst_mem.fb_base[plane].size = ctx->picinfo.fb_sz[plane];

			mtk_v4l2_debug(4,
				"[%d] Cache sync- FD for %p sz=%d dev %p pfb %p",
				ctx->id,
				(void *)dst_mem.fb_base[plane].dma_addr,
				(unsigned int)dst_mem.fb_base[plane].size,
				&ctx->dev->plat_dev->dev,
				&buf->frame_buffer);
		}
	}

}

int vb2ops_vdec_buf_init(struct vb2_buffer *vb)
{
	struct vb2_v4l2_buffer *vb2_v4l2 = container_of(vb,
					struct vb2_v4l2_buffer, vb2_buf);
	struct mtk_video_dec_buf *buf = container_of(vb2_v4l2,
					struct mtk_video_dec_buf, vb);

	if (vb->vb2_queue->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		/* User could use different struct dma_buf*
		 * with the same index & enter this buf_init.
		 * once this buffer buf->used == true will reset in mistake
		 * VB2 use kzalloc for struct mtk_video_dec_buf,
		 * so init could be no need
		 */
		if (buf->used == false)
			buf->queued_in_v4l2 = false;
	} else	if (vb->vb2_queue->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		/* Do not reset EOS for 1st buffer with Early EOS*/
		//buf->lastframe = NON_EOS;
	} else {
		mtk_v4l2_err("%s: unknown queue type", __func__);
		return -EINVAL;
	}

	return 0;
}

int vb2ops_vdec_start_streaming(struct vb2_queue *q, unsigned int count)
{
	struct mtk_vcodec_ctx *ctx = vb2_get_drv_priv(q);

	if (ctx->state == MTK_STATE_FLUSH)
		ctx->state = MTK_STATE_HEADER;

	if (q->type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
		mtk_v4l2_debug(4, "V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE q->num_buffers = %d",
			q->num_buffers);
		ctx->dec_params.total_frame_bufq_count = q->num_buffers;
		ctx->dec_param_change |= MTK_DEC_PARAM_TOTAL_FRAME_BUFQ_COUNT;
	}

	mtk_vdec_set_param(ctx);
	return 0;
}

void vb2ops_vdec_stop_streaming(struct vb2_queue *q)
{
	struct vb2_v4l2_buffer *src_buf = NULL, *dst_buf = NULL;
	struct vb2_v4l2_buffer *last_buf = NULL;
	struct mtk_vcodec_ctx *ctx = vb2_get_drv_priv(q);
	int i = 0;
	int ret;

	mtk_v4l2_debug(3, "[%d] (%d) state=(%x) ctx->decoded_frame_cnt=%d",
			ctx->id, q->type, ctx->state, ctx->decoded_frame_cnt);

	last_buf = &ctx->empty_flush_buf->vb;
	if (q->type == V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE) {
		if (ctx->state >= MTK_STATE_HEADER)
			ctx->dev->vdec_pdata->flush_decoder(ctx);
		while ((src_buf = v4l2_m2m_src_buf_remove(ctx->m2m_ctx))) {
			if (last_buf != src_buf)
				v4l2_m2m_buf_done(src_buf, VB2_BUF_STATE_ERROR);
		}
		return;
	}

	if (ctx->state >= MTK_STATE_HEADER) {

		/* Until STREAMOFF is called on the CAPTURE queue
		 * (acknowledging the event), the driver operates
		 * as if the resolution hasn't changed yet, i.e.
		 * VIDIOC_G_FMT< etc. return previous resolution.
		 * So we update picinfo here
		 */

		mtk_v4l2_debug(2,
				"[%d]-> new(%d,%d), old(%d,%d), real(%d,%d), DPB(%d,%d)",
				ctx->id, ctx->last_decoded_picinfo.pic_w,
				ctx->last_decoded_picinfo.pic_h,
				ctx->picinfo.pic_w, ctx->picinfo.pic_h,
				ctx->last_decoded_picinfo.buf_w,
				ctx->last_decoded_picinfo.buf_h,
				ctx->dpb_size, ctx->last_dpb_size);
		ctx->picinfo = ctx->last_decoded_picinfo;
		ctx->dpb_size = ctx->last_dpb_size;

		ret = ctx->dev->vdec_pdata->flush_decoder(ctx);
		if (ret)
			mtk_v4l2_err("DecodeFinal failed, ret=%d", ret);
	}
	ctx->state = MTK_STATE_FLUSH;

	while ((dst_buf = v4l2_m2m_dst_buf_remove(ctx->m2m_ctx))) {
		for (i = 0; i < dst_buf->vb2_buf.num_planes; i++)
			vb2_set_plane_payload(&dst_buf->vb2_buf, i, 0);
		v4l2_m2m_buf_done(dst_buf, VB2_BUF_STATE_ERROR);
	}

}

static void m2mops_vdec_device_run(void *priv)
{
	struct mtk_vcodec_ctx *ctx = priv;
	struct mtk_vcodec_dev *dev = ctx->dev;

	queue_work(dev->decode_workqueue, &ctx->decode_work);
}

static int m2mops_vdec_job_ready(void *m2m_priv)
{
	struct mtk_vcodec_ctx *ctx = m2m_priv;

	mtk_v4l2_debug(3, "[%d]", ctx->id);

	if (ctx->state == MTK_STATE_ABORT)
		return 0;

	if ((ctx->last_decoded_picinfo.pic_w != ctx->picinfo.pic_w) ||
	    (ctx->last_decoded_picinfo.pic_h != ctx->picinfo.pic_h)	||
	    (ctx->last_dpb_size != ctx->dpb_size))
		return 0;

	if (ctx->state != MTK_STATE_HEADER)
		return 0;

	return 1;
}

int mtk_vdec_g_v_ctrl(struct v4l2_ctrl *ctrl)
{
	struct mtk_vcodec_ctx *ctx = ctrl_to_ctx(ctrl);
	int ret = 0;
	static unsigned int value;
	struct mtk_color_desc *color_desc;

	switch (ctrl->id) {
	case V4L2_CID_MIN_BUFFERS_FOR_CAPTURE:
		if (ctx->state >= MTK_STATE_HEADER) {
			ctrl->val = ctx->dpb_size;
		} else {
			mtk_v4l2_debug(0, "Seqinfo not ready");
			ctrl->val = 0;
		}
		break;
	case V4L2_CID_MPEG_MTK_FRAME_INTERVAL:
		if (vdec_if_get_param(ctx,
				GET_PARAM_FRAME_INTERVAL, &value) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot get param",
				ctx->id);
			ret = -EINVAL;
		}
		ctrl->p_new.p_u32 = &value;
		mtk_v4l2_debug(2, "V4L2_CID_MPEG_MTK_FRAME_INTERVAL val = %u",
			*(ctrl->p_new.p_u32));
		break;
	case V4L2_CID_MPEG_MTK_COLOR_DESC:
		color_desc = (struct mtk_color_desc *)ctrl->p_new.p_u32;
		if (vdec_if_get_param(ctx,
				GET_PARAM_COLOR_DESC, color_desc) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot get param",
				ctx->id);
			ret = -EINVAL;
		}
	break;
	case V4L2_CID_MPEG_MTK_ASPECT_RATIO:
		if (vdec_if_get_param(ctx,
				GET_PARAM_ASPECT_RATIO, &ctrl->val) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot get param",
				ctx->id);
			ret = -EINVAL;
		}
	break;
	case V4L2_CID_MPEG_MTK_FIX_BUFFERS:
		if (vdec_if_get_param(ctx,
				GET_PARAM_PLATFORM_SUPPORTED_FIX_BUFFERS,
				&ctrl->val) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot get param",
				ctx->id);
			ret = -EINVAL;
		}
	break;
	case V4L2_CID_MPEG_MTK_FIX_BUFFERS_SVP:
		if (vdec_if_get_param(ctx,
				GET_PARAM_PLATFORM_SUPPORTED_FIX_BUFFERS_SVP,
				&ctrl->val) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot get param",
				ctx->id);
			ret = -EINVAL;
		}
	break;
	case V4L2_CID_MPEG_MTK_INTERLACING:
		if (vdec_if_get_param(ctx,
				GET_PARAM_INTERLACING, &ctrl->val) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot get param",
				ctx->id);
			ret = -EINVAL;
		}
	break;
	case V4L2_CID_MPEG_MTK_CODEC_TYPE:
		if (vdec_if_get_param(ctx,
				GET_PARAM_CODEC_TYPE, &ctrl->val) != 0) {
			mtk_v4l2_err("[%d] Error!! Cannot get param",
				ctx->id);
			ret = -EINVAL;
		}
	break;
	default:
		ret = -EINVAL;
	}
	return ret;
}

const struct v4l2_m2m_ops mtk_vdec_m2m_ops = {
	.device_run	= m2mops_vdec_device_run,
	.job_ready	= m2mops_vdec_job_ready,
};

const struct v4l2_ioctl_ops mtk_vdec_ioctl_ops = {
	.vidioc_streamon	= v4l2_m2m_ioctl_streamon,
	.vidioc_streamoff	= v4l2_m2m_ioctl_streamoff,
	.vidioc_reqbufs		= v4l2_m2m_ioctl_reqbufs,
	.vidioc_querybuf	= v4l2_m2m_ioctl_querybuf,
	.vidioc_expbuf		= v4l2_m2m_ioctl_expbuf,

	.vidioc_qbuf		= vidioc_vdec_qbuf,
	.vidioc_dqbuf		= vidioc_vdec_dqbuf,

	.vidioc_try_fmt_vid_cap_mplane	= vidioc_try_fmt_vid_cap_mplane,
	.vidioc_try_fmt_vid_out_mplane	= vidioc_try_fmt_vid_out_mplane,

	.vidioc_s_fmt_vid_cap_mplane	= vidioc_vdec_s_fmt,
	.vidioc_s_fmt_vid_out_mplane	= vidioc_vdec_s_fmt,
	.vidioc_g_fmt_vid_cap_mplane	= vidioc_vdec_g_fmt,
	.vidioc_g_fmt_vid_out_mplane	= vidioc_vdec_g_fmt,

	.vidioc_create_bufs		= v4l2_m2m_ioctl_create_bufs,

	.vidioc_enum_fmt_vid_cap_mplane	= vidioc_vdec_enum_fmt_vid_cap_mplane,
	.vidioc_enum_fmt_vid_out_mplane	= vidioc_vdec_enum_fmt_vid_out_mplane,
	.vidioc_enum_framesizes	= vidioc_enum_framesizes,

	.vidioc_querycap		= vidioc_vdec_querycap,
	.vidioc_subscribe_event		= vidioc_vdec_subscribe_evt,
	.vidioc_unsubscribe_event	= v4l2_event_unsubscribe,
	.vidioc_g_selection             = vidioc_vdec_g_selection,
	.vidioc_s_selection             = vidioc_vdec_s_selection,

	.vidioc_decoder_cmd = vidioc_decoder_cmd,
	.vidioc_try_decoder_cmd = vidioc_try_decoder_cmd,
};

int mtk_vcodec_dec_queue_init(void *priv, struct vb2_queue *src_vq,
			   struct vb2_queue *dst_vq)
{
	struct mtk_vcodec_ctx *ctx = priv;
	int ret = 0;

	mtk_v4l2_debug(3, "[%d]", ctx->id);

	src_vq->type		= V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE;
	src_vq->io_modes	= VB2_DMABUF | VB2_MMAP;
	src_vq->drv_priv	= ctx;
	src_vq->buf_struct_size = sizeof(struct mtk_video_dec_buf);
	src_vq->allow_zero_bytesused = 1;
	src_vq->ops		= ctx->dev->vdec_pdata->vdec_vb2_ops;
#ifdef CONFIG_VB2_MEDIATEK_DMA
	src_vq->mem_ops         = &mtk_dma_contig_memops;
	mtk_v4l2_debug(4, "src_vq use mtk_dma_contig_memops");
#else
	src_vq->mem_ops         = &vb2_dma_contig_memops;
	mtk_v4l2_debug(4, "src_vq use vb2_dma_contig_memops");
#endif
	src_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	src_vq->lock		= &ctx->dev->dev_mutex;
	src_vq->dev             = &ctx->dev->plat_dev->dev;

	ret = vb2_queue_init(src_vq);
	if (ret) {
		mtk_v4l2_err("Failed to initialize videobuf2 queue(output)");
		return ret;
	}
	dst_vq->type		= V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	dst_vq->io_modes	= VB2_DMABUF | VB2_MMAP;
	dst_vq->drv_priv	= ctx;
	dst_vq->buf_struct_size = sizeof(struct mtk_video_dec_buf);
	dst_vq->allow_zero_bytesused = 1;
	dst_vq->ops		= ctx->dev->vdec_pdata->vdec_vb2_ops;
#ifdef CONFIG_VB2_MEDIATEK_DMA
	dst_vq->mem_ops         = &mtk_dma_contig_memops;
	mtk_v4l2_debug(4, "dst_vq use mtk_dma_contig_memops");
#else
	dst_vq->mem_ops         = &vb2_dma_contig_memops;
	mtk_v4l2_debug(4, "dst_vq use vb2_dma_contig_memops");
#endif
	dst_vq->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_COPY;
	dst_vq->lock		= &ctx->dev->dev_mutex;
	dst_vq->dev             = &ctx->dev->plat_dev->dev;

	ret = vb2_queue_init(dst_vq);
	if (ret) {
		vb2_queue_release(src_vq);
		mtk_v4l2_err("Failed to initialize videobuf2 queue(capture)");
	}

	return ret;
}
