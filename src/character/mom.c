#include "mom.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Mom character structure
enum
{
	Mom_ArcMain_Idle0,
	Mom_ArcMain_Idle1,
	Mom_ArcMain_Left,
	Mom_ArcMain_Down,
	Mom_ArcMain_Up,
	Mom_ArcMain_Right,
	
	Mom_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Mom_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Mom;

//Mom character definitions
static const CharFrame char_mom_frame[] = {
	{Mom_ArcMain_Idle0, {  0,   0, 128, 256}, { 42, 163}}, //0 idle 1
	{Mom_ArcMain_Idle0, {128,   0, 128, 256}, { 41, 165}}, //1 idle 2
	{Mom_ArcMain_Idle1, {  0,   0, 128, 256}, { 41, 166}}, //2 idle 3
	{Mom_ArcMain_Idle1, {128,   0, 128, 256}, { 41, 165}}, //3 idle 4
	
	{Mom_ArcMain_Left, {  0,   0, 128, 256}, { 65, 151}}, //4 left 1
	{Mom_ArcMain_Left, {128,   0, 128, 256}, { 63, 152}}, //5 left 1
	
	{Mom_ArcMain_Down, {  0,   0, 128, 128}, { 41, 111}}, //6 down 1
	{Mom_ArcMain_Down, {128,   0, 128, 128}, { 42, 114}}, //7 down 2
	
	{Mom_ArcMain_Up, {  0,   0, 128, 256}, { 34, 196}}, //8 up 1
	{Mom_ArcMain_Up, {128,   0, 128, 256}, { 35, 193}}, //9 up 2
	
	{Mom_ArcMain_Right, {  0,   0, 128, 256}, { 62, 150}}, //10 right 1
	{Mom_ArcMain_Right, {128,   0, 128, 256}, { 61, 151}}, //11 right 2
};

static const Animation char_mom_anim[CharAnim_Max] = {
	{4, (const u8[]){ 0,  1,  2,  3, ASCR_BACK, 1}},                                           //CharAnim_Idle
	{2, (const u8[]){ 4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                                             //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                                             //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                                             //CharAnim_UpAlt
	{2, (const u8[]){10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, ASCR_CHGANI, CharAnim_Idle}}, //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},                                             //CharAnim_RightAlt
};

//Mom character functions
void Char_Mom_SetFrame(void *user, u8 frame)
{
	Char_Mom *this = (Char_Mom*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_mom_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Mom_Tick(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	if (stage.just_step)
	{
		//Perform idle dance
		if ((stage.song_step & 0x7) == 0 && character->animatable.anim == CharAnim_Idle)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Mom_SetFrame);
	Character_Draw(character, &this->tex, &char_mom_frame[this->frame]);
}

void Char_Mom_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_Mom_Free(Character *character)
{
	Char_Mom *this = (Char_Mom*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Mom_New(fixed_t x, fixed_t y)
{
	//Allocate mom object
	Char_Mom *this = Mem_Alloc(sizeof(Char_Mom));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Mom_New] Failed to allocate mom object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Mom_Tick;
	this->character.set_anim = Char_Mom_SetAnim;
	this->character.free = Char_Mom_Free;
	
	Animatable_Init(&this->character.animatable, char_mom_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character stage information
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\MOM\\MAIN.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Mom_ArcMain_Idle0
		"idle1.tim", //Mom_ArcMain_Idle1
		"left.tim",  //Mom_ArcMain_Left
		"down.tim",  //Mom_ArcMain_Down
		"up.tim",    //Mom_ArcMain_Up
		"right.tim", //Mom_ArcMain_Right
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (u8 i = 0; i < Mom_Arc_Max; i++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp++);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}