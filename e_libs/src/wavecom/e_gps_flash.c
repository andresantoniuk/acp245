/*=============================================================================
        Copyright (c) 2009 by EDANTECH (ILWICK S.A.),Montevideo, URUGUAY

        This software is furnished under a license and may be used and copied
        only in accordance with the terms of such license and with the
        inclusion of the above copyright notice. This software or any other
        copies thereof may not be provided or otherwise made available to any
        other person. No title to and ownership of the software is hereby
        transferred.
==============================================================================*/
#include "e_libs_config.h"

#include "e_gps_flash.h"

#include "adl_global.h"
#include "eRide.h"

#include "e_port.h"
#include "e_errors.h"
#include "e_log.h"

#define NVDATA_FLASH_ID     0

static ascii e_gps_flash_handle[] = "GPSData";

static u8 e_gps_flash[ER_NVDATA_SIZE+1] = {
	/* 0x000 */  0x01,0x00,0x00,0x00,0x82,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,

	/* 0x00C */  0x02,0x00,0x00,0x00,0xB5,0x98,0x26,0x02,0x7C,0xFF,0x51,0x65,0xD8,0x3C,0x57,0x18,
	/* 0x01C */  0x6C,0x84,0xB4,0xB4,0x81,0x45,0xD4,0xF3,0x66,0x76,0x58,0x28,0x00,0x49,0x1F,0x0E,
	/* 0x02C */  0xEF,0xD6,0x26,0xC1,0x65,0xC0,0xE9,0xF3,0x74,0x4D,0x7C,0x51,0xE2,0x9C,0x2F,0x82,
	/* 0x03C */  0xE4,0xCC,0xCC,0xCC,0x9B,0xC8,0x21,0x00,0x78,0xDE,0x11,0x00,0x00,0x90,0x8B,0x01,
	/* 0x04C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x05C */  0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xFE,0x57,0xE5,0x02,0x90,0x3F,0x50,0x65,
	/* 0x06C */  0x44,0xDE,0x57,0x18,0x6C,0x24,0x41,0x5D,0xC1,0x43,0xD4,0xF3,0x66,0xE6,0x93,0x26,
	/* 0x07C */  0x00,0xBC,0x3C,0x3D,0xFD,0xE4,0x01,0x32,0xF5,0x2D,0x0F,0xD8,0xF4,0xEE,0x7B,0x51,
	/* 0x08C */  0x5D,0x0E,0x4B,0xAF,0xE4,0xCC,0xCC,0xCC,0xA8,0x5C,0x1B,0x00,0x78,0xDE,0x11,0x00,
	/* 0x09C */  0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x0AC */  0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xD6,0x34,0xFF,0x02,
	/* 0x0BC */  0x6A,0xCE,0x51,0x65,0x86,0x4E,0x57,0x18,0x6C,0xC3,0xD4,0x2E,0x21,0x41,0xD4,0xF3,
	/* 0x0CC */  0x66,0x56,0xBE,0x25,0x00,0x42,0xCB,0x24,0x89,0x53,0xDC,0x2A,0x31,0x50,0x87,0x2F,
	/* 0x0DC */  0xFC,0xDF,0x7B,0x51,0xA7,0x2E,0x67,0xB5,0xE4,0xCC,0xCC,0xCC,0x2F,0x7E,0x09,0x00,
	/* 0x0EC */  0xF0,0xBC,0x23,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x0FC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,
	/* 0x10C */  0xEC,0xDF,0x88,0x02,0xEA,0x67,0x52,0x65,0x36,0x17,0x57,0x18,0x6C,0x05,0x0B,0x5E,
	/* 0x11C */  0x61,0x43,0xD4,0xF3,0x66,0xE6,0x8D,0x26,0x00,0xA2,0x90,0xA6,0x53,0x95,0xF1,0x10,
	/* 0x12C */  0xAD,0x64,0xB7,0x3D,0x40,0x20,0x7C,0x51,0x3A,0x15,0x6C,0x99,0xE4,0xCC,0xCC,0xCC,
	/* 0x13C */  0x48,0x83,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,
	/* 0x14C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,
	/* 0x15C */  0x02,0x00,0x00,0x00,0xD9,0xDD,0x82,0x02,0xB4,0xB9,0x52,0x65,0xBE,0xF9,0x56,0x18,
	/* 0x16C */  0x6C,0x11,0xA4,0x04,0x21,0x42,0xD4,0xF3,0x66,0x46,0x46,0x26,0x00,0xEC,0x65,0x97,
	/* 0x17C */  0x3C,0x5C,0x62,0x3A,0x08,0x51,0x37,0x1A,0x3A,0x23,0x7C,0x51,0x09,0x2D,0x00,0x98,
	/* 0x18C */  0xE4,0xCC,0xCC,0xCC,0xF2,0x48,0x0C,0x00,0x88,0x21,0xEE,0xFF,0x00,0x90,0x8B,0x01,
	/* 0x19C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x1AC */  0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xEA,0x4A,0xE7,0x01,0xD0,0xF2,0x4F,0x65,
	/* 0x1BC */  0xEE,0xF9,0x57,0x18,0x6C,0xFC,0x4F,0x31,0x81,0x42,0xD4,0xF3,0x66,0x16,0x0D,0x26,
	/* 0x1CC */  0x00,0xAD,0xBC,0xD2,0xD3,0xCE,0xFF,0xC0,0xC2,0x19,0xBC,0xF4,0xA8,0x66,0x7C,0x51,
	/* 0x1DC */  0x6C,0xFE,0x38,0x73,0xE4,0xCC,0xCC,0xCC,0xD8,0xFD,0x1E,0x00,0xD1,0x36,0x6B,0x00,
	/* 0x1EC */  0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x1FC */  0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0x9A,0x3B,0x6B,0x03,
	/* 0x20C */  0x41,0xDC,0x51,0x65,0x8A,0x49,0x57,0x18,0x6C,0xBF,0x43,0x30,0xC1,0x42,0xD4,0xF3,
	/* 0x21C */  0x66,0x86,0x1D,0x26,0x00,0x6D,0x64,0xC9,0x9F,0x03,0xBF,0xC0,0x1A,0x79,0x41,0xF6,
	/* 0x22C */  0xF1,0x9B,0x7B,0x51,0xC8,0xFD,0xF1,0xCE,0xE4,0xCC,0xCC,0xCC,0xAD,0xE3,0x6C,0x00,
	/* 0x23C */  0x1F,0x86,0xB8,0xFF,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x24C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,
	/* 0x25C */  0xEC,0x2A,0x2C,0x03,0xD6,0xE8,0x51,0x65,0x01,0x45,0x57,0x18,0x6C,0x77,0x4C,0xDF,
	/* 0x26C */  0x81,0x46,0xD4,0xF3,0x66,0x56,0xCE,0x27,0x00,0xB3,0x30,0xD3,0xD3,0x18,0xBB,0x17,
	/* 0x27C */  0x35,0xEF,0x8F,0xC4,0xBE,0xC4,0x7B,0x51,0xC9,0x99,0x08,0xC0,0xE4,0xCC,0xCC,0xCC,
	/* 0x28C */  0x89,0x67,0xE2,0xFF,0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,
	/* 0x29C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,
	/* 0x2AC */  0x02,0x00,0x00,0x00,0xAD,0xA9,0x1A,0x06,0x48,0xBA,0x51,0x65,0xC8,0x55,0x57,0x18,
	/* 0x2BC */  0x6C,0x0C,0xE4,0xDB,0xC1,0x44,0xD4,0xF3,0x66,0x06,0x4F,0x27,0x00,0xAD,0xDA,0xC4,
	/* 0x2CC */  0x46,0x87,0xE0,0x3D,0x0F,0xDE,0x58,0x10,0x2C,0x19,0x79,0x51,0x2F,0x1F,0x5F,0x5C,
	/* 0x2DC */  0xE6,0xCC,0xCC,0xCC,0x6C,0x6D,0x12,0x00,0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,
	/* 0x2EC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x2FC */  0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xED,0x09,0x79,0x02,0x8A,0x88,0x50,0x65,
	/* 0x30C */  0xF7,0xC3,0x57,0x18,0x6C,0x70,0x1E,0x89,0x01,0x43,0xD4,0xF3,0x66,0x76,0x83,0x27,
	/* 0x31C */  0x00,0x7A,0x9A,0x56,0x66,0x86,0x57,0x1D,0xB0,0xA5,0xE0,0x38,0x08,0x28,0x7C,0x51,
	/* 0x32C */  0xEB,0xEC,0xAE,0x95,0xE4,0xCC,0xCC,0xCC,0x83,0x09,0x1D,0x00,0x00,0x00,0x00,0x00,
	/* 0x33C */  0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x34C */  0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0x2F,0x56,0x2A,0x02,
	/* 0x35C */  0x2E,0x84,0x51,0x65,0x47,0x69,0x57,0x18,0x6C,0x48,0xBC,0x56,0x61,0x3E,0xD4,0xF3,
	/* 0x36C */  0x66,0xB6,0x6F,0x24,0x00,0x86,0xFB,0xE4,0xE6,0xB2,0x0D,0x1E,0x1F,0x37,0x81,0x38,
	/* 0x37C */  0xDF,0x4B,0x7C,0x51,0xAC,0x50,0x12,0x83,0xE4,0xCC,0xCC,0xCC,0xB6,0x59,0x03,0x00,
	/* 0x38C */  0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x39C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,
	/* 0x3AC */  0xE4,0x62,0x28,0x01,0x2B,0x36,0x51,0x65,0x64,0x85,0x57,0x18,0x6C,0x6B,0x09,0x08,
	/* 0x3BC */  0xC1,0x44,0xD4,0xF3,0x66,0x86,0x2A,0x27,0x00,0xF5,0x9C,0xFD,0x1E,0x5C,0xC5,0xC2,
	/* 0x3CC */  0xA2,0xC3,0xA0,0x12,0x07,0xA0,0x7C,0x51,0x7A,0x89,0x14,0x46,0xE4,0xCC,0xCC,0xCC,
	/* 0x3DC */  0x1A,0x28,0xF3,0xFF,0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,
	/* 0x3EC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,
	/* 0x3FC */  0x02,0x00,0x00,0x00,0x6D,0x2E,0x0B,0x01,0x2A,0x0F,0x51,0x65,0x72,0x93,0x57,0x18,
	/* 0x40C */  0x6C,0x46,0x2A,0xB4,0x01,0x46,0xD4,0xF3,0x66,0xE6,0x83,0x28,0x00,0x39,0xDB,0x50,
	/* 0x41C */  0xCB,0xEB,0xCF,0x3C,0x9F,0x65,0xF2,0x13,0x56,0xA6,0x7C,0x51,0xB0,0x21,0xB3,0xFC,
	/* 0x42C */  0xE2,0xCC,0xCC,0xCC,0x14,0xCA,0x2D,0x00,0x78,0xDE,0x11,0x00,0x00,0x90,0x8B,0x01,
	/* 0x43C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x44C */  0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xBB,0x6A,0xF5,0x00,0x12,0x33,0x52,0x65,
	/* 0x45C */  0x41,0x2A,0x57,0x18,0x6C,0xBD,0x8F,0xB3,0x41,0x45,0xD4,0xF3,0x66,0x56,0x45,0x28,
	/* 0x46C */  0x00,0xE4,0x5C,0x37,0x7C,0x7B,0xBB,0xC7,0x4D,0x40,0x81,0xE1,0xA0,0xAA,0x7C,0x51,
	/* 0x47C */  0x9F,0x1F,0x1C,0xE8,0xE2,0xCC,0xCC,0xCC,0xAA,0xE8,0x03,0x00,0x00,0x00,0x00,0x00,
	/* 0x48C */  0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x49C */  0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x4AC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x4BC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x4CC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x4DC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x4EC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
	/* 0x4FC */  0x70,0x2D,0x48,0x01,0x30,0xC6,0x50,0x65,0xBF,0xAD,0x57,0x18,0x6C,0x7B,0xAB,0x08,
	/* 0x50C */  0xE1,0x44,0xD4,0xF3,0x66,0x36,0x3F,0x27,0x00,0x3E,0xE1,0x93,0x00,0x4C,0xBF,0xD4,
	/* 0x51C */  0xCE,0x09,0x2C,0x2F,0x68,0x98,0x7C,0x51,0x29,0x0E,0x99,0x4D,0xE4,0xCC,0xCC,0xCC,
	/* 0x52C */  0xA7,0x16,0x27,0x00,0x78,0xDE,0x11,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,
	/* 0x53C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,
	/* 0x54C */  0x02,0x00,0x00,0x00,0xD3,0x54,0xD8,0x00,0xC4,0x84,0x50,0x65,0x53,0xC5,0x57,0x18,
	/* 0x55C */  0x6C,0x38,0x02,0x33,0x81,0x45,0xD4,0xF3,0x66,0x96,0x1F,0x27,0x00,0x28,0x61,0x63,
	/* 0x56C */  0x88,0xD5,0xA8,0xFB,0x21,0xB9,0x25,0xC0,0xC4,0xAF,0x7C,0x51,0x57,0xA6,0x9B,0xCC,
	/* 0x57C */  0xE2,0xCC,0xCC,0xCC,0x22,0xC7,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,
	/* 0x58C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x59C */  0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0x36,0x7E,0x94,0x02,0xC7,0x2C,0x52,0x65,
	/* 0x5AC */  0x85,0x2C,0x57,0x18,0x6C,0x41,0xD3,0x89,0x21,0x41,0xD4,0xF3,0x66,0xA6,0xDD,0x26,
	/* 0x5BC */  0x00,0x41,0x34,0x8F,0x4B,0x94,0xD8,0xDE,0x87,0xC0,0x41,0xC9,0x6A,0x1A,0x7C,0x51,
	/* 0x5CC */  0x0A,0x83,0x2B,0x9C,0xE4,0xCC,0xCC,0xCC,0xD6,0x02,0xB6,0xFF,0x00,0x00,0x00,0x00,
	/* 0x5DC */  0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x5EC */  0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xA3,0xF5,0x34,0x01,
	/* 0x5FC */  0x37,0xB0,0x51,0x65,0x68,0x59,0x57,0x18,0x6C,0xA3,0x63,0x35,0x01,0x45,0xD4,0xF3,
	/* 0x60C */  0x66,0x66,0x09,0x27,0x00,0x43,0x90,0x54,0xEB,0x5E,0xF3,0xCB,0x0D,0x9C,0x3D,0x25,
	/* 0x61C */  0x1D,0x9D,0x7C,0x51,0x5E,0x6F,0x0D,0x49,0xE4,0xCC,0xCC,0xCC,0x0B,0x4E,0x05,0x00,
	/* 0x62C */  0x78,0xDE,0x11,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x63C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,
	/* 0x64C */  0xF3,0x5B,0x0F,0x01,0x0C,0x3D,0x50,0x65,0x2C,0xDF,0x57,0x18,0x6C,0x2A,0xA8,0x87,
	/* 0x65C */  0x21,0x41,0xD4,0xF3,0x66,0xF6,0xD7,0x26,0x00,0x4B,0xB0,0x92,0x79,0x22,0x77,0x3D,
	/* 0x66C */  0x90,0x31,0xD5,0x11,0x7A,0xA5,0x7C,0x51,0x41,0xEE,0x29,0x40,0xE4,0xCC,0xCC,0xCC,
	/* 0x67C */  0x93,0x4C,0xF9,0xFF,0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,
	/* 0x68C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,
	/* 0x69C */  0x02,0x00,0x00,0x00,0x70,0x84,0xF9,0x03,0x22,0x0A,0x51,0x65,0x42,0x95,0x57,0x18,
	/* 0x6AC */  0x6C,0x24,0xCA,0x5E,0xC1,0x42,0xD4,0xF3,0x66,0x96,0x4A,0x26,0x00,0x5F,0x39,0xC7,
	/* 0x6BC */  0x88,0x6A,0x75,0xEF,0x87,0xB0,0x2C,0xC2,0xA8,0x34,0x7B,0x51,0xCB,0x78,0x97,0xF0,
	/* 0x6CC */  0xE4,0xCC,0xCC,0xCC,0x22,0xC7,0x15,0x00,0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,
	/* 0x6DC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x6EC */  0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xFE,0xB6,0x88,0x01,0xFB,0x65,0x51,0x65,
	/* 0x6FC */  0x29,0x74,0x57,0x18,0x6C,0xBD,0x20,0x8A,0x01,0x41,0xD4,0xF3,0x66,0x76,0xC4,0x26,
	/* 0x70C */  0x00,0x2F,0x82,0x54,0xA5,0x7B,0x51,0xC0,0x47,0x02,0xA0,0xF9,0x90,0x86,0x7C,0x51,
	/* 0x71C */  0x7D,0x4E,0xDB,0x5C,0xE4,0xCC,0xCC,0xCC,0xA6,0xD0,0x32,0x00,0x78,0xDE,0x11,0x00,
	/* 0x72C */  0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x73C */  0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xFB,0x22,0x8C,0x01,
	/* 0x74C */  0x35,0x62,0x51,0x65,0x85,0x75,0x57,0x18,0x6C,0x84,0xCC,0xB2,0x61,0x43,0xD4,0xF3,
	/* 0x75C */  0x66,0x06,0x8C,0x27,0x00,0x3E,0x3D,0x2F,0xAA,0x5A,0xE1,0x21,0xE2,0x0E,0xB4,0xC9,
	/* 0x76C */  0x8A,0x85,0x7C,0x51,0x74,0x70,0xAA,0x5D,0xE4,0xCC,0xCC,0xCC,0x14,0xCA,0x2D,0x00,
	/* 0x77C */  0x78,0xDE,0x11,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x78C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,
	/* 0x79C */  0x4C,0xE9,0xE3,0x02,0x8E,0x4B,0x51,0x65,0xAF,0x7D,0x57,0x18,0x6C,0xEC,0x8D,0x5F,
	/* 0x7AC */  0x81,0x44,0xD4,0xF3,0x66,0x06,0xF8,0x26,0x00,0x75,0xB9,0x89,0x6D,0xF4,0xCF,0xD1,
	/* 0x7BC */  0x0C,0x67,0x4D,0x2C,0xC3,0xEF,0x7B,0x51,0x50,0x72,0xF3,0xAE,0xE4,0xCC,0xCC,0xCC,
	/* 0x7CC */  0xCC,0xAF,0x19,0x00,0x78,0xDE,0x11,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,
	/* 0x7DC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,
	/* 0x7EC */  0x02,0x00,0x00,0x00,0xDD,0xC7,0x1A,0x04,0x23,0x92,0x61,0x65,0x50,0xA1,0x51,0x18,
	/* 0x7FC */  0x6C,0xF6,0x98,0xD9,0x81,0x44,0xD4,0xF3,0x66,0x66,0x0A,0x27,0x00,0x86,0xFA,0xBD,
	/* 0x80C */  0xAC,0x60,0x60,0xC2,0x91,0x41,0x47,0x11,0x45,0x1A,0x7B,0x51,0x95,0xAE,0x60,0xF8,
	/* 0x81C */  0xE4,0xCC,0xCC,0xCC,0x04,0x87,0x51,0x00,0xB6,0xEA,0x82,0xFF,0x00,0x90,0x8B,0x01,
	/* 0x82C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x83C */  0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xD8,0x20,0xE1,0x05,0xCF,0xFE,0x50,0x65,
	/* 0x84C */  0x57,0x99,0x57,0x18,0x6C,0xBD,0x29,0xB4,0xE1,0x45,0xD4,0xF3,0x66,0xC6,0x6F,0x28,
	/* 0x85C */  0x00,0x2F,0x00,0x10,0x21,0x14,0x9A,0x2F,0x3A,0x7B,0xC7,0x2A,0xDE,0x5C,0x79,0x51,
	/* 0x86C */  0x33,0xCC,0xF8,0x58,0xE6,0xCC,0xCC,0xCC,0x28,0x02,0xE1,0xFF,0xA7,0xA7,0xA6,0xFF,
	/* 0x87C */  0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x88C */  0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xB1,0x9D,0xB2,0x06,
	/* 0x89C */  0xAE,0x1D,0x52,0x65,0xF6,0x31,0x57,0x18,0x6C,0x22,0xF2,0xDA,0x01,0x45,0xD4,0xF3,
	/* 0x8AC */  0x66,0xE6,0x39,0x27,0x00,0xA6,0x8A,0xA0,0x8A,0xA7,0x12,0xC2,0xC4,0xF3,0xD7,0xEF,
	/* 0x8BC */  0x28,0x5A,0x78,0x51,0x17,0x5F,0x5A,0x65,0xE6,0xCC,0xCC,0xCC,0x77,0x98,0x1D,0x00,
	/* 0x8CC */  0x78,0xDE,0x11,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x8DC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,
	/* 0x8EC */  0xC7,0xFC,0x11,0x04,0x90,0x72,0x51,0x65,0xA0,0x6F,0x57,0x18,0x6C,0x89,0x0B,0x09,
	/* 0x8FC */  0xE1,0x44,0xD4,0xF3,0x66,0x76,0x2B,0x27,0x00,0x07,0x56,0x71,0x8F,0x4E,0xF2,0xCB,
	/* 0x90C */  0xA9,0xE0,0xC3,0xDA,0x52,0x21,0x7B,0x51,0x50,0x2D,0x60,0xF6,0xE4,0xCC,0xCC,0xCC,
	/* 0x91C */  0x61,0x65,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,
	/* 0x92C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,
	/* 0x93C */  0x02,0x00,0x00,0x00,0xC3,0xA8,0x4F,0x03,0x84,0xF8,0x50,0x65,0x9C,0x9B,0x57,0x18,
	/* 0x94C */  0x6C,0x59,0xB7,0xB2,0x41,0x45,0xD4,0xF3,0x66,0x06,0x4B,0x28,0x00,0x9B,0x66,0x58,
	/* 0x95C */  0x73,0xF1,0x4C,0xD5,0x2B,0x67,0xAC,0x2F,0x29,0xAE,0x7B,0x51,0x3F,0xD6,0x6D,0xC8,
	/* 0x96C */  0xE4,0xCC,0xCC,0xCC,0x79,0x47,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x90,0x8B,0x01,
	/* 0x97C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x98C */  0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0xFD,0x8D,0x1F,0x03,0x93,0xB4,0x50,0x65,
	/* 0x99C */  0x18,0xB4,0x57,0x18,0x6C,0x45,0x9F,0x06,0xE1,0x42,0xD4,0xF3,0x66,0x66,0x89,0x26,
	/* 0x9AC */  0x00,0xA4,0x2D,0x80,0xA1,0xB7,0x05,0x3E,0x84,0x7A,0xC9,0x0F,0x8C,0xCC,0x7B,0x51,
	/* 0x9BC */  0x8C,0x41,0x0E,0xBD,0xE4,0xCC,0xCC,0xCC,0x48,0x60,0x08,0x00,0x00,0x00,0x00,0x00,
	/* 0x9CC */  0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0x9DC */  0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x02,0x00,0x00,0x00,0x36,0xA8,0x13,0x02,
	/* 0x9EC */  0x04,0x6B,0x51,0x65,0x58,0x72,0x57,0x18,0x6C,0x34,0x45,0xDD,0xC1,0x44,0xD4,0xF3,
	/* 0x9FC */  0x66,0xF6,0x2A,0x27,0x00,0x0E,0x86,0xE5,0xE7,0xA3,0x76,0xC1,0xDD,0xA7,0x9B,0x0D,
	/* 0xA0C */  0x52,0x55,0x7C,0x51,0x73,0x91,0xB5,0x7D,0xE4,0xCC,0xCC,0xCC,0x91,0x06,0x05,0x00,
	/* 0xA1C */  0x78,0xDE,0x11,0x00,0x00,0x90,0x8B,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xA2C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xCC,0xCC,0x00,0x00,0x00,0x00,
	/* 0xA3C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xA4C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xA5C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xA6C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xA7C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xA8C */  0x00,0x7B,0x8D,0x36,0x07,0x36,0xDD,0x0A,0x09,0xB8,0xB4,0x62,0x2F,0xE0,0x8D,0x8C,
	/* 0xA9C */  0x00,0xE1,0x0B,0x62,0x0E,0x60,0x37,0x63,0x8B,0x38,0xB2,0x8D,0xB3,0xE1,0x8B,0xDF,
	/* 0xAAC */  0xB6,0x00,0xC7,0x99,0x44,0xB2,0xC7,0x8C,0x84,0x45,0xFB,0x6B,0xFA,0xCA,0x85,0xE4,
	/* 0xABC */  0x00,0x76,0xE7,0x26,0x2F,0xC8,0x53,0x11,0x99,0x6A,0x8A,0x33,0x57,0x19,0x3B,0xB7,
	/* 0xACC */  0xAF,0x00,0x02,0x07,0x02,0x01,0x00,0x08,0x02,0x02,0x03,0x06,0x03,0x04,0x05,0x01,
	/* 0xADC */  0x00,0x06,0x06,0x01,0x03,0x07,0x05,0x04,0x04,0x04,0xD5,0x05,0x02,0x04,0x05,0x06,
	/* 0xAEC */  0x04,0x00,0x8B,0x01,0xFF,0xBF,0xFF,0x7F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xAFC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xB0C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xB1C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xB2C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
	/* 0xB3C */  0x0C,0x01,0xFF,0xFF,0x2C,0x01,0xFD,0xFF,0xFA,0xFF,0xFF,0xFF,0xF0,0xFF,0xFF,0xFF,
	/* 0xB4C */  0x90,0x8B,0x0E,0x07,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xB5C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xB6C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xB7C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xB8C */  0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0xDC,0x05,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xB9C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xBAC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xBBC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xBCC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xBDC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xBEC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xBFC */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xC0C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xC1C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xC2C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xC3C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	/* 0xC4C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xC5C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xC6C */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xC7C */  0x00,0x00,0x00,0xE8,0x03,0x00,0x01,0x00,0x00,0x94,0x11,0x00,0x85,0x03,0xB8,0x0B,
	/* 0xC8C */  0x00,0x00,

	/* 0xC8E */  0x18,0x7B,0x03,0x00,0x01,0x00,0x00,0x00,

	/* 0xC96 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xCA6 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xCB6 */  0x00,0x00,0x00,0x00,0xDC,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xCC6 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xCD6 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xCE6 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xCF6 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xD06 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xD16 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xD26 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xD36 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xD46 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* 0xD56 */  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,


	/* 0x-- */   0x00
};

