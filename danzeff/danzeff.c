#include "danzeff.h"

// headerfiles needed for the GU version
#ifdef DANZEFF_SCEGU
#include <malloc.h>
#include "pspkernel.h"
#include "pspgu.h"
#include "png.h"
#include "psp/pg.h"
#endif // #ifdef DANZEFF_SCEGU

#define false 0
#define true 1

// structures used for drawing the keyboard
#ifdef DANZEFF_SCEGU
	struct danzeff_vertex
	{
		float u, v;
		unsigned int color;
		float x,y,z;
	};
	
	struct danzeff_vertex16
	{		
		float u, v;		
		float x,y,z;
	};

	struct danzeff_gu_surface
	{
		u32		surface_width;
		u32		surface_height;
		u32		texture_width;
		u32		texture_height;
		u32		*texture;
	};

#endif //#ifdef DANZEFF_SCEGU

/*bool*/ int holding = false;     //user is holding a button
/*bool*/ int dirty = true;        //keyboard needs redrawing
/*bool*/ int shifted = false;     //user is holding shift
int mode = 0;             //charset selected. (0 - letters or 1 - numbers)
/*bool*/ int initialized = false; //keyboard is initialized

//Position on the 3-3 grid the user has selected (range 0-2)
int selected_x = 1;
int selected_y = 1;

//Variable describing where each of the images is
#define guiStringsSize 12 /* size of guistrings array */
#define PICS_BASEDIR "DATA/graphics/"
char *guiStrings[] =
{
	PICS_BASEDIR "keys.png", PICS_BASEDIR "keys_t.png", PICS_BASEDIR "keys_s.png",
	PICS_BASEDIR "keys_c.png", PICS_BASEDIR "keys_c_t.png", PICS_BASEDIR "keys_s_c.png",
	PICS_BASEDIR "nums.png", PICS_BASEDIR "nums_t.png", PICS_BASEDIR "nums_s.png",
	PICS_BASEDIR "nums_c.png", PICS_BASEDIR "nums_c_t.png", PICS_BASEDIR "nums_s_c.png"
};

//amount of modes (non shifted), each of these should have a corresponding shifted mode.
#define MODE_COUNT 2
//this is the layout of the keyboard
char modeChar[MODE_COUNT*2][3][3][5] =
{
	{	//standard letters
		{ ",abc",  ".def","!ghi" },
		{ "-jkl","\010m n", "?opq" },
		{ "(rst",  ":uvw",")xyz" }
	},

	{	//capital letters
		{ "^ABC",  "@DEF","*GHI" },
		{ "_JKL","\010M N", "\"OPQ" },
		{ "=RST",  ";UVW","/XYZ" }
	},

	{	//numbers
		{ "\0\0\0001","\0\0\0002","\0\0\0003" },
		{ "\0\0\0004",  "\010\0 5","\0\0\0006" },
		{ "\0\0\0007","\0\0\0008", "\0\00009" }
	},

	{	//special characters
		{ "'(.)",  "\"<'>","-[_]" },
		{ "!{?}","\010\0 \0", "+\\=/" },
		{ ":@;#",  "~$`%","*^|&" }
	}
};

/*bool*/ int danzeff_isinitialized()
{
	return initialized;
}

/*bool*/ int danzeff_dirty()
{
	return dirty;
}

