//
// DirectXPage.xaml.cpp
// Implementation of the DirectXPage class.
//

#include "pch.h"
#include "DirectXPage.xaml.h"

using namespace Game;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace concurrency;

//using namespace Windows::Phone::UI::Input;


DirectXPage::DirectXPage() :
	m_windowVisible(true),
	m_coreInput(nullptr),
	bLoaded(false)
{
	InitializeComponent();

	// Register event handlers for page lifecycle.
	CoreWindow^ window = Window::Current->CoreWindow;

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &DirectXPage::OnVisibilityChanged);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &DirectXPage::OnDisplayContentsInvalidated);

	swapChainPanel->CompositionScaleChanged +=
		ref new TypedEventHandler<SwapChainPanel^, Object^>(this, &DirectXPage::OnCompositionScaleChanged);

	swapChainPanel->SizeChanged +=
		ref new SizeChangedEventHandler(this, &DirectXPage::OnSwapChainPanelSizeChanged);

	i_total_visible = 0;
	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
	m_deviceResources->SetSwapChainPanel(swapChainPanel);

	m_main = std::unique_ptr<GameMain>(new GameMain(m_deviceResources, this));




	SetPaintList();

	//SetDiceNum(1);

	rotate_start_point = 0.0f;
	rotate_new_point = 0.0f;
	bLoaded = true;
	m_main->StartRenderLoop();
}


Platform::String^ DirectXPage::StringFromAscIIChars(char* chars)
{
	size_t newsize = strlen(chars) + 1;
	wchar_t * wcstring = new wchar_t[newsize];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring, newsize, chars, _TRUNCATE);
	String^ str = ref new Platform::String(wcstring);
	delete[] wcstring;
	return str;
}

void DirectXPage::AddPaint(char* name, int index,char* b_color)
{
	static char last_col[20]="";


	new_paint = ref new PaintList();
	if (strlen(b_color) > 0)
	{
		strcpy_s(last_col, b_color);
		new_paint->BackColor = StringFromAscIIChars(b_color);
	}
	else
	{
		new_paint->BackColor = StringFromAscIIChars(last_col);
	}
	new_paint->Image = "";
	new_paint->Name = StringFromAscIIChars(name);

	new_paint->Index = index;
	Paints->Append(new_paint);
}

void DirectXPage::SetPaintList()
{
	int i;


	Paints = ref new Platform::Collections::Vector<PaintList^>();

	AddPaint("Terrain", 1, "#473625");
	AddPaint("Path", 2, "#474675");
	AddPaint("Clear", -1, "#cccccc");
	AddPaint("Tree1", 100, "#124467");
	AddPaint("Checkpt", 101);
	AddPaint("Flag", 102);
	AddPaint("Lamp", 103);
	AddPaint("Fire", 104);
	AddPaint("Mushroom", 105);
	AddPaint("Heart", 106);
	AddPaint("Gem white", 107);
	AddPaint("Gem blue", 108);
	AddPaint("Gem red", 109);
	AddPaint("Tree2", 110);
	AddPaint("Star", 111);
	AddPaint("Hedge", 112);
	AddPaint("HedgeLong", 113);
	AddPaint("Hedgearch", 114);
	AddPaint("Hedgeround", 115);
	AddPaint("AnimTree1", 116);
	AddPaint("Statue1", 117);
	AddPaint("House", 118);
	AddPaint("Lighthouse", 119);

	AddPaint("Box", 300, "#764467");
	AddPaint("Barrel", 301);
	AddPaint("Beachball", 302);
	AddPaint("Column", 303);
	AddPaint("Block", 304);
	AddPaint("Wood board", 305);
	AddPaint("Big Disk", 306);
	AddPaint("Pumpkin", 307);
	AddPaint("Post", 308);
	AddPaint("Pin", 309);

	//items_list->ItemsSource = Paints;
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this]()
	{
		items_list->ItemsSource = Paints;
	}));
}

void DirectXPage::SetDesigner(bool _bDesign)
{
	//items_list->ItemsSource = Paints;
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, _bDesign]()
	{
		if (_bDesign == true)
		{
			right_editor->Visibility = Windows::UI::Xaml::Visibility::Visible;
		}
		else
		{
			right_editor->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		}
	}));

}

