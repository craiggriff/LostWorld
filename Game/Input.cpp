#include "pch.h"
#include "Input.h"



using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Platform;
using namespace std;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI;
using namespace Windows::Foundation;
using namespace Windows::Devices::Input;
using namespace Windows::Gaming::Input;
//using namespace Windows::Phone::UI::Input;
using namespace Windows::System;

#define THUMBSTICK_DEADZONE 0.25f

// Trigger range is defined as [0, 1].
#define TRIGGER_DEADZONE 0.1f


PadInput::PadInput(_In_ Windows::UI::Core::CoreWindow^ window):
m_activeGamepad(nullptr),
m_gamepadStartButtonInUse(false),
m_gamepadTriggerInUse(false),
m_gamepadsChanged(true)
{
	m_window = window;
	InitWindow(window);
	bPointer = false;
}

bool PadInput::KeyState(Windows::System::VirtualKey key, bool _reset)
{
	bool key_state = keys[(int)key];
	if (_reset == true)
	{
		keys[(int)key] = false;
	}
	return key_state;

}

void PadInput::InitWindow(_In_ CoreWindow^ window)
{
	ResetKeyState();

	//window->

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PadInput::OnPointerPressed);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PadInput::OnPointerMoved);

	window->PointerReleased +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PadInput::OnPointerReleased);

	window->PointerExited +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &PadInput::OnPointerExited);


	window->KeyDown +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &PadInput::OnKeyDown);

	window->KeyUp +=
		ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &PadInput::OnKeyUp);

	// There is a separate handler for mouse only relative mouse movement events.
	//MouseDevice::GetForCurrentView()->MouseMoved +=
	//	ref new TypedEventHandler<MouseDevice^, MouseEventArgs^>(this, &MoveLookController::OnMouseMoved);

	// Hardware back button is only available on some device families such as Phone.
	if (Windows::Foundation::Metadata::ApiInformation::IsTypePresent("Windows.Phone.UI.Input.HardwareButtons"))
	{
		//HardwareButtons::BackPressed +=
		//	ref new EventHandler<BackPressedEventArgs^>(this, &MoveLookController::OnHardwareBackButtonPressed);
	}



	

	//Gamepad::
	// Detect gamepad connection and disconnection events.
	Gamepad::GamepadAdded +=
		ref new EventHandler<Gamepad^>(this, &PadInput::OnGamepadAdded);

	Gamepad::GamepadRemoved +=
		ref new EventHandler<Gamepad^>(this, &PadInput::OnGamepadRemoved);

	
}

void PadInput::Update()
{
	UpdatePollingDevices();
}

