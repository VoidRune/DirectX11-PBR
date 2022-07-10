#include "Input.h"

unsigned int Input::keyStates[348];
unsigned int Input::keyPrevStates[348];
unsigned int Input::mouseButtonStates[7];
unsigned int Input::mouseButtonPrevStates[7];
int Input::mouseScrollState[2];

char Input::keyBuffer[8];
char Input::keyBufferIndex = 0;
char Input::keyBufferSearchIndex = 0;

vec2f Input::mousePosition = vec2f(0.0f, 0.0f);
vec2f Input::mouseLastPosition = vec2f(0.0f, 0.0f);
bool Input::mouseHasMoved = false;
int Input::mouseScroll = 0;

Input::Input() { }

Input::~Input() { }

vec2f Input::MousePosition() {
	return mousePosition;
}
vec2f Input::MousePosDifference()
{
	return { mousePosition.x - mouseLastPosition.x, mousePosition.y - mouseLastPosition.y };
}
bool Input::MouseHasMoved() {
	return mouseHasMoved;
}
int Input::MouseDeltaPosition() {
	return mouseScroll / 120;
}

bool Input::IsMouseKeyDown(const int& button) {
	if (button < 0 || button > 7) return false;
	if (mouseButtonStates[button] == true)
		return true;
	return false;
}

bool Input::IsMouseKeyPressed(const int& button) {
	if (button < 0 || button > 7) return false;
	if (mouseButtonStates[button] == true && mouseButtonPrevStates[button] == false)
		return true;
	return false;
}
bool Input::IsLeftMouseKeyDown()
{
	if (mouseButtonStates[0] == true)
		return true;
	return false;
}
bool Input::IsLeftMouseKeyPressed()
{
	if (mouseButtonStates[0] == true && mouseButtonPrevStates[0] == false)
		return true;
	return false;
}
bool Input::IsRightMouseKeyDown()
{
	if (mouseButtonStates[1] == true)
		return true;
	return false;
}
bool Input::IsRightMouseKeyPressed()
{
	if (mouseButtonStates[1] == true && mouseButtonPrevStates[1] == false)
		return true;
	return false;
}

bool Input::IsMouseKeyUp(const int& button) {
	if (button < 0 || button > 7) return false;
	if (mouseButtonStates[button] == false)
		return true;
	return false;
}
bool Input::IsMouseKeyReleased(const int& button) {
	if (button < 0 || button > 7) return false;
	if (mouseButtonStates[button] == false && mouseButtonPrevStates[button] == true)
		return true;
	return false;
}
bool Input::IsLeftMouseKeyUp()
{
	if (mouseButtonStates[0] == false)
		return true;
	return false;
}
bool Input::IsLeftMouseKeyReleased()
{
	if (mouseButtonStates[0] == false && mouseButtonPrevStates[0] == true)
		return true;
	return false;
}
bool Input::IsRightMouseKeyUp()
{
	if (mouseButtonStates[1] == false)
		return true;
	return false;
}
bool Input::IsRightMouseKeyReleased()
{
	if (mouseButtonStates[1] == false && mouseButtonPrevStates[1] == true)
		return true;
	return false;
}

bool Input::IsKeyDown(const int& key) {
	if (key < 0 || key > 348) return false;
	if (keyStates[key] == true)
		return true;
	return false;
}

bool Input::IsKeyPressed(const int& key) {
	if (key < 0 || key > 348) return false;
	if (keyStates[key] == true && keyPrevStates[key] == false)
		return true;
	return false;
}

bool Input::getCharInputBuffer(char* c)
{
	if (keyBufferIndex - keyBufferSearchIndex > 0)
	{
		*c = keyBuffer[keyBufferSearchIndex];
		keyBufferSearchIndex++;
		return true;
	}
	return false;
}

double Input::GetMouseScroll(Scroll direction) {
	return mouseScrollState[(int)direction];
}
void Input::Update() {
	// Update keyboard states
	for (int i = 0; i < 348; i++)
		keyPrevStates[i] = keyStates[i];

	// Update mouse states
	for (int i = 0; i < 7; i++)
		mouseButtonPrevStates[i] = mouseButtonStates[i];

	mouseScrollState[0] = 0;
	mouseScrollState[1] = 0;

	keyBuffer[0] = 0;
	keyBuffer[1] = 0;
	keyBuffer[2] = 0;
	keyBuffer[3] = 0;
	keyBuffer[4] = 0;
	keyBuffer[5] = 0;
	keyBuffer[6] = 0;
	keyBuffer[7] = 0;

	keyBufferIndex = 0;
	keyBufferSearchIndex = 0;

	mouseLastPosition = mousePosition;
	mouseScroll = 0;
	mouseHasMoved = false;
}
