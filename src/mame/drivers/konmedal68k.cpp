// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

 konmedal68k.cpp: Konami 68000 based medal games

 Pittanko Zaurus (ピッタンコ　ザウルス)
 GS562
 (c) 1995 Konami

 Konami ICs:
 K058143 + K056832 = tilemaps
 K055555 = priority blender
 K056766 = color DAC
 K056879 = input/EEPROM interface

 800000 = control
 bit 3 = write 1 to enable or ack IRQ 3
 bit 4 = write 1 to enable or ack IRQ 4

***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/ticket.h"
#include "sound/ymz280b.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k055555.h"
#include "video/konami_helper.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class konmedal68k_state : public driver_device
{
public:
	konmedal68k_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_k056832(*this, "k056832"),
		m_k055555(*this, "k055555"),
		m_palette(*this, "palette"),
		m_ymz(*this, "ymz")
	{ }

	void kzaurus(machine_config &config);
	void koropens(machine_config &config);
	void slot(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	uint32_t screen_update_konmedal68k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void fill_backcolor(bitmap_ind16 &bitmap, const rectangle &cliprect, int pen_idx, int mode);

	K056832_CB_MEMBER(tile_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	void control_w(uint16_t data)
	{
		m_control = data & 0xff;

		if (!(data & 0x8))
		{
			m_maincpu->set_input_line(M68K_IRQ_3, CLEAR_LINE);
		}

		if (!(data & 0x10))
		{
			m_maincpu->set_input_line(M68K_IRQ_4, CLEAR_LINE);
		}
	}
	void control2_w(uint16_t data) { m_control2 = data & 0xff; }

	uint16_t vrom_r(offs_t offset)
	{
		if (m_control2 & 0x10)
		{
			offset |= 0x1000;
		}

		return m_k056832->piratesh_rom_r(offset);
	}

	uint16_t vrom_koropens_r(offs_t offset)
	{
		if (m_control2 & 0x10)
		{
			return 0;
		}

		return m_k056832->rom_word_r(offset);
	}

	void common_main(address_map &map);
	void kzaurus_main(address_map &map);
	void koropens_main(address_map &map);
	void slot_main(address_map &map);

	static constexpr int NUM_LAYERS = 4;

	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<k055555_device> m_k055555;
	required_device<palette_device> m_palette;
	required_device<ymz280b_device> m_ymz;

	u8 m_control, m_control2;
};

void konmedal68k_state::video_start()
{
	m_k056832->set_layer_offs(0, -4, -8);  // title on title screen
	m_k056832->set_layer_offs(1, 12, 8);    // konami logo on title screen
	m_k056832->set_layer_offs(2, 6, -8);
	m_k056832->set_layer_offs(3, 6, -8);
}

TIMER_DEVICE_CALLBACK_MEMBER(konmedal68k_state::scanline)
{
	int scanline = param;

	if ((scanline == 240) && (m_control & 0x8))
	{
		m_maincpu->set_input_line(M68K_IRQ_3, ASSERT_LINE);

	}

	if ((scanline == 255) && (m_control & 0x10))
	{
		m_maincpu->set_input_line(M68K_IRQ_4, ASSERT_LINE);
	}
}

K056832_CB_MEMBER(konmedal68k_state::tile_callback)
{
}

// modified from version in mame/video/k054338.cpp
void konmedal68k_state::fill_backcolor(bitmap_ind16 &bitmap, const rectangle &cliprect, int pen_idx, int mode)
{
	if ((mode & 0x02) == 0) // solid fill
	{
		bitmap.fill(pen_idx, cliprect);
	}
	else
	{
		uint16_t *dst_ptr = &bitmap.pix16(cliprect.min_y);
		int dst_pitch = bitmap.rowpixels();

		if ((mode & 0x01) == 0) // vertical gradient fill
		{
			pen_idx += cliprect.min_y;
			for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
			{
				for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
				{
					dst_ptr[x] = pen_idx;
				}

				pen_idx++;
				dst_ptr += dst_pitch;
			}
		}
		else    // horizontal gradient fill
		{
			pen_idx += cliprect.min_x;
			dst_ptr += cliprect.min_x;
			for(int y = cliprect.min_y; y<= cliprect.max_y; y++)
			{
				for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
				{
					dst_ptr[x] = pen_idx;
				}
				dst_ptr += dst_pitch;
			}
		}
	}
}

uint32_t konmedal68k_state::screen_update_konmedal68k(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	static const int order[4] = { 3, 2, 0, 1 };
	int enables = m_k055555->K055555_read_register(K55_INPUT_ENABLES);

	screen.priority().fill(0, cliprect);

	fill_backcolor(bitmap, cliprect, (m_k055555->K055555_read_register(0) << 9), m_k055555->K055555_read_register(1));

	for (int i = 0; i < NUM_LAYERS; i++)
	{
		int layer = order[i];

		if (enables & (K55_INP_VRAM_A << layer))
		{
			m_k056832->tilemap_draw(screen, bitmap, cliprect, layer, 0, 1 << i);
		}
	}

	return 0;
}

void konmedal68k_state::common_main(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("maincpu", 0);
	map(0x400000, 0x403fff).ram().share("nvram");
	map(0x800000, 0x800001).w(FUNC(konmedal68k_state::control_w));
	map(0x800004, 0x800005).portr("DSW");
	map(0x800006, 0x800007).portr("IN1");
	map(0x800008, 0x800009).portr("IN0");
	map(0x820000, 0x820001).portw("OUT");
	map(0x810000, 0x810001).w(FUNC(konmedal68k_state::control2_w));
	map(0x830000, 0x83003f).rw(m_k056832, FUNC(k056832_device::word_r), FUNC(k056832_device::word_w));
	map(0x840000, 0x84000f).w(m_k056832, FUNC(k056832_device::b_word_w));
	map(0x85001c, 0x85001f).nopw();
	map(0x870000, 0x87005f).w(m_k055555, FUNC(k055555_device::K055555_word_w));
	map(0x880000, 0x880003).rw(m_ymz, FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask16(0xff00);
	map(0xa00000, 0xa01fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));
	map(0xa02000, 0xa03fff).rw(m_k056832, FUNC(k056832_device::ram_word_r), FUNC(k056832_device::ram_word_w));
}

void konmedal68k_state::kzaurus_main(address_map &map)
{
	common_main(map);
	map(0xb00000, 0xb03fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xc00000, 0xc01fff).r(FUNC(konmedal68k_state::vrom_r));
}

void konmedal68k_state::koropens_main(address_map &map)
{
	common_main(map);
	map(0xb00000, 0xb03fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0xc00000, 0xc01fff).r(FUNC(konmedal68k_state::vrom_koropens_r));
}

void konmedal68k_state::slot_main(address_map &map)
{
	common_main(map);
	map(0xb00000, 0xb03fff).ram().lrw16(
		NAME([this](offs_t offset) -> u16 { return (offset & 1) ? m_palette->read16(offset / 2) : 0; }),
		NAME([this](offs_t offset, u16 data) { if (offset & 1) m_palette->write16(offset / 2, data); })
	).share("palette");
	map(0xc00000, 0xc01fff).r(FUNC(konmedal68k_state::vrom_r));
}

static INPUT_PORTS_START( kzaurus )
	PORT_START("IN0")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Test") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xff1f, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", hopper_device, line_r)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN3 )    // medal
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xf0ff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("OUT")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("hopper", hopper_device, motor_w)

	PORT_START("DSW")
	PORT_DIPNAME( 0x07, 0x07, "Coin Slot 1" )   PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, "5 Coins/2 Credits" )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x78, 0x00, "Coin Slot 2" )   PORT_DIPLOCATION("SW1:4,5,6,7")
	PORT_DIPSETTING(    0x00, "16 Medals" )
	PORT_DIPSETTING(    0x08, "15 Medals" )
	PORT_DIPSETTING(    0x10, "14 Medals" )
	PORT_DIPSETTING(    0x18, "13 Medals" )
	PORT_DIPSETTING(    0x20, "12 Medals" )
	PORT_DIPSETTING(    0x28, "11 Medals" )
	PORT_DIPSETTING(    0x30, "10 Medals" )
	PORT_DIPSETTING(    0x38, "9 Medals" )
	PORT_DIPSETTING(    0x40, "8 Medals" )
	PORT_DIPSETTING(    0x48, "7 Medals" )
	PORT_DIPSETTING(    0x50, "6 Medals" )
	PORT_DIPSETTING(    0x58, "5 Medals" )
	PORT_DIPSETTING(    0x60, "4 Medals" )
	PORT_DIPSETTING(    0x68, "3 Medals" )
	PORT_DIPSETTING(    0x70, "2 Medals" )
	// PORT_DIPSETTING(    0x78, "2 Medals" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )

	PORT_DIPNAME( 0x0f00, 0x0000, "Standard of Payout" ) PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x0000, "90%" )
	PORT_DIPSETTING(    0x0100, "85%" )
	PORT_DIPSETTING(    0x0200, "80%" )
	PORT_DIPSETTING(    0x0300, "75%" )
	PORT_DIPSETTING(    0x0400, "70%" )
	PORT_DIPSETTING(    0x0500, "65%" )
	PORT_DIPSETTING(    0x0600, "60%" )
	PORT_DIPSETTING(    0x0700, "55%" )
	PORT_DIPSETTING(    0x0800, "50%" )
	PORT_DIPSETTING(    0x0900, "45%" )
	PORT_DIPSETTING(    0x0a00, "40%" )
	PORT_DIPSETTING(    0x0b00, "35%" )
	PORT_DIPSETTING(    0x0c00, "30%" )
	PORT_DIPSETTING(    0x0d00, "25%" )
	PORT_DIPSETTING(    0x0e00, "20%" )
	PORT_DIPSETTING(    0x0f00, "15%" )
	PORT_DIPNAME( 0x3000, 0x0000, "Play Timer" )         PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x0000, "30 sec" )
	PORT_DIPSETTING(    0x1000, "24 sec" )
	PORT_DIPSETTING(    0x2000, "18 sec" )
	PORT_DIPSETTING(    0x3000, "12 sec" )
	PORT_DIPNAME( 0x4000, 0x0000, "Backup Memory" )      PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x4000, "Keep" )
	PORT_DIPSETTING(    0x0000, "Clear" )
	PORT_DIPNAME( 0x8000, 0x0000, "Demo Sound" )         PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

