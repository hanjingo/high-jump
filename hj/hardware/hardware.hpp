#ifndef HARDWARE_HPP
#define HARDWARE_HPP

#include <hj/hardware/bios.h>

#ifdef HJ_ENABLE_USB_BT
#include <hj/hardware/bluetooth.h>
#endif

#include <hj/hardware/cpu.h>

#include <hj/hardware/disk.h>

#include <hj/hardware/dpdk.h>

#ifdef HJ_ENABLE_GPU
#include <hj/hardware/gpu.h>
#endif

#include <hj/hardware/keyboard.h>

#include <hj/hardware/mainboard.h>

#include <hj/hardware/mouse.h>

#include <hj/hardware/nic.h>

#include <hj/hardware/ram.h>

#include <hj/hardware/rom.h>

#include <hj/hardware/simd.h>

#ifdef HJ_ENABLE_USB_BT
#include <hj/hardware/usb.h>
#endif

#endif