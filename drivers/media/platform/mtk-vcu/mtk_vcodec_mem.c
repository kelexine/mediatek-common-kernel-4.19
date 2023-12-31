// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 * Author: Yunfei Dong <yunfei.dong@mediatek.com>
 */

#include "mtk_vcodec_mem.h"
#include <linux/module.h>
#include <linux/scatterlist.h>

/*
 * #undef pr_debug
 * #define pr_debug pr_info
 */

#define MODULE_VCODEC_MEM_DEBUEG pr_debug
#define MODULE_VCODEC_MEM_INFO pr_info

struct mtk_vcu_queue *mtk_vcu_dec_init(struct device *dev)
{
	struct mtk_vcu_queue *vcu_queue;

	MODULE_VCODEC_MEM_DEBUEG("Allocate new vcu queue !\n");
	vcu_queue = kzalloc(sizeof(struct mtk_vcu_queue), GFP_KERNEL);
	if (vcu_queue == NULL) {
		pr_info("Allocate new vcu queue fail!\n");
		return NULL;
	}

	vcu_queue->mem_ops = &vb2_dma_contig_memops;
	vcu_queue->dev = dev;
	vcu_queue->num_buffers = 0;
	mutex_init(&vcu_queue->mmap_lock);

	return vcu_queue;
}

void mtk_vcu_dec_release(struct mtk_vcu_queue *vcu_queue)
{
	struct mtk_vcu_mem *vcu_buffer;
	unsigned int buffer;

	mutex_lock(&vcu_queue->mmap_lock);
	MODULE_VCODEC_MEM_DEBUEG("Release vcu queue !\n");
	if (vcu_queue->num_buffers != 0) {
		for (buffer = 0; buffer < vcu_queue->num_buffers; buffer++) {
			vcu_buffer = &vcu_queue->bufs[buffer];
			vcu_queue->mem_ops->put(vcu_buffer->mem_priv);
			vcu_queue->bufs[buffer].mem_priv = NULL;
			vcu_queue->bufs[buffer].size = 0;
		}
	}
	mutex_unlock(&vcu_queue->mmap_lock);
	kfree(vcu_queue);
	vcu_queue = NULL;
}

void *mtk_vcu_get_buffer(struct mtk_vcu_queue *vcu_queue,
						 struct mem_obj *mem_buff_data)
{
	void *cook, *dma_addr;
	struct mtk_vcu_mem *vcu_buffer;
	unsigned int buffers;

	buffers = vcu_queue->num_buffers;
	if (mem_buff_data->len > CODEC_ALLOCATE_MAX_BUFFER_SIZE ||
		mem_buff_data->len == 0U || buffers >= CODEC_MAX_BUFFER) {
		pr_info("Get buffer fail: buffer len = %u num_buffers = %d !!\n",
			   mem_buff_data->len, buffers);
		return ERR_PTR(-EINVAL);
	}

	mutex_lock(&vcu_queue->mmap_lock);
	vcu_buffer = &vcu_queue->bufs[buffers];
	vcu_buffer->mem_priv = vcu_queue->mem_ops->alloc(vcu_queue->dev, 0,
		mem_buff_data->len, 0, 0);
	vcu_buffer->size = mem_buff_data->len;
	if (IS_ERR(vcu_buffer->mem_priv)) {
		mutex_unlock(&vcu_queue->mmap_lock);
		goto free;
	}

	cook = vcu_queue->mem_ops->vaddr(vcu_buffer->mem_priv);
	dma_addr = vcu_queue->mem_ops->cookie(vcu_buffer->mem_priv);

	mem_buff_data->iova = *(dma_addr_t *)dma_addr;
	mem_buff_data->va = (unsigned long)cook;
	vcu_queue->num_buffers++;
	mutex_unlock(&vcu_queue->mmap_lock);

	MODULE_VCODEC_MEM_DEBUEG("Num_buffers = %d iova = %x va = %llx size = %d mem_priv = %lx\n",
			 vcu_queue->num_buffers, mem_buff_data->iova,
			 mem_buff_data->va,
			 (unsigned int)vcu_buffer->size,
			 (unsigned long)vcu_buffer->mem_priv);
	return vcu_buffer->mem_priv;

free:
	vcu_queue->mem_ops->put(vcu_buffer->mem_priv);