void PadInput::UpdatePollingDevices()
{
	if (m_gamepadsChanged)
	{
		m_gamepadsChanged = false;
		unsigned int index = 0;

		if (Gamepad::Gamepads->Size == 0)
		{
			m_activeGamepad = nullptr;
		}
		// Check if the cached gamepad is still connected.
		else if (!Gamepad::Gamepads->IndexOf(m_activeGamepad, &index))
		{
			// MoveLookController is intended to handle input for a single player, so it
			// defaults to the first active gamepad.
			m_activeGamepad = Gamepad::Gamepads->GetAt(0);
		}
	}

	if (m_activeGamepad == nullptr)
	{
		return;
	}

	GamepadReading reading = m_activeGamepad->GetCurrentReading();

	if ((reading.Buttons & GamepadButtons::X) == GamepadButtons::X)
	{
		b_button_X = true;
	}
	else
	{
		b_button_X = false;
	}
	if ((reading.Buttons & GamepadButtons::Y) == GamepadButtons::Y)
	{
		b_button_Y = true;
	}
	else
	{
		b_button_Y = false;
	}
	if ((reading.Buttons & GamepadButtons::A) == GamepadButtons::A)
	{
		b_button_A = true;
	}
	else
	{
		b_button_A = false;
	}
	if ((reading.Buttons & GamepadButtons::B) == GamepadButtons::B)
	{
		b_button_B = true;
	}
	else
	{
		b_button_B = false;
	}

	m_state = MoveLookControllerState::Active;
	switch (m_state)
	{
	case MoveLookControllerState::WaitForInput:
		if ((reading.Buttons & GamepadButtons::Menu) == GamepadButtons::Menu)
		{
			m_gamepadStartButtonInUse = true;
		}
		else if (m_gamepadStartButtonInUse)
		{
			// Trigger once only on button release.
			m_gamepadStartButtonInUse = false;
			m_buttonPressed = true;
		}
		break;

	case MoveLookControllerState::Active:
		if ((reading.Buttons & GamepadButtons::Menu) == GamepadButtons::Menu)
		{
			m_gamepadStartButtonInUse = true;
		}
		else if (m_gamepadStartButtonInUse)
		{
			// Trigger once only on button release.
			m_gamepadStartButtonInUse = false;
			m_pausePressed = true;
		}
		left_trigger = reading.LeftTrigger;
		right_trigger = reading.RightTrigger;
		// Use the left thumbstick on the game controller to control
		// the eye point position control. Thumbstick input is defined from [-1, 1].
		// We use a deadzone in the middle range to avoid drift.
		m_moveCommand.x = 0.0f;
		m_moveCommand.y = 0.0f;
		if (reading.LeftThumbstickX > THUMBSTICK_DEADZONE ||
			reading.LeftThumbstickX < -THUMBSTICK_DEADZONE)
		{
			float x = static_cast<float>(reading.LeftThumbstickX);
			m_moveCommand.x = x;// (x > 0) ? 1 : -1;
		}

		if (reading.LeftThumbstickY > THUMBSTICK_DEADZONE ||
			reading.LeftThumbstickY < -THUMBSTICK_DEADZONE)
		{
			float y = static_cast<float>(reading.LeftThumbstickY);
			m_moveCommand.y = y;//(y > 0) ? 1 : -1;
		}

		// Use the right thumbstick on the game controller to control
		// the look at control. Thumbstick input is defined from [-1, 1].
		// We use a deadzone in the middle range to avoid drift.
		XMFLOAT2 pointerDelta = XMFLOAT2(0.0f,0.0f);
		if (reading.RightThumbstickX > THUMBSTICK_DEADZONE ||
			reading.RightThumbstickX < -THUMBSTICK_DEADZONE)
		{
			float x = static_cast<float>(reading.RightThumbstickX);
			m_panCommand.x = x;// *x * x;2334
		}
		else
		{
			m_panCommand.x = 0.0f;
		}

		if (reading.RightThumbstickY > THUMBSTICK_DEADZONE ||
			reading.RightThumbstickY < -THUMBSTICK_DEADZONE)
		{
			float y = static_cast<float>(reading.RightThumbstickY);
			m_panCommand.y = y;// *y * y;
		}
		else
		{
			m_panCommand.y = 0.0f;
		}

		XMFLOAT2 rotationDelta;
		rotationDelta.x = pointerDelta.x *  0.08f;       // Scale for control sensitivity.
		rotationDelta.y = pointerDelta.y *  0.08f;

		// Update our orientation based on the command.
		m_pitch += rotationDelta.y;
		m_yaw += rotationDelta.x;

		// Limit pitch to straight up or straight down.
		m_pitch = __max(-XM_PI / 2.0f, m_pitch);
		m_pitch = __min(+XM_PI / 2.0f, m_pitch);

		// Check the state of the Right Trigger button.  This is used to indicate fire control.

		if (reading.RightTrigger > TRIGGER_DEADZONE)
		{
			if (!m_autoFire && !m_gamepadTriggerInUse)
			{
				m_firePressed = true;
			}

			m_gamepadTriggerInUse = true;
		}
		else
		{
			m_gamepadTriggerInUse = false;
		}
		break;
	}
}



