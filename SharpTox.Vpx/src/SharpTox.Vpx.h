// SharpTox.Vpx.h

#pragma once

#include "../vpx/vpx_image.h"
#include <stdint.h>

using namespace System;

namespace SharpTox
{
	namespace Vpx
	{
		public enum class VpxImageFormat
		{
			VPX_IMG_FMT_NONE,
			VPX_IMG_FMT_RGB24,
			VPX_IMG_FMT_RGB32,
			VPX_IMG_FMT_RGB565,
			VPX_IMG_FMT_RGB555,
			VPX_IMG_FMT_UYVY,
			VPX_IMG_FMT_YUY2,
			VPX_IMG_FMT_YVYU,
			VPX_IMG_FMT_BGR24,
			VPX_IMG_FMT_RGB32_LE,
			VPX_IMG_FMT_ARGB,
			VPX_IMG_FMT_ARGB_LE,
			VPX_IMG_FMT_RGB565_LE,
			VPX_IMG_FMT_RGB555_LE,
			VPX_IMG_FMT_YV12 = VPX_IMG_FMT_PLANAR | VPX_IMG_FMT_UV_FLIP | 1,
			VPX_IMG_FMT_I420 = VPX_IMG_FMT_PLANAR | 2,
			VPX_IMG_FMT_VPXYV12 = VPX_IMG_FMT_PLANAR | VPX_IMG_FMT_UV_FLIP | 3,
			VPX_IMG_FMT_VPXI420 = VPX_IMG_FMT_PLANAR | 4,
			VPX_IMG_FMT_I422 = VPX_IMG_FMT_PLANAR | 5,
			VPX_IMG_FMT_I444 = VPX_IMG_FMT_PLANAR | 6,
			VPX_IMG_FMT_444A = VPX_IMG_FMT_PLANAR | VPX_IMG_FMT_HAS_ALPHA | 7
		};

		public ref class VpxImage
		{
		private:
			vpx_image_t* img;

		internal:
			VpxImage(vpx_image_t* img)
			{
				this->img = img;
			}

		public:
			static VpxImage^ Create(VpxImageFormat fmt, unsigned int d_w, unsigned int d_h, unsigned int align)
			{
				vpx_image_t* img = vpx_img_alloc(nullptr, (vpx_img_fmt)fmt, d_w, d_h, align);
				return gcnew VpxImage(img);
			}

			void Free()
			{
				vpx_img_free(img);
			}

			property void* Pointer
			{
				void* get()
				{
					return img;
				}
			}

			property unsigned int w
			{
				unsigned int get()
				{
					return img->w;
				}
			}

			static VpxImage^ FromPointer(void* p)
			{
				return gcnew VpxImage((vpx_image_t*)p);
			}

			property unsigned int h
			{
				unsigned int get()
				{
					return img->h;
				}
			}

			property unsigned int d_w
			{
				unsigned int get()
				{
					return img->d_w;
				}
			}

			property unsigned int d_h
			{
				unsigned int get()
				{
					return img->d_h;
				}
			}

			property int fmt
			{
				int get()
				{
					return img->fmt;
				}
			}

			property int x_chroma_shift
			{
				int get()
				{
					return img->x_chroma_shift;
				}
			}

			property int y_chroma_shift
			{
				int get()
				{
					return img->y_chroma_shift;
				}
			}

			property int strides[int]
			{
				int get(int index)
				{
					return img->stride[index];
				}
			}

			property int bps
			{
				int get()
				{
					return img->bps;
				}
			}

			property void* user_priv
			{
				void* get()
				{
					return img->user_priv;
				}
			}
		};

		public ref class VpxHelper
		{
		private:
			VpxHelper(){}

			/*
				The following 2 functions, Yuv420ToRgb and RgbToYuv420 were taken from the uTox project: https://github.com/notsecure/uTox
				This code is licensed under GPLv3.
			*/
		public:
			static void Yuv420ToRgb(VpxImage^ image, uint8_t* out)
			{
				vpx_image_t* img = (vpx_image_t*)image->Pointer;

				unsigned long int i, j;
				for (i = 0; i < img->d_h; ++i)
				{
					for (j = 0; j < img->d_w; ++j)
					{
						uint8_t *point = out + 4 * ((i * img->d_w) + j);
						int y = img->planes[0][((i * img->stride[0]) + j)];
						int u = img->planes[1][(((i / 2) * img->stride[1]) + (j / 2))];
						int v = img->planes[2][(((i / 2) * img->stride[2]) + (j / 2))];

						int r = (298 * (y - 16) + 409 * (v - 128) + 128) >> 8;
						int g = (298 * (y - 16) - 100 * (u - 128) - 208 * (v - 128) + 128) >> 8;
						int b = (298 * (y - 16) + 516 * (u - 128) + 128) >> 8;
						point[0] = r>255 ? 255 : r<0 ? 0 : r;
						point[1] = g>255 ? 255 : g<0 ? 0 : g;
						point[2] = b>255 ? 255 : b < 0 ? 0 : b;
						point[3] = ~0;
					}
				}
			}

			static void RgbToYuv420(VpxImage^ image, uint8_t *rgb, uint16_t width, uint16_t height)
			{
				uint16_t x, y;
				uint8_t *p;
				uint8_t r, g, b;

				vpx_image_t* img = (vpx_image_t*)image->Pointer;

				uint8_t* plane_y = img->planes[0];
				uint8_t* plane_u = img->planes[1];
				uint8_t* plane_v = img->planes[2];

				for (y = 0; y != height; y += 2) {
					p = rgb;
					for (x = 0; x != width; x++) {
						r = *rgb++;
						g = *rgb++;
						b = *rgb++;
						*plane_y++ = ((66 * r + 129 * g + 25 * b) >> 8) + 16;
					}

					for (x = 0; x != width / 2; x++) {
						r = *rgb++;
						g = *rgb++;
						b = *rgb++;
						*plane_y++ = ((66 * r + 129 * g + 25 * b) >> 8) + 16;

						r = *rgb++;
						g = *rgb++;
						b = *rgb++;
						*plane_y++ = ((66 * r + 129 * g + 25 * b) >> 8) + 16;

						r = ((int)r + (int)*(rgb - 6) + (int)*p + (int)*(p + 3) + 2) / 4; p++;
						g = ((int)g + (int)*(rgb - 5) + (int)*p + (int)*(p + 3) + 2) / 4; p++;
						b = ((int)b + (int)*(rgb - 4) + (int)*p + (int)*(p + 3) + 2) / 4; p++;

						*plane_u++ = ((-38 * r + -74 * g + 112 * b) >> 8) + 128;
						*plane_v++ = ((112 * r + -94 * g + -18 * b) >> 8) + 128;

						p += 3;
					}
				}
			}
		};
	}
}
