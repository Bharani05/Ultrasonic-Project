/*
        SIC LABORATORY, LG ELECTRONICS INC., SEOUL, KOREA
        Copyright(c) 2016 by LG Electronics Inc.

        This program is free software; you can redistribute it and/or
        modify it under the terms of the GNU General Public License
        version 2 as published by the Free Software Foundation.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
        GNU General Public License for more details.
*/

/** @file
 *
 *  Brief description.
 *  Detailed description starts here.
 *
 *  @author             harish.singh
 *  @version            1.0
 *  @date               2019-04-24
 *  @note               Additional information.
 */

/*----------------------------------------------------------------------------------------
        Control Constants
----------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
        File Inclusions
----------------------------------------------------------------------------------------*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include <media/videobuf2-v4l2.h>
#include <media/v4l2-ctrls.h>
#include <media/videobuf2-vmalloc.h>
#include "linux/v4l2-ext/v4l2-ext-renderer.h"

#include "os_util.h"
#include "base_device.h"
#include "v4l2_dolby.h"
#include "de_def.h"
#include "de_kapi.h"
#include "de_io.h"
#include "de_sync.h"
#include "v4l2_vsc_event.h"
#include "dolby_io.h"


// =====================================================================

struct dolby_dev {
        struct v4l2_device      v4l2_dolby_dev;
        struct video_device     v4l2_dolby_video_dev;
        struct mutex        v4l2_dolby_video_device_lock;
        struct list_head        vb_dolby_cap_list;
        struct vb2_queue    vb_dolby_cap_q;
        spinlock_t                      slock;
        struct task_struct *dolby_cap_thread;
        BOOLEAN             _IsCapThreadRunning;
};


struct dolby_cap_buffer {
        /* common v4l buffer stuff -- must be first */
        struct vb2_v4l2_buffer vb;
        struct list_head list;
};


struct dolby_fh {
        struct v4l2_fh fh;
// need to add some private data
        struct dolby_priv *dolby_priv;
};

static struct v4l2_ext_dolby_gd_delay_param g_stdolbyGDVal;
static struct v4l2_ext_dolby_picture_mode g_stPictureMode = {1,1};//default for  bright mode for pioneer
static struct v4l2_ext_dolby_picture_data g_stPictureData = {0,1,100};//default for backlight value for pionner
static struct v4l2_ext_dolby_ambient_light_param g_stAmbientData;
static int g_nPDStatus;
static int g_dbgBackligthValue;
static int g_dbgBrightnessValue;
static int g_dbgContrastValue;
static int g_dbgColorValue;

char *g_cfgpaths[3] = {NULL,NULL,NULL};
char _g_strBestPQPath[MAX_NAME_LENGTH];
char _g_strMainPQPath[MAX_NAME_LENGTH];
DOLBY_LEVEL_L11_DATA_T g_slevel_MD;
enum v4l2_ext_dolby_config_type g_eConfigType;
static int _g_dev_opend = 0;
static LX_DOLBY_MODE_T _g_dolby_state = LX_DOLBY_DEFAULT_MODE;
static LX_DOLBY_IO_MAP_T g_sDolbyInitParam;
struct dolby_priv *gp_prvData;
/*Coverity issue 276120 fix*/
static int _gExitStatus = 0;

struct dolby_priv *new_dolby_priv(void);
static int vpq_get_ext_dolby_data(struct v4l2_ext_control *ctrl);
static int v4l2_set_ext_ctrl_dolby_driver(struct v4l2_ext_control *ctrl);
extern void     DOLBY_PROC_Init (void);
static inline struct dolby_cap_buffer *to_dolby_cap_buffer(struct vb2_buffer *vb2)
{
        struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb2);
        return container_of(vbuf, struct dolby_cap_buffer, vb);
}

static char *ctrl_id_ext_to_str(int id)
{
    switch (id)
        {
                case V4L2_CID_EXT_DOLBY_CFG_PATH:
                        return "V4L2_CID_EXT_DOLBY_CFG_PATH";
                case V4L2_CID_EXT_DOLBY_GD_DELAY:
                        return "V4L2_CID_EXT_DOLBY_GD_DELAY";
                case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
                        return "V4L2_CID_EXT_DOLBY_PICTURE_MENU";
                case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
                        return "V4L2_CID_EXT_DOLBY_PICTURE_MODE";
                case V4L2_CID_EXT_DOLBY_SW_VERSION:
                        return "V4L2_CID_EXT_DOLBY_SW_VERSION";
                case V4L2_CID_EXT_VSC_GET_TIMING_INFO:
                        return "V4L2_CID_EXT_VSC_GET_TIMING_INFO";
                case V4L2_CID_EXT_DOLBY_CONTENTS_TYPE:
                        return "V4L2_CID_EXT_DOLBY_CONTENTS_TYPE";
                case V4L2_CID_EXT_DHDR_INIT:
                        return "V4L2_CID_EXT_DHDR_INIT";
                case V4L2_CID_EXT_DHDR_GET_HDMI_MD:
                        return "V4L2_CID_EXT_DHDR_GET_HDMI_MD";
                case V4L2_CID_EXT_DHDR_GET_OTT_MD_INTERRUPT:
                        return "V4L2_CID_EXT_DHDR_GET_OTT_MD_INTERRUPT";
                case V4L2_CID_EXT_DHDR_MD_UNBLOCK:
                        return "V4L2_CID_EXT_DHDR_MD_UNBLOCK";
                case V4L2_CID_EXT_DHDR_UNINIT:
                        return "V4L2_CID_EXT_DHDR_UNINIT";
                case V4L2_CID_EXT_DOLBY_PD_CTRL:
                        return "V4L2_CID_EXT_DOLBY_PD_CTRL";
                default:
                        return "Ext. CID Unkown";
        }
}


static int dolby_notification(struct dolby_priv *p)
{
        struct v4l2_event evt;
        int info = 0x11223344;

        memset(&evt, 0, sizeof(evt));
        evt.type = V4L2_EVENT_CTRL;
        evt.id   = p->event;
        memcpy(evt.u.data, &info, sizeof(info));
        v4l2_event_queue(p->dolby_vdev, &evt);

        DE_NOTI("Post  %x - evt.type[0x%x] evt.id[0x%x]", \
                info, evt.type, evt.id);

        return 0;
}

static void dolby_noti_ws(struct work_struct *work)
{
        struct dolby_priv *p;
        p = container_of(work, struct dolby_priv, dolby_noti_work);
        dolby_notification(p);
}

static void process_poll(struct dolby_priv *p)
{
        queue_work(system_wq, &p->dolby_noti_work);
}

struct dolby_priv *new_dolby_priv(void)
{
        struct dolby_priv *p;

        p = kcalloc(1, sizeof(struct dolby_priv), GFP_KERNEL);
        if(!p)
        {
                DE_ERROR("alloc error for vt priv.");
                return NULL;
        }
        INIT_LIST_HEAD(&p->list);
        init_waitqueue_head(&p->dolby_poll_wq);

        return p;
}


