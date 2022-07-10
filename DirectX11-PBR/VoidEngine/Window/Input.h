#pragma once
#include "Utility/Types.h"
class Window;

class Input {
	friend Window;
public:
	Input();
	~Input();

	enum class Scroll {
		HORIZONTAL = 0,
		VERTICAL = 1
	};

	static vec2f MousePosition();
	static vec2f MousePosDifference();
	static bool MouseHasMoved();
	static int MouseDeltaPosition();

	static bool IsMouseKeyDown(const int& button);
	static bool IsMouseKeyPressed(const int& button);
	static bool IsLeftMouseKeyDown();
	static bool IsLeftMouseKeyPressed();
	static bool IsRightMouseKeyDown();
	static bool IsRightMouseKeyPressed();
	static bool IsMouseKeyUp(const int& button);
	static bool IsMouseKeyReleased(const int& button);
	static bool IsLeftMouseKeyUp();
	static bool IsLeftMouseKeyReleased();
	static bool IsRightMouseKeyUp();
	static bool IsRightMouseKeyReleased();
	static bool IsKeyDown(const int& key);			// Key held down
	static bool IsKeyPressed(const int& key);		// Key pressed

	static bool getCharInputBuffer(char* c);

	static double GetMouseScroll(Scroll direction = Scroll::VERTICAL);

protected:
	static void Update();

private:
	static int mouseScrollState[2];

	static char keyBuffer[8];
	static char keyBufferIndex;
	static char keyBufferSearchIndex;

	static unsigned int keyStates[348];
	static unsigned int keyPrevStates[348];
	static unsigned int mouseButtonStates[7];
	static unsigned int mouseButtonPrevStates[7];
	static vec2f mousePosition;
	static vec2f mouseLastPosition;
	static bool mouseHasMoved;
	static int mouseScroll;
};