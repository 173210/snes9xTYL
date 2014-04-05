#include "psp.h"

enum {
	HELP_EN,
	HELP_JA,

	HELP_LANG_TOTAL
};

enum {
	HELP_SHOWPASS,
	HELP_APPLYHACKS,
	HELP_IGNORE_FIXCOL,
	HELP_IGNORE_WINCLIP,
	HELP_IGNORE_ADDSUB,
	HELP_IGNORE_PALWRITE,
	HELP_OBJ,
	HELP_BG0,
	HELP_BG1,
	HELP_BG2,
	HELP_BG3,
	HELP_EASY,
	HELP_FASTSPRITE,

	HELP_TOTAL_ENTRIES
};

static const char *help_data_ml[HELP_LANG_TOTAL][HELP_TOTAL_ENTRIES] = {
	[HELP_EN] = {
	//english
		[HELP_SHOWPASS] = "Show how much call to rendering function was needed to compute the screen\nThe lowest, the fastest. This is usefull to check the impact of hacks (below)\n",
		[HELP_APPLYHACKS] = "Use SNESAdvance speedhacks if available in the database.\nOnly used at rom loading time.",
		[HELP_IGNORE_FIXCOL] = "Ignore Fixed Colour Changes.\nSuch changes won't trigger the screen update process.\nGFX may be altered (gradient, ...)",
		[HELP_IGNORE_WINCLIP] = "Ignore Windows clipping Changes.\nSuch changes won't trigger the screen update process.\nGFX may be altered (missing area, ...)",
		[HELP_IGNORE_ADDSUB] = "Ignore Add/Sub modes Changes.\nSuch changes won't trigger the screen update process.\nGFX may be altered (pixels being opaque instead of transparent, ...)",
		[HELP_IGNORE_PALWRITE] = "Ignore Palette writes Changes.\nSuch changes won't trigger the screen update process.\nGFX may be altered (wrong colours, ...)",
		[HELP_OBJ] = "OBJ On/Off.\nUse this to disactivate(Off) SPRITES drawing.",
		[HELP_BG0] = "BG0 On/Off.\nUse this to disactivate(Off) BACKGROUND 0 drawing.",
		[HELP_BG1] = "BG1 On/Off.\nUse this to disactivate(Off) BACKGROUND 1 drawing.",
		[HELP_BG2] = "BG2 On/Off.\nUse this to disactivate(Off) BACKGROUND 2 drawing.",
		[HELP_BG3] = "BG3 On/Off.\nUse this to disactivate(Off) BACKGROUND 3 drawing.",
		[HELP_EASY] = "No Transparency.\nUse this to force no transparency rendering.\nGFX may be altered (pixels being opaque instead of transparent, ...)",
		[HELP_FASTSPRITE] = "Fast sprites.\nUse this to disactivate intra-sprites priorities handling.\nGFX may be altered"
	},
	[HELP_JA] = {
	//japanese
		[HELP_SHOWPASS] = "いくつ画面を描画するのに必要とされる描画関数を呼び出しているかを表\示します。\n低ければ、早くなります。これはハック(詳細は以下)の影響を確認するのに使えます。\n",
		[HELP_APPLYHACKS] = "データベースに存在する場合SNESAdvanceスピードハックを使用します。\nROM読み込み時のみに使われます。",
		[HELP_IGNORE_FIXCOL] = "固定した色彩変化を無視します。\nそのような変更は画面の変化が進行するきっかけにはなりません。\nCGが変化する可能\性があります。(グレデーションなど)",
		[HELP_IGNORE_WINCLIP] = "画面を切り取る変化を無視します。\nそのような変更は画面の変化が進行するきっかけにはなりません。\nCGが変化する可能\性があります。(missing areaなど)",
		[HELP_IGNORE_ADDSUB] = "追加/サブモードの変化を無視します。\nそのような変更は画面の変化が進行するきっかけにはなりません。\nCGが変化する可能\性があります。(ピクセルが透過の代わりにくすむなど)",
		[HELP_IGNORE_PALWRITE] = "パレットが書く変化を無視します。\nそのような変更は画面の変化が進行するきっかけにはなりません。\nCGが変化する可能\性があります。(誤った色になるなど)",
		[HELP_OBJ] = "オブジェクトのオン/オフです。\nオブジェクト描画を無効化(オフ)にするために使います。",
		[HELP_BG0] = "背景0のオン/オフです。\n背景0の描画を無効化(オフ)にするために使います。",
		[HELP_BG1] = "背景1のオン/オフです。\n背景1の描画を無効化(オフ)にするために使います。",
		[HELP_BG2] = "背景2のオン/オフです。\n背景2の描画を無効化(オフ)にするために使います。",
		[HELP_BG3] = "背景3のオン/オフです。\n背景3の描画を無効化(オフ)にするために使います。",
		[HELP_EASY] = "透過をさせません。\n透過描画を強制的になくすために使います。\nCGが変化する可能\性があります。(ピクセルが透過の代わりにくすむなど)",
		[HELP_FASTSPRITE] = "高速スプライト\n内臓スプライトの優先操作を無効にするために使います。\nCGが変化する可能\性があります。"
	}
};