static int v4l2_dolby_fh_open(struct file *filp)
{
        int ret = 0;
        struct video_device *vdev = video_devdata(filp);
        struct dolby_priv *p;
        struct dolby_fh *fh;

        if(_g_dev_opend){
                DE_NOTI("dolby: Already in use...\n");
                return -EBUSY;
        }

        DE_NOTI("dolby: open...\n");

        fh = kzalloc(sizeof(struct dolby_fh), GFP_KERNEL);
        if (!fh)
        {
                DE_ERROR("dolby: allocation of file handle failed \n");
                ret = -ENOMEM;
                return ret;
        }

        p = new_dolby_priv();
        if(!p)
        {
                DE_ERROR("alloc of dolby priv failed");
                ret = -ENOMEM;
                goto fail_fh;
        }

        INIT_WORK(&p->dolby_noti_work, dolby_noti_ws);
        p->process_poll = process_poll;
        p->dolby_vdev = vdev;
        fh->dolby_priv = p;
        filp->private_data = fh;
        v4l2_fh_init(&fh->fh,vdev);
        v4l2_fh_add(&fh->fh);

        _g_dev_opend++;
        gp_prvData = p;
        return ret;
fail_fh:
        kfree(fh);
        return ret;
}

static int v4l2_dolby_fh_release(struct file *file)
{
        int ret = 0;
        struct video_device *vdev = video_devdata(file);
        struct dolby_fh *fh = file->private_data;
        struct dolby_priv *pvt = ((struct dolby_fh *)(file->private_data))->dolby_priv;

        v4l2_fh_del(&fh->fh);
        v4l2_fh_exit(&fh->fh);

        if(pvt){
                kfree(pvt);
                fh->dolby_priv = NULL;

                kfree(fh);
                file->private_data = NULL;
        }
        gp_prvData = NULL;
        _g_dev_opend--;
        _gExitStatus = 1;
        (void) vdev;
        DE_NOTI("Dolby: close\n");
        return ret;
}

static int v4l2_dolby_probe(struct platform_device *pdev)
{
        int ret = RET_OK;
        DE_NOTI("Dolby  %s\n", __func__);

        return ret;
}

static int v4l2_dolby_remove(struct platform_device *pdev)
{
        DE_NOTI("Dolby  %s\n", __func__);
        return 0;
}

static int v4l2_dolby_resume(struct device *dev)
{
        DE_NOTI("%s\n", __func__);
        return 0;
}

static int v4l2_dolby_suspend(struct device *dev)
{
        DE_NOTI("%s\n", __func__);
        return 0;
}

static unsigned int v4l2_dolby_poll(struct file *file, poll_table *wait)
{
        struct video_device *vdev = video_devdata(file);
        struct dolby_fh *fh = (struct dolby_fh *)file->private_data;
        int rc = 0;
        unsigned long req_events = poll_requested_events(wait);

        DE_NOTI("dolby:req_events 0x%lx\n", req_events);

        poll_wait(file,  &fh->fh.wait, wait);

        if (v4l2_event_pending(&fh->fh))
                rc |= POLLPRI;
        (void) vdev;
        return rc;
}



static int dolby_set_ctrl(struct v4l2_control *ctrl)
{
        int ret = 0;

        switch (ctrl->id) {
                case V4L2_CID_EXT_DOLBY_PD_CTRL:
                {
                        ret = vpq_v4l2_set_ctrl_dolby(ctrl);
                }break;
                default:
                {
                        DE_NOTI("Not implemented - %s", \
                                ctrl_id_ext_to_str(ctrl->id));
                        ret = -EINVAL;
                }
        }

        /* TODO: Update the hardware */

        return ret;
}

static int dolby_get_ctrl(struct v4l2_control *ctrl)
{
        int ret = 0;

        switch (ctrl->id) {
        case V4L2_CID_EXT_DOLBY_PD_CTRL:
        {
                ret = vpq_v4l2_get_ctrl_dolby(ctrl);
        }
        break;
        default:
                {
                        DE_NOTI("Not implemented - %s", \
                                ctrl_id_ext_to_str(ctrl->id));
                        return -EINVAL;
                }
        }

        /* TODO: Update the hardware */

        return ret;
}

static int dolby_set_ext_ctrl(struct v4l2_ext_control *ctrl)
{
        int ret = 0;

        switch (ctrl->id) {
        case V4L2_CID_EXT_DOLBY_CFG_PATH:
        case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
        case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
        case V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT:
        case V4L2_CID_EXT_DOLBY_GD_DELAY:
        case V4L2_CID_EXT_DOLBY_CONTENTS_TYPE:
        case V4L2_CID_EXT_DHDR_INIT:
        case V4L2_CID_EXT_DHDR_GET_HDMI_MD:
        case V4L2_CID_EXT_DHDR_GET_OTT_MD_INTERRUPT:
        case V4L2_CID_EXT_DHDR_MD_UNBLOCK:
        case V4L2_CID_EXT_DHDR_UNINIT:
        {
                ret = vpq_v4l2_set_ext_ctrl_dolby(ctrl);
        }
        break;
        default:
                {
                        DE_NOTI("Not implemented - %s", \
                                ctrl_id_ext_to_str(ctrl->id));
                }
                return -EINVAL;
        }

        /* TODO: Update the hardware */

        return ret;
}

int dolby_get_ext_ctrl(struct v4l2_ext_control *ctrl)
{
        int ret = 0;

        switch (ctrl->id) {
        case V4L2_CID_EXT_DOLBY_CFG_PATH:
        case V4L2_CID_EXT_DOLBY_GD_DELAY:
        case V4L2_CID_EXT_DOLBY_SW_VERSION:
        case V4L2_CID_EXT_VSC_GET_TIMING_INFO:
        case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
        case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
        case V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT:
        case V4L2_CID_EXT_DOLBY_CONTENTS_TYPE:
        {
                ret = vpq_get_ext_dolby_data(ctrl);
        }
        break;
        default:
                return -EINVAL;
        }
        return ret;
}


static int v4l2_dolby_vidioc_querycap(struct file *file, void *__fh, struct v4l2_capability *cap)
{
//      struct vsc_fh *fh = __fh;

        strncpy(cap->card, "v4l2_dolby", sizeof(cap->card)-1);
        strncpy(cap->driver, "v4l2_dolby", sizeof(cap->driver)-1);
        cap->bus_info[0] = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,4,0)
        cap->device_caps = V4L2_CAP_VIDEO_CAPTURE;
        cap->capabilities = cap->device_caps | V4L2_CAP_DEVICE_CAPS;
#endif

        DE_NOTI("Query cap !!!!! \n");
        return 0;
}

static int v4l2_dolby_vidioc_s_ctrl(struct file *file, void *__fh,
                                struct v4l2_control *ctrl)
{
        int ret = 0;

        DE_NOTI("vt : id 0x%x (%s), value %d\n", \
                ctrl->id, ctrl_id_ext_to_str(ctrl->id),ctrl->value);

        switch (ctrl->id) {
                case V4L2_CID_EXT_DOLBY_PD_CTRL:
                {
                        ret = dolby_set_ctrl(ctrl);
                }break;
                default:
                        ret = -EINVAL;
        }
        return ret;
}

static int v4l2_dolby_vidioc_g_ctrl(struct file *file, void *__fh,
                                struct v4l2_control *ctrl)
{
        int ret = 0;
        struct vsc_fh *fh = __fh;

