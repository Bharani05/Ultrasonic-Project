@echo off

set TEST_VECTORS_ROOT=D:\Dolby_SDK_IDK\IDK-5.0\MP4_Generation\Test_Vectors
set TEST_VECTORS_SDK=D:\5.0LA_simulation\CinemaHome\vesInputs
set TEST_VECTORS_CUSTOM=D:\5.0LA_simulation\SCDCR-4585_4584


ves2mp4.exe --input-ves D:\Dolby_SDK_IDK\IDK-5.0\Test_Vectors\6302\6302_TestSet_24fps_1920x1080_20mbps_dvhe-stn.265 --output-dir D:\Dolby_SDK_IDK\IDK-5.0\Test_Vectors\6302 --dv-profile dvhe.stn --num-track 1
ves2mp4.exe --input-ves D:\Dolby_SDK_IDK\IDK-5.0\Test_Vectors\6303\6303_ArtGlass_dvhe-stn_24fps_720x480_10bits_10000kbps_hevc.265 --output-dir D:\Dolby_SDK_IDK\IDK-5.0\Test_Vectors\6303 --dv-profile dvhe.stn --num-track 1

::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5305\5305_TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --num-track 1 --dv-profile 5
::ves2mp4.exe --input-ves %TEST_VECTORS_CUSTOM%\DMB_dvhe08-01_3840x2160_5994_10b_16000kbps_L9_Normal_Color_gamut.265 --output-dir %TEST_VECTORS_CUSTOM% --num-track 1 --dv-profile 8.1

::ves2mp4.exe --input-ves %TEST_VECTORS_SDK%\PQ_Obj_AvgAPL_DM3_P3D65_L4max_5s_FHD24_clip.h265 --output-dir %TEST_VECTORS_SDK% --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_SDK%\PQ_Sub_DM3_P3D65_5s_FHD24_clip.h265 --output-dir %TEST_VECTORS_SDK% --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_SDK%\PQ_Sub_DM3_P3D65_5s_FHD24_noL4_clip.h265 --output-dir %TEST_VECTORS_SDK% --dv-profile dvhe.stn --num-track 1

::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5036\5036_ArtGlass_60fps_1920x1080_30000kbps_dvav-ser.264 --output-dir %TEST_VECTORS_ROOT%\5036 --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5037\5037_ArtGlass_60fps_3840x2160_20000kbps_1920x1080_74kbps_dvhe-dtr.265 --output-dir %TEST_VECTORS_ROOT%\5037 --dv-profile dvhe.dtr --num-track 1

::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5003\5003_Zion_120fps_3840x2160_60000kbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5003 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5004\5004_Volcanos_23-976fps_1280x720_5000kbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5004 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5007\5007_TestSet_24fps_1920x1080_20mbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5007 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5009\5009_ArtGlass_60fps_3840x2160_20000kbps_1920x1080_74kbps_dvhe-dtr.265 --output-dir %TEST_VECTORS_ROOT%\5009 --dv-profile dvhe.dtr --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5005\5005_Volcanos_60fps_3840x2160_40000kbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5005 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5106\VES_24fps_720x480_hevc_360x240_hevc_dvhe-dtr.265 --output-dir %TEST_VECTORS_ROOT%\5106 --dv-profile dvhe.dtr --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5030\5030_Volcanos_60fps_3840x2160_30000kbps_dvhe-sth_4000nitsBL.265 --output-dir %TEST_VECTORS_ROOT%\5030 --dv-profile dvhe.dtr --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5035\5035_Tahoe_59-94fps_3840x2160_30000kbps_dvhe-stg.265 --output-dir %TEST_VECTORS_ROOT%\5035 --dv-profile dvhe.stg --num-track 1
::ves2mp4_latest.exe --input-ves %TEST_VECTORS_ROOT%\5035\5035_Tahoe_59-94fps_3840x2160_30000kbps_dvhe-stg.265 --output-dir %TEST_VECTORS_ROOT%\5035 --num-track 1 --dv-profile 8.4
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5039\5039_ArtGlass_60fps_1920x1080_30000kbps_dvav-ser.264 --output-dir %TEST_VECTORS_ROOT%\5039 --dv-profile dvav.ser --num-track 1