/** Attempts to read a character from the controller
* If no character is pressed then we return 0
* Other special values: 1 = move left, 2 = move right, 3 = select, 4 = start
* Every other value should be a standard ascii value.
* An unsigned int is returned so in the future we can support unicode input
*/
unsigned int danzeff_readInput(SceCtrlData pspctrl)
{
	//Work out where the analog stick is selecting
	int x = 1;
	int y = 1;
	if (pspctrl.Lx < 85)     x -= 1;
	else if (pspctrl.Lx > 170) x += 1;

	if (pspctrl.Ly < 85)     y -= 1;
	else if (pspctrl.Ly > 170) y += 1;

	if (selected_x != x || selected_y != y) //If they've moved, update dirty
	{
		dirty = true;
		selected_x = x;
		selected_y = y;
	}
	//if they are changing shift then that makes it dirty too
	if ((!shifted && (pspctrl.Buttons & PSP_CTRL_RTRIGGER)) || (shifted && !(pspctrl.Buttons & PSP_CTRL_RTRIGGER)))
		dirty = true;

	unsigned int pressed = 0; //character they have entered, 0 as that means 'nothing'
	shifted = (pspctrl.Buttons & PSP_CTRL_RTRIGGER)?true:false;

	if (!holding)
	{
		if (pspctrl.Buttons& (PSP_CTRL_CROSS|PSP_CTRL_CIRCLE|PSP_CTRL_TRIANGLE|PSP_CTRL_SQUARE)) //pressing a char select button
		{
			int innerChoice = 0;
			if      (pspctrl.Buttons & PSP_CTRL_TRIANGLE)
				innerChoice = 0;
			else if (pspctrl.Buttons & PSP_CTRL_SQUARE)
				innerChoice = 1;
			else if (pspctrl.Buttons & PSP_CTRL_CROSS)
				innerChoice = 2;
			else //if (pspctrl.Buttons & PSP_CTRL_CIRCLE)
				innerChoice = 3;

			//Now grab the value out of the array
			pressed = modeChar[ mode*2 + shifted][y][x][innerChoice];
		}
		else if (pspctrl.Buttons& PSP_CTRL_LTRIGGER) //toggle mode
		{
			dirty = true;
			mode++;
			mode %= MODE_COUNT;
		}
		else if (pspctrl.Buttons& PSP_CTRL_DOWN)
		{
			pressed = '\n';
		}
		else if (pspctrl.Buttons& PSP_CTRL_UP)
		{
			pressed = 8; //backspace
		}
		else if (pspctrl.Buttons& PSP_CTRL_LEFT)
		{
			pressed = DANZEFF_LEFT; //LEFT
		}
		else if (pspctrl.Buttons& PSP_CTRL_RIGHT)
		{
			pressed = DANZEFF_RIGHT; //RIGHT
		}
		else if (pspctrl.Buttons& PSP_CTRL_SELECT)
		{
			pressed = DANZEFF_SELECT; //SELECT
		}
		else if (pspctrl.Buttons& PSP_CTRL_START)
		{
			pressed = DANZEFF_START; //START
		}
	}

	holding = pspctrl.Buttons & ~PSP_CTRL_RTRIGGER; //RTRIGGER doesn't set holding

	return pressed;
}

///-------------------------------------------------------------------------------
///These are specific to the implementation, they should have the same behaviour across implementations.
///-------------------------------------------------------------------------------


///-------------------------------------------------------------------------------
///This is the original SDL implementation
#ifdef DANZEFF_SDL

SDL_Surface* keyBits[guiStringsSize];
int keyBitsSize = 0;
int moved_x = 0, moved_y = 0; // location that we are moved to

///variable needed for rendering in SDL, the screen surface to draw to, and a function to set it!
SDL_Surface* danzeff_screen;
SDL_Rect danzeff_screen_rect;
void danzef_set_screen(SDL_Surface* screen)
{
	danzeff_screen = screen;
	danzeff_screen_rect.x = 0;
	danzeff_screen_rect.y = 0;
	danzeff_screen_rect.h = screen->h;
	danzeff_screen_rect.w = screen->w;
}


///Internal function to draw a surface internally offset
//Render the given surface at the current screen position offset by screenX, screenY
//the surface will be internally offset by offsetX,offsetY. And the size of it to be drawn will be intWidth,intHeight
void surface_draw_offset(SDL_Surface* pixels, int screenX, int screenY, int offsetX, int offsetY, int intWidth, int intHeight)
{
	//move the draw position
	danzeff_screen_rect.x = moved_x + screenX;
	danzeff_screen_rect.y = moved_y + screenY;

	//Set up the rectangle
	SDL_Rect pixels_rect;
	pixels_rect.x = offsetX;
	pixels_rect.y = offsetY;
	pixels_rect.w = intWidth;
	pixels_rect.h = intHeight;

	SDL_BlitSurface(pixels, &pixels_rect, danzeff_screen, &danzeff_screen_rect);
}

///Draw a surface at the current moved_x, moved_y
void surface_draw(SDL_Surface* pixels)
{
	surface_draw_offset(pixels, 0, 0, 0, 0, pixels->w, pixels->h);
}