        DE_NOTI("vt : id 0x%x (%s), value %d\n", \
                ctrl->id, ctrl_id_ext_to_str(ctrl->id),ctrl->value);

        switch (ctrl->id) {
                case V4L2_CID_EXT_DOLBY_PD_CTRL:
                        {
                                dolby_get_ctrl(ctrl);
                        }break;
                default:
                        ret = -EINVAL;
        }
        (void) fh;
        return ret;
}

static int v4l2_dolby_vidioc_s_ext_ctrls(struct file *file, void *__fh,
                                struct v4l2_ext_controls *ctrls)
{
        int i, ret = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0)
        if (ctrls->which == V4L2_CTRL_CLASS_USER) {
#else
        if (ctrls->ctrl_class == V4L2_CTRL_CLASS_USER) {
#endif
                for (i = 0; i < ctrls->count; i++) {
                        struct v4l2_ext_control *ctrl = ctrls->controls + i;

                        DE_TRACE("vt : id 0x%x (%s)\n", \
                                ctrl->id, ctrl_id_ext_to_str(ctrl->id));

                        ret = dolby_set_ext_ctrl(ctrl);
                        if (ret) {
                                ctrls->error_idx = i;
                                break;
                        }
                }
                return ret;
        }
        return ret;
}

static int v4l2_dolby_vidioc_g_ext_ctrls(struct file *file, void *__fh,
                                struct v4l2_ext_controls *ctrls)
{
        //struct vsc_fh *fh = __fh;
        int i, ret = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0)
        if (ctrls->which == V4L2_CTRL_CLASS_USER) {
#else
        if (ctrls->ctrl_class == V4L2_CTRL_CLASS_USER) {
#endif
                for (i = 0; i < ctrls->count; i++) {
                        struct v4l2_ext_control *ctrl = ctrls->controls + i;

                        DE_TRACE("dolby : id 0x%x (%s)\n", \
                                ctrl->id, ctrl_id_ext_to_str(ctrl->id));

                        ret = dolby_get_ext_ctrl(ctrl);
                        if (ret) {
                                ctrls->error_idx = i;
                                break;
                        }
                }
                return ret;
        }

        return -EINVAL;
}


static int v4l2_dolby_vidioc_subscribe_event(struct v4l2_fh *__fh,
                                const struct v4l2_event_subscription *sub)
{
        int ret = 0;
        ret = v4l2_event_subscribe(__fh, sub, 32, NULL);
        if(!ret)
                DE_NOTI("Event:: %d Subscribed",sub->id);
        else
                DE_NOTI("Event:: %d Subscription failed",sub->id);
        return ret;
}

static int v4l2_dolby_vidioc_unsubscribe_event(struct v4l2_fh *__fh,
                                const struct v4l2_event_subscription *sub)
{
//      int ret = 0;
        return v4l2_event_unsubscribe(__fh, sub);
}


static struct platform_device v4l2_dolby_pdev = {
        .name = "v4l2_dolby",
        .id = -1,
};

static struct dev_pm_ops const dolby_pm_ops = {
        .resume = v4l2_dolby_resume,
        .suspend = v4l2_dolby_suspend,
};

static struct platform_driver v4l2_dolby_driver = {
        .probe      = v4l2_dolby_probe,
        .driver     = {
                .name   = "v4l2_dolby",
                .owner  = THIS_MODULE,
                .pm     = &dolby_pm_ops,
        },
        .remove         = v4l2_dolby_remove,
};

struct v4l2_file_operations v4l2_dolby_fops = {
        .owner          = THIS_MODULE,
        .open           = v4l2_dolby_fh_open,
        .release        = v4l2_dolby_fh_release,
        .unlocked_ioctl = video_ioctl2,
//      .read       = vb2_fop_read,
        .mmap       = vb2_fop_mmap,
        .poll       = v4l2_dolby_poll,
};


struct v4l2_ioctl_ops v4l2_dolby_ioctl_ops = {
        .vidioc_querycap                = v4l2_dolby_vidioc_querycap,
        .vidioc_s_ctrl                  = v4l2_dolby_vidioc_s_ctrl,
        .vidioc_g_ctrl                  = v4l2_dolby_vidioc_g_ctrl,
        .vidioc_s_ext_ctrls             = v4l2_dolby_vidioc_s_ext_ctrls,
        .vidioc_g_ext_ctrls             = v4l2_dolby_vidioc_g_ext_ctrls,
        .vidioc_log_status              = v4l2_ctrl_log_status,
        .vidioc_subscribe_event = v4l2_dolby_vidioc_subscribe_event,
        .vidioc_unsubscribe_event = v4l2_dolby_vidioc_unsubscribe_event,
};

int v4l2_dolby_register(void *arg)
{
        int ret = 0;
    struct dolby_dev *dev;
    struct video_device *vfd;
    struct vb2_queue *q;

        platform_driver_register(&v4l2_dolby_driver);
        platform_device_register(&v4l2_dolby_pdev);

    DE_NOTI("dolby  %s\n", __func__);
        (void) q;
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    do{
        ret = v4l2_device_register(&v4l2_dolby_pdev.dev, &dev->v4l2_dolby_dev);
        if(ret < 0){
            DE_NOTI("dolby : v4l2_device_register error[%d]\n", ret);
            return ret;
        }

        spin_lock_init(&dev->slock);
        mutex_init(&dev->v4l2_dolby_video_device_lock);
        INIT_LIST_HEAD(&dev->vb_dolby_cap_list);

        // win_0
        vfd = &dev->v4l2_dolby_video_dev;
        strlcpy(vfd->name, "dolby-metadata-dev", sizeof(vfd->name));
        vfd->v4l2_dev  = &dev->v4l2_dolby_dev;
        vfd->fops      = &v4l2_dolby_fops;
        vfd->ioctl_ops = &v4l2_dolby_ioctl_ops;
        vfd->release   = video_device_release_empty;
#if 0 //No need queue
        vfd->queue     = &dev->vb_dolby_cap_q;                //Link vb_queue with videodevice
#endif
        vfd->lock      = &dev->v4l2_dolby_video_device_lock;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0)
        vfd->device_caps = V4L2_CAP_VIDEO_CAPTURE;
#endif
        video_set_drvdata(vfd, dev);

        ret = video_register_device(vfd, VFL_TYPE_GRABBER, V4L2_EXT_DEV_NO_DHDR);
        if (ret < 0) {
            DE_NOTI("video device register error: %d\n", ret);
            return -1;
        }
                DOLBY_PROC_Init();
    } while(0);

        return ret;
}


static inline struct dolby_priv *file_prvdata(struct file *file){
        return ((struct dolby_fh *)(file->private_data))->dolby_priv;
}

