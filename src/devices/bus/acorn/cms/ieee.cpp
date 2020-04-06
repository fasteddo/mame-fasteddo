// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS IEEE Controller Board

**********************************************************************/


#include "emu.h"
#include "ieee.h"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CMS_IEEE, cms_ieee_device, "cms_ieee", "CMS IEEE Controller Board");


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cms_ieee_device::device_add_mconfig(machine_config &config)
{
	TMS9914(config, m_tms9914, 4_MHz_XTAL); // TODO: verify clock
	m_tms9914->int_write_cb().set(FUNC(cms_ieee_device::bus_irq_w));
	m_tms9914->dio_read_cb().set(IEEE488_TAG, FUNC(ieee488_device::dio_r));
	m_tms9914->dio_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dio_w));
	m_tms9914->eoi_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_eoi_w));
	m_tms9914->dav_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_dav_w));
	m_tms9914->nrfd_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_nrfd_w));
	m_tms9914->ndac_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ndac_w));
	m_tms9914->ifc_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ifc_w));
	m_tms9914->srq_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_srq_w));
	m_tms9914->atn_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_atn_w));
	m_tms9914->ren_write_cb().set(IEEE488_TAG, FUNC(ieee488_device::host_ren_w));

	IEEE488(config, m_ieee);
	m_ieee->eoi_callback().set(m_tms9914, FUNC(tms9914_device::eoi_w));
	m_ieee->dav_callback().set(m_tms9914, FUNC(tms9914_device::dav_w));
	m_ieee->nrfd_callback().set(m_tms9914, FUNC(tms9914_device::nrfd_w));
	m_ieee->ndac_callback().set(m_tms9914, FUNC(tms9914_device::ndac_w));
	m_ieee->ifc_callback().set(m_tms9914, FUNC(tms9914_device::ifc_w));
	m_ieee->srq_callback().set(m_tms9914, FUNC(tms9914_device::srq_w));
	m_ieee->atn_callback().set(m_tms9914, FUNC(tms9914_device::atn_w));
	m_ieee->ren_callback().set(m_tms9914, FUNC(tms9914_device::ren_w));
	IEEE488_SLOT(config, "ieee_dev", 0, cbm_ieee488_devices, nullptr);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cms_ieee_device - constructor
//-------------------------------------------------

cms_ieee_device::cms_ieee_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, CMS_IEEE, tag, owner, clock)
	, device_acorn_bus_interface(mconfig, *this)
	, m_ieee(*this, IEEE488_TAG)
	, m_tms9914(*this, "hpib")
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cms_ieee_device::device_start()
{
	address_space &space = m_bus->memspace();

	space.install_readwrite_handler(0xfc60, 0xfc6f, read8sm_delegate(*m_tms9914, FUNC(tms9914_device::read)), write8sm_delegate(*m_tms9914, FUNC(tms9914_device::write)));
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

WRITE_LINE_MEMBER(cms_ieee_device::bus_irq_w)
{
	m_bus->irq_w(state);
}
