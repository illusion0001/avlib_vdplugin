#include "gopro.h"
#include <stdio.h>
#include <iterator>
#include <cassert>
#include "Utils.h"

// parts from GoProInfo - HYPOXIC

void GoproInfo::get_camera_type()
{
	static const CameraType types[] = {
		{ "HD6.01", "GoPro HERO6 Black"   },
		{ "HD5.03", "GoPro HERO5 Session" },
		{ "HD5.02", "GoPro HERO5 Black"   },
		{ "HDX.01", "GoPro HERO4 Session" },
		{ "HD4.02", "GoPro HERO4 Black"   },
		{ "HD4.01", "GoPro HERO4 Silver"  },
		{ "HD3.11", "GoPro HERO3+ Black"  },
		{ "HD3.10", "GoPro HERO3+ Silver" },
		{ "HD3.03", "GoPro HERO3 Black"   },
		{ "HD3.02", "GoPro HERO3 Silver"  },
		{ "HD3.01", "GoPro HERO3 White"   },
		{ "HD3.20", "GoPro HERO"          },
		{ "HD3.21", "GoPro HERO+ LCD"     },
		{ "HD3.22", "GoPro HERO+"         },
		{ "HD9.01", "GoPro HERO 9"        },
		{ "H21.01", "GoPro HERO 10"       },
		{ "H23.01", "GoPro HERO 12"       },
	};

	if (firmware.length() >= 6) {
		for (int i = 0; i < std::size(types); i++) {
			if (memcmp(firmware.data(), types[i].Compare, 6) == 0) {
				type = &types[i];
				break;
			}
		}
	}
}

const char* strs_vmode[] = { "Video","Timelapse Video","Looping","Video/Photo" };
const char* strs_fov[] = { "Wide","Medium","Narrow" };

const char* strs_orient[] = { "Up","Down" };

const char* strs_OnOff[] = { "On","Off" };
const char* strs_OffOn[] = { "Off","On" };

const char* strs_pt_wb[] = { "Auto*", "3000K", "5500K", "6500K", "Native" };
const char* strs_pt_color[] = { "Flat","GoPro Color*" };
const char* strs_pt_iso_video[] = { "6400*","3200","1600","800","400" };
const char* strs_pt_sharpness[] = { "Low","Medium","High*" };
const char* strs_pt_ev[] = { "+2","+1.5","+1","+0.5","0","-0.5","-1","-1.5","-2" };
const char str_unknown[] = "Unknown";

const char* DecipherValue(const char** strarray, size_t count, unsigned int value)
{
	if (value < count)
		return(strarray[value]);
	else
		return str_unknown;
}

template <typename... Args>
inline int print_to_string(char* p, const char* fmt, Args&& ...args)
{
	int ret = sprintf(p, fmt, args...);
	assert(ret > 0);
	if (ret > 0) {
		return ret;
	}
	return 0;
};