/* load all the guibits that make up the OSK */
void danzeff_load(char *path)
{
	char str_tmp[256];
	if (initialized) return;

	int a;
	for (a = 0; a < guiStringsSize; a++)
	{
		sprintf(str_tmp,"%s/%s",path,guiStrings[a]);
		keyBits[a] = IMG_Load(str_tmp);
		if (keyBits[a] == NULL)
		{
			//ERROR! out of memory.
			//free all previously created surfaces and set initialized to false
			int b;
			for (b = 0; b < a; b++)
			{
				SDL_FreeSurface(keyBits[b]);
				keyBits[b] = NULL;
			}
			initialized = false;
			return;
		}
	}
	initialized = true;
}

/* remove all the guibits from memory */
void danzeff_free()
{
	if (!initialized) return;

	int a;
	for (a = 0; a < guiStringsSize; a++)
	{
		SDL_FreeSurface(keyBits[a]);
		keyBits[a] = NULL;
	}
	initialized = false;
}

/* draw the keyboard at the current position */
void danzeff_render()
{
	printf("Drawing Keyboard at %i,%i\n", selected_x, selected_y);
	dirty = false;

	///Draw the background for the selected keyboard either transparent or opaque
	///this is the whole background image, not including the special highlighted area
	//if center is selected then draw the whole thing opaque
	if (selected_x == 1 && selected_y == 1)
		surface_draw(keyBits[6*mode + shifted*3]);
	else
		surface_draw(keyBits[6*mode + shifted*3 + 1]);

	///Draw the current Highlighted Selector (orange bit)
	surface_draw_offset(keyBits[6*mode + shifted*3 + 2],
	//Offset from the current draw position to render at
	selected_x*43, selected_y*43,
	//internal offset of the image
	selected_x*64,selected_y*64,
	//size to render (always the same)
	64, 64);
}

/* move the position the keyboard is currently drawn at */
void danzeff_moveTo(const int newX, const int newY)
{
	moved_x = newX;
	moved_y = newY;
}

#endif //DANZEFF_SDL


///-------------------------------------------------------------------------------
///This is the GU implementation
#ifdef DANZEFF_SCEGU

struct danzeff_gu_surface	keyTextures[guiStringsSize];

int moved_x = 0, moved_y = 0; // location that we are moved to


///Internal function to draw a surface internally offset
//Render the given surface at the current screen position offset by screenX, screenY
//the surface will be internally offset by offsetX,offsetY. And the size of it to be drawn will be intWidth,intHeight
void surface_draw_offset(struct danzeff_gu_surface* surface, int screenX, int screenY, int offsetX, int offsetY, int intWidth, int intHeight)
{
	sceGuAlphaFunc( GU_GREATER, 0, 0xff );
	sceGuEnable( GU_ALPHA_TEST );
	sceGuTexFunc(GU_TFX_BLEND,GU_TCC_RGBA);
	sceGuTexEnvColor(0xFF000000);
	sceGuBlendFunc( GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0 );
	sceGuEnable( GU_BLEND );
	sceGuTexMode(GU_PSM_8888,0,0,GU_FALSE);
	sceGuTexImage(0,surface->surface_width, surface->surface_height,surface->surface_width, surface->texture);
	sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);

	struct danzeff_vertex* c_vertices = (struct danzeff_vertex*)sceGuGetMemory(2 * sizeof(struct danzeff_vertex));

	c_vertices[0].u 		= offsetX;
	c_vertices[0].v 		= offsetY;
	c_vertices[0].x 		= moved_x + screenX;
	c_vertices[0].y 		= moved_y + screenY;
	c_vertices[0].z 		= 0;
	c_vertices[0].color 	= 0xFFFFFFFF;

	c_vertices[1].u 		= offsetX + intWidth;
	c_vertices[1].v 		= offsetY + intHeight;
	c_vertices[1].x 		= moved_x + screenX + intWidth;
	c_vertices[1].y 		= moved_y + screenY + intHeight;
	c_vertices[1].z 		= 0;
	c_vertices[1].color 	= 0xFFFFFFFF;

	sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,c_vertices);

	sceGuDisable( GU_BLEND );
	sceGuDisable( GU_ALPHA_TEST );
}