int vsc_v4l2_dolby_event( UINT8 win,UINT32 event_id,struct vsc_dolby_data *pInfo)
{
        struct v4l2_event hdr_event;
        enum v4l2_ext_vsc_input_src input_type;
        SINT32 hdr_type ;

        if(!_g_dev_opend || !pInfo || !pInfo->ctrl || !gp_prvData) {
                DE_ERROR("NUll file handler Error!!!!\n");
                return RET_ERROR;
        }

        hdr_type = pInfo->ctrl->value;
        input_type = (enum v4l2_ext_vsc_input_src)pInfo->extra[VSC_SRC_TYPE_INDEX];
        hdr_event.type = V4L2_EVENT_CTRL;
        DE_NOTI("hdr type = %d input_type = %d\n",hdr_type,input_type);


        if(input_type == V4L2_EXT_VSC_INPUT_SRC_HDMI || input_type == V4L2_EXT_VSC_INPUT_SRC_VDEC) {
                /*Disconnect if already connected to Dolby mode*/
                if(_g_dolby_state != LX_DOLBY_DEFAULT_MODE) {
                        hdr_event.id = V4L2_CID_EXT_VSC_SUBSCRIBE_DHDR_STOP;
                        hdr_event.u.data[0] = 0xFF;
                        hdr_event.u.data[1] = win;
                        _g_dolby_state = LX_DOLBY_DEFAULT_MODE;
                        v4l2_event_queue(gp_prvData->dolby_vdev, &hdr_event);
                }
                switch(hdr_type)
                {
                        case V4L2_EXT_VSC_HDR_TYPE_DOLBY:
                                {
                                        if(input_type == V4L2_EXT_VSC_INPUT_SRC_HDMI) {
                                                hdr_event.id = V4L2_CID_EXT_VSC_SUBSCRIBE_DHDR_START;
                                                hdr_event.u.data[0] = 2;
                                                _g_dolby_state = LX_DOLBY_HDMI_MODE;
                                        }
                                        else if(input_type == V4L2_EXT_VSC_INPUT_SRC_VDEC)
                                        {
                                                hdr_event.id = V4L2_CID_EXT_VSC_SUBSCRIBE_DHDR_START;
                                                hdr_event.u.data[0] = 0;
                                                DE_NOTI("INPUT IS OTT##########\n");
                                                _g_dolby_state = LX_DOLBY_OTT_MODE;
                                        }
                                        hdr_event.u.data[1] = win;
                                        v4l2_event_queue(gp_prvData->dolby_vdev, &hdr_event);
                                }break;
                        case V4L2_EXT_VSC_HDR_TYPE_DOLBY_RF:
                                {
                                        hdr_event.id = V4L2_CID_EXT_VSC_SUBSCRIBE_DHDR_START;
                                        hdr_event.u.data[0] = 1;
                                        _g_dolby_state = LX_DOLBY_RF_MODE;
                                        hdr_event.u.data[1] = win;
                                        DE_NOTI("INPUT IS RF##########\n");
                                        v4l2_event_queue(gp_prvData->dolby_vdev, &hdr_event);
                                }break;
                        case V4L2_EXT_VSC_HDR_TYPE_DOLBY_LL:
                                {
                                        hdr_event.id = V4L2_CID_EXT_VSC_SUBSCRIBE_DHDR_START;
                                        hdr_event.u.data[0] = 3;
                                        _g_dolby_state = LX_DOLBY_HDMI_LL_MODE;
                                        hdr_event.u.data[1] = win;
                                        v4l2_event_queue(gp_prvData->dolby_vdev, &hdr_event);
                                }break;
                        default:
                                {
                                        DE_NOTI("Dolby Status[%d],Set to HDR Mode = %d(0-SDR,1-HDR10,2-Dolby,3-HLG,4-PRIME,5-Dolby_LL,6 to 8-RESERVED)\\n",_g_dolby_state,hdr_type);
                                }
                }
                DE_NOTI("DOLBY VSC EVENTS Are success fully posted!!!!\n");
        }
        else {
                DE_NOTI("Wrong input type(%d) for DOlby HDR COnnection/Disconnection so not posting any vsc events\n",input_type);
        }

        return RET_OK;
}

int vpq_v4l2_dolby_event(UINT8 win,UINT32 event_id,void *pData)
{
        struct v4l2_event pq_dolby_event;
        struct v4l2_ext_dolby_picture_mode *st_PictureMode = NULL;
        struct v4l2_ext_dolby_picture_data *pictureData = NULL;
        struct v4l2_ext_dolby_ambient_light_param *ambData = NULL;
        UINT32 unrawdata[3] = {0,};

        if(!gp_prvData){
                DE_NOTI("Dolby driver is not opened yet");
                return -EINVAL;
        }
        pq_dolby_event.type = V4L2_EVENT_CTRL;

        switch(event_id){
                case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
                {
                        st_PictureMode = (struct v4l2_ext_dolby_picture_mode *)pData;
                        pq_dolby_event.id = V4L2_CID_EXT_VPQ_SUBSCRIBE_DHDR_PQ_MODE;
                        pq_dolby_event.u.data[0] = (UINT8)st_PictureMode->uPictureMode;
                        pq_dolby_event.u.data[1] = (UINT8)win;
                        pq_dolby_event.u.data[2] = (UINT8)st_PictureMode->bOnOff;
                        DE_NOTI("Posting PQ Mode EVENT %d %d############",pq_dolby_event.u.data[0],st_PictureMode->bOnOff);
                }break;
                case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
                {
                        pictureData = (struct v4l2_ext_dolby_picture_data *)pData;
                        pq_dolby_event.id = V4L2_CID_EXT_VPQ_SUBSCRIBE_DHDR_PQ_SUBMODE;
                        pq_dolby_event.u.data[0] = (UINT8 )(pictureData->picture_menu);
                        pq_dolby_event.u.data[1] = (UINT8 )(pictureData->setting_value);
                        pq_dolby_event.u.data[2] = (UINT8)win;
                        pq_dolby_event.u.data[3] = (UINT8)(pictureData->on_off);
                        DE_NOTI("Posting PQ SUB Mode EVENT ############");
                }break;
                case V4L2_CID_EXT_DOLBY_CFG_PATH:
                {
                        pq_dolby_event.id = V4L2_CID_EXT_VPQ_SUBSCRIBE_DHDR_CONFIG;
                        pq_dolby_event.u.data[0] = win;
                        pq_dolby_event.u.data[1] = (UINT8 )g_eConfigType;
                        DE_NOTI("Posting PQ CONFIG EVENT ############");
                }break;
                case V4L2_CID_EXT_DOLBY_GD_DELAY:
                {
                        pq_dolby_event.id = V4L2_CID_EXT_VPQ_SUBSCRIBE_DHDR_GD_DELAY;
                        pq_dolby_event.u.data[0] = win;
                }break;
                case V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT:
                {
                        ambData = (struct v4l2_ext_dolby_ambient_light_param *)&g_stAmbientData;
                        memcpy((void *)&unrawdata[0],(void *)pData,sizeof(unrawdata));
                        pq_dolby_event.id = V4L2_CID_EXT_VPQ_SUBSCRIBE_DHDR_PQ_AMBIENT;
                        pq_dolby_event.u.data[0] = (UINT8)(unrawdata[1] & 0xFF);
                        pq_dolby_event.u.data[1] = (UINT8)((unrawdata[1]>>8) & 0xFF);
                        pq_dolby_event.u.data[2] = (UINT8)((unrawdata[1]>>16) & 0xFF);
                        pq_dolby_event.u.data[3] = (UINT8)((unrawdata[1]>>24) & 0xFF);
                        pq_dolby_event.u.data[4] = (UINT8)(ambData->onoff & 0xFF);
                        pq_dolby_event.u.data[5] = (UINT8)((ambData->onoff>>8) & 0xFF);
                        pq_dolby_event.u.data[6] = (UINT8)((ambData->onoff>>16) & 0xFF);
                        pq_dolby_event.u.data[7] = (UINT8)((ambData->onoff>>24) & 0xFF);
                        pq_dolby_event.u.data[8] = (UINT8)(ambData->luxdata & 0xFF);
                        pq_dolby_event.u.data[9] = (UINT8)((ambData->luxdata>>8) & 0xFF);
                        pq_dolby_event.u.data[10] = (UINT8)((ambData->luxdata>>16) & 0xFF);
                        pq_dolby_event.u.data[11] = (UINT8)((ambData->luxdata>>24) & 0xFF);
                        pq_dolby_event.u.data[12] = win;
                }break;
                case V4L2_CID_EXT_DOLBY_PD_CTRL:
                {
                        pq_dolby_event.id = V4L2_CID_EXT_VPQ_SUBSCRIBE_DHDR_PQ_PD;
                        pq_dolby_event.u.data[0] = *((UINT8 *)pData);
                        pq_dolby_event.u.data[1] = win;
                }break;
                default:
                {
                        DE_ERROR("Unknown CID");
                }break;
        }

        v4l2_event_queue(gp_prvData->dolby_vdev, &pq_dolby_event);
        if(pq_dolby_event.id != V4L2_CID_EXT_VPQ_SUBSCRIBE_DHDR_PQ_AMBIENT)
                DE_NOTI("#######EVENTS are successfully posted\n");
        return RET_OK;
}