void DirectXPage::SetInfoVal(int val_id, float val)
{

	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, val_id, val]()
	{
		static const int bufferLength = 20;
		static char16 wsbuffer[bufferLength];
		int length;

		switch (val_id)
		{
		case 0:
			length = swprintf_s(wsbuffer, bufferLength, L"CX:%3.2f", val);
			cx->Text = ref new Platform::String(wsbuffer, length); break;
		case 1:
			length = swprintf_s(wsbuffer, bufferLength, L"CZ:%3.2f", val);
			cz->Text = ref new Platform::String(wsbuffer, length); break;
		case 2:
			length = swprintf_s(wsbuffer, bufferLength, L"CA:%3.2f", val);
			ca->Text = ref new Platform::String(wsbuffer, length); break;
		case 3:
			length = swprintf_s(wsbuffer, bufferLength, L"BX:%3.2f", val);
			bx->Text = ref new Platform::String(wsbuffer, length); break;
		case 4:
			length = swprintf_s(wsbuffer, bufferLength, L"BZ:%3.2f", val);
			bz->Text = ref new Platform::String(wsbuffer, length); break;
		case 5:
			length = swprintf_s(wsbuffer, bufferLength, L"BA:%3.2f", val);
			ba->Text = ref new Platform::String(wsbuffer, length); break;
		}


	})
	);

}


void DirectXPage::SetDiceNum(int dice)
{

	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, dice]()
	{
		static const int bufferLength = 20;
		static char16 wsbuffer[bufferLength];

		int length = swprintf_s(wsbuffer, bufferLength, L"%d", dice);
		dice_num->Text = ref new Platform::String(wsbuffer, length);

	})
	);

}

void DirectXPage::SetFPS(int val)
{
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, val]()
	{
		static const int bufferLength = 20;
		static char16 wsbuffer[bufferLength];

		int length = swprintf_s(wsbuffer, bufferLength, L"%d", val);
		fps->Text = ref new Platform::String(wsbuffer, length);

	})
	);
}
void DirectXPage::SetTimer(double t_val)
{
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, t_val]()
	{
		static const int bufferLength = 20;
		static char16 wsbuffer[bufferLength];

		int length = swprintf_s(wsbuffer, bufferLength, L"%f", t_val);
		timer->Text = ref new Platform::String(wsbuffer, length);

	})
	);
}



void DirectXPage::SetLevel(int _level)
{
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, _level]()
	{
		static const int bufferLength = 20;
		static char16 wsbuffer[bufferLength];

		int length = swprintf_s(wsbuffer, bufferLength, L"%d", _level);
		level->Text = ref new Platform::String(wsbuffer, length);

	})
	);
}

void DirectXPage::SetDiceTextureNum(int dice)
{
	static int score;

	if (dice != score)
	{
		score = dice;

		Dispatcher->RunAsync(
			CoreDispatcherPriority::Normal,
			ref new DispatchedHandler([this, dice]()
		{
			static const int bufferLength = 20;
			static char16 wsbuffer[bufferLength];

			int length = swprintf_s(wsbuffer, bufferLength, L"%d", dice);
			dice_tex->Text = ref new Platform::String(wsbuffer, length);
			//total_box->Visibility = Windows::UI::Xaml::Visibility::Visible;

		})
		);
	}

}


void DirectXPage::SetFeltTextureNum(int dice)
{
	static int score;

	if (dice != score)
	{
		score = dice;

		Dispatcher->RunAsync(
			CoreDispatcherPriority::Normal,
			ref new DispatchedHandler([this, dice]()
		{
			static const int bufferLength = 20;
			static char16 wsbuffer[bufferLength];

			int length = swprintf_s(wsbuffer, bufferLength, L"%d", dice);
			felt_tex->Text = ref new Platform::String(wsbuffer, length);
			//total_box->Visibility = Windows::UI::Xaml::Visibility::Visible;

		})
		);
	}

}


DirectXPage::~DirectXPage()
{
	// Stop rendering and processing events on destruction.
	m_main->StopRenderLoop();
	m_coreInput->Dispatcher->StopProcessEvents();
}

