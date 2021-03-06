// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#define _CRT_SECURE_NO_WARNINGS

#ifdef _DEBUG
//#pragma comment(lib,"opencv_world341d.lib")
#pragma comment(lib,"IlmImfd.lib")
#pragma comment(lib,"ippicvmt.lib")
#pragma comment(lib,"ippiwd.lib")
#pragma comment(lib,"ittnotifyd.lib")
#pragma comment(lib,"libjasperd.lib")
#pragma comment(lib,"libjpegd.lib")
#pragma comment(lib,"libpngd.lib")
#pragma comment(lib,"libprotobufd.lib")
#pragma comment(lib,"libtiffd.lib")
#pragma comment(lib,"libwebpd.lib")
#pragma comment(lib,"opencv_aruco341d.lib")
#pragma comment(lib,"opencv_bgsegm341d.lib")
#pragma comment(lib,"opencv_bioinspired341d.lib")
#pragma comment(lib,"opencv_calib3d341d.lib")
#pragma comment(lib,"opencv_ccalib341d.lib")
#pragma comment(lib,"opencv_core341d.lib")
#pragma comment(lib,"opencv_datasets341d.lib")
#pragma comment(lib,"opencv_dnn341d.lib")
#pragma comment(lib,"opencv_dnn_objdetect341d.lib")
#pragma comment(lib,"opencv_dpm341d.lib")
#pragma comment(lib,"opencv_face341d.lib")
#pragma comment(lib,"opencv_features2d341d.lib")
#pragma comment(lib,"opencv_flann341d.lib")
#pragma comment(lib,"opencv_fuzzy341d.lib")
#pragma comment(lib,"opencv_hfs341d.lib")
#pragma comment(lib,"opencv_highgui341d.lib")
#pragma comment(lib,"opencv_imgcodecs341d.lib")
#pragma comment(lib,"opencv_imgproc341d.lib")
#pragma comment(lib,"opencv_img_hash341d.lib")
#pragma comment(lib,"opencv_line_descriptor341d.lib")
#pragma comment(lib,"opencv_ml341d.lib")
#pragma comment(lib,"opencv_objdetect341d.lib")
#pragma comment(lib,"opencv_optflow341d.lib")
#pragma comment(lib,"opencv_phase_unwrapping341d.lib")
#pragma comment(lib,"opencv_photo341d.lib")
#pragma comment(lib,"opencv_plot341d.lib")
#pragma comment(lib,"opencv_reg341d.lib")
#pragma comment(lib,"opencv_rgbd341d.lib")
#pragma comment(lib,"opencv_saliency341d.lib")
#pragma comment(lib,"opencv_shape341d.lib")
#pragma comment(lib,"opencv_stereo341d.lib")
#pragma comment(lib,"opencv_stitching341d.lib")
#pragma comment(lib,"opencv_structured_light341d.lib")
#pragma comment(lib,"opencv_superres341d.lib")
#pragma comment(lib,"opencv_surface_matching341d.lib")
#pragma comment(lib,"opencv_text341d.lib")
#pragma comment(lib,"opencv_tracking341d.lib")
#pragma comment(lib,"opencv_video341d.lib")
#pragma comment(lib,"opencv_videoio341d.lib")
#pragma comment(lib,"opencv_videostab341d.lib")
#pragma comment(lib,"opencv_xfeatures2d341d.lib")
#pragma comment(lib,"opencv_ximgproc341d.lib")
#pragma comment(lib,"opencv_xobjdetect341d.lib")
#pragma comment(lib,"opencv_xphoto341d.lib")
#pragma comment(lib,"zlibd.lib")

#else
//#pragma comment(lib,"opencv_world341.lib")
#pragma comment(lib,"IlmImf.lib") 
#pragma comment(lib,"ippicvmt.lib") 
#pragma comment(lib,"ippiw.lib") 
#pragma comment(lib,"ittnotify.lib") 
#pragma comment(lib,"libjasper.lib") 
#pragma comment(lib,"libjpeg.lib") 
#pragma comment(lib,"libpng.lib") 
#pragma comment(lib,"libprotobuf.lib") 
#pragma comment(lib,"libtiff.lib") 
#pragma comment(lib,"libwebp.lib") 
#pragma comment(lib,"opencv_aruco341.lib") 
#pragma comment(lib,"opencv_bgsegm341.lib") 
#pragma comment(lib,"opencv_bioinspired341.lib") 
#pragma comment(lib,"opencv_calib3d341.lib") 
#pragma comment(lib,"opencv_ccalib341.lib") 
#pragma comment(lib,"opencv_core341.lib") 
#pragma comment(lib,"opencv_datasets341.lib") 
#pragma comment(lib,"opencv_dnn341.lib") 
#pragma comment(lib,"opencv_dnn_objdetect341.lib") 
#pragma comment(lib,"opencv_dpm341.lib") 
#pragma comment(lib,"opencv_face341.lib") 
#pragma comment(lib,"opencv_features2d341.lib") 
#pragma comment(lib,"opencv_flann341.lib") 
#pragma comment(lib,"opencv_fuzzy341.lib") 
#pragma comment(lib,"opencv_hfs341.lib") 
#pragma comment(lib,"opencv_highgui341.lib") 
#pragma comment(lib,"opencv_imgcodecs341.lib") 
#pragma comment(lib,"opencv_imgproc341.lib") 
#pragma comment(lib,"opencv_img_hash341.lib") 
#pragma comment(lib,"opencv_line_descriptor341.lib") 
#pragma comment(lib,"opencv_ml341.lib") 
#pragma comment(lib,"opencv_objdetect341.lib") 
#pragma comment(lib,"opencv_optflow341.lib") 
#pragma comment(lib,"opencv_phase_unwrapping341.lib") 
#pragma comment(lib,"opencv_photo341.lib") 
#pragma comment(lib,"opencv_plot341.lib") 
#pragma comment(lib,"opencv_reg341.lib") 
#pragma comment(lib,"opencv_rgbd341.lib") 
#pragma comment(lib,"opencv_saliency341.lib") 
#pragma comment(lib,"opencv_shape341.lib") 
#pragma comment(lib,"opencv_stereo341.lib") 
#pragma comment(lib,"opencv_stitching341.lib") 
#pragma comment(lib,"opencv_structured_light341.lib") 
#pragma comment(lib,"opencv_superres341.lib") 
#pragma comment(lib,"opencv_surface_matching341.lib") 
#pragma comment(lib,"opencv_text341.lib") 
#pragma comment(lib,"opencv_tracking341.lib") 
#pragma comment(lib,"opencv_video341.lib") 
#pragma comment(lib,"opencv_videoio341.lib") 
#pragma comment(lib,"opencv_videostab341.lib") 
#pragma comment(lib,"opencv_xfeatures2d341.lib") 
#pragma comment(lib,"opencv_ximgproc341.lib") 
#pragma comment(lib,"opencv_xobjdetect341.lib") 
#pragma comment(lib,"opencv_xphoto341.lib") 
#pragma comment(lib,"zlib.lib") 
#endif



#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: reference additional headers your program requires here
