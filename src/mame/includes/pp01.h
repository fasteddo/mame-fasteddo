// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/*****************************************************************************
 *
 * includes/pp01.h
 *
 ****************************************************************************/

#ifndef MAME_INCLUDES_PP01_H
#define MAME_INCLUDES_PP01_H

#pragma once

#include "cpu/i8085/i8085.h"
#include "machine/ram.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/i8255.h"
#include "sound/spkrdev.h"
#include "imagedev/cassette.h"
#include "machine/timer.h"
#include "emupal.h"

class pp01_state : public driver_device
{
public:
	pp01_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pit(*this, "pit8253")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_speaker(*this, "speaker")
		, m_ram(*this, RAM_TAG)
		, m_uart(*this, "uart")
		, m_cass(*this, "cassette")
	{ }

	void pp01(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pit8253_device> m_pit;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	required_device<speaker_sound_device> m_speaker;
	required_device<ram_device> m_ram;
	required_device<i8251_device> m_uart;
	required_device<cassette_image_device> m_cass;
	uint8_t m_video_scroll;
	uint8_t m_memory_block[16];
	uint8_t m_video_write_mode;
	uint8_t m_key_line;
	bool m_txe, m_txd, m_rts, m_casspol;
	u8 m_cass_data[4];

	DECLARE_WRITE8_MEMBER(video_write_mode_w);
	DECLARE_WRITE8_MEMBER(video_r_1_w);
	DECLARE_WRITE8_MEMBER(video_g_1_w);
	DECLARE_WRITE8_MEMBER(video_b_1_w);
	DECLARE_WRITE8_MEMBER(video_r_2_w);
	DECLARE_WRITE8_MEMBER(video_g_2_w);
	DECLARE_WRITE8_MEMBER(video_b_2_w);
	DECLARE_WRITE8_MEMBER(mem_block_w);
	DECLARE_READ8_MEMBER(mem_block_r);
	TIMER_DEVICE_CALLBACK_MEMBER(kansas_r);
	void pp01_palette(palette_device &palette) const;
	uint32_t screen_update_pp01(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(z2_w);
	DECLARE_READ8_MEMBER(ppi1_porta_r);
	DECLARE_WRITE8_MEMBER(ppi1_porta_w);
	DECLARE_READ8_MEMBER(ppi1_portb_r);
	DECLARE_WRITE8_MEMBER(ppi1_portb_w);
	DECLARE_WRITE8_MEMBER(ppi1_portc_w);
	DECLARE_READ8_MEMBER(ppi1_portc_r);
	void video_w(uint8_t block,uint16_t offset,uint8_t data,uint8_t part);
	void set_memory(uint8_t block, uint8_t data);
	void io_map(address_map &map);
	void mem_map(address_map &map);
};

#endif // MAME_INCLUDES_PP01_H