	return ERR_PTR(-ENOMEM);
}

int mtk_vcu_free_buffer(struct mtk_vcu_queue *vcu_queue,
						struct mem_obj *mem_buff_data)
{
	struct mtk_vcu_mem *vcu_buffer;
	void *cook, *dma_addr;
	unsigned int buffer, num_buffers, last_buffer;
	int ret = -EINVAL;

	mutex_lock(&vcu_queue->mmap_lock);
	num_buffers = vcu_queue->num_buffers;
	if (num_buffers != 0U) {
		for (buffer = 0; buffer < num_buffers; buffer++) {
			vcu_buffer = &vcu_queue->bufs[buffer];
			cook = vcu_queue->mem_ops->vaddr(vcu_buffer->mem_priv);
			dma_addr =
				vcu_queue->mem_ops->cookie(
				    vcu_buffer->mem_priv);

			if (mem_buff_data->va == (unsigned long)cook &&
			mem_buff_data->iova == *(dma_addr_t *)dma_addr &&
				mem_buff_data->len == vcu_buffer->size) {
				MODULE_VCODEC_MEM_DEBUEG("Free buff = %d iova = %x va = %llx, queue_num = %d\n",
						 buffer, mem_buff_data->iova,
						 mem_buff_data->va,
						 num_buffers);
				vcu_queue->mem_ops->put(vcu_buffer->mem_priv);
				last_buffer = num_buffers - 1U;
				if (last_buffer != buffer)
					vcu_queue->bufs[buffer] =
						vcu_queue->bufs[last_buffer];
				vcu_queue->bufs[last_buffer].mem_priv = NULL;
				vcu_queue->bufs[last_buffer].size = 0;
				vcu_queue->num_buffers--;
				ret = 0;
				break;
			}
		}
	}
	mutex_unlock(&vcu_queue->mmap_lock);

	if (ret != 0)
		pr_info("Can not free memory va %llx iova %x len %u!\n",
			   mem_buff_data->va, mem_buff_data->iova,
			   mem_buff_data->len);

	return ret;
}


int vcu_buffer_flush_all(struct device *dev, struct mtk_vcu_queue *vcu_queue)
{
	struct mtk_vcu_mem *vcu_buffer;
	unsigned int buffer, num_buffers;
	void *dma_addr = NULL;
	void *cook = NULL;
	struct vb2_dc_buf *buf;
	struct dma_buf *dbuf;
	struct dma_buf_attachment *buf_att;
	struct sg_table *sgt;

	num_buffers = vcu_queue->num_buffers;
	if (num_buffers != 0U) {
		for (buffer = 0; buffer < num_buffers; buffer++) {
			vcu_buffer = &vcu_queue->bufs[buffer];
			dma_addr =
				vcu_queue->mem_ops->cookie(
					vcu_buffer->mem_priv);
			cook = vcu_queue->mem_ops->vaddr(vcu_buffer->mem_priv);
			MODULE_VCODEC_MEM_DEBUEG("Cache flush buffer=%d, iova=%lx, va=%p, size=%d, Q_num = %d\n",
				buffer, *(unsigned int long *)dma_addr,
				vcu_queue->mem_ops->vaddr(vcu_buffer->mem_priv),
				(unsigned int)vcu_buffer->size, num_buffers);

			dbuf = vcu_queue->mem_ops->get_dmabuf(
				((struct vb2_dc_buf *)vcu_buffer->mem_priv),
				O_ACCMODE);

			buf_att = dma_buf_attach(dbuf, vcu_queue->dev);
			sgt = dma_buf_map_attachment(buf_att, DMA_TO_DEVICE);

			if (!sgt) {
				pr_info("can't sync cache for NULL sgt");
				break;
			}

			MODULE_VCODEC_MEM_DEBUEG("sgt :0x%08x", sgt);

			dma_sync_sg_for_device(buf->dev, sgt->sgl,
				sgt->orig_nents,
				DMA_TO_DEVICE);

			dma_buf_unmap_attachment(buf_att, sgt, DMA_TO_DEVICE);
			dma_buf_detach(dbuf, buf_att);
			dma_buf_put(dbuf);

		}
	}
	return 0;
}