void konmedal68k_state::machine_start()
{
	save_item(NAME(m_control));
	save_item(NAME(m_control2));
}

void konmedal68k_state::machine_reset()
{
	m_control = m_control2 = 0;
}

void konmedal68k_state::kzaurus(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(33'868'800)/4);    // 33.8688 MHz crystal verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &konmedal68k_state::kzaurus_main);
	TIMER(config, "scantimer").configure_scanline(FUNC(konmedal68k_state::scanline), "screen", 0, 1);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	HOPPER(config, "hopper", attotime::from_msec(100), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.62);  /* verified on pcb */
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(40, 400-1, 16, 240-1);
	screen.set_screen_update(FUNC(konmedal68k_state::screen_update_konmedal68k));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_888, 8192).enable_shadows();

	K056832(config, m_k056832, 0);
	m_k056832->set_tile_callback(FUNC(konmedal68k_state::tile_callback));
	m_k056832->set_config(K056832_BPP_4dj, 1, 0);
	m_k056832->set_palette(m_palette);

	K055555(config, m_k055555, 0);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YMZ280B(config, m_ymz, XTAL(33'868'800)/2); // 33.8688 MHz xtal verified on PCB
	m_ymz->add_route(0, "lspeaker", 1.0);
	m_ymz->add_route(1, "rspeaker", 1.0);
}

