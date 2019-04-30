#include "d3dUtility.h"

IDirect3DDevice9* Device = 0;

const int Width = 640;
const int Height = 480;
const float ScreenSize[2] = {(float)Width, (float)Height};
const float Fov = D3DX_PI * 0.5f; // 90 deg, tan(fov/2) = 1
const float ViewAspect = (float)Width / (float)Height;
const float TanHalfFov = tanf(Fov / 2);

ID3DXMesh* ball = NULL;
const int scale = 8;

IDirect3DVertexBuffer9* vb = 0;
// define vertex list
struct Vertex {
	float x, y, z, w;
};


ID3DXEffect* g_buffer_effect = 0;
ID3DXEffect* directional_light_effect = 0;
ID3DXEffect* point_light_effect = 0;
ID3DXEffect* spot_light_effect = 0;

HRESULT hr;
ID3DXBuffer* errorBuffer = 0;

IDirect3DSurface9* originRenderTarget = 0;

IDirect3DTexture9* normalTex = 0;
IDirect3DSurface9* normalSurface = 0;

IDirect3DTexture9* depthTex = 0;
IDirect3DSurface9* depthSurface = 0;

IDirect3DTexture9* diffuseTex = 0;
IDirect3DSurface9* diffuseSurface = 0;

IDirect3DTexture9* specularTex = 0;
IDirect3DSurface9* specularSurface = 0;

D3DXMATRIX world;
D3DXMATRIX view;
D3DXMATRIX proj;

bool Setup()
{
	D3DXCreateSphere(Device, 0.5f, 20, 20, &ball, 0);

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
		::MessageBox(0, "D3DXCreateEffectFromFile( GBuffer.hlsl ) - Failed", 0, 0);
		return false;
	}

	hr = D3DXCreateEffectFromFile(
		Device,
		"DirectionalLight.hlsl",
		0,
		0,
		D3DXSHADER_DEBUG,
		0,
		&directional_light_effect,
		&errorBuffer
	);

	if (errorBuffer) {
		::MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		d3d::Release<ID3DXBuffer*>(errorBuffer);
	}

	if (FAILED(hr)) {
		::MessageBox(0, "D3DXCreateEffectFromFile( DirectionalLight.hlsl ) - Failed", 0, 0);
		return false;
	}

	hr = D3DXCreateEffectFromFile(
		Device,
		"PointLight.hlsl",
		0,
		0,
		D3DXSHADER_DEBUG,
		0,
		&point_light_effect,
		&errorBuffer
	);

	if (errorBuffer) {
		::MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		d3d::Release<ID3DXBuffer*>(errorBuffer);
	}

	if (FAILED(hr)) {
		::MessageBox(0, "D3DXCreateEffectFromFile( PointLight.hlsl ) - Failed", 0, 0);
		return false;
	}

	hr = D3DXCreateEffectFromFile(
		Device,
		"SpotLight.hlsl",
		0,
		0,
		D3DXSHADER_DEBUG,
		0,
		&spot_light_effect,
		&errorBuffer
	);

	if (errorBuffer) {
		::MessageBox(0, (char*)errorBuffer->GetBufferPointer(), 0, 0);
		d3d::Release<ID3DXBuffer*>(errorBuffer);
	}

	if (FAILED(hr)) {
		::MessageBox(0, "D3DXCreateEffectFromFile( SpotLight.hlsl ) - Failed", 0, 0);
		return false;
	}

	hr = D3DXCreateTexture(
		Device,
		Width,
		Height,
		D3DX_DEFAULT,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&normalTex
	);

	normalTex->GetSurfaceLevel(0, &normalSurface);

	hr = D3DXCreateTexture(
		Device,
		Width,
		Height,
		D3DX_DEFAULT,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&depthTex
	);

	depthTex->GetSurfaceLevel(0, &depthSurface);

	hr = D3DXCreateTexture(
		Device,
		Width,
		Height,
		D3DX_DEFAULT,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&diffuseTex
	);

	diffuseTex->GetSurfaceLevel(0, &diffuseSurface);

	hr = D3DXCreateTexture(
		Device,
		Width,
		Height,
		D3DX_DEFAULT,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&specularTex
	);

	specularTex->GetSurfaceLevel(0, &specularSurface);

	Device->CreateVertexBuffer(
		6 * sizeof(Vertex),
		0,
		D3DFVF_XYZ,
		D3DPOOL_MANAGED,
		&vb,
		0
	);

	/*
	-1,1	       1,1
	v0             v1
	 +-------------+
	 |             |
	 |    screen   |
	 |             |
	 +-------------+
	v2             v3
	-1,-1          1,-1
	*/

	Vertex v0 = {
		-1, 1, 0
	};
	Vertex v1 = {
		1, 1, 0
	};
	Vertex v2 = {
		-1, -1, 0
	};
	Vertex v3 = {
		1, -1, 0
	};

	// lock buffer and draw it
	Vertex* vertices;
	vb->Lock(0, 0, (void**)&vertices, 0);

	vertices[0] = v0;
	vertices[1] = v1;
	vertices[2] = v2;
	vertices[3] = v1;
	vertices[4] = v3;
	vertices[5] = v2;

	vb->Unlock();

	return true;
}