int vpq_v4l2_set_ctrl_dolby(struct v4l2_control *ctrl)
{
        int nRetVal = RET_OK;
        LX_DOLBY_IOCTL_IPC_FEATURE_T sParams;
        void *arg = NULL;
        int win_id = 0;

        switch(ctrl->id){
                case V4L2_CID_EXT_DOLBY_PD_CTRL:
                {
                        sParams.ucWinID = 0;
                        sParams.ucFlag = (UINT8)ctrl->value;
                        sParams.eDoviFeature = LX_DOVI_FEATURE_PD;
                        arg = &(sParams.ucFlag);

                        if((g_stPictureMode.uPictureMode == HDR_MOVIE_DARK || g_stPictureMode.uPictureMode == HDR_MOVIE_GAME) && sParams.ucFlag){
                                //Don't set PD on for dark mode and game mode
                                return -EBUSY;
                        }
                        if(DE_IO_SetIPCPDMode((unsigned long)&sParams,1)!=0){
                                DE_ERROR("Setting PR IPC failed");
                                nRetVal = -EBUSY;
                        }
                        else {
                                nRetVal = vpq_v4l2_dolby_event(win_id,ctrl->id,arg);
                                g_nPDStatus = ctrl->value;
                        }
                }break;
                default:
                        DE_ERROR("%s::  %s",__func__,ctrl_id_ext_to_str(ctrl->id));
        }

        return nRetVal;
}

int vpq_v4l2_get_ctrl_dolby(struct v4l2_control *ctrl)
{
        int nRetVal = RET_OK;

        switch(ctrl->id){
                case V4L2_CID_EXT_DOLBY_PD_CTRL:
                {
                        ctrl->value = g_nPDStatus;
                }break;
                default:
                        DE_ERROR("%s::  %s",__func__,ctrl_id_ext_to_str(ctrl->id));
        }

        return nRetVal;
}