// Saves the current state of the app for suspend and terminate events.
void DirectXPage::SaveInternalState(IPropertySet^ state)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->Trim();

	// Stop rendering when the app is suspended.
	m_main->StopRenderLoop();

	// Put code to save app state here.
}

void DirectXPage::ShowTotal()
{
	return;
	//if (total_box->Visibility == Windows::UI::Xaml::Visibility::Visible)
	//	return;
	if (i_total_visible == 0 || i_total_visible == 2)
	{
		i_total_visible = 1;
		Dispatcher->RunAsync(
			CoreDispatcherPriority::Normal,
			ref new DispatchedHandler([this]()
		{
			//pause_area->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			total_box->Visibility = Windows::UI::Xaml::Visibility::Visible;
		}));
	}
}

void DirectXPage::HideTotal()
{

	//if (total_box->Visibility == Windows::UI::Xaml::Visibility::Collapsed)
	//	return;
	if (i_total_visible == 0 || i_total_visible == 1)
	{
		i_total_visible = 2;
		Dispatcher->RunAsync(
			CoreDispatcherPriority::Normal,
			ref new DispatchedHandler([this]()
		{
			total_box->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			//quit_quest->Visibility = Windows::UI::Xaml::Visibility::Visible;
		}));
	}
}

void DirectXPage::SetDiceTotal(int dice)
{
	return;
	static int score;

	if (dice != score)
	{
		score = dice;

		Dispatcher->RunAsync(
			CoreDispatcherPriority::Normal,
			ref new DispatchedHandler([this, dice]()
		{
			static const int bufferLength = 20;
			static char16 wsbuffer[bufferLength];

			int length = swprintf_s(wsbuffer, bufferLength, L"%d", dice);
			dice_total->Text = ref new Platform::String(wsbuffer, length);
			total_box->Visibility = Windows::UI::Xaml::Visibility::Visible;

		})
		);
	}

}




// Loads the current state of the app for resume events.
void DirectXPage::LoadInternalState(IPropertySet^ state)
{
	// Put code to load app state here.

	// Start rendering when the app is resumed.
	m_main->StartRenderLoop();
}

// Window event handlers.

void DirectXPage::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
	if (m_windowVisible)
	{
		m_main->StartRenderLoop();
	}
	else
	{
		m_main->StopRenderLoop();
	}
}

// DisplayInformation event handlers.

void DirectXPage::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	// Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
	// if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
	// you should always retrieve it using the GetDpi method.
	// See DeviceResources.cpp for more details.
	m_deviceResources->SetDpi(sender->LogicalDpi);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->ValidateDevice();
}

// Called when the app bar button is clicked.
void DirectXPage::AppBarButton_Click(Object^ sender, RoutedEventArgs^ e)
{
	// Use the app bar if it is appropriate for your app. Design the app bar, 
	// then fill in event handlers (like this one).
}


void DirectXPage::OnCompositionScaleChanged(SwapChainPanel^ sender, Object^ args)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());
	m_deviceResources->SetCompositionScale(sender->CompositionScaleX, sender->CompositionScaleY);
	m_main->CreateWindowSizeDependentResources();
}

void DirectXPage::OnSwapChainPanelSizeChanged(Object^ sender, SizeChangedEventArgs^ e)
{
	critical_section::scoped_lock lock(m_main->GetCriticalSection());

	m_deviceResources->SetLogicalSize(e->NewSize);
	
	m_main->CreateWindowSizeDependentResources();

}






void Game::DirectXPage::SetPause()
{

}


void Game::DirectXPage::button_menu_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	SetPause();
}


void Game::DirectXPage::Button_Click_dl(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void Game::DirectXPage::Button_Click_dr(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}

void Game::DirectXPage::Button_Click_dl(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this]()
	{
		if (m_main->m_sceneRenderer->m_Scene->current_level > 1)
		{
			m_main->m_sceneRenderer->m_Scene->current_level_to--;
			SetDiceNum(m_main->m_sceneRenderer->m_Scene->current_level_to);
		}
	}));
}

void Game::DirectXPage::Button_Click_dr(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this]()
	{
		if (m_main->m_sceneRenderer->m_Scene->current_level < 10)
		{
			m_main->m_sceneRenderer->m_Scene->current_level_to++;
			SetDiceNum(m_main->m_sceneRenderer->m_Scene->current_level_to);
		}
	}));
}


