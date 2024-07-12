#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "afe_kapi.h"
#include "linux/v4l2-ext/v4l2-ext-avd.h"

#define V4L2_FUNC_CHECK(__ret, __checker)	\
{ int tmp_ret = __checker; __ret |= tmp_ret; \
	if(tmp_ret < 0) {printf("'%s' is FAILED in [%d] err[%d].\n", __func__, __LINE__, tmp_ret);}}

struct test_afe_ctx
{
	int fd_afe;
	char* name;
};

struct test_afe_entry
{
	int id;
	int on;
	char* name;
	int (*func)(struct test_afe_ctx *);
};

static int test_afe_run_unit(struct test_afe_ctx *p_ctx);
static int test_afe_run_v4l2_funcs(struct test_afe_ctx *p_ctx);
static int test_afe_run_ntsc_check(struct test_afe_ctx *p_ctx);
static int test_afe_run_pal_check(struct test_afe_ctx *p_ctx);

struct test_afe_entry _g_afe_entry[] = {
	{.id=0,.on=0,.name="run_unit_test",.func=test_afe_run_unit},
	{.id=1,.on=0,.name="run_unit_v4l2_funcs",.func=test_afe_run_v4l2_funcs},
	{.id=2,.on=0,.name="run_unit_ntsc_check",.func=test_afe_run_ntsc_check},
	{.id=3,.on=0,.name="run_unit_pal_check",.func=test_afe_run_pal_check},
#if 0
	{.id=1,.on=0,.name="cvd open/connect RF",.func=test_afe_conn_rf},
	{.id=2,.on=0,.name="cvd open/connect AV",.func=test_afe_conn_av},
	{.id=3,.on=0,.name="cvd disconnect/close",.func=test_afe_disconn},
#endif
	{.name=NULL}
};

static int test_afe_v4l2_open(struct test_afe_ctx *p_ctx) {

	int ret = 0;

	if(p_ctx == NULL)
		return -1;

	if (0/*p_ctx->fd_afe > 0*/) {
		printf("avd is already opend!![%d]\n", p_ctx->fd_afe);
		ret = -1;
	}
	else {
		p_ctx->fd_afe = open(V4L2_EXT_DEV_PATH_AV, O_RDWR);
		printf("open AVD fd[%d]\n", p_ctx->fd_afe);
		if(p_ctx->fd_afe <0)
			return -1;
	}

	return ret;
}

static int test_afe_v4l2_querycap(struct test_afe_ctx *p_ctx) {

	struct v4l2_capability caps;
	int ret = 0;

	if(p_ctx == NULL)
		return -1;
	if (p_ctx->fd_afe <= 0) {
		printf("avd is NOT opend!!\n");
		ret = -1;
	}

	V4L2_FUNC_CHECK(ret, ioctl(p_ctx->fd_afe, VIDIOC_QUERYCAP, &caps));

	printf("VIDIOC_QUERYCAP, ret=%i\n", ret);
	printf("Driver: \"%s\"\n", caps.driver);
	printf("Card: \"%s\"\n", caps.card);
	printf("Version: 0x%x\n", caps.version);
	printf("Capabilities: %08x\n", caps.capabilities);

	return ret;
}

static int test_afe_v4l2_close(struct test_afe_ctx *p_ctx) {
	int ret = 0;

	if(p_ctx == NULL)
		return -1;

	V4L2_FUNC_CHECK(ret, close(p_ctx->fd_afe));
	return ret;
}

static int test_afe_v4l2_connect(struct test_afe_ctx *p_ctx, struct v4l2_control control, enum v4l2_ext_avd_input_src input ) {
	int ret = 0;

	if(p_ctx == NULL)
		return -1;
	if (p_ctx->fd_afe <= 0) {
		printf("avd is NOT opend!!\n");
		ret = -1;
	}
	V4L2_FUNC_CHECK(ret, ioctl(p_ctx->fd_afe, VIDIOC_S_CTRL, &control));
	V4L2_FUNC_CHECK(ret, ioctl(p_ctx->fd_afe, VIDIOC_S_INPUT, &input));
	return ret;
}

static int test_afe_v4l2_disconnect(struct test_afe_ctx *p_ctx) {
	int ret = 0;
	enum v4l2_ext_avd_input_src avd_input_src;

	if(p_ctx == NULL)
		return -1;
	if (p_ctx->fd_afe <= 0) {
		printf("avd is NOT opend!!\n");
		ret = -1;
	}

	avd_input_src = V4L2_EXT_AVD_INPUT_SRC_NONE;
	V4L2_FUNC_CHECK(ret, ioctl(p_ctx->fd_afe, VIDIOC_S_INPUT, &avd_input_src));
		return ret;
}

