/*
 * Demod driver wrappers for aml_dvb_extern module
 *
 * Copyright (C) 2025 Marek Czerski <ma.czerski@gmail.com>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <linux/amlogic/aml_demod_common.h>
#include <linux/amlogic/aml_tuner.h>
#include "avl6862.h"
#include "cxd2878.h"
#include "mxl603.h"
#include "m88rs6060.h"

struct dvb_frontend *aml_avl68xx_attach(const struct demod_config *cfg)
{
	struct avl6862_config avl68xxcfg = {
		.demod_address = cfg->i2c_addr,
		.dual_tuner = cfg->tuner1.id != AM_TUNER_NONE ? 1 : 0,
		.ts_serial = cfg->ts_out_mode ? 0 : 1, /* ts_out_mode: serial or parallel; 0: serial, 1: parallel. */
		.gpio_lock_led = 0,
	};

	struct dvb_frontend *fe = avl6862_attach(&avl68xxcfg, cfg->i2c_adap);
	if (IS_ERR_OR_NULL(fe))
		return NULL;

	if (cfg->tuner0.id != AM_TUNER_NONE) {
		const struct tuner_module * tuner = aml_get_tuner_module(cfg->tuner0.id);
		if (tuner->attach(tuner, fe, &cfg->tuner0) == NULL) {
			pr_err("AVL68xx: failed to attach tuner0 %s\n", tuner->name);
		}
	}
	else {
		pr_err("AVL68xx: Missing tuner0 config\n");
	}

	if (cfg->tuner1.id != AM_TUNER_NONE) {
		pr_err("AVL68xx: failed to attach tuner1, dual tuner not supported\n");
	}

	return fe;
}
EXPORT_SYMBOL_GPL(aml_avl68xx_attach);

struct dvb_frontend *aml_cxd2856_attach(const struct demod_config *cfg)
{
	struct cxd2878_config cxd2878cfg = {
		.addr_slvt = cfg->i2c_addr,
		.xtal = cfg->xtal, /* XTAL freq: 0: 16 MHz, 1: 24 MHz, 2: 32 MHz */
		.ts_mode = cfg->ts_out_mode, /* TS out mode: 0: Serial output, 1: Parallel output (Default) */
		.ts_ser_data = cfg->ts_data_pin, /* Serial output pin of TS data. 0: Output from TSDATA0, 1: Output from TSDATA7 (Default) */
		.ts_clk = cfg->ts_clk, /* Serial TS clock gated on valid TS data or is continuous. 0: Gated, 1: Continuous (Default) */
		.ts_clk_mask = 1, /* Disable/Enable TS clock during specified TS region. 
							bit flags: ( can be bitwise ORed )
							- 0 : Always Active
							- 1 : Disable during TS packet gap (default)
							- 2 : Disable during TS parity (default)
							- 4 : Disable during TS payload
							- 8 : Disable during TS header
							- 16: Disable during TS sync */
		.ts_valid = 0, /* Disable/Enable TSVALID during specified TS region.
							bit flags: ( can be bitwise ORed )
							- 0 : Always Active
							- 1 : Disable during TS packet gap (default)
							- 2 : Disable during TS parity (default)
							- 4 : Disable during TS payload
							- 8 : Disable during TS header
							- 16: Disable during TS sync */
		.atscCoreDisable = 0,
		.lock_flag = 1,
	};

	struct dvb_frontend *fe = cxd2878_attach(&cxd2878cfg, cfg->i2c_adap);
	if (IS_ERR_OR_NULL(fe))
		return NULL;

	if (cfg->tuner0.id != AM_TUNER_NONE) {
		const struct tuner_module * tuner = aml_get_tuner_module(cfg->tuner0.id);
		if (tuner->attach(tuner, fe, &cfg->tuner0) == NULL) {
			pr_err("CXD2856: failed to attach tuner0 %s\n", tuner->name);
		}
	}
	else {
		pr_err("CXD2856: Missing tuner0 config\n");
	}

	return fe;
}
EXPORT_SYMBOL_GPL(aml_cxd2856_attach);

struct dvb_frontend *aml_m88dm6k_attach(const struct demod_config *cfg)
{
	struct m88rs6060_config m88rs6060cfg = {
		.demod_address = cfg->i2c_addr,
		.pin_ctrl = 0x82,
		.ci_mode = 0,
		.ts_mode = 0,
	};

	return m88rs6060_attach(&m88rs6060cfg, cfg->i2c_adap);
}

EXPORT_SYMBOL_GPL(aml_m88dm6k_attach);

struct dvb_frontend *aml_mxl603_attach(struct dvb_frontend *fe,
				       const struct tuner_config *cfg)
{
	struct mxl603_config mxl603cfg = {
		.xtal_freq_hz = cfg->xtal, /* XTAL Frequency, 0: 16MHz; 1: 24MHz */
		.if_freq_hz = cfg->if_hz, /* 0  = 3.65MHz
									 1  = 4MHz
									 2  = 4.1MHz
									 3  = 4.15MHz
									 4  = 4.5MHz
									 5  = 4.57MHz
									 6  = 5MHz
									 7  = 5.38MHz
									 8  = 6MHz
									 9  = 6.28MHz
									 10 = 7.2MHz
									 11 = 8.25MHz
									 12 = 35.25MHz
									 13 = 36MHz
									 14 = 36.15MHz
									 15 = 36.65MHz
									 16 = 44MHz */
		.agc_type = cfg->if_agc, /* AGC mode selection, self (0) or closed loop (1) */
		.xtal_cap = cfg->xtal_cap, /* XTAL capacity, 1 LSB = 1pF, maximum is 31pF */
		.gain_level = cfg->if_amp, /* IF out gain level */
		.if_out_gain_level = 11, /* IF out gain level (only for terrestial) */
		.agc_set_point = 66, /* AGC attack point set value */
		.agc_invert_pol = 0, /* Config AGC Polarity inversion */
		.invert_if = cfg->if_invert, /* IF spectrum is inverted or not */
		.loop_thru_enable = cfg->lt_out, /* Loop-Through enable */
		.clk_out_enable = 1, /* enable or disable clock out */
		.clk_out_div = 0, /* indicate if XTAL frequency is dived by 4 or not */
		.clk_out_ext = 0, /* enable or disable external clock out */
		.xtal_sharing_mode = cfg->xtal_mode, /* XTAL sharing mode. default Master, MXL608_ENABLE to config Slave mode */
		.single_supply_3_3V = cfg->dual_power ? 0 : 1, /* dual_power: 0: 3.3v, 1: 1.8v and 3.3v. */
	};
	return mxl603_attach(fe, cfg->i2c_adap, cfg->i2c_addr, &mxl603cfg);
}
EXPORT_SYMBOL_GPL(aml_mxl603_attach);

MODULE_DESCRIPTION("DVB demodulator driver wrappers for aml_dvb_extern module");
MODULE_AUTHOR("Marek Czerski (ma.czerski@gmail.com)");
MODULE_LICENSE("GPL");