void Game::DirectXPage::Button_Click_cl(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
}


void Game::DirectXPage::Button_Click_cr(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
}


void Game::DirectXPage::Button_Click_fl(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
}


void Game::DirectXPage::Button_Click_fr(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
}


void Game::DirectXPage::items_list_SelectionChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^ e)
{
	Windows::UI::Xaml::Controls::ListView^ view = (Windows::UI::Xaml::Controls::ListView^)sender;
	PaintList^ paint = (PaintList^)view->SelectedItem;

	m_main->m_sceneRenderer->m_Scene->m_LevelEdit->SetEditing(paint->Index);
}




void Game::DirectXPage::phy_check(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Stuff->SetPhysical(true);
}


void Game::DirectXPage::phy_uncheck(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Stuff->SetPhysical(false);
}


void Game::DirectXPage::rand_check(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Statics->SetRandomness(true);
}


void Game::DirectXPage::rand_uncheck(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Statics->SetRandomness(false);
}








void Game::DirectXPage::in_car_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this]()
	{
		m_main->m_sceneRenderer->m_Logic->SetPlayerState();

	}));
}

void Game::DirectXPage::CarControlsVisible(bool _bVisible)
{

	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, _bVisible]()
	{
		if (_bVisible == true)
		{

			right_buttons->Visibility = Windows::UI::Xaml::Visibility::Visible;
			left_buttons->Visibility = Windows::UI::Xaml::Visibility::Visible;
		}
		else
		{
			right_buttons->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			left_buttons->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		}

	}));

}

void Game::DirectXPage::PlayerControlsVisible(bool _bVisible)
{

	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this, _bVisible]()
	{
		if (_bVisible == true)
		{

			char_right_buttons->Visibility = Windows::UI::Xaml::Visibility::Visible;
			char_left_buttons->Visibility = Windows::UI::Xaml::Visibility::Visible;
		}
		else
		{
			char_right_buttons->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
			char_left_buttons->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		}

	}));

}


void Game::DirectXPage::button_right_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetRight(true);
}


void Game::DirectXPage::button_right_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetRight(false);
}

void Game::DirectXPage::button_left_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetLeft(true);
}


void Game::DirectXPage::button_left_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetLeft(false);
}


void Game::DirectXPage::button_grab_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{

}


void Game::DirectXPage::button_grab_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{

}


void Game::DirectXPage::button_up_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetUp(true);
}


void Game::DirectXPage::button_up_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetUp(false);
}


void Game::DirectXPage::button_down_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetDown(true);
}


void Game::DirectXPage::button_up_PointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{

}


void Game::DirectXPage::button_down_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetDown(false);
}


void Game::DirectXPage::button_jump_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Bert->DoTouchJump();
}


void Game::DirectXPage::button_jump_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{

}


void Game::DirectXPage::button_move_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Windows::UI::Input::PointerPoint^ point = e->GetCurrentPoint((Windows::UI::Xaml::UIElement^)sender);
	m_main->m_sceneRenderer->m_Scene->m_Bert->SetTouchMove(point->Position.X, point->Position.Y);
	//SetLevel((int)point->Position.X);
}


void Game::DirectXPage::button_move_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Bert->SetTouchMove(0.0f, 0.0f);
}


void Game::DirectXPage::button_move_PointerMoved(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Windows::UI::Input::PointerPoint^ point = e->GetCurrentPoint((Windows::UI::Xaml::UIElement^)sender);
	m_main->m_sceneRenderer->m_Scene->m_Bert->SetTouchMove(point->Position.X, point->Position.Y);
	//SetLevel((int)point->Position.X);
}


void Game::DirectXPage::button_rotate_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Windows::UI::Input::PointerPoint^ point = e->GetCurrentPoint((Windows::UI::Xaml::UIElement^)sender);
	rotate_start_point = point->Position.X;

}


void Game::DirectXPage::button_rotate_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	rotate_start_point = 0.0f;
	rotate_new_point = 0.0f;
	m_main->m_sceneRenderer->m_Scene->m_Bert->SetTouchRotate(0.0f);
}