static int test_afe_v4l2_getTimingInfo(struct test_afe_ctx *p_ctx, struct v4l2_ext_avd_timing_info* avd_timing_info) {
	int ret = 0;
	struct v4l2_ext_controls ext_controls;
	struct v4l2_ext_control ext_control;

	if(p_ctx == NULL)
		return -1;
	if (p_ctx->fd_afe <= 0) {
		printf("avd is NOT opend!!\n");
		ret = -1;
	}

	memset(&ext_controls, 0, sizeof(struct v4l2_ext_controls));
	memset(&ext_control, 0, sizeof(struct v4l2_ext_control));
	memset(avd_timing_info, 0, sizeof(struct v4l2_ext_avd_timing_info));

	ext_controls.ctrl_class = V4L2_CTRL_CLASS_USER;
	ext_controls.count = 1;
	ext_controls.controls = &ext_control;
	ext_controls.controls->id = V4L2_CID_EXT_AVD_TIMING_INFO;
	ext_controls.controls->size = sizeof(struct v4l2_ext_avd_timing_info);
	ext_controls.controls->string = (void *)avd_timing_info;

	V4L2_FUNC_CHECK(ret, ioctl(p_ctx->fd_afe, VIDIOC_G_EXT_CTRLS, &ext_controls));

	return ret;
}


static int test_afe_v4l2_getColorSystem(struct test_afe_ctx *p_ctx, v4l2_std_id *pcolor_system) {

	int ret = 0;

	if(p_ctx == NULL)
		return -1;
	if (p_ctx->fd_afe <= 0) {
		printf("avd is NOT opend!!\n");
		ret = -1;
	}

	V4L2_FUNC_CHECK(ret, ioctl(p_ctx->fd_afe, VIDIOC_G_STD, pcolor_system));

	return ret;
}

#if 0
static int test_afe_v4l2_syncDetectionForTuning(struct test_afe_ctx *p_ctx, struct v4l2_control control) {

	int ret = 0;

	if(p_ctx == NULL)
		return -1;
	if (p_ctx->fd_afe <= 0) {
		printf("avd is NOT opend!!\n");
		ret = -1;
	}

	V4L2_FUNC_CHECK(ret, ioctl(p_ctx->fd_afe, VIDIOC_S_CTRL, &control));

	return ret;
}
#endif

static int test_afe_v4l2_setColorSystem(struct test_afe_ctx *p_ctx, v4l2_std_id v4l2_support_color_system) {

	int ret = 0;

	if(p_ctx == NULL)
		return -1;
	if (p_ctx->fd_afe <= 0) {
		printf("avd is NOT opend!!\n");
		ret = -1;
	}

	if(v4l2_support_color_system == 0)
		return -1;

	V4L2_FUNC_CHECK(ret, ioctl(p_ctx->fd_afe, VIDIOC_S_STD, &v4l2_support_color_system));

	return ret;
}

static int test_afe_v4l2_syncExist(struct test_afe_ctx *p_ctx, BOOLEAN *pcvd_sync) {

	int ret = 0;

	struct v4l2_control control;

	if(p_ctx == NULL)
		return -1;
	if (p_ctx->fd_afe <= 0) {
		printf("avd is NOT opend!!\n");
		ret = -1;
	}

	control.id = V4L2_CID_EXT_AVD_VIDEO_SYNC;
	V4L2_FUNC_CHECK(ret, ioctl(p_ctx->fd_afe, VIDIOC_G_CTRL, &control));

	//	printf("cvd sync check : %d\n", control.value);

	if(control.value == 1)
		*pcvd_sync = TRUE;
	else
		*pcvd_sync = FALSE;

	return ret;
}

static void test_afe_func_on(int id)
{
	struct test_afe_entry *p_entry = _g_afe_entry;
	while (p_entry->name)
	{
		if (p_entry->id == id)
		{
			p_entry->on = 1;
			printf("(%s) id(%d) name (%s)\n",__func__,p_entry->id,p_entry->name);
			break;
		}
		p_entry++;
	}
}

static void print_help(void)
{
	struct test_afe_entry *p_entry = _g_afe_entry;

	printf("(%s, %s)\n",__DATE__,__TIME__);
	printf("$ test_afe <argument> ...\n");
	printf("    arguments:\n");

	while (p_entry->name)
	{
		printf("    -%d : %s\n",p_entry->id,p_entry->name);
		p_entry++;
	}
}

