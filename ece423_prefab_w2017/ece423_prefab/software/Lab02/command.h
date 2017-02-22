#ifndef COMMAND_H_
#define COMMAND_H_

typedef enum
{
	START,
	PAUSE,
	CYCLE,
	REWIND,
	FORWARD,
	NO_CMD
}BUTTON_COMMANDS;

extern volatile BUTTON_COMMANDS command;

extern bool is_video_playing;

#endif /* COMMAND_H_ */

