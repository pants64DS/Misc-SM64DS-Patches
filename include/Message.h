#ifndef MESSAGE_INCLUDED
#define MESSAGE_INCLUDED

#include "SM64DS_Common.h"

//0xfd is new line
//0xff is end of text
/*0xfe is special
	Next byte is length of structure
	Next comes where to branch
	if 0:
		Next halfword ???
		if 0:
			Next byte is what to do
			Last byte (L)
			if 0:
				str = L - numStars
*/

struct Message
{
	/*static wchar_t chars = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F',
							'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
							'W', 'X', 'Y', 'Z', '「', '」', '?', '!', '~', ',', '“', '”', '•', 'a', 'b', 'c',
							'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
							't', 'u', 'v', 'w', 'x', 'y', 'z', '-', '.','\'', ':', ';', '&', ' ', '/', ' ',
							'€', ' ', '‚', 'ƒ', '„', '…', '†', '‡', '^', '‰', 'Š', '<', 'Œ', ' ', 'Ž', ' ',
							' ', '‘', '’', ' ', ' ', '•', '–', '—', '˜', '™', 'š', '>', 'œ', ' ', 'ž', 'Ÿ',
							' ', '¡', '¢', '£', '¤', '¥', '¦', '§', '¨', '©', 'ª', '«', '¬', '­', '®', '¯',
							'°', '±', '²', '³', '´', 'µ', '¶', '•', '¸', '¹', 'º', '»', '¼', '½', '¾', '¿',
							'À', 'Á', 'Â', 'Ã', 'Ä', 'Å', 'Æ', 'Ç', 'È', 'É', 'Ê', 'Ë', 'Ì', 'Í', 'Î', 'Ï',
							'Ð', 'Ñ', 'Ò', 'Ó', 'Ô', 'Õ', 'Ö', '×', 'Ø', 'Ù', 'Ú', 'Û', 'Ü', 'Ý', 'Þ', 'ß'};*/
							
	unsigned textOffset;
	short textBoxWidth;
	short textBoxHeight; // in lines
	
	static void PrepareTalk();
	static void EndTalk();
	
	static void AddChar(char charInFontEncoding);
	static void Display(unsigned msgID);
};

struct SpriteRef
{
	unsigned unk0;
	unsigned unk4;

	static SpriteRef COIN;
	static SpriteRef* NUMBER_PTRS[20]; // first 10 are gold versions of 0-9, last 10 are red versions of 0-9
	static SpriteRef TIMES;
};

// Add 9 to the horizontal spacing to show multiple digits
// Usual values given as default arguments
void ShowNumber(bool onBottomScreen, SpriteRef& number, int x, int y, int gradientSetting = -1, unsigned arg5 = 1, unsigned arg6 = 0);

inline void ShowDecimalInt(unsigned val, int x, int y, bool onBottomScreen = false, bool isRed = false)
{
	uint8_t digits[10]; // 32-bit integers can have up to 10 decimal digits
	int numDigits = 0;

	if (val == 0)
	{
		digits[0] = 0;
		numDigits = 1;
	}
	else while (val)
	{
		digits[numDigits++] = val % 10;
		val /= 10;
	}

	const unsigned offset = isRed * 10;

	for (int i = numDigits - 1; i >= 0; --i)
	{
		ShowNumber(onBottomScreen, *SpriteRef::NUMBER_PTRS[offset + digits[i]], x, y);
		x += 9;
	}
}

struct MsgFile
{
	unsigned magic;
	unsigned sectionSize;
	uint16_t numMsgs;
	uint16_t unk0a;
	unsigned unk0c;
	
	Message& message(int id) {return *(Message*)((char*)this + 0x10 + 0x08 * id);}
	char* rawMsgText(int id) {return *(char**)((char*)this + 0x08 + sectionSize + message(id).textOffset);}
};

struct MsgIDCharEntry
{
	uint16_t perCharID;
	uint16_t msgID;
};

extern "C"
{
	extern uint8_t MSG_BOX_CURR_LEFT_X;
	extern uint8_t MSG_BOX_CURR_TOP_Y;
	extern uint8_t MSG_BOX_MIN_LEFT_X;
	extern uint8_t MSG_BOX_MIN_TOP_Y;
	extern uint8_t MSG_BOX_CENTER_X;
	extern uint8_t MSG_BOX_CENTER_Y;

	extern int MSG_LINE_HEIGHT;
	extern int CURR_MSG_ID;
	extern Message* CURR_MSG_PTR;
	extern char* CURR_MSG_TEXT_CHAR;
	extern Message* MSG_ARR_PTR;
	extern MsgFile* MSG_FILE_PTR;
	extern MsgIDCharEntry MSG_ID_CHAR_MAP[0x62];
	extern uint16_t UTF16_TO_FONT_TABLE[0x100];
	extern uint8_t TALK_FONT_CHAR_WIDTHS[0x100];
}

using MsgGenTextFunc = void(*)();

#endif