int main (int argc, char **argv)
{
	int ret = 0;
	int opt;
	struct test_afe_ctx ctx;
	struct test_afe_entry *p_entry = _g_afe_entry;

	if (argc<=1)
	{
		print_help();
		return -1;
	}

	while ((opt = getopt(argc, argv, "-0123456789")) != -1)
	{
		switch (opt)
		{
			default:
				printf("invalid arg\n");
				print_help();
				return 1;

			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				printf("opt:%d\n",(opt-'0'));
				test_afe_func_on((opt-'0'));
				break;
		}
	}

	/* run func */
	while (p_entry->name)
	{
		if (p_entry->on == 1 && p_entry->func != NULL)
		{
			ctx.name = p_entry->name;
			if (0 != (ret = p_entry->func(&ctx)))  break;
		}
		p_entry++;
	}

	return ret;
}

static int test_afe_run_unit(struct test_afe_ctx *p_ctx)
{
	int ret = 0;

	/* test 1 : open and close */
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_open(p_ctx));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_close(p_ctx));

	/* test 2 : read querycap */
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_open(p_ctx));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_querycap(p_ctx));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_close(p_ctx));


	/* test 3 : connect to RF and get timing/sync/color system */
	{
		struct v4l2_control control;
		enum v4l2_ext_avd_input_src input;
		struct v4l2_ext_avd_timing_info avd_timing_info;
		v4l2_std_id color_system;
		BOOLEAN cvd_sync;

		V4L2_FUNC_CHECK(ret, test_afe_v4l2_open(p_ctx));

		control.id = V4L2_CID_EXT_AVD_PORT;
		control.value = 1;
		input = V4L2_EXT_AVD_INPUT_SRC_ATV;
		V4L2_FUNC_CHECK(ret, test_afe_v4l2_connect(p_ctx, control, input));
		usleep(1000*1000);
		V4L2_FUNC_CHECK(ret, test_afe_v4l2_getTimingInfo(p_ctx, &avd_timing_info));
		V4L2_FUNC_CHECK(ret, test_afe_v4l2_getColorSystem(p_ctx, &color_system));
		V4L2_FUNC_CHECK(ret, test_afe_v4l2_syncExist(p_ctx, &cvd_sync));
		printf("timing v_freq[%d],h_lock[%d],v_lock[%d]\n", avd_timing_info.v_freq, avd_timing_info.h_lock, avd_timing_info.v_lock );
		printf("color system  : 0x%llx\n", color_system);
		printf("cvd sync  : 0x%x\n", (int)cvd_sync);
		V4L2_FUNC_CHECK(ret, test_afe_v4l2_disconnect(p_ctx));
		V4L2_FUNC_CHECK(ret, test_afe_v4l2_close(p_ctx));
	}

	return ret;
}

static int test_afe_run_v4l2_funcs(struct test_afe_ctx *p_ctx)
{
	int ret = 0;
	struct v4l2_control control;
	enum v4l2_ext_avd_input_src input;
	struct v4l2_ext_avd_timing_info avd_timing_info;
	v4l2_std_id color_system;
	BOOLEAN cvd_sync;

	/* test 1 : open and close */
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_open(p_ctx));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_close(p_ctx));

	/* test 2 : read querycap */
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_open(p_ctx));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_querycap(p_ctx));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_close(p_ctx));

	V4L2_FUNC_CHECK(ret, test_afe_v4l2_open(p_ctx));
	control.id = V4L2_CID_EXT_AVD_PORT;
	control.value = 1;
	input = V4L2_EXT_AVD_INPUT_SRC_ATV;
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_connect(p_ctx, control, input));

	color_system |= V4L2_STD_NTSC;
	color_system |= V4L2_STD_PAL;
	color_system |= V4L2_STD_PAL_Nc;
	color_system |= V4L2_STD_PAL_M;
	color_system |= V4L2_STD_SECAM;
	color_system |= V4L2_STD_NTSC_443;
	color_system |= V4L2_STD_PAL_60;

	V4L2_FUNC_CHECK(ret, test_afe_v4l2_setColorSystem(p_ctx, color_system));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_getColorSystem(p_ctx, &color_system));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_getTimingInfo(p_ctx, &avd_timing_info));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_syncExist(p_ctx, &cvd_sync));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_disconnect(p_ctx));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_close(p_ctx));

	return ret;
}