void Game::DirectXPage::button_rotate_PointerMove(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Windows::UI::Input::PointerPoint^ point = e->GetCurrentPoint((Windows::UI::Xaml::UIElement^)sender);

	rotate_new_point = point->Position.X;
	m_main->m_sceneRenderer->m_Scene->m_Bert->SetTouchRotate((rotate_start_point - rotate_new_point)*0.04f);
	rotate_start_point = rotate_new_point;
}


void Game::DirectXPage::attack_grab_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Bert->SetTouchAttack(true);
}


void Game::DirectXPage::attackn_grab_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Bert->SetTouchAttack(false);
}


void Game::DirectXPage::button_carsteer_PointerPressed(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Windows::UI::Input::PointerPoint^ point = e->GetCurrentPoint((Windows::UI::Xaml::UIElement^)sender);

	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetSteerPosition(((point->Position.X) - 92.5f)*0.00925f);

}


void Game::DirectXPage::button_carsteer_PointerReleased(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Windows::UI::Input::PointerPoint^ point = e->GetCurrentPoint((Windows::UI::Xaml::UIElement^)sender);

	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetSteerPosition(0.0f);
}


void Game::DirectXPage::button_carsteer_PointerMove(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	Windows::UI::Input::PointerPoint^ point = e->GetCurrentPoint((Windows::UI::Xaml::UIElement^)sender);

	m_main->m_sceneRenderer->m_Scene->m_Buggy->SetSteerPosition(((point->Position.X) - 92.5f)*0.00925f);

}


void Game::DirectXPage::Button_Click_cl(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Statics->Reset();
}


void Game::DirectXPage::Button_Click_cr(Platform::Object^ sender, Windows::UI::Xaml::Input::PointerRoutedEventArgs^ e)
{
	m_main->m_sceneRenderer->m_Scene->m_Stuff->Reset();
}


void Game::DirectXPage::slider_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{

	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->design_spacing = slide->Value;
	}
}


void Game::DirectXPage::sliderrx_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->pointer_rot_x = (M_PI*0.25)* slide->Value;
	}
}


void Game::DirectXPage::sliderry_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->pointer_rot_y = (M_PI*0.25)* slide->Value;
	}
}


void Game::DirectXPage::sliderrz_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->pointer_rot_z = (M_PI*0.25)* slide->Value;
	}
}


void Game::DirectXPage::clear_paths_click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->ClearPaths();
	}
}


void Game::DirectXPage::path_new_click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->NewPath();
	}
}


void Game::DirectXPage::path_bake_click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->MakePath();
	}
}




void Game::DirectXPage::loop_check(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->bLoopPath = true;
	}
}


void Game::DirectXPage::loop_uncheck(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->bLoopPath = false;
	}
}


void Game::DirectXPage::dip_check(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->bDipPath = false;
	}
}


void Game::DirectXPage::dip_uncheck(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->bDipPath = false;
	}
}


void Game::DirectXPage::gen_ter_click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->bGenerateTerrain = true;
	}
}


void Game::DirectXPage::tval1_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->t1val = (float)slide->Value;
	}
}


void Game::DirectXPage::tval2_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->t2val = (float)slide->Value;
	}
}


void Game::DirectXPage::tval3_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->t3val = (float)slide->Value;
	}
}


void Game::DirectXPage::tval4_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->t4val = (float)slide->Value;
	}
}


void Game::DirectXPage::blend_num_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->m_Scene->m_LevelEdit->SetBlendNum(((int)slide->Value) - 1);
		
	}
}


void Game::DirectXPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this]()
	{
		right_cols->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
		right_chooser->Visibility = Windows::UI::Xaml::Visibility::Visible;
	}));
}


void Game::DirectXPage::Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this]()
	{
		right_cols->Visibility = Windows::UI::Xaml::Visibility::Visible;
		right_chooser->Visibility = Windows::UI::Xaml::Visibility::Collapsed;
	}));
}