void GoproInfo::get_settings(unsigned int* sett, int n)
{
	unsigned int sett1 = sett[0];
	unsigned int sett2 = sett[1];

	// setting 1
	unsigned int mode = sett1 & 0xf;
	unsigned int submode = (sett1 >> 4) & ((1 << 4) - 1);
	unsigned int timelapse_rate = (sett1 >> 8) & ((1 << 4) - 1);
	unsigned int orientation = (sett1 >> 16) & ((1 << 1) - 1);
	unsigned int spotmeter = (sett1 >> 17) & ((1 << 1) - 1);
	unsigned int protune = (sett1 >> 30) & ((1 << 1) - 1);
	//unsigned int white_bal = 0;

	// setting 2
	unsigned int fov = (sett2 >> 1) & ((1 << 2) - 1);
	unsigned int lowlight = (sett2 >> 4) & ((1 << 1) - 1);
	unsigned int superview = (sett2 >> 5) & ((1 << 1) - 1);
	unsigned int protune_sharpness = (sett2 >> 6) & ((1 << 2) - 1);
	unsigned int protune_color = (sett2 >> 8) & ((1 << 1) - 1);
	unsigned int protune_iso = (sett2 >> 9) & ((1 << 3) - 1);
	unsigned int protune_ev = (sett2 >> 0xc) & ((1 << 4) - 1);
	unsigned int protune_wb = (sett2 >> 0x10) & ((1 << 2) - 1);
	unsigned int broadcast_privacy = (sett2 >> 0x12) & ((1 << 2) - 1);

	setup_info.resize(1024);
	char* p = setup_info.data();

	// 0 video
	if (mode) {
		p += print_to_string(p, "\tmode:\t%u\r\n", mode);
		p += print_to_string(p, "\tsubmode:\t%u\r\n", submode);
	}
	else {
		p += print_to_string(p, "\tmode:\t%s\r\n", DecipherValue(strs_vmode, std::size(strs_vmode), submode));
	}

	p += print_to_string(p, "\torientation:\t%s\r\n", DecipherValue(strs_orient, std::size(strs_orient), orientation));
	p += print_to_string(p, "\tspotmeter:\t%s\r\n", DecipherValue(strs_OffOn, std::size(strs_OffOn), spotmeter));

	if (n > 1) {
		p += print_to_string(p, "\tfov:\t%s\r\n", DecipherValue(strs_fov, std::size(strs_fov), fov));

		p += print_to_string(p, "\tlowlight:\t%s\r\n", DecipherValue(strs_OffOn, std::size(strs_OffOn), lowlight));
		p += print_to_string(p, "\tsuperview:\t%s\r\n", DecipherValue(strs_OffOn, std::size(strs_OffOn), superview));
	}

	p += print_to_string(p, "\tprotune:\t%s\r\n", DecipherValue(strs_OffOn, std::size(strs_OffOn), protune));
	if (protune && n > 1) {
		p += print_to_string(p, "\tprotune_wb:\t%s\r\n", DecipherValue(strs_pt_wb, std::size(strs_pt_wb), protune_wb));
		p += print_to_string(p, "\tprotune_color:\t%s\r\n", DecipherValue(strs_pt_color, std::size(strs_pt_color), protune_color));
		p += print_to_string(p, "\tprotune_iso:\t%s\r\n", DecipherValue(strs_pt_iso_video, std::size(strs_pt_iso_video), protune_iso));
		p += print_to_string(p, "\tprotune_sharpness:\t%s\r\n", DecipherValue(strs_pt_sharpness, std::size(strs_pt_sharpness), protune_sharpness));
		p += print_to_string(p, "\tprotune_ev:\t%s\r\n", DecipherValue(strs_pt_ev, std::size(strs_pt_ev), protune_ev));
	}

	setup_info.erase(p - setup_info.data(), std::string::npos);
}

void GoproInfo::find_info(const wchar_t* name)
{
	MovParser parser(name);
	while (1) {
		MovAtom a;
		if (!parser.read(a)) return;
		if (a.t == 'moov') {
			while (parser.can_read(a)) {
				MovAtom b;
				if (!parser.read(b)) return;
				if (b.t == 'udta') {
					while (parser.can_read(b)) {
						MovAtom c;
						if (!parser.read(c)) return;

						switch (c.t) {
						case 'FIRM':
						{
							char* fw;
							int size;
							parser.read(c, fw, size, 1);
							firmware = fw;
							get_camera_type();
						}
						break;

						case 'LENS':
							break;

						case 'CAME':
						{
							char* buf;
							int size;
							parser.read(c, buf, size);
							cam_serial.resize(size * 2);
							for (int i = 0; i < size; i++) {
								unsigned val = ((unsigned char*)buf)[i];
								print_to_string(cam_serial.data() + i * 2, "%02x", val);
							}
							free(buf);
						}
						break;

						case 'SETT':
						{
							unsigned int sett[3] = { 0,0,0 };
							int n = 1;
							sett[0] = parser.read4();
							if (parser.can_read(c)) {
								sett[1] = parser.read4();
								n++;
							}
							if (parser.can_read(c)) {
								sett[2] = parser.read4(); // does not exist with GoPRO HERO3
								n++;
							}
							get_settings(sett, n);
						}
						break;

						case 'MUID':
							break;

						case 'HMMT':
							break;
						};

						parser.skip(c);
					}
				}
				parser.skip(b);
			}
			break;
		}
		parser.skip(a);
	}
}