void setMRT()
{
	// save origin render target
	Device->GetRenderTarget(0, &originRenderTarget);

	Device->SetRenderTarget(0, normalSurface);
	Device->SetRenderTarget(1, depthSurface);
	Device->SetRenderTarget(2, diffuseSurface);
	Device->SetRenderTarget(3, specularSurface);
}

void resumeRender()
{
	Device->SetRenderTarget(0, originRenderTarget);
	Device->SetRenderTarget(1, NULL);
	Device->SetRenderTarget(2, NULL);
	Device->SetRenderTarget(3, NULL);
}

void drawScreenQuad()
{
	Device->SetStreamSource(0, vb, 0, sizeof(Vertex));
	Device->SetFVF(D3DFVF_XYZ);
	Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
}

void drawBall()
{
	Device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL);
	Device->SetMaterial(&(d3d::BLUE_MTRL));
	ball->DrawSubset(0);
}

void fixedPipeline()
{
	//
	// Setup a point light.  Note that the point light
	// is positioned at the origin.
	//

	D3DXVECTOR3 pos(7.0f, 0.0f, 0.0f);
	D3DXCOLOR   c = d3d::WHITE;
	D3DLIGHT9 point = d3d::InitPointLight(&pos, &c);

	//
	// Set and Enable the light.
	//

	Device->SetLight(0, &point);
	Device->LightEnable(0, true);

	//
	// Set lighting related render states.
	//

	Device->SetRenderState(D3DRS_NORMALIZENORMALS, true);
	Device->SetRenderState(D3DRS_SPECULARENABLE, true);

	Device->SetTransform(D3DTS_VIEW, &view);
	Device->SetTransform(D3DTS_PROJECTION, &proj);

	//
	// Draw the scene:
	//
	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0);
	Device->BeginScene();

	for (int x = -scale; x <= scale; x++) {
		for (int y = -scale; y <= scale; y++) {
			D3DXMatrixTranslation(&world, x, y, 0);
			Device->SetTransform(D3DTS_WORLD, &world);

			drawBall();
		}
	}

	Device->EndScene();
	Device->Present(0, 0, 0, 0);
}