void Game::DirectXPage::SetCols(float ar, float ag, float ab, float dr, float dg, float db, float dx, float dy, float dz, float sr, float sg, float sb, float sp)
{
	Dispatcher->RunAsync(
		CoreDispatcherPriority::Normal,
		ref new DispatchedHandler([this,ar,ag,ab,dr,dg,db,dx,dy,dz,sr,sg,sb,sp]()
	{


		sambr->Value = (int)(ar*256.0f);
		sambg->Value = (int)(ag*256.0f);
		sambb->Value = (int)(ab*256.0f);

		sdiffr->Value = (int)(dr*256.0f);
		sdiffg->Value = (int)(dg*256.0f);
		sdiffb->Value = (int)(db*256.0f);

		sdiffx->Value = (int)(dx*128.0f)+128;
		sdiffy->Value = (int)(dy*128.0f)+128;
		sdiffz->Value = (int)(dz*128.0f)+128;

		sspecr->Value = (int)(sr*256.0f);
		sspecg->Value = (int)(sg*256.0f);
		sspecb->Value = (int)(sb*256.0f);

		sspecpow->Value = (int)(sp);


	}));

}


void Game::DirectXPage::sdiffr_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.diffuseColor.x = (float)slide->Value/256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sdiffg_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.diffuseColor.y = (float)slide->Value / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sdiffb_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.diffuseColor.z = (float)slide->Value / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sambr_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.ambientColor.x = (float)slide->Value / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sambg_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.ambientColor.y = (float)slide->Value / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sambb_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.ambientColor.z = (float)slide->Value / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sspecr_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.specularColor.x = (float)slide->Value / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sspecg_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.specularColor.y = (float)slide->Value / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sspecb_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.specularColor.z = (float)slide->Value / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sspecpow_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.specularPower = (float)slide->Value;// / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}






void Game::DirectXPage::col_reset_click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->GetResources()->SetLighting();
	}
}


void Game::DirectXPage::col_save_click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	if (bLoaded == true)
	{
		m_main->m_sceneRenderer->GetResources()->SaveLighting();
	}
}


void Game::DirectXPage::sdiffx_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.lightDirection.x = ((float)slide->Value-128.0f) / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sdiffy_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.lightDirection.y = ((float)slide->Value - 128.0f) / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sdiffz_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->m_Lights->m_lightBufferData.lightDirection.z = ((float)slide->Value - 128.0f) / 256.0f;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}

Windows::UI::Color Game::DirectXPage::getColor(Platform::String^ colorString)
{
	Color newColor = Colors::Transparent;

	/*
	if (colorLookup->HasKey(colorString))
	{
		newColor = colorLookup->Lookup(colorString);
	}
	*/
	return newColor;
}


void Game::DirectXPage::sliderrr_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->col_r = slide->Value;
		
		Windows::UI::Color color;// = new Windows::UI::Color();
		color.R = m_main->m_sceneRenderer->GetResources()->col_r;
		color.G = m_main->m_sceneRenderer->GetResources()->col_g;
		color.B = m_main->m_sceneRenderer->GetResources()->col_b;
		color.A = 255;

		Windows::UI::Xaml::Media::Brush^ Col = ref new SolidColorBrush(color);
		col_view->Background = Col;
		//col_view->Bac
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sliderrg_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->col_g = slide->Value;
		Windows::UI::Color color;// = new Windows::UI::Color();
		color.R = m_main->m_sceneRenderer->GetResources()->col_r;
		color.G = m_main->m_sceneRenderer->GetResources()->col_g;
		color.B = m_main->m_sceneRenderer->GetResources()->col_b;
		color.A = 255;

		Windows::UI::Xaml::Media::Brush^ Col = ref new SolidColorBrush(color);
		col_view->Background = Col;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}


void Game::DirectXPage::sliderrb_ValueChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs^ e)
{
	if (bLoaded == true)
	{
		Windows::UI::Xaml::Controls::Slider^ slide = (Windows::UI::Xaml::Controls::Slider^)sender;
		m_main->m_sceneRenderer->GetResources()->col_b = slide->Value;
		Windows::UI::Color color;// = new Windows::UI::Color();
		color.R = m_main->m_sceneRenderer->GetResources()->col_r;
		color.G = m_main->m_sceneRenderer->GetResources()->col_g;
		color.B = m_main->m_sceneRenderer->GetResources()->col_b;
		color.A = 255;

		Windows::UI::Xaml::Media::Brush^ Col = ref new SolidColorBrush(color);
		col_view->Background = Col;
		//m_main->m_sceneRenderer->GetResources()->m_Lights->UpdateConstantBuffer();
	}
}