void konmedal68k_state::koropens(machine_config &config)
{
	kzaurus(config);

	M68000(config.replace(), m_maincpu, XTAL(33'868'800)/4);    // 33.8688 MHz crystal verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &konmedal68k_state::koropens_main);
}

void konmedal68k_state::slot(machine_config &config)
{
	kzaurus(config);

	M68000(config.replace(), m_maincpu, XTAL(33'868'800) / 4);    // 33.8688 MHz crystal verified on PCB
	m_maincpu->set_addrmap(AS_PROGRAM, &konmedal68k_state::slot_main);

	PALETTE(config.replace(), "palette").set_format(palette_device::xBGR_444, 4096).enable_shadows();
}

ROM_START( kzaurus )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP( "540-b05-2n.bin", 0x000000, 0x080000, CRC(110d4ecb) SHA1(8903783f62ad5a983242a0fe8d835857964abc43) )

	ROM_REGION( 0x100000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD( "540-a06-14n.bin", 0x000000, 0x080000, CRC(260ad79e) SHA1(fb56bf6e59e78b2bd1f8df17c9c8fd0d1700dced) )
	ROM_LOAD( "540-a07-17n.bin", 0x080000, 0x080000, CRC(442bcec2) SHA1(3100de8c146a28284ae3ab8763e5b1c6fb1755c2) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "540-a01-2f.bin", 0x000000, 0x080000, CRC(391c6ee6) SHA1(a345934687a8abf818350d0597843a1159395fc0) )
ROM_END

ROM_START( koropens )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP( "741-c05-2n.bin", 0x000000, 0x080000, CRC(5c3ebe2a) SHA1(f42e15db386e63992844ded4dc30a16aea9918ac) )

	ROM_REGION( 0x100000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD( "741-a06-14n.bin", 0x000000, 0x080000, CRC(dbd9b892) SHA1(d36dc192465b894fed03c83ac64738296ab12ca1) )
	ROM_LOAD( "741-a07-17n.bin", 0x080000, 0x080000, CRC(a50c9a91) SHA1(3c44f78bdf016d39002b0d1e34c6dbc70872a92e) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "741-a01-2f.bin", 0x000000, 0x080000, CRC(56823bfb) SHA1(5c23af55ef0053345f853f4c6ea361182d3ce14a) )
	ROM_LOAD( "741-a02-1f.bin", 0x080000, 0x080000, CRC(31918688) SHA1(70a1699a3914883f502e021dbb9c0847f4ebee04) )
ROM_END

ROM_START( pwrchanc )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP( "960-a05-2n.bin", 0x000000, 0x080000, CRC(a8e24c7e) SHA1(f66bbdb36379fb0f7a87bd18f7cd36cfd8d4fdfa) )

	ROM_REGION( 0x200000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD( "960-a06-14n.bin", 0x000000, 0x080000, CRC(3b47ffc1) SHA1(40b1521758267a302dcf8f488296d7731a1165f5) )
	ROM_LOAD( "960-a07-17n.bin", 0x080000, 0x080000, CRC(a6582197) SHA1(66994ee62329c61ec0c1ad39c0ed7eca16f40fdc) )
	ROM_LOAD( "960-a08-19n.bin", 0x100000, 0x080000, CRC(17fc22fb) SHA1(b0d7670fd4176e4b6f111c83ff4d4a5db0c33e45) )
	ROM_LOAD( "960-a09-22n.bin", 0x180000, 0x080000, CRC(76863ddd) SHA1(384a4d6b473a132bf15dc165c2582bfc2428c422) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "960-a01-2f.bin", 0x000000, 0x080000, CRC(4c9b0e07) SHA1(f1ab1a6a1440204eb844dcdae6e5f55cd75e9d9b) )
	ROM_LOAD( "960-a02-4f.bin", 0x080000, 0x080000, CRC(48422b4b) SHA1(464a1c7e2309ec77b0c16e24ceac3c304b7133c5) )
ROM_END

ROM_START( ymcapsul )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP( "834-a05-2n.bin", 0x000000, 0x080000, CRC(34773912) SHA1(0e3832f51effa5495562796da4dea1e79dcd0ad3) )

	ROM_REGION( 0x100000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD( "834-a06-14n.bin", 0x000000, 0x080000, CRC(444b0172) SHA1(b305457e11a4382855bc37e39a6d23f74a646bd4) )
	ROM_LOAD( "834-a07-17n.bin", 0x080000, 0x080000, CRC(e5a9533e) SHA1(7ad7b1603dbb2e31acae469a97a8b2a5d80b3026) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "834-a01-2f.bin", 0x000000, 0x080000, CRC(d9be9af9) SHA1(d2877e1c62f6b5ee1a24026560bba66c97daa065) )
	ROM_LOAD( "834-a02-4f.bin", 0x080000, 0x080000, CRC(47b6ab29) SHA1(5bff269fca6f664db8749d036f649a480af2588e) )
ROM_END

ROM_START( kattobas )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP( "841-a05-2n.bin", 0x000000, 0x080000, CRC(e5d9f04c) SHA1(58829945728ed93441ba40cded3f289f3c54df1b) )

	ROM_REGION( 0x100000, "k056832", 0 )   /* tilemaps */
	ROM_LOAD( "841-a06-14n.bin", 0x000000, 0x080000, CRC(5aecf05c) SHA1(9d76730ace595ff725686d5d0d3610326cb0ec9e) )
	ROM_LOAD( "841-a07-17n.bin", 0x080000, 0x080000, CRC(5a1fb6d1) SHA1(b8ad45b04e346aabf1c97e6146d68a0351ce83a3) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "841-a01-2f.bin", 0x000000, 0x080000, CRC(e3474fe8) SHA1(5e27c59f952088821eb34714484e288407d48d3d) )
	ROM_LOAD( "841-a02-4f.bin", 0x080000, 0x080000, CRC(685c1c10) SHA1(9884940df8c079e8129fc8d870f90e5b7987e6f4) )
ROM_END

ROM_START( dobouchn )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "640-a05-2n.bin", 0x000000, 0x080000, CRC(7643dbc6) SHA1(3b55a782f04a741088b93954279b35c1c90af622) )

	ROM_REGION( 0x100000, "k056832", 0 )
	ROM_LOAD( "640-a06-14n.bin", 0x000000, 0x080000, CRC(c6c5016c) SHA1(ad0b5258e2c1d0ba95dfc0d8fc6332b524f2c1e2) )
	ROM_LOAD( "640-a07-17n.bin", 0x080000, 0x080000, CRC(614fee32) SHA1(080fea72c0417752eb0a0b109b524d87379b2921) )

	ROM_REGION( 0x200000, "ymz", 0 )
	ROM_LOAD( "640-a01-2f.bin", 0x000000, 0x080000, CRC(326e2844) SHA1(62ce14ffe5d0a35c37c9a5a98c9c3a5df63d4512) )
	ROM_LOAD( "640-a02-4f.bin", 0x080000, 0x080000, CRC(ab6593f5) SHA1(95907ee4a2cdf3bf27b7c0c1283b2bc36b868d9d) )
ROM_END

// GS562 PCB with no K056766 color DAC and no IC 20D 8Kbyte SRAM (palette RAM?), possible have no video output or have it implemented in some unusual way.
// at 1st boot press Service1 to initialise NVRAM
ROM_START( konslot )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "669-a05-2n.bin", 0x000000, 0x080000, CRC(3330848e) SHA1(24c2ac03fe5d099659081d1f9611c707c746c768) )

	ROM_REGION( 0x80000, "k056832", 0 )
	ROM_LOAD( "669-a06-14n.bin", 0x000000, 0x080000, CRC(b058fa04) SHA1(b277e814f1814d8892ccc5279f75bff0eec678b5) )

	ROM_REGION( 0x80000, "ymz", 0 )
	ROM_LOAD( "669-a01-2d.bin", 0x000000, 0x080000, CRC(08438dad) SHA1(b4ef8fc37deca5b6537cc581fc99968c86e6ec2c) )
ROM_END

ROM_START( konslot2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "660-a05.2n", 0x000000, 0x080000, CRC(d7460250) SHA1(490588181b3b558e03752f4c6cbdadb807b990b3) )

	ROM_REGION( 0x80000, "k056832", 0 )
	ROM_LOAD( "660-a06.14n", 0x000000, 0x080000, CRC(17d2dbb7) SHA1(b32b4123c6006ad7ee0b2d12542bad6b9ccb4cf2) )

	ROM_REGION( 0x100000, "ymz", 0 )
	ROM_LOAD( "660-a01.2f", 0x000000, 0x080000, CRC(b350de2f) SHA1(56d6054b5b9fbabc12cfa3979c8b563c66f687c9) )
	ROM_LOAD( "660-a02.1f", 0x080000, 0x080000, CRC(e3199b0d) SHA1(8805be14388c73c5a8e0b2eb98fb8efb5def1714) )
ROM_END


GAME( 1995, kzaurus,  0, kzaurus,  kzaurus, konmedal68k_state, empty_init, ROT0, "Konami", "Pittanko Zaurus", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1996, dobouchn, 0, kzaurus,  kzaurus, konmedal68k_state, empty_init, ROT0, "Konami", "Dobou-Chan (ver JAA)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
GAME( 199?, konslot,  0, slot,     kzaurus, konmedal68k_state, empty_init, ROT0, "Konami", "unknown Konami slot medal game (set 1)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
GAME( 199?, konslot2, 0, slot,     kzaurus, konmedal68k_state, empty_init, ROT0, "Konami", "unknown Konami slot medal game (set 2)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1997, koropens, 0, koropens, kzaurus, konmedal68k_state, empty_init, ROT0, "Konami", "Korokoro Pensuke", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1998, kattobas, 0, koropens, kzaurus, konmedal68k_state, empty_init, ROT0, "Konami", "Kattobase Power Pro Kun", MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, pwrchanc, 0, koropens, kzaurus, konmedal68k_state, empty_init, ROT0, "Konami", "Powerful Chance", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1999, ymcapsul, 0, kzaurus,  kzaurus, konmedal68k_state, empty_init, ROT0, "Konami", "Yu-Gi-Oh Monster Capsule", MACHINE_IMPERFECT_GRAPHICS )