void surface_draw_offset16(struct danzeff_gu_surface* surface, int screenX, int screenY, int offsetX, int offsetY, int intWidth, int intHeight)
{
	unsigned short *dst;
	unsigned short *src;
	
	int y;
	for (y=0;y<intHeight;y++) {
		src=((unsigned short *)(surface->texture)) + offsetX + (y+offsetY)*surface->surface_width;
		dst=(unsigned short *)pgGetVramAddr(moved_x+screenX,moved_y+screenY+y);
		memcpy(dst,src,intWidth*2);
	}
}

void danzeff_block_copy16(struct danzeff_gu_surface* surface, u32 *texture)
{
	u16 *dest = (u16*)(surface->texture);	
	u32 y, x,col,col16;

	for (y = 0 ; y < surface->texture_height ; y++)
	{
		for (x = 0 ; x < surface->texture_width ; x++)
		{
		//*dest++ = *texture++;
			col=*texture++;
			col16=(col>>(16+3-10)&(0x1f<<10)) | (col>>(8+3-5)&(0x1f<<5))| (col>>(3-0)&(0x1f<<0));
			*dest++=col16;
		}		
	}
}

void danzeff_block_copy(struct danzeff_gu_surface* surface, u32 *texture)
{
	u32 *dest = surface->texture;
	u32 stride = surface->surface_width - surface->texture_width;
	u32 y, x;

	for (y = 0 ; y < surface->texture_height ; y++)
	{
		for (x = 0 ; x < surface->texture_width ; x++)
		{
		*dest++ = *texture++;
		}
		// skip at the end of each line
		if (stride > 0)
		{
			dest += stride;
		}
	}
}

static void danzeff_user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
	// ignore PNG warnings
}

/* Get the width and height of a png image */
int danzeff_get_png_image_size(const char* filename, u32 *png_width, u32 *png_height)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL) return -1;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return -1;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, danzeff_user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return -1;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);

	*png_width = width;
	*png_height = height;
	return 0;
}

/* Load a texture from a png image */
int danzeff_load_png_image(const char* filename, u32 *ImageBuffer)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	size_t x, y;
	u32* line;
	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL) return -1;
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL) {
		fclose(fp);
		return -1;
	}
	png_set_error_fn(png_ptr, (png_voidp) NULL, (png_error_ptr) NULL, danzeff_user_warning_fn);
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return -1;
	}
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, sig_read);
	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	png_set_strip_16(png_ptr);
	png_set_packing(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
	png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	line = (u32*) malloc(width * 4);
	if (!line) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return -1;
	}
	for (y = 0; y < height; y++)
	{
		png_read_row(png_ptr, (u8*) line, NULL);
		for (x = 0; x < width; x++)
		{
			ImageBuffer[y*width+x] = line[x];
		}
	}
	free(line);
	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	return 0;
}

u32 danzeff_convert_pow2(u32 size)
{
	u32 pow_counter = 0;

	for ( ; pow_counter < 32 ; pow_counter++)
	{
		// Find the first value which is higher
		if ((size >> pow_counter) == 0)
		{
			// take already good values into account
			if ((1 << pow_counter) != size)
			{
				return (1 << pow_counter);
			}
			else
			{
				return (1 << (pow_counter-1));
			}
		}
	}
	return 0;
}

/* load all the guibits that make up the OSK */
void danzeff_load(char *path)
{
	char str_tmp[256];
	u32 *temp_texture;

	if (initialized) return;

	int a;
	for (a = 0; a < guiStringsSize; a++)
	{
		u32 height, width;
		
		sprintf(str_tmp,"%s%s",path,guiStrings[a]);		
		if (danzeff_get_png_image_size(str_tmp, &width, &height) == 0)
		{
			// The png is always converted to PSM_8888 format when read
			temp_texture = (u32 *)malloc(width*height*4);
			if (danzeff_load_png_image(str_tmp, temp_texture) != 0)
			{
				// Error .. Couldn't get png info from one of the needed files
				int b;
	
				for (b = 0; b < a; b++)
				{
					free(keyTextures[b].texture);
					keyTextures[b].texture = NULL;
				}
				initialized = false;
				return;
			}
			else
			{
				// we need to store the texture in an image of width and heights of 2^n sizes
				keyTextures[a].texture_width 	= (float)width;
				keyTextures[a].texture_height 	= (float)height;
				keyTextures[a].surface_width 	= (float)danzeff_convert_pow2(width);
				keyTextures[a].surface_height 	= (float)danzeff_convert_pow2(height);
				keyTextures[a].texture 			= (u32 *)malloc(keyTextures[a].surface_width*keyTextures[a].surface_height*4);
				// block copy the texture into the surface
				danzeff_block_copy(&keyTextures[a], temp_texture);
				free(temp_texture);
			}
		}
		else
		{
			// Error .. Couldn't get png info from one of the needed files
			int b;
	
			for (b = 0; b < a; b++)
			{
				free(keyTextures[b].texture);
				keyTextures[b].texture = NULL;
			}
			initialized = false;
			return;
		}
	}
	initialized = true;
}