::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5260\5260_TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5260 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5280\5280_TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5280 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5290\5290_TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5290 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5106\5106_VES_24fps_720x480_hevc_360x240_hevc_dvhe-dtr.265 --output-dir %TEST_VECTORS_ROOT%\5106 --dv-profile dvhe.dtr --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5320\5320_TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5320 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100a\5100a_VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100a --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100b\5100b_VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100b --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100c\5100c_VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100c --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100d\5100d_VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100d --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100e\5100e_VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100e --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100f\5100f_VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100f --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100g\VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100g --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100h\VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100h --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100i\VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100i --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5100j\VES_24fps_720x480_avc.264 --output-dir %TEST_VECTORS_ROOT%\5100j --dv-profile dvav.ser --num-track 1

::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101a\5101a_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101a --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101b\5101b_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101b --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101c\5101c_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101c --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101d\5101d_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101d --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101e\5101e_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101e --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101f\5101f_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101f --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101g\5101g_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101g --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101h\5101h_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101h --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101i\5101i_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101i --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5101j\5101j_VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5101j --dv-profile dvhe.str --num-track 1

::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102a\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102a --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102b\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102b --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102c\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102c --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102d\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102d --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102e\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102e --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102f\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102f --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102g\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102g --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102h\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102h --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102i\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102i --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5102j\VES_24fps_720x480_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5102j --dv-profile dvhe.stn --num-track 1

::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5210\ArtGlass_dvhe-stn_24fps_720x480_10bits_10000kbps_hevc.265 --output-dir %TEST_VECTORS_ROOT%\5210 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5220\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5220 --dv-profile dvhe.stn --num-track 1
::mp4box -add ..\Test_Materials\5270\TestSet_sdr_24fps_1920x1080_16000kbps_hevc.265 ..\Test_Materials\5270\TestSet_sdr_24fps_1920x1080_16000kbps_hevc.mp4 


::ves2mp4.exe --input-ves ..\Test_Materials\5200\TestSet_dvhe-dtr_24fps_1920x1080_16000kbps_hevc_960x540_30kbps_hevc.265 --output-dir ..\Test_Materials\5200 --dv-profile dvhe.dtr --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5032\5032_Zion_60fps_3840x2160_30000kbps_dvhe-str.265 --output-dir ..\Test_Materials\5032 --dv-profile 
::ves2mp4.exe --input-ves ..\Test_Materials\5290\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5290 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5034\5034_ArtGlass_60fps_3840x2160_30000kbps_dvhe-stg.265 --output-dir ..\Test_Materials\5034 --dv-profile dvhe.stg --num-track 1
::mp4box -add ..\Test_Materials\5230\TestSet_hdr10_24fps_1920x1080_20000kbps_hevc.265 ..\Test_Materials\5230\TestSet_hdr10_24fps_1920x1080_20000kbps_hevc.mp4 
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5002\5002_Volcanos_60fps_6400x3240_80mbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5002 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5005\5005_Volcanos_60fps_3840x2160_40000kbps_dvhe-stn.265 --output-dir ..\Test_Materials\5005 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5522\5522_Graffiti_dvhe-stn_3840x2160_120fps_20mbps_2000frm.265 --output-dir ..\Test_Materials\5522 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5320\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5320 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5540\5540_dvhe-stn_24fps_3840x2160_20mbps.265 --output-dir ..\Test_Materials\5540 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5541\tc01_dvhe-stn_23-976fps_3840x2160_1mbps.265 --output-dir ..\Test_Materials\5541 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5530\5530_VitalFilms_dvhe-stn_3840x2160_60fps_40mbps_0-1799.265 --output-dir ..\Test_Materials\5530 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5501\5501_TestPattern_24fps_3840x2160_1mbps_dvhe-stn.265 --output-dir ..\Test_Materials\5501 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5106\VES_24fps_720x480_hevc_360x240_hevc_dvhe-dtr.265 --output-dir ..\Test_Materials\5106 --dv-profile dvhe.dtr --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5500\5500_TestPattern_24fps_3840x2160_1mbps_dvhe-stn.265 --output-dir ..\Test_Materials\5500 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5531\5531_VitalFilms_dvhe-stn_1920x1080_120fps_25mbps_0-3599.265 --output-dir ..\Test_Materials\5531 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5102j\VES_24fps_720x480_hevc.265 --output-dir ..\Test_Materials\5102j --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5101e\VES_24fps_720x480_hevc.265 --output-dir ..\Test_Materials\5101e --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5101b\VES_24fps_720x480_hevc.265 --output-dir ..\Test_Materials\5101b --dv-profile dvhe.str --num-track 1
::ffmpeg.exe -i ..\Test_Materials\5021\5021_Zion_23-976fps_3840x2160_90mbps_dav1_10_0.ivf -c copy -dv_profile 0 ..\Test_Materials\5021\5021_Zion_23-976fps_3840x2160_90mbps_dav1_10_0.mp4
::ves2mp4.exe --input-ves ..\Test_Materials\5101c\VES_24fps_720x480_hevc.265 --output-dir ..\Test_Materials\5101c --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5101d\VES_24fps_720x480_hevc.265 --output-dir ..\Test_Materials\5101d --dv-profile dvhe.str --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5240\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5240 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5000\5000_Volcanos_60fps_5120x2160_60mbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5000 --dv-profile dvhe.stn --num-track 1