int vcu_buffer_cache_sync(struct device *dev, struct mtk_vcu_queue *vcu_queue,
	dma_addr_t dma_addr, size_t size, int op)
{
	struct mtk_vcu_mem *vcu_buffer;
	unsigned int buffer, num_buffers;
	dma_addr_t *iova = NULL;
	void *cook = NULL;
	struct vb2_dc_buf *buf;
	struct dma_buf *dbuf;
	struct dma_buf_attachment *buf_att;
	struct sg_table *sgt;

	num_buffers = vcu_queue->num_buffers;
	if (num_buffers != 0U) {
		for (buffer = 0; buffer < num_buffers; buffer++) {
			vcu_buffer = &vcu_queue->bufs[buffer];
			iova = vcu_queue->mem_ops->cookie(vcu_buffer->mem_priv);
			cook = vcu_queue->mem_ops->vaddr(vcu_buffer->mem_priv);
			buf = vcu_buffer->mem_priv;
			sgt = buf->dma_sgt;

			if (*iova == dma_addr) {
				MODULE_VCODEC_MEM_DEBUEG("Cache %s buffer, iova = %p, *iova = 0x%08x, size = %d, vcu_buffer->size = %d\n",
					(op == DMA_TO_DEVICE) ?
					"flush" : "invalidate",
					(void *)dma_addr,
					*iova, (unsigned int)size,
					(unsigned int)vcu_buffer->size);

				if (op == DMA_TO_DEVICE) {
					dbuf = vcu_queue->mem_ops->get_dmabuf(
						((struct vb2_dc_buf *)
						vcu_buffer->mem_priv),
						O_ACCMODE);

					buf_att =
						dma_buf_attach(dbuf,
						vcu_queue->dev);
					sgt =
						dma_buf_map_attachment(buf_att,
						DMA_TO_DEVICE);

					if (!sgt) {
						pr_info("can't sync cache for NULL sgt");
						break;
					}

					MODULE_VCODEC_MEM_DEBUEG("sgt :0x%08x",
						sgt);

					dma_sync_sg_for_device(buf->dev,
						sgt->sgl,
						sgt->orig_nents,
						DMA_TO_DEVICE);

					dma_buf_unmap_attachment(buf_att, sgt,
						DMA_TO_DEVICE);
					dma_buf_detach(dbuf, buf_att);
					dma_buf_put(dbuf);
				} else {

					dbuf = vcu_queue->mem_ops->get_dmabuf(
						((struct vb2_dc_buf *)
						vcu_buffer->mem_priv),
						O_ACCMODE);

					buf_att = dma_buf_attach(dbuf,
						vcu_queue->dev);
					sgt = dma_buf_map_attachment(buf_att,
						DMA_FROM_DEVICE);


					if (!sgt) {
						pr_info("can't invalidate cache for NULL sgt");
						break;
					}

					MODULE_VCODEC_MEM_DEBUEG("sgt :0x%08x",
						sgt);

					dma_sync_sg_for_cpu(buf->dev, sgt->sgl,
						sgt->orig_nents,
						DMA_FROM_DEVICE);

					dma_buf_unmap_attachment(buf_att, sgt,
						DMA_FROM_DEVICE);
					dma_buf_detach(dbuf, buf_att);
					dma_buf_put(dbuf);
				}
				break;
			}
		}
		if (buffer == num_buffers) {
			MODULE_VCODEC_MEM_DEBUEG("Cache %s buffer fail, iova = %p, size = %d, Not VCU allocated\n",
				(op == DMA_TO_DEVICE) ? "flush" : "invalidate",
				(void *)dma_addr, (unsigned int)size);
		}
	}
	return 0;
}

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Mediatek Video driver memory helper functions");