void e_gps_flash_init(void)
{
    s8 res;
    res = adl_flhSubscribe(e_gps_flash_handle, 1);
    if (res != OK && res != ADL_RET_ERR_ALREADY_SUBSCRIBED) {
        ERR_LOG(adl_flhSubscribe, res);
    }
}

u8* e_gps_flash_default(void) {
    return e_gps_flash;
}

s32 e_gps_read_flash(u8 *nv_data, u16 length) {
    s32 res;
    res = adl_flhExist(e_gps_flash_handle, NVDATA_FLASH_ID);
    /* Make sure that the data object exists and is the correct size.  
     * Otherwise, do cold start */
    if (res != length) {
        if (res != OK) {
            ERR_LOG(adl_flhExist, res);
            return -1;
        } else {
            return 0;
        }
    } else {
        s8 res2;
        res2 = adl_flhRead(e_gps_flash_handle, NVDATA_FLASH_ID, length, nv_data);
        if (res2 == OK) {
            return length;
        } else {
            ERR_LOG(adl_flhRead, res2);
		    return -1;
        }
    }
}

s8 e_gps_write_flash(u8 *nv_data, u32 length) 
{
    s8 res;

    res = adl_flhWrite(e_gps_flash_handle, NVDATA_FLASH_ID, length, nv_data);
    if (res != OK) {
        ERR_LOG(adl_flhWrite, res);
        return -1;
    } else {
        return 0;
    }
}

s32 e_gps_erase_flash(void) 
{
    s32 length;

    /* Make sure that the data object exists and is the correct size.  
     * Otherwise, do not erase */
    length = adl_flhExist(e_gps_flash_handle, NVDATA_FLASH_ID);
    if (length > 0) {
        s8 res;
        res = adl_flhErase(e_gps_flash_handle, NVDATA_FLASH_ID );
        if(res != OK) {
            ERR_LOG(adl_flhErase, res);
        }
        return (u32) length;

    } else if (length != OK) {
        ERR_LOG(adl_flhExist, length);
        return -1;
    } else {
        return 0;
    }
}
