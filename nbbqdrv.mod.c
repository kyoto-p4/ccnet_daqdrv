#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0x67fc9960, "ccnet_read16" },
	{ 0xd62c6d0f, "ccnet_read24" },
	{ 0xb1ad68e1, "ccnet_block_read24" },
	{ 0x41fb9b1a, "ccnet_block_read16" },
	{ 0x6226b655, "ccnet_crate_define_lam" },
	{ 0xdbe0e146, "ccnet_crate_reset" },
	{ 0x1f4bcd5, "ccnet_rfs_enable_interrupt" },
	{ 0xf18d3e76, "ccnet_write16" },
	{ 0x6fbfbfdf, "ccnet_exec" },
	{ 0xae1b865b, "ccnet_control" },
	{ 0xc87ef0ce, "ccnet_exec_pio" },
	{ 0x35feb414, "ccnet_check_lam" },
	{ 0x36296653, "ccnet_exec_dma" },
	{ 0x7af15c35, "ccnet_rfs_disable_interrupt" },
	{ 0x9f353f8a, "ccnet_get_irq" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=bb-ccnet";