::ves2mp4.exe --input-ves ..\Test_Materials\5038\5038_Zion_60fps_3840x2160_30000kbps_dvhe-str.265 --output-dir ..\Test_Materials\5038 --dv-profile dvhe.str --num-track 1

::ves2mp4.exe --input-ves ..\Test_Materials\5031\5031_Zion_60fps_3840x2160_30000kbps_dvhe-sth_1000nitsBL.265 --output-dir ..\Test_Materials\5031 --dv-profile 
::ves2mp4.exe --input-ves .\Test_Materials\5036\5036_ArtGlass_60fps_1920x1080_30000kbps_dvav-ser.264 --output-dir .\Test_Materials\5036 --dv-profile dvav.ser --num-track 1


::ves2mp4.exe --input-ves ..\Test_Materials\5250\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5250 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5037\5037_ArtGlass_60fps_3840x2160_20000kbps_1920x1080_74kbps_dvhe-dtr.265 --output-dir ..\Test_Materials\5037 --dv-profile dvhe.dtr --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5008\5008_Volcanos_60fps_7680x4320_90mbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5008 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5006\5006_Volcanos_30fps_7680x4320_50mbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5006 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5039\5039_ArtGlass_60fps_1920x1080_30000kbps_dvav-ser.264 --output-dir ..\Test_Materials\5039 --dv-profile dvav.ser --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5001\5001_Volcanos_60fps_5760x3240_70mbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5001 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5260\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5260 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5521\5521_VitalFilms_dvhe-stn_1920x1080_120fps_25mbps_0-3599.265 --output-dir ..\Test_Materials\5521 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5542\tc02_dvhe-stn_23-976fps_3840x2160_1mbps.265 --output-dir ..\Test_Materials\5542 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5520\5520_VitalFilms_dvhe-stn_3840x2160_60fps_40mbps_0-1799.265 --output-dir ..\Test_Materials\5520 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves %TEST_VECTORS_ROOT%\5511\5511_TestPattern_24fps_3840x2160_1mbps_dvhe-stn.265 --output-dir %TEST_VECTORS_ROOT%\5511 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5300\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5300 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5532\5532_Graffiti_dvhe-stn_3840x2160_120fps_20mbps_2000frm.265 --output-dir ..\Test_Materials\5532 --dv-profile dvhe.stn --num-track 1 
::mp4box -add ..\Test_Materials\5330\TestSet_hlg_24fps_1920x1080_20000kbps_hevc.265 ..\Test_Materials\5330\TestSet_hlg_24fps_1920x1080_20000kbps_hevc.mp4 
::ves2mp4.exe --input-ves ..\Test_Materials\5260\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5260 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5280\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5280 --dv-profile dvhe.stn --num-track 1
::ves2mp4.exe --input-ves ..\Test_Materials\5290\TestSet_dvhe-stn_24fps_1920x1080_20000kbps_hevc.265 --output-dir ..\Test_Materials\5290 --dv-profile dvhe.stn --num-track 1

::ffmpeg.exe -i ..\Test_Materials\5023\5023_Graffiti_23-976fps_1920x1080_5800kbps_dav1-10-1.ivf -c copy -dv_profile 1 ..\Test_Materials\5023\5023_Graffiti_23-976fps_1920x1080_5800kbps_dav1-10-1.mp4
::ffmpeg.exe -i ..\Test_Materials\5024\5024_ArtGlass_60fps_3840x2160_30mbps_dav1_10_4.ivf -c copy -dv_profile 4 ..\Test_Materials\5024\5024_ArtGlass_60fps_3840x2160_30mbps_dav1_10_4.mp4
::ffmpeg.exe -i ..\Test_Materials\5025\5025_Graffiti_23-976fps_1920x1080_10mbps_dav1-10-2.ivf -c copy -dv_profile 2 ..\Test_Materials\5025\5025_Graffiti_23-976fps_1920x1080_10mbps_dav1-10-2.mp4
