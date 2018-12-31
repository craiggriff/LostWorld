#pragma once

#include <wrl.h>
#include <wrl/client.h>
#include <dxgi1_4.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <wincodec.h>
#include <DirectXColors.h>
#include <DirectXMath.h>
#include <memory>
#include <agile.h>
//#include <concrt.h>
#include <collection.h>

#define XAUDIO2_HELPER_FUNCTIONS 1
#include <xaudio2.h>
#include <xaudio2fx.h>

#include <memory>
#include <agile.h>
//#include <concrt.h>
#include <collection.h>
#include "App.xaml.h"

#include <mmreg.h>
#include <mfidl.h>
#include <mfapi.h>
#include <mfreadwrite.h>
#include <mfmediaengine.h>

#include <stdio.h>
#include <vector>
#include <memory>
#include <random>

//#include <ppltasks.h>
#include <agile.h>
// #include <concrt.h>
#include <BasicMath.h>

#include <concrt.h>
#include <pplinterface.h>

#include <DirectXCollision.h>

#include "../Bullet/src/btBulletDynamicsCommon.h"

#include "Textures.h"

#include "Physics.h"


#include "Common/DirectXHelper.h"






#define M_PI 3.14159265358979323846

#define FIN_DIST 0.0f

#define BIT(x) (1<<(x))
enum collisiontypes {
	COL_NOTHING = 0, //<Collide with nothing
	COL_RAY = 1, //<Collide with ray
	COL_TERRAIN = 4, //<Collide with ships
	COL_WHEEL = 8, //<Collide with walls
	COL_CARBODY = 16, //<Collide with powerups

	COL_WHEELLIFT = 32, //<Collide with powerups
	COL_OBJECTS = 64,
	COL_WALLS = 128,
	COL_CAAARBODY = 256,
	COL_CHAR = 512
}; // +1 collide with ray cast