static int test_afe_run_ntsc_check(struct test_afe_ctx *p_ctx)
{
	int ret = 0;
	struct v4l2_control control;
	enum v4l2_ext_avd_input_src input;
	struct v4l2_ext_avd_timing_info avd_timing_info;
	v4l2_std_id color_system;
	BOOLEAN cvd_sync;

	V4L2_FUNC_CHECK(ret, test_afe_v4l2_open(p_ctx));
	control.id = V4L2_CID_EXT_AVD_PORT;
	control.value = 1;
	input = V4L2_EXT_AVD_INPUT_SRC_ATV;
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_connect(p_ctx, control, input));

	color_system |= V4L2_STD_NTSC;
	color_system |= V4L2_STD_PAL;
	color_system |= V4L2_STD_PAL_Nc;
	color_system |= V4L2_STD_PAL_M;
	color_system |= V4L2_STD_SECAM;
	color_system |= V4L2_STD_NTSC_443;
	color_system |= V4L2_STD_PAL_60;

	V4L2_FUNC_CHECK(ret, test_afe_v4l2_setColorSystem(p_ctx, color_system));
	usleep(1000*1000);
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_getTimingInfo(p_ctx, &avd_timing_info));
	if( (avd_timing_info.v_lock == 0) || (avd_timing_info.h_lock == 0)) {
		printf("cvd not lock:vlock[%d] hlock[%d] [%s][%d]\n", avd_timing_info.v_lock, avd_timing_info.h_lock, __func__, __LINE__);
		return -1;
	}
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_getColorSystem(p_ctx, &color_system));
	if(color_system != V4L2_STD_NTSC) {
		printf("cvd not lock:[0x%llx] [%s][%d]\n", color_system, __func__, __LINE__);
		return -1;
	}
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_syncExist(p_ctx, &cvd_sync));
	if(cvd_sync != TRUE) {
		printf("cvd not lock:[%d] [%s][%d]\n",cvd_sync, __func__, __LINE__);
		return -1;
	}
	printf("timing v_freq[%d],h_lock[%d],v_lock[%d]\n", avd_timing_info.v_freq, avd_timing_info.h_lock, avd_timing_info.v_lock );
	printf("color system  : 0x%llx\n", color_system);
	printf("cvd sync  : 0x%x\n", (int)cvd_sync);
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_disconnect(p_ctx));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_close(p_ctx));

	return ret;
}

static int test_afe_run_pal_check(struct test_afe_ctx *p_ctx)
{
	int ret = 0;
	struct v4l2_control control;
	enum v4l2_ext_avd_input_src input;
	struct v4l2_ext_avd_timing_info avd_timing_info;
	v4l2_std_id color_system;
	BOOLEAN cvd_sync;

	V4L2_FUNC_CHECK(ret, test_afe_v4l2_open(p_ctx));
	control.id = V4L2_CID_EXT_AVD_PORT;
	control.value = 1;
	input = V4L2_EXT_AVD_INPUT_SRC_ATV;
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_connect(p_ctx, control, input));

	color_system |= V4L2_STD_NTSC;
	color_system |= V4L2_STD_PAL;
	color_system |= V4L2_STD_PAL_Nc;
	color_system |= V4L2_STD_PAL_M;
	color_system |= V4L2_STD_SECAM;
	color_system |= V4L2_STD_NTSC_443;
	color_system |= V4L2_STD_PAL_60;

	V4L2_FUNC_CHECK(ret, test_afe_v4l2_setColorSystem(p_ctx, color_system));
	usleep(1000*1000);
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_getTimingInfo(p_ctx, &avd_timing_info));
	if( (avd_timing_info.v_lock == 0) || (avd_timing_info.h_lock == 0)) {
		printf("cvd not lock:vlock[%d] hlock[%d] [%s][%d]\n", avd_timing_info.v_lock, avd_timing_info.h_lock, __func__, __LINE__);
		return -1;
	}
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_getColorSystem(p_ctx, &color_system));
	if(color_system != V4L2_STD_PAL) {
		printf("cvd not lock:[0x%llx] [%s][%d]\n", color_system, __func__, __LINE__);
		return -1;
	}
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_syncExist(p_ctx, &cvd_sync));
	if(cvd_sync != TRUE) {
		printf("cvd not lock:[%d] [%s][%d]\n",cvd_sync, __func__, __LINE__);
		return -1;
	}
	printf("timing v_freq[%d],h_lock[%d],v_lock[%d]\n", avd_timing_info.v_freq, avd_timing_info.h_lock, avd_timing_info.v_lock );
	printf("color system  : 0x%llx\n", color_system);
	printf("cvd sync  : 0x%x\n", (int)cvd_sync);
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_disconnect(p_ctx));
	V4L2_FUNC_CHECK(ret, test_afe_v4l2_close(p_ctx));

	return ret;
}
