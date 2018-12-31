#pragma once

using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI;

enum class MoveLookControllerState
{
	None,
	WaitForInput,
	Active,
};


ref class PadInput 
{
internal:
	PadInput(	_In_ Windows::UI::Core::CoreWindow^ window);

	void InitWindow(_In_ CoreWindow^ window);

	void OnKeyDown(
		_In_ Windows::UI::Core::CoreWindow^ sender,
		_In_ Windows::UI::Core::KeyEventArgs^ args
		);
	void OnKeyUp(
		_In_ Windows::UI::Core::CoreWindow^ sender,
		_In_ Windows::UI::Core::KeyEventArgs^ args
		);
	void OnPointerPressed(
		_In_ Windows::UI::Core::CoreWindow^ sender,
		_In_ Windows::UI::Core::PointerEventArgs^ args
		);
	void OnPointerMoved(
		_In_ Windows::UI::Core::CoreWindow^ sender,
		_In_ Windows::UI::Core::PointerEventArgs^ args
		);
	void OnPointerReleased(
		_In_ Windows::UI::Core::CoreWindow^ sender,
		_In_ Windows::UI::Core::PointerEventArgs^ args
		);
	void OnPointerExited(
		_In_ Windows::UI::Core::CoreWindow^ sender,
		_In_ Windows::UI::Core::PointerEventArgs^ args
		);

	void OnMouseMoved(
		_In_ Windows::Devices::Input::MouseDevice^ mouseDevice,
		_In_ Windows::Devices::Input::MouseEventArgs^ args
		);

	void OnGamepadAdded(
		_In_ Platform::Object^ sender,
		_In_ Windows::Gaming::Input::Gamepad^ gamepad
		);
	void OnGamepadRemoved(
		_In_ Platform::Object^ sender,
		_In_ Windows::Gaming::Input::Gamepad^ gamepad
		);

	void UpdatePollingDevices();
public:
	void ResetKeyState();

	void Update();

	bool KeyState(Windows::System::VirtualKey key, bool _reset = false);

	Windows::Foundation::Point GetPointerPos() { return m_pointer_pos; };
	void GetPointerVector(float* x,float* z);
	bool GetPointerPressed() { return bPointer; }
	float MoveCommandX() { return m_moveCommand.x; }
	float MoveCommandY() { return m_moveCommand.y; }
	float PanCommandX() { return m_panCommand.x; }
	float PanCommandY() { return m_panCommand.y; }
	
	float LeftTrigger() { return left_trigger; }
	float RightTrigger() { return right_trigger; }

	bool ButtonX() { return b_button_X; }
	bool ButtonY() { return b_button_Y; }
	bool ButtonA() { return b_button_A; }
	bool ButtonB() { return b_button_B; }

	bool ButtonXwas() { if (b_button_X == true && b_button_X_was == false) {b_button_X_was = true;	return b_button_X_was;}	else{ if(b_button_X==false) b_button_X_was = false;return false;}}
	bool ButtonYwas() { if (b_button_Y == true && b_button_Y_was == false) { b_button_Y_was = true;	return b_button_Y_was; } else{ if(b_button_Y==false) b_button_Y_was = false;return false;}}
	bool ButtonAwas() { if (b_button_A == true && b_button_A_was == false) { b_button_A_was = true;	return b_button_A_was; } else{ if(b_button_A==false) b_button_A_was = false;return false;}}
	bool ButtonBwas() { if (b_button_B == true && b_button_B_was == false) { b_button_B_was = true;	return b_button_B_was; } else{ if(b_button_B==false) b_button_B_was = false;return false;}}

private:
	bool bPointer;

	float left_trigger, right_trigger;

	bool keys[255];
	Windows::UI::Core::CoreDispatcher^  m_dispatcher;
	Windows::UI::Core::CoreWindow^ m_window;
	Windows::Foundation::Point m_pointer_pos;
	Windows::Foundation::Point m_pointer_down_pos;

	MoveLookControllerState     m_state;

	bool                        m_buttonInUse;
	uint32                      m_buttonPointerID;
	DirectX::XMFLOAT2           m_buttonUpperLeft;
	DirectX::XMFLOAT2           m_buttonLowerRight;
	bool                        m_buttonPressed;
	bool                        m_pausePressed;

	DirectX::XMFLOAT3           m_moveCommand;          // The net command from the move control.
	DirectX::XMFLOAT3           m_panCommand;          // The net command from the move control.
	float                       m_pitch;
	float                       m_yaw;                  // Orientation euler angles in radians.

	// Properties of the Fire control.
	bool                        m_autoFire;
	bool                        m_firePressed;
	bool b_button_Y;
	bool b_button_X;
	bool b_button_A;
	bool b_button_B;

	bool b_button_Y_was;
	bool b_button_X_was;
	bool b_button_A_was;
	bool b_button_B_was;
	// Game controller related members.
	Windows::Gaming::Input::Gamepad^    m_activeGamepad;
	std::atomic<bool>                   m_gamepadsChanged;
	bool                                m_gamepadStartButtonInUse;
	bool                                m_gamepadTriggerInUse;

};

