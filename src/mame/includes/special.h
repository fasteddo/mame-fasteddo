// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/special.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_SPECIAL_H
#define MAME_INCLUDES_SPECIAL_H

#pragma once

#include "cpu/z80/z80.h"
#include "cpu/i8085/i8085.h"
#include "audio/special.h"
#include "sound/dac.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "imagedev/cassette.h"
#include "imagedev/floppy.h"
#include "formats/smx_dsk.h"
#include "formats/rk_cas.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"
#include "emupal.h"


class special_state : public driver_device
{
public:
	special_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi(*this, "ppi")
		, m_fdc(*this, "fdc")
		, m_dac(*this, "dac")
		, m_pit(*this, "pit")
		, m_cassette(*this, "cassette")
		, m_fdd(*this, "fd%u", 0U)
		, m_ram(*this, RAM_TAG)
		, m_vram(*this, "videoram")
		, m_rom(*this, "maincpu")
		, m_mram(*this, "mainram")
		, m_bank1(*this, "bank1")
		, m_bank2(*this, "bank2")
		, m_bank3(*this, "bank3")
		, m_bank4(*this, "bank4")
		, m_bank5(*this, "bank5")
		, m_bank6(*this, "bank6")
		, m_io_keyboard(*this, "LINE%u", 0U)
		, m_palette(*this, "palette")
	{ }

	void special(machine_config &config);
	void erik(machine_config &config);
	void specimx(machine_config &config);
	void specialp(machine_config &config);
	void specialm(machine_config &config);

	void init_erik();
	void init_special();

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum
	{
		TIMER_PIT8253_GATES
	};

	// specimx
	void specimx_select_bank(offs_t offset, uint8_t data);
	void video_memory_w(offs_t offset, uint8_t data);
	void specimx_video_color_w(uint8_t data);
	uint8_t specimx_video_color_r();
	uint8_t specimx_disk_ctrl_r();
	void specimx_disk_ctrl_w(offs_t offset, uint8_t data);
	uint8_t specimx_portb_r();
	void specimx_portc_w(uint8_t data);
	void specimx_set_bank(offs_t i, uint8_t data);
	// erik
	uint8_t erik_rr_reg_r();
	void erik_rr_reg_w(uint8_t data);
	uint8_t erik_rc_reg_r();
	void erik_rc_reg_w(uint8_t data);
	uint8_t erik_disk_reg_r();
	void erik_disk_reg_w(uint8_t data);
	void erik_set_bank();
	// special
	u8 kbd_r();
	uint8_t porta_r();
	uint8_t portb_r();
	uint8_t portc_r();
	void porta_w(uint8_t data);
	void portb_w(uint8_t data);
	void portc_w(uint8_t data);

	void erik_palette(palette_device &palette) const;
	void specimx_palette(palette_device &palette) const;
	uint32_t screen_update_special(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_erik(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_specialp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_specimx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq);
	DECLARE_FLOPPY_FORMATS( specimx_floppy_formats );
	void machine_start() override;
	void machine_reset() override;

	void erik_io(address_map &map);
	void erik_mem(address_map &map);
	void specialist_mem(address_map &map);
	void specialp_mem(address_map &map);
	void specimx_mem(address_map &map);

	std::unique_ptr<uint8_t[]> m_specimx_colorram;
	uint8_t m_erik_color_1;
	uint8_t m_erik_color_2;
	uint8_t m_erik_background;
	uint8_t m_specimx_color;
	u8 m_porta;
	u8 m_portb;
	u8 m_portc;
	uint8_t m_RR_register;
	uint8_t m_RC_register;
	u8 m_drive;
	required_device<cpu_device> m_maincpu;
	optional_device<i8255_device> m_ppi;
	optional_device<fd1793_device> m_fdc;
	optional_device<dac_bit_interface> m_dac;
	optional_device<pit8253_device> m_pit;
	optional_device<cassette_image_device> m_cassette;
	optional_device_array<floppy_connector, 2> m_fdd;
	optional_device<ram_device> m_ram;
	optional_shared_ptr<uint8_t> m_vram;
	required_region_ptr<u8> m_rom;
	optional_shared_ptr<u8> m_mram;
	required_memory_bank m_bank1;
	optional_memory_bank m_bank2;
	optional_memory_bank m_bank3;
	optional_memory_bank m_bank4;
	optional_memory_bank m_bank5;
	optional_memory_bank m_bank6;
	required_ioport_array<13> m_io_keyboard;
	required_device<palette_device> m_palette;
};

#endif // MAME_INCLUDES_SPECIAL_H