/* load all the guibits that make up the OSK */
void danzeff_load16(char *path)
{
	char str_tmp[256];
	u32 *temp_texture;

	if (initialized) return;

	int a;
	for (a = 0; a < guiStringsSize; a++)
	{
		u32 height, width;
		
		sprintf(str_tmp,"%s%s",path,guiStrings[a]);		
		if (danzeff_get_png_image_size(str_tmp, &width, &height) == 0)
		{
			// The png is always converted to PSM_8888 format when read
			temp_texture = (u32 *)malloc(width*height*4);
			if (danzeff_load_png_image(str_tmp, temp_texture) != 0)
			{
				// Error .. Couldn't get png info from one of the needed files
				int b;
	
				for (b = 0; b < a; b++)
				{
					free(keyTextures[b].texture);
					keyTextures[b].texture = NULL;
				}
				initialized = false;
				return;
			}
			else
			{
				// we need to store the texture in an image of width and heights of 2^n sizes
				keyTextures[a].texture_width 	= (float)width;
				keyTextures[a].texture_height 	= (float)height;
				keyTextures[a].surface_width 	= /*(float)danzeff_convert_pow2*/(width);
				keyTextures[a].surface_height 	= /*(float)danzeff_convert_pow2*/(height);
				keyTextures[a].texture 			= (u32 *)malloc(keyTextures[a].surface_width*keyTextures[a].surface_height*2);
				// block copy the texture into the surface
				danzeff_block_copy16(&keyTextures[a], temp_texture);
				free(temp_texture);
			}
		}
		else
		{
			// Error .. Couldn't get png info from one of the needed files
			int b;
	
			for (b = 0; b < a; b++)
			{
				free(keyTextures[b].texture);
				keyTextures[b].texture = NULL;
			}
			initialized = false;
			return;
		}
	}
	initialized = true;
}

/* remove all the guibits from memory */
void danzeff_free()
{
	if (!initialized) return;

	int a;
	for (a = 0; a < guiStringsSize; a++)
	{
		free(keyTextures[a].texture);
		keyTextures[a].texture = NULL;
	}
	initialized = false;
}

/* draw the keyboard at the current position */
void danzeff_render()
{
	dirty = false;

	///Draw the background for the selected keyboard either transparent or opaque
	///this is the whole background image, not including the special highlighted area
	//if center is selected then draw the whole thing opaque
	if (selected_x == 1 && selected_y == 1)
		surface_draw_offset16(&keyTextures[6*mode + shifted*3], 0, 0, 0, 0, keyTextures[6*mode + shifted*3].texture_width,
																		  keyTextures[6*mode + shifted*3].texture_height);
	else
		surface_draw_offset16(&keyTextures[6*mode + shifted*3 + 1], 0, 0, 0, 0, keyTextures[6*mode + shifted*3 + 1].texture_width,
																			  keyTextures[6*mode + shifted*3 + 1].texture_height);

	///Draw the current Highlighted Selector (orange bit)
	surface_draw_offset16(&keyTextures[6*mode + shifted*3 + 2],
	//Offset from the current draw position to render at
	selected_x*43, selected_y*43,
	//internal offset of the image
	selected_x*64,selected_y*64,
	//size to render (always the same)
	64, 64);
}

/* move the position the keyboard is currently drawn at */
void danzeff_moveTo(const int newX, const int newY)
{
	moved_x = newX;
	moved_y = newY;
}

#endif //DANZEFF_SCEGU
