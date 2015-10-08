/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006-2013 Belledonne Communications, Grenoble

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#include "mediastreamer2/mediastream.h"
#include "mediastreamer2/dtmfgen.h"
#include "mediastreamer2/msfileplayer.h"
#include "mediastreamer2/msfilerec.h"
#include "mediastreamer2/msrtp.h"
#include "mediastreamer2/mstonedetector.h"
#include "private.h"
#include "mediastreamer2_tester.h"
#include "mediastreamer2_tester_private.h"

#include <stdio.h>
#include "CUnit/Basic.h"


static int tester_init(void) {
/*	ms_init();
	ms_filter_enable_statistics(TRUE);
	ortp_init();*/
	return 0;
}

static int tester_cleanup(void) {
/*	ms_exit();*/
	return 0;
}

static void filter_register_tester(void) {
	MSFilter* filter;

	ms_init();
	ms_init();

	CU_ASSERT_PTR_NOT_NULL(ms_filter_lookup_by_name("MSVoidSource"));
	filter= ms_filter_create_decoder("pcma");
	CU_ASSERT_PTR_NOT_NULL(filter);
	ms_filter_destroy(filter);

	ms_exit();

	CU_ASSERT_PTR_NOT_NULL(ms_filter_lookup_by_name("MSVoidSource"));
	filter= ms_filter_create_decoder("pcma");
	CU_ASSERT_PTR_NOT_NULL(filter);
	ms_filter_destroy(filter);

	ms_exit();
	CU_ASSERT_PTR_NULL(ms_factory_get_fallback());
}
#ifdef VIDEO_ENABLED
static void test_video_processing (void) {
	MSVideoSize src_size = MS_VIDEO_SIZE_VGA;
	MSVideoSize src_dest = MS_VIDEO_SIZE_VGA;
	mblk_t * yuv_block2;
	YuvBuf yuv;
	int y_bytes_per_row = src_size.width + src_size.width%32 ;
	uint8_t* y = (uint8_t*)ms_malloc(y_bytes_per_row*src_size.height); /*to allow bloc to work with multiple of 32*/
	int crcb_bytes_per_row = src_size.width/2 + (src_size.width/2)%32 ;
	uint8_t* cbcr = (uint8_t*)ms_malloc(crcb_bytes_per_row*src_size.height);
	int i,j;

	for (i=0;i<src_size.height*src_size.width;i++) {
		y[i]=i%256;
	}
	for (i=0;i<src_size.height*src_size.width/2;i++) {
		cbcr[i]=i%256;
	}

	yuv_block2 = copy_ycbcrbiplanar_to_true_yuv_with_rotation_and_down_scale_by_2(	y
																					,cbcr
																					,0
																					, src_size.width
																					, src_size.height
																					, y_bytes_per_row
																					, crcb_bytes_per_row
																					, 1
																					, 0);

	CU_ASSERT_FALSE(ms_yuv_buf_init_from_mblk(&yuv, yuv_block2));

	CU_ASSERT_EQUAL(src_dest.width,yuv.w);
	CU_ASSERT_EQUAL(src_dest.height,yuv.h);

	/*check y*/
	for (i=0;i<yuv.h;i++) {
		for (j=0;j<yuv.w;j++)
		if (yuv.planes[0][i*yuv.strides[0]+j] != y[i*y_bytes_per_row+j]) {
			ms_error("Wrong value  [%i] at ofset [%i], should be [%i]",yuv.planes[0][i*yuv.strides[0]+j],i*yuv.strides[0]+j,y[i*y_bytes_per_row+j]);
			CU_FAIL("bad y value");
			break;
		}
	}

	/*check cb*/
	for (i=0;i<yuv.h/2;i++) {
		for (j=0;j<yuv.w/2;j++)
		if (yuv.planes[1][i*yuv.strides[1]+j] != cbcr[i*crcb_bytes_per_row+2*j]) {
			ms_error("Wrong value  [%i] at ofset [%i], should be [%i]",yuv.planes[1][i*yuv.strides[1]+j],i*yuv.strides[1]+j,y[i*crcb_bytes_per_row+2*j]);
			CU_FAIL("bad cb value");
			break;
		}
	}

	/*check cr*/
	for (i=0;i<yuv.h/2;i++) {
		for (j=0;j<yuv.w/2;j++)
		if (yuv.planes[2][i*yuv.strides[2]+j] != cbcr[i*crcb_bytes_per_row+2*j+1]) {
			ms_error("Wrong value  [%i] at ofset [%i], should be [%i]",yuv.planes[2][i*yuv.strides[2]+j],i*yuv.strides[2]+j,y[i*crcb_bytes_per_row+2*j+1]);
			CU_FAIL("bad cr value");
			break;
		}
	}

	ms_free(y);
	ms_free(cbcr);

}
#endif

static void test_is_multicast(void) {

	CU_ASSERT_TRUE(ms_is_multicast("224.1.2.3"));
	CU_ASSERT_TRUE(ms_is_multicast("239.0.0.0"));
	CU_ASSERT_TRUE(ms_is_multicast("ff02::3:2"));
	CU_ASSERT_FALSE(ms_is_multicast("192.68.0.1"));
	CU_ASSERT_FALSE(ms_is_multicast("::1"));

}
static test_t tests[] = {
	  { "Multiple ms_voip_init", filter_register_tester }
	, { "Is multicast", test_is_multicast}
#ifdef VIDEO_ENABLED
	, { "Video processing function", test_video_processing}
#endif
};

test_suite_t framework_test_suite = {
	"Framework",
	tester_init,
	tester_cleanup,
	sizeof(tests) / sizeof(tests[0]),
	tests
};
