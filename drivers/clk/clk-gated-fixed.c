// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2026 Daniele Briguglio <hello@superkali.me>
 *
 * Clock driver for a fixed-rate oscillator gated by a regulator
 * supply (Linux DT binding "gated-fixed-clock").
 */

#define LOG_CATEGORY UCLASS_CLK

#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <power/regulator.h>
#include <dm/device_compat.h>

struct clk_gated_fixed_priv {
	struct udevice	*supply;
	ulong		rate;
};

static int clk_gated_fixed_enable(struct clk *clk)
{
	struct clk_gated_fixed_priv *priv = dev_get_priv(clk->dev);

	return regulator_set_enable_if_allowed(priv->supply, true);
}

static int clk_gated_fixed_disable(struct clk *clk)
{
	struct clk_gated_fixed_priv *priv = dev_get_priv(clk->dev);

	return regulator_set_enable_if_allowed(priv->supply, false);
}

static ulong clk_gated_fixed_get_rate(struct clk *clk)
{
	struct clk_gated_fixed_priv *priv = dev_get_priv(clk->dev);

	return priv->rate;
}

static const struct clk_ops clk_gated_fixed_ops = {
	.enable		= clk_gated_fixed_enable,
	.disable	= clk_gated_fixed_disable,
	.get_rate	= clk_gated_fixed_get_rate,
};

static int clk_gated_fixed_probe(struct udevice *dev)
{
	struct clk_gated_fixed_priv *priv = dev_get_priv(dev);
	u32 rate;
	int ret;

	ret = dev_read_u32(dev, "clock-frequency", &rate);
	if (ret) {
		dev_err(dev, "missing clock-frequency: %d\n", ret);
		return ret;
	}
	priv->rate = rate;

	ret = device_get_supply_regulator(dev, "vdd-supply", &priv->supply);
	if (ret) {
		dev_err(dev, "failed to get vdd-supply: %d\n", ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id clk_gated_fixed_match[] = {
	{ .compatible = "gated-fixed-clock" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(gated_fixed_clock) = {
	.name		= "gated_fixed_clock",
	.id		= UCLASS_CLK,
	.of_match	= clk_gated_fixed_match,
	.probe		= clk_gated_fixed_probe,
	.priv_auto	= sizeof(struct clk_gated_fixed_priv),
	.ops		= &clk_gated_fixed_ops,
};
