#include "d3dUtility.h"

//
// Globals
//

IDirect3DDevice9* Device = 0; 

const int Width  = 640;
const int Height = 480;
 
ID3DXMesh* ball = NULL;

ID3DXEffect* g_buffer_effect = 0;

//
// Framework Functions
//
bool Setup()
{
	//
	// Create objects.
	//

	D3DXCreateSphere(Device, 1.0f, 20, 20, &ball, 0);

	//
	// Set lighting related render states.
	//

	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);


	// compile shader

	HRESULT hr;
	ID3DXBuffer* errorBuffer = 0;

	hr = D3DXCreateEffectFromFile(
		Device,
		"GBuffer.hlsl",
		0,
		0,
		D3DXSHADER_DEBUG,
		0,
		&g_buffer_effect,
		&errorBuffer
	);

	if (errorBuffer) {
		::MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		d3d::Release<ID3DXBuffer*>(errorBuffer);
	}

	if (FAILED(hr)) {
		::MessageBox(0, "D3DXCreateEffectFromFile() - Failed", 0, 0);
		return false;
	}

	D3DVERTEXELEMENT9 decl[] = {
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
		D3DDECL_END(),
	};

	IDirect3DVertexDeclaration9* _decl = 0;
	hr = Device->CreateVertexDeclaration(decl, &_decl);

	Device->SetVertexDeclaration(_decl);

	return true;
}

void Cleanup()
{
	for(int i = 0; i < 4; i++)
		d3d::Release<ID3DXMesh*>(ball);
}

bool Display(float timeDelta)
{
	if( Device )
	{
		// 
		// Update the scene: update camera position.
		//

		static float angle  = (3.0f * D3DX_PI) / 2.0f;
		static float height = 5.0f;
	
		if( ::GetAsyncKeyState(VK_LEFT) & 0x8000f )
			angle -= 0.5f * timeDelta;

		if( ::GetAsyncKeyState(VK_RIGHT) & 0x8000f )
			angle += 0.5f * timeDelta;

		if( ::GetAsyncKeyState(VK_UP) & 0x8000f )
			height += 5.0f * timeDelta;

		if( ::GetAsyncKeyState(VK_DOWN) & 0x8000f )
			height -= 5.0f * timeDelta;


		//
		// Draw the scene:
		//
		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00000000, 1.0f, 0);
		Device->BeginScene();

		D3DXHANDLE hTech = 0;
		hTech = g_buffer_effect->GetTechniqueByName("gbuffer");


		// TODO: set parameters here
		D3DXMATRIX world;
		D3DXMatrixTranslation(&world,  0.0f,  2.0f, 0.0f);


		D3DXVECTOR3 position( cosf(angle) * 7.0f, height, sinf(angle) * 7.0f );
		D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		D3DXMATRIX view;
		D3DXMatrixLookAtLH(&view, &position, &target, &up);

		//
		// Set the projection matrix.
		//

		D3DXMATRIX proj;
		D3DXMatrixPerspectiveFovLH(
				&proj,
				D3DX_PI * 0.25f, // 45 - degree
				(float)Width / (float)Height,
				1.0f,
				1000.0f);


		D3DXMATRIX m = world * view * proj;

		D3DXHANDLE matrixHandle = g_buffer_effect->GetParameterByName(0, "WorldViewProj");
		g_buffer_effect->SetMatrix(matrixHandle, &m);


		g_buffer_effect->SetTechnique(hTech);

		UINT numPasses = 0;
		g_buffer_effect->Begin(&numPasses, 0);

		for (int i = 0; i < numPasses; i++)
		{
			g_buffer_effect->BeginPass(i);


			// draw
			ball->DrawSubset(0);

			g_buffer_effect->EndPass();
		}

		g_buffer_effect->End();


		Device->EndScene();
		Device->Present(0, 0, 0, 0);
	}
	return true;
}


//
// WndProc
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch( msg )
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;
		
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE )
			::DestroyWindow(hwnd);
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

//
// WinMain
//
int WINAPI WinMain(HINSTANCE hinstance,
				   HINSTANCE prevInstance, 
				   PSTR cmdLine,
				   int showCmd)
{
	if(!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}
		
	if(!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop( Display );

	Cleanup();

	Device->Release();

	return 0;
}