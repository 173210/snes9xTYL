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
		[HELP_SHOWPASS] = "������ʂ�`�悷��̂ɕK�v�Ƃ����`��֐����Ăяo���Ă��邩��\\�����܂��B\n�Ⴏ��΁A�����Ȃ�܂��B����̓n�b�N(�ڍׂ͈ȉ�)�̉e�����m�F����̂Ɏg���܂��B\n",
		[HELP_APPLYHACKS] = "�f�[�^�x�[�X�ɑ��݂���ꍇSNESAdvance�X�s�[�h�n�b�N���g�p���܂��B\nROM�ǂݍ��ݎ��݂̂Ɏg���܂��B",
		[HELP_IGNORE_FIXCOL] = "�Œ肵���F�ʕω��𖳎����܂��B\n���̂悤�ȕύX�͉�ʂ̕ω����i�s���邫�������ɂ͂Ȃ�܂���B\nCG���ω�����\\��������܂��B(�O���f�[�V�����Ȃ�)",
		[HELP_IGNORE_WINCLIP] = "��ʂ�؂���ω��𖳎����܂��B\n���̂悤�ȕύX�͉�ʂ̕ω����i�s���邫�������ɂ͂Ȃ�܂���B\nCG���ω�����\\��������܂��B(missing area�Ȃ�)",
		[HELP_IGNORE_ADDSUB] = "�ǉ�/�T�u���[�h�̕ω��𖳎����܂��B\n���̂悤�ȕύX�͉�ʂ̕ω����i�s���邫�������ɂ͂Ȃ�܂���B\nCG���ω�����\\��������܂��B(�s�N�Z�������߂̑���ɂ����ނȂ�)",
		[HELP_IGNORE_PALWRITE] = "�p���b�g�������ω��𖳎����܂��B\n���̂悤�ȕύX�͉�ʂ̕ω����i�s���邫�������ɂ͂Ȃ�܂���B\nCG���ω�����\\��������܂��B(������F�ɂȂ�Ȃ�)",
		[HELP_OBJ] = "�I�u�W�F�N�g�̃I��/�I�t�ł��B\n�I�u�W�F�N�g�`��𖳌���(�I�t)�ɂ��邽�߂Ɏg���܂��B",
		[HELP_BG0] = "�w�i0�̃I��/�I�t�ł��B\n�w�i0�̕`��𖳌���(�I�t)�ɂ��邽�߂Ɏg���܂��B",
		[HELP_BG1] = "�w�i1�̃I��/�I�t�ł��B\n�w�i1�̕`��𖳌���(�I�t)�ɂ��邽�߂Ɏg���܂��B",
		[HELP_BG2] = "�w�i2�̃I��/�I�t�ł��B\n�w�i2�̕`��𖳌���(�I�t)�ɂ��邽�߂Ɏg���܂��B",
		[HELP_BG3] = "�w�i3�̃I��/�I�t�ł��B\n�w�i3�̕`��𖳌���(�I�t)�ɂ��邽�߂Ɏg���܂��B",
		[HELP_EASY] = "���߂������܂���B\n���ߕ`��������I�ɂȂ������߂Ɏg���܂��B\nCG���ω�����\\��������܂��B(�s�N�Z�������߂̑���ɂ����ނȂ�)",
		[HELP_FASTSPRITE] = "�����X�v���C�g\n�����X�v���C�g�̗D�摀��𖳌��ɂ��邽�߂Ɏg���܂��B\nCG���ω�����\\��������܂��B"
	}
};