void PadInput::OnPointerPressed(
	_In_ CoreWindow^ /* sender */,
	_In_ PointerEventArgs^ args
	)
{

	bPointer = true;

	m_pointer_pos.X = args->CurrentPoint->Position.X;
	m_pointer_pos.Y = args->CurrentPoint->Position.Y;

	m_pointer_down_pos.X = args->CurrentPoint->Position.X;
	m_pointer_down_pos.Y = args->CurrentPoint->Position.Y;
	
}

void PadInput::GetPointerVector(float* x, float* z)
{
	static DirectX::XMFLOAT2 vec;
	Windows::Foundation::Point out_point;
	btVector3 bvec;

	if (m_pointer_pos.X == m_pointer_down_pos.X &&
		m_pointer_pos.Y == m_pointer_down_pos.Y)
	{
		vec = XMFLOAT2(0.0f, 0.0f);
	}
	else
	{
		vec.x = m_pointer_pos.X - m_pointer_down_pos.X;
		vec.y = m_pointer_pos.Y - m_pointer_down_pos.Y;
		bvec = btVector3(m_pointer_pos.X - m_pointer_down_pos.X, 0.0f, m_pointer_pos.Y - m_pointer_down_pos.Y);
		bvec.normalize();

		/*
		float distance = sqrt((vec.x) * (vec.x) +
			(vec.y) * (vec.y));

		XMVECTOR xvec = XMLoadFloat2(&vec);
		xvec = XMVector2Normalize(xvec);
		XMStoreFloat2(&vec, xvec);

		if (distance > 50.0f)
		{
			m_pointer_down_pos.X = m_pointer_pos.X - (vec.x*50.0f);
			m_pointer_down_pos.Y = m_pointer_pos.Y - (vec.y*50.0f);
		}
		*/
	}
	*x = bvec.getX();
	*z = bvec.getZ();

}

void PadInput::OnPointerReleased(
	_In_ CoreWindow^ /* sender */,
	_In_ PointerEventArgs^ args
	)
{
	bPointer = false;
}


void PadInput::OnPointerMoved(
	_In_ CoreWindow^ /* sender */,
	_In_ PointerEventArgs^ args
	)
{
	
	m_pointer_pos.X = args->CurrentPoint->Position.X;
	m_pointer_pos.Y = args->CurrentPoint->Position.Y;


}

void PadInput::OnMouseMoved(
	_In_ MouseDevice^ /* mouseDevice */,
	_In_ MouseEventArgs^ args
	)
{

}


void PadInput::OnPointerExited(
	_In_ CoreWindow^ /* sender */,
	_In_ PointerEventArgs^ args
	)
{


}

void PadInput::OnGamepadAdded(_In_ Object^ sender, _In_ Gamepad^ gamepad)
{
	// OnGamepadAdded and OnGamepadRemoved can be called from a worker thread. For simplicity,
	// defer updating the active gamepad until Update().
	m_gamepadsChanged = true;
}

//----------------------------------------------------------------------

void PadInput::OnGamepadRemoved(_In_ Object^ sender, _In_ Gamepad^ gamepad)
{
	// OnGamepadAdded and OnGamepadRemoved can be called from a worker thread. For simplicity,
	// defer updating the active gamepad until Update().
	m_gamepadsChanged = true;
}



void PadInput::ResetKeyState()
{
	int i;

	for (i = 0; i < 255; i++)
	{
		keys[i] = false;
	}
}


void PadInput::OnKeyDown(
	_In_ CoreWindow^ /* sender */,
	_In_ KeyEventArgs^ args
	)
{
	Windows::System::VirtualKey Key;
	Key = args->VirtualKey;

	keys[(int)args->VirtualKey] = true;


}

//----------------------------------------------------------------------

void PadInput::OnKeyUp(
	_In_ CoreWindow^ /* sender */,
	_In_ KeyEventArgs^ args
	)
{
	Windows::System::VirtualKey Key;
	Key = args->VirtualKey;

	keys[(int)args->VirtualKey] = false;

}