int vpq_v4l2_set_ext_ctrl_dolby(struct v4l2_ext_control *ctrl)
{
        int nRetVal = RET_OK;
        void *arg = NULL;
        int nTmpVar;
        UINT32 win_id;
        UINT32 unDolbyEeventCnd = 0;
        struct v4l2_ext_dolby_picture_mode st_PictureMode;
        E_HDR_MOVIE_MODES eHDRMovieMode = HDR_MOVIE_VIVID;
        struct v4l2_ext_dolby_picture_data st_PictureData;
        struct v4l2_ext_dolby_ambient_light_param st_AmbientData;
        struct v4l2_ext_vpq_cmn_data param;
        struct v4l2_ext_dolby_config_path sConfigPath;
        int val;
        UINT32 unrawdata[3] = {0,};
        LX_DOLBY_IOCTL_IPC_FEATURE_T sParams;

        /* Blocking setting of PQ config file path temporarily */
        if (!ctrl || !ctrl->string || !_g_dev_opend || !gp_prvData)
        {
                DE_NOTI("ctrl is null or not implemented\n");
                return -EINVAL;
        }

        switch (ctrl->id) {
                case V4L2_CID_EXT_DOLBY_CFG_PATH:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user(&sConfigPath,(void __user *)param.p_data,param.length))
                        {
                                DE_ERROR("copy_from_user file paths error %p\n",param.p_data);
                                nRetVal = RET_ERROR;
                                break;
                        }
                        else {
                                g_eConfigType = sConfigPath.eConfigType;
                                switch(g_eConfigType)
                                {
                                        case V4L2_EXT_DOLBY_CONFIG_MAIN:
                                        {
                                                memcpy((void *)&_g_strMainPQPath[0],(void *)&sConfigPath.sConfigPath[0],MAX_NAME_LENGTH);
                                                DE_NOTI("MAIN file paths :: %s size  = %d %s \n",sConfigPath.sConfigPath,(int)strlen(sConfigPath.sConfigPath),_g_strMainPQPath);
                                        }break;
                                        case V4L2_EXT_DOLBY_CONFIG_BEST:
                                        {
                                                memcpy((void *)&_g_strBestPQPath[0],(void *)&sConfigPath.sConfigPath[0],MAX_NAME_LENGTH);
                                                DE_NOTI("BEST file paths :: %s size  = %d %s \n",sConfigPath.sConfigPath,(int)strlen(sConfigPath.sConfigPath),_g_strBestPQPath);
                                        }break;
                                        default:
                                        {
                                                DE_ERROR("Unknown COnfig File type\n");
                                                return -EINVAL;
                                        }break;
                                }
                        }
                        arg = (void *)&g_eConfigType;
                        win_id = param.wid;
                }break;
                case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user((void *)&st_PictureMode,(void __user *)param.p_data,param.length))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        //DE_NOTI("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Dolby Picture Mode 0x%x %d %d!!!!!!!!!!!!!!!!!!!!!!!!!\n",picmode,*ctrl->string,(int)*ctrl->string);

                        switch(st_PictureMode.uPictureMode)
                        {
                                case MOVIE_DARK:
                                        {
                                                eHDRMovieMode = HDR_MOVIE_DARK;
                                        }break;
                                case MOVIE_BRIGHTNESS:
                                        {
                                                eHDRMovieMode = HDR_MOVIE_BRIGHT;
                                        }break;
                                case VIVID_MODE:
                                        {
                                                eHDRMovieMode = HDR_MOVIE_VIVID;
                                        }break;
                                case STANDRD_MODE:
                                        {
                                                eHDRMovieMode = HDR_MOVIE_STANDARD;
                                        } break;
                                case GAME_MODE:
                                        {
                                                eHDRMovieMode = HDR_MOVIE_GAME;
                                        } break;
                                default:
                                        {
                                                eHDRMovieMode = HDR_MOVIE_VIVID;
                                                DE_NOTI("PQ Setting to default: VIVID[%d]\n",HDR_MOVIE_VIVID);
                                        }break;
                        }
                        st_PictureMode.uPictureMode = eHDRMovieMode;
                        DE_NOTI("picmode = %d eHDRMovieMode = %d \n",st_PictureMode.uPictureMode,eHDRMovieMode);
                        arg = (void *)&st_PictureMode;
                        win_id = param.wid;
                        sParams.ucWinID = win_id;
                        sParams.ucFlag = (UINT8)0;
                        sParams.eDoviFeature = LX_DOVI_FEATURE_PD;
                        memcpy((void *)&g_stPictureMode,(void *)&st_PictureMode,sizeof(st_PictureMode));
                        if((st_PictureMode.uPictureMode == HDR_MOVIE_DARK || st_PictureMode.uPictureMode == HDR_MOVIE_GAME) && g_nPDStatus){
                                /* Set PD off to avoid any error */
                                if(DE_IO_SetIPCPDMode((unsigned long)&sParams,1)!=0){
                                        DE_ERROR("Setting PR IPC failed");
                                        nRetVal = -EBUSY;
                                        return nRetVal;
                                }
                                else {
                                        /* sent event to application also */
                                        nTmpVar = 0;
                                        vpq_v4l2_dolby_event(win_id,V4L2_CID_EXT_DOLBY_PD_CTRL,&nTmpVar);
                                        g_nPDStatus = 0;
                                }
                        }
                }break;
                case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user(&st_PictureData,(void __user *)param.p_data,sizeof(st_PictureData)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        DE_NOTI("DOLBY PQ SUBMODE chnaging for %d %d value = %d #####!!!\n",st_PictureData.picture_menu,st_PictureData.on_off,st_PictureData.setting_value);
                        arg = &st_PictureData;
                        win_id = param.wid;
                        memcpy((void *)&g_stPictureData,(void *)&st_PictureData,sizeof(st_PictureData));
                        switch(st_PictureData.picture_menu)
                        {
                                case V4L2_EXT_DOLBY_BACKLIGHT:
                                {
                                        g_dbgBackligthValue = st_PictureData.setting_value;
                                }break;
                                case V4L2_EXT_DOLBY_BRIGHTNESS:
                                {
                                        g_dbgBrightnessValue = st_PictureData.setting_value;
                                }break;
                                case V4L2_EXT_DOLBY_COLOR:
                                {
                                        g_dbgColorValue = st_PictureData.setting_value;
                                }break;
                                case V4L2_EXT_DOLBY_CONTRAST:
                                {
                                        g_dbgContrastValue = st_PictureData.setting_value;
                                }break;
                                default:
                                {
                                        g_stPictureData.setting_value = 100;
                                }break;
                        }
                }break;
                case V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if (!param.p_data)
                        {
                                DE_ERROR("p_data is null\n");
                                nRetVal = -EINVAL;
                                break;
                        }
                        if(copy_from_user((void *)&st_AmbientData,(void __user *)param.p_data,sizeof(struct v4l2_ext_dolby_ambient_light_param)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user((void *)&unrawdata[0],(void __user *)st_AmbientData.rawdata,sizeof(unrawdata)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        memcpy((void *)&g_stAmbientData,(void *)&st_AmbientData,sizeof(st_AmbientData));
                        arg = &unrawdata[0];
                        win_id = param.wid;
                }break;
                case V4L2_CID_EXT_DOLBY_GD_DELAY:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user((void *)&g_stdolbyGDVal,(void __user *)param.p_data,sizeof(g_stdolbyGDVal)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        arg = &val;
                        win_id = param.wid;
                }break;
                case V4L2_CID_EXT_DOLBY_CONTENTS_TYPE:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_NOTI("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user((void *)&g_slevel_MD,(void __user *)param.p_data,sizeof(g_slevel_MD)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        DE_TRACE("driver Level MD Data %d %d %d %d\n",g_slevel_MD.content_type,\
                                g_slevel_MD.desired_white_point,g_slevel_MD.L11_byte2,g_slevel_MD.L11_byte3);
                }break;
                case V4L2_CID_EXT_DHDR_INIT:
                case V4L2_CID_EXT_DHDR_GET_HDMI_MD:
                case V4L2_CID_EXT_DHDR_GET_OTT_MD_INTERRUPT:
                case V4L2_CID_EXT_DHDR_UNINIT:
                case V4L2_CID_EXT_DHDR_MD_UNBLOCK:
                {
                        if(v4l2_set_ext_ctrl_dolby_driver(ctrl) != RET_OK) {
                                DE_ERROR("dolby driver ioctl error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                }break;
                default:
                {
                        DE_ERROR("Dolby:: Unknow extension ioctl");
                        nRetVal = -EINVAL;
                }break;
        }
        /*condtion to  to call Dolby EVENT when below cid are called from app*/
        unDolbyEeventCnd = ((ctrl->id == V4L2_CID_EXT_DOLBY_PICTURE_MODE) || (ctrl->id == V4L2_CID_EXT_DOLBY_PICTURE_MENU)\
                                || (ctrl->id == V4L2_CID_EXT_DOLBY_CFG_PATH) || (ctrl->id == V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT)\
                                || (ctrl->id == V4L2_CID_EXT_DOLBY_GD_DELAY));
        if(unDolbyEeventCnd)
                nRetVal = vpq_v4l2_dolby_event(win_id,ctrl->id,arg);
        return nRetVal;
}

static int vpq_get_ext_dolby_data(struct v4l2_ext_control *ctrl)
{
        struct v4l2_ext_vpq_cmn_data param;
        struct v4l2_ext_dolby_config_path sConfigPath;
        unsigned char *p_user_data;
        char dolbysoftwareversion[128];
        unsigned char level1md[4];
        int nRetVal = RET_OK;

        /* Blocking setting of PQ config file path temporarily */
        if (!ctrl || !ctrl->string)
        {
                DE_NOTI("ctrl is null or not implemented\n");
                return -EINVAL;
        }

        switch(ctrl->id) {
                case V4L2_CID_EXT_DOLBY_CFG_PATH:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user(&sConfigPath,(void __user *)param.p_data,param.length))
                        {
                                DE_ERROR("copy_from_user file paths error %p\n",param.p_data);
                                nRetVal = RET_ERROR;
                                break;
                        }
                        switch(sConfigPath.eConfigType)
                        {
                                case V4L2_EXT_DOLBY_CONFIG_MAIN:
                                {
                                        memcpy((void *)&sConfigPath.sConfigPath[0],(void *)&_g_strMainPQPath[0],MAX_NAME_LENGTH);
                                        DE_NOTI("MAIN file paths :: %s size  = %d %s \n",sConfigPath.sConfigPath,(int)strlen(sConfigPath.sConfigPath),_g_strMainPQPath);
                                }break;
                                case V4L2_EXT_DOLBY_CONFIG_BEST:
                                {
                                        memcpy((void *)&sConfigPath.sConfigPath[0],(void *)&_g_strBestPQPath[0],MAX_NAME_LENGTH);
                                        DE_NOTI("BEST file paths :: %s size  = %d %s \n",sConfigPath.sConfigPath,(int)strlen(sConfigPath.sConfigPath),_g_strBestPQPath);
                                }break;
                                default:
                                {
                                        DE_ERROR("Unknown COnfig File type\n");
                                        nRetVal = -EINVAL;
                                }break;
                        }
                        if(copy_to_user((void __user *)param.p_data,&sConfigPath,sizeof(sConfigPath)))
                        {
                                DE_ERROR("copy_to_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                }break;
                case V4L2_CID_EXT_DOLBY_GD_DELAY:
                {
                        do {
                                if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                                {
                                        DE_ERROR("copy_from_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                p_user_data = param.p_data;
                                if(copy_to_user((void __user *)p_user_data,&g_stdolbyGDVal,sizeof(g_stdolbyGDVal)))
                                {
                                        DE_ERROR("copy_to_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                else
                                        DE_NOTI("copied GD delay table to user space p_user_data = %p\n",p_user_data);
                        } while(0);
                }break;
                case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
                {
                        do {
                                if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                                {
                                        DE_ERROR("copy_from_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                p_user_data = param.p_data;
                                if(copy_to_user((void __user *)p_user_data,&g_stPictureMode,sizeof(g_stPictureMode)))
                                {
                                        DE_ERROR("copy_to_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                else
                                        DE_NOTI("copied PQMODE %d %d data to user space p_user_data = %p\n",g_stPictureMode.bOnOff,g_stPictureMode.uPictureMode,p_user_data);
                        } while(0);
                }break;
                case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
                {
                        do {
                                if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                                {
                                        DE_ERROR("copy_from_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                p_user_data = param.p_data;
                                if(copy_to_user((void __user *)p_user_data,&g_stPictureData,sizeof(g_stPictureData)))
                                {
                                        DE_ERROR("copy_to_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                else
                                        DE_NOTI("copied PQSUBMODE data to user space p_user_data = %p\n",p_user_data);
                        } while(0);
                }break;
                case V4L2_CID_EXT_DOLBY_SW_VERSION:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        snprintf(dolbysoftwareversion,sizeof(dolbysoftwareversion),"%02d.%02d",K_DOVI_SW_MAJOR_NUM,K_DOVI_SW_MINOR_NUM);
                        p_user_data = param.p_data;
                        if(copy_to_user((void __user *)p_user_data,&dolbysoftwareversion[0],strlen(dolbysoftwareversion)))
                        {
                                DE_ERROR("copy_to_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        else
                                DE_NOTI("copied software version %s to user space p_user_data = %p\n",dolbysoftwareversion,p_user_data);
                }break;
                case V4L2_CID_EXT_VSC_GET_TIMING_INFO:
                {
                        kwrap_vp_timing_info_t sTimingInfo;
                        if(vsc_get_timinginfo(0,&sTimingInfo) != RET_OK) {
                                DE_ERROR("TIming Info ERROR\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        else {
                                if(copy_to_user((void __user *)ctrl->string,(void *)&sTimingInfo, sizeof(sTimingInfo)))
                                {
                                        DE_ERROR("copy_to_user error in TIMING INFO");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                        }
                }break;
                case V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT:
                {
                        do {
                                if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                                {
                                        DE_ERROR("copy_from_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                p_user_data = param.p_data;
                                if(copy_to_user((void __user *)p_user_data,&g_stAmbientData,sizeof(g_stAmbientData)))
                                {
                                        DE_ERROR("copy_to_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                else
                                        DE_NOTI("copied Ambient Data to user space p_user_data = %p\n",p_user_data);
                        } while(0);
                }break;
                case V4L2_CID_EXT_DOLBY_CONTENTS_TYPE:
                {
                        do {
                                if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                                {
                                        DE_ERROR("copy_from_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                p_user_data = param.p_data;
                                level1md[0] = g_slevel_MD.content_type;
                                level1md[1] = g_slevel_MD.desired_white_point;
                                level1md[2] = g_slevel_MD.L11_byte2;
                                level1md[3] = g_slevel_MD.L11_byte3;
                                if(copy_to_user((void __user *)p_user_data,&level1md[0],sizeof(level1md)))
                                {
                                        DE_ERROR("copy_to_user error\n");
                                        nRetVal = RET_ERROR;
                                        break;
                                }
                                else
                                        DE_TRACE("copied Level MD  Data to user space p_user_data = %p with Data %d %d %d %d\n",p_user_data,\
                                        level1md[0],level1md[1],level1md[2],level1md[3]);
                        } while(0);
                }break;
                default:
                {
                        DE_ERROR("Unsupported Control");
                        nRetVal = -EINVAL;
                }break;
        }
        return nRetVal;
}

static int v4l2_set_ext_ctrl_dolby_driver(struct v4l2_ext_control *ctrl)
{
        int nRetVal = RET_OK;
        struct v4l2_ext_vpq_cmn_data param;
        LX_DOLBY_IOCTL_INIT_T sDolbyDrvInitParam;
        LX_DOLBY_IOCTL_INIT_T sDolbyDrvActualInitParam;
        LX_DOLBY_IOCTL_HDMI_MD_T sDolbyHdmiParam;
        LX_DOLBY_IOCTL_HDMI_MD_T sDolbyActualHdmiParam;
        LX_DOLBY_OTT_MD_IOCTL_T sDolbyOttParam;
        LX_DOLBY_DM_MD_TYPE e_HdmiMDtype;
        LX_DOLBY_DM_MD_T sHdmiMetadata = {0,};

        switch(ctrl->id)
        {
                case V4L2_CID_EXT_DHDR_INIT:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user((void *)&sDolbyDrvInitParam,(void __user *)param.p_data,sizeof(LX_DOLBY_IOCTL_INIT_T)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user((void *)&g_sDolbyInitParam,(void __user *)sDolbyDrvInitParam.sptrDolbyInit,sizeof(g_sDolbyInitParam)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        sDolbyDrvActualInitParam.ucWinID = param.wid;
                        sDolbyDrvActualInitParam.sptrDolbyInit = NULL;
                        sDolbyDrvActualInitParam.sptrDolbyInit = kmalloc (sizeof(LX_DOLBY_IO_MAP_T), GFP_KERNEL);
                        memcpy((void *)sDolbyDrvActualInitParam.sptrDolbyInit,(void *)&g_sDolbyInitParam,sizeof(g_sDolbyInitParam));
                        if(DE_DHDR_IP_IO_Init(&sDolbyDrvActualInitParam) != RET_OK) {
                                DE_ERROR("Driver Init ERROR\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_to_user((void __user *)sDolbyDrvInitParam.sptrDolbyInit,(void *)sDolbyDrvActualInitParam.sptrDolbyInit,sizeof(g_sDolbyInitParam)))
                        {
                                DE_ERROR("copy_to_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        kfree(sDolbyDrvActualInitParam.sptrDolbyInit);
                        DE_NOTI("Dolby Driver INIT done \n");
                }break;
                case V4L2_CID_EXT_DHDR_GET_HDMI_MD:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(copy_from_user((void *)&sDolbyHdmiParam,(void __user *)param.p_data,sizeof(LX_DOLBY_IOCTL_HDMI_MD_T)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        sDolbyActualHdmiParam.ucWinID = param.wid;
                        sDolbyActualHdmiParam.eMDType = NULL;
                        sDolbyActualHdmiParam.sptrMetadata = NULL;
                        sDolbyActualHdmiParam.eMDType = kmalloc (sizeof(LX_DOLBY_DM_MD_TYPE), GFP_KERNEL);
                        sDolbyActualHdmiParam.sptrMetadata = kmalloc (sizeof(LX_DOLBY_DM_MD_T), GFP_KERNEL);
                        if(     DE_DHDR_IP_IO_GetHDMIMetaData(&sDolbyActualHdmiParam,&e_HdmiMDtype,&sHdmiMetadata) != RET_OK) {
                                DE_ERROR("Dolby Driver Get HDMI MD ERROR\n");
                                nRetVal = RET_ERROR;
                        }
                        else {
                                if(copy_to_user((void __user *)sDolbyHdmiParam.eMDType,(void *)&e_HdmiMDtype,sizeof(LX_DOLBY_DM_MD_TYPE)))
                                {
                                        DE_ERROR("copy_to_user error\n");
                                        nRetVal = RET_ERROR;
                                }
                                if(g_sDolbyInitParam.eCurMode == LX_DOLBY_HDMI_MODE) {
                                        if(copy_to_user((void __user *)sDolbyHdmiParam.sptrMetadata,(void *)&sHdmiMetadata,sizeof(LX_DOLBY_DM_MD_T)))
                                        {
                                                DE_ERROR("copy_to_user error\n");
                                                nRetVal = RET_ERROR;
                                        }
                                }
                        }
                        kfree(sDolbyActualHdmiParam.sptrMetadata);
                        kfree(sDolbyActualHdmiParam.eMDType);
                }break;
                case V4L2_CID_EXT_DHDR_GET_OTT_MD_INTERRUPT:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        sDolbyOttParam.ucWinID = param.wid;
                        if(DE_DHDR_IP_IO_GetOTTMetaData(&sDolbyOttParam) != RET_OK) {
                                DE_ERROR("Dolby Driver OTT MD ERROR\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                }break;
                case V4L2_CID_EXT_DHDR_MD_UNBLOCK:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(DE_DHDR_IP_INTR_MDReadUnblock(&param.wid) != RET_OK) {
                                DE_ERROR("Dolby Driver UnInit ERROR\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                }break;
                case V4L2_CID_EXT_DHDR_UNINIT:
                {
                        if(copy_from_user(&param,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                        if(DE_DHDR_IP_IO_Uninit(&param.wid) != RET_OK) {
                                DE_ERROR("Dolby Driver UnInit ERROR\n");
                                nRetVal = RET_ERROR;
                                break;
                        }
                }break;
                default:
                {
                        DE_ERROR("Dolby:: Unknown extension ioctl");
                        nRetVal = -EINVAL;
                }break;
        }
        return nRetVal;
}

/**
* pq-status
*    see SCDCR-2425 (SEETV-PQStatus-210619-1011-1722.pdf)
*    see SCDCR-2482 (v4l2_pq_v1.1.7.pdf)
*
* In case of simple integer :
*    CID=value
*    (ex: "LED_DEMOMODE=0")
*
* In case of array :
*    CID={value1,value2,value3,,,,,}
*    (ex: "HLG_Y_GAIN_TBL={0,1,2,3,,,,,,}")
*
* In case of struct :
*    CID={member1:value1, member2:value2, .....}
*    (ex: "MEMC_MOTION_COMP={type:0,de-blur:0,de-judder:10}")
*
* In case of struct which include array :
*    CID={member1:value1, member2:{value1 of member2, value2 of member2, ...}, ....}
*    (ex: "VPQ_GAMUT_MATRIX_POST={gamma:0, degamma:0,matrix:{0,1,2,3,,,,},
*    mux_blend:{mux_l3d_in:1, ....,b4p_lut_x:{0,1,2,1,,,}}}")
*/
int vpq_proc_show_status_dolby(struct seq_file *m, void *v, int id)
{
        int ret = RET_OK;

        if (!m) return 0;

        switch (id) {
                case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
                {
                        switch(g_stPictureMode.uPictureMode)
                        {
                                case HDR_MOVIE_VIVID:
                                {
                                        seq_printf(m, "{bOnOff:%d, uPictureMode:vivid}\n",g_stPictureMode.bOnOff);
                                }break;
                                case HDR_MOVIE_BRIGHT:
                                {
                                        seq_printf(m, "{bOnOff:%d, uPictureMode:cinemahome}\n",g_stPictureMode.bOnOff);
                                }break;
                                case HDR_MOVIE_DARK:
                                {
                                        seq_printf(m, "{bOnOff:%d, uPictureMode:cinema}\n",g_stPictureMode.bOnOff);
                                }break;
                                case HDR_MOVIE_STANDARD:
                                {
                                        seq_printf(m, "{bOnOff:%d, uPictureMode:standard}\n",g_stPictureMode.bOnOff);
                                }break;
                                case HDR_MOVIE_GAME:
                                {
                                        seq_printf(m, "{bOnOff:%d, uPictureMode:game}\n",g_stPictureMode.bOnOff);
                                }break;
                                case HDR_MOVIE_NONE:
                                default:
                                {
                                        seq_printf(m, "{bOnOff:%d, uPictureMode:none}\n",g_stPictureMode.bOnOff);
                                }break;
                        }
                }break;
                case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
                {
                        seq_printf(m, "{backlight:%d, brightness:%d, color:%d, contrast:%d}\n",g_dbgBackligthValue,g_dbgBrightnessValue,g_dbgColorValue,g_dbgContrastValue);
                }break;
                default:
                        seq_printf(m, "{not support}\n");
                        break;
        }

        return ret;
}
int vpq_v4l2_compat_ext_ctrl_dolby(struct v4l2_ext_control *ctrl)
{
        int ret = 0;

        if (!ctrl)
        {
                DE_ERROR("ctrl is null\n");
                return -EINVAL;
        }
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
        #ifdef CONFIG_COMPAT
        switch (ctrl->id) {
                case V4L2_CID_EXT_DOLBY_CFG_PATH:
                case V4L2_CID_EXT_DOLBY_PICTURE_MODE:
                case V4L2_CID_EXT_DOLBY_PICTURE_MENU:
                case V4L2_CID_EXT_DOLBY_SW_VERSION:
                case V4L2_CID_EXT_DOLBY_GD_DELAY:
                case V4L2_CID_EXT_DOLBY_AMBIENT_LIGHT:
                case V4L2_CID_EXT_DOLBY_CONTENTS_TYPE:
                {
                        struct v4l2_ext_vpq_cmn_data arg_buf;
                        if(copy_from_user(&arg_buf,(void __user *)ctrl->string,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_from_user error\n");
                                ret = -EINVAL;
                                break;
                        }
                        arg_buf.p_data = (unsigned char *)(uintptr_t)arg_buf.compat_data;
                        if(copy_to_user((void __user *)ctrl->string,&arg_buf,sizeof(struct v4l2_ext_vpq_cmn_data)))
                        {
                                DE_ERROR("copy_to_user error\n");
                                ret = -EINVAL;
                                break;
                        }
                }break;
                default:
                        break;
        }
        #endif
        #endif
        return ret;
}