void deferredPipeline()
{
	D3DXHANDLE worldHandle = g_buffer_effect->GetParameterByName(0, "world");
	D3DXHANDLE viewHandle = g_buffer_effect->GetParameterByName(0, "view");
	D3DXHANDLE projHandle = g_buffer_effect->GetParameterByName(0, "proj");

	g_buffer_effect->SetMatrix(viewHandle, &view);
	g_buffer_effect->SetMatrix(projHandle, &proj);

	// G buffer phase
	setMRT();

	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0);
	Device->BeginScene();

	D3DXHANDLE hTech = 0;
	UINT numPasses = 0;

	hTech = g_buffer_effect->GetTechniqueByName("main");
	g_buffer_effect->SetTechnique(hTech);

	for (int x = -scale; x <= scale; x++) {
		for (int y = -scale; y <= scale; y++) {
			D3DXMatrixTranslation(&world, x, y, 0);
			g_buffer_effect->SetMatrix(worldHandle, &world);

			g_buffer_effect->Begin(&numPasses, 0);

			for (int i = 0; i < numPasses; i++)
			{
				g_buffer_effect->BeginPass(i);

				drawBall();

				g_buffer_effect->EndPass();
			}
			g_buffer_effect->End();
		}
	}


	// deferred light phase
	resumeRender();

	//ID3DXEffect* effect = directional_light_effect;
	ID3DXEffect* effect = spot_light_effect;

	viewHandle = effect->GetParameterByName(0, "view");
	D3DXHANDLE screenSizeHandle = effect->GetParameterByName(0, "screenSize");
	D3DXHANDLE viewAspectHandle = effect->GetParameterByName(0, "viewAspect");
	D3DXHANDLE tanHalfFovHandle = effect->GetParameterByName(0, "tanHalfFov");

	effect->SetMatrix(viewHandle, &view);
	effect->SetFloatArray(screenSizeHandle, ScreenSize, 2);
	effect->SetFloat(viewAspectHandle, ViewAspect);
	effect->SetFloat(tanHalfFovHandle, TanHalfFov);


	D3DXHANDLE normalHandle = effect->GetParameterByName(0, "normalTex");
	D3DXHANDLE depthHandle = effect->GetParameterByName(0, "depthTex");
	D3DXHANDLE diffuseHandle = effect->GetParameterByName(0, "diffuseTex");
	D3DXHANDLE specularHandle = effect->GetParameterByName(0, "specularTex");

	effect->SetTexture(normalHandle, normalTex);
	effect->SetTexture(depthHandle, depthTex);
	effect->SetTexture(diffuseHandle, diffuseTex);
	effect->SetTexture(specularHandle, specularTex);

	hTech = effect->GetTechniqueByName("Plain");
	effect->SetTechnique(hTech);

	numPasses = 0;
	effect->Begin(&numPasses, 0);

	for (int i = 0; i < numPasses; i++)
	{
		effect->BeginPass(i);

		drawScreenQuad();

		effect->EndPass();
	}

	effect->End();


	Device->EndScene();
	Device->Present(0, 0, 0, 0);

}

bool Display(float timeDelta)
{
	if (Device)
	{
		static float angle = 0;
		static float radius = 2.0f;

		if (::GetAsyncKeyState(VK_LEFT) & 0x8000f)
			angle += timeDelta;

		if (::GetAsyncKeyState(VK_RIGHT) & 0x8000f)
			angle -= timeDelta;

		if (::GetAsyncKeyState(VK_UP) & 0x8000f)
			radius -= 2.0f * timeDelta;

		if (::GetAsyncKeyState(VK_DOWN) & 0x8000f)
			radius += 2.0f * timeDelta;


		D3DXMatrixTranslation(&world, 0, 0, 0);

		D3DXVECTOR3 position(cosf(angle) * radius, sinf(angle) * radius, radius);
		D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 up(0.0f, 0.0f, 1.0f);
		D3DXMatrixLookAtLH(&view, &position, &target, &up);

		// https://docs.microsoft.com/en-us/windows/desktop/direct3d9/d3dxmatrixperspectivefovlh
		D3DXMatrixPerspectiveFovLH(
			&proj,
			Fov,
			ViewAspect,
			1.0f,
			1000.0f);

		//fixedPipeline();
		deferredPipeline();
	}
	return true;
}

void Cleanup()
{
	d3d::Release<ID3DXMesh*>(ball);
}


//
// WndProc
//
LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
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
	if (!d3d::InitD3D(hinstance,
		Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}