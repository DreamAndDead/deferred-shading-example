#include <cstdlib>
#include "d3dUtility.h"

#define STENCIL_CULLING

IDirect3DDevice9* Device = 0;

const int Width = 640;
const int Height = 480;
const float ScreenSize[2] = { (float)Width, (float)Height };
const float Fov = D3DX_PI * 0.5f; // 90 deg, tan(fov/2) = 1
const float ViewAspect = (float)Width / (float)Height;
const float TanHalfFov = tanf(Fov / 2);

const int ballMesh = 50;

ID3DXMesh* ball = NULL;

IDirect3DVertexBuffer9* vb = 0;

// define vertex list
struct Vertex {
	float x, y, z;
};


#define LIGHT_NUM 10

/*
 * in x-y plane
 *
 * -LIGHT_MOVEMENT_LIMIT <= x <= LIGHT_MOVEMENT_LIMIT 
 * -LIGHT_MOVEMENT_LIMIT <= y <= LIGHT_MOVEMENT_LIMIT 
*/
#define LIGHT_MOVEMENT_LIMIT 15


D3DLIGHT9 lights[LIGHT_NUM];

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

IDirect3DTexture9* stashTex = 0;
IDirect3DSurface9* stashSurface = 0;

D3DXMATRIX world;
D3DXMATRIX view;
D3DXMATRIX proj;

bool Setup()
{
	D3DXCreateSphere(Device, 0.5f, ballMesh, ballMesh, &ball, 0);

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

	hr = D3DXCreateTexture(
		Device,
		Width,
		Height,
		D3DX_DEFAULT,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&stashTex
	);

	stashTex->GetSurfaceLevel(0, &stashSurface);

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


	// init lights here
	for (int i = 0; i < LIGHT_NUM; i++) {
		lights[i] = d3d::InitLight(D3DLIGHT_POINT);

		// random position
		lights[i].Position.x = rand() % (LIGHT_MOVEMENT_LIMIT * 2) - LIGHT_MOVEMENT_LIMIT;
		lights[i].Position.y = rand() % (LIGHT_MOVEMENT_LIMIT * 2) - LIGHT_MOVEMENT_LIMIT;
	}

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
	ball->DrawSubset(0);
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

	for (int x = -LIGHT_MOVEMENT_LIMIT; x <= LIGHT_MOVEMENT_LIMIT; x++) {
		for (int y = -LIGHT_MOVEMENT_LIMIT; y <= LIGHT_MOVEMENT_LIMIT; y++) {
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

	Device->EndScene();

	// deferred light phase
	resumeRender();

	Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0);
	Device->ColorFill(stashSurface, NULL, D3DXCOLOR(0.f, 0.f, 0.f, 0.f));

	Device->BeginScene();

	// a big loop of lights array
	for (int i = 0; i < LIGHT_NUM; i++) {
		D3DLIGHT9 light = lights[i];

		ID3DXEffect* effect = NULL;

		switch (light.Type) {
		case D3DLIGHT_DIRECTIONAL:
			effect = directional_light_effect;
			break;
		case D3DLIGHT_POINT:
			effect = point_light_effect;
			break;
		case D3DLIGHT_SPOT:
			effect = spot_light_effect;
			break;
		}

#ifdef STENCIL_CULLING
		worldHandle = effect->GetParameterByName(0, "world");
		projHandle = effect->GetParameterByName(0, "proj");

		D3DXMATRIX scale;
		D3DXMatrixScaling(&scale, light.Range * 2, light.Range * 2, light.Range * 2);
		D3DXMatrixTranslation(&world, light.Position.x, light.Position.y, light.Position.z);

		world = scale * world;

		effect->SetMatrix(worldHandle, &world);
		effect->SetMatrix(projHandle, &proj);
#endif

		viewHandle = effect->GetParameterByName(0, "view");
		effect->SetMatrix(viewHandle, &view);

		D3DXHANDLE screenSizeHandle = effect->GetParameterByName(0, "screenSize");
		D3DXHANDLE viewAspectHandle = effect->GetParameterByName(0, "viewAspect");
		D3DXHANDLE tanHalfFovHandle = effect->GetParameterByName(0, "tanHalfFov");

		effect->SetFloatArray(screenSizeHandle, ScreenSize, 2);
		effect->SetFloat(viewAspectHandle, ViewAspect);
		effect->SetFloat(tanHalfFovHandle, TanHalfFov);


		// light parameters here
		D3DXHANDLE lightAmbientHandle = effect->GetParameterByName(0, "light_ambient");
		D3DXHANDLE lightDiffuseHandle = effect->GetParameterByName(0, "light_diffuse");
		D3DXHANDLE lightSpecularHandle = effect->GetParameterByName(0, "light_specular");

		D3DXHANDLE lightPositionHandle = effect->GetParameterByName(0, "light_position");
		D3DXHANDLE lightDirectionHandle = effect->GetParameterByName(0, "light_direction");

		D3DXHANDLE lightRangeHandle = effect->GetParameterByName(0, "light_range");
		D3DXHANDLE lightFalloffHandle = effect->GetParameterByName(0, "light_falloff");

		D3DXHANDLE lightAttenuation0Handle = effect->GetParameterByName(0, "light_attenuation0");
		D3DXHANDLE lightAttenuation1Handle = effect->GetParameterByName(0, "light_attenuation1");
		D3DXHANDLE lightAttenuation2Handle = effect->GetParameterByName(0, "light_attenuation2");

		D3DXHANDLE lightThetaHandle = effect->GetParameterByName(0, "light_theta");
		D3DXHANDLE lightPhiHandle = effect->GetParameterByName(0, "light_phi");

		float floatArray[3];

		floatArray[0] = light.Ambient.r;
		floatArray[1] = light.Ambient.g;
		floatArray[2] = light.Ambient.b;
		effect->SetFloatArray(lightAmbientHandle, floatArray, 3);

		floatArray[0] = light.Diffuse.r;
		floatArray[1] = light.Diffuse.g;
		floatArray[2] = light.Diffuse.b;
		effect->SetFloatArray(lightDiffuseHandle, floatArray, 3);

		floatArray[0] = light.Specular.r;
		floatArray[1] = light.Specular.g;
		floatArray[2] = light.Specular.b;
		effect->SetFloatArray(lightSpecularHandle, floatArray, 3);


		floatArray[0] = light.Position.x;
		floatArray[1] = light.Position.y;
		floatArray[2] = light.Position.z;
		effect->SetFloatArray(lightPositionHandle, floatArray, 3);

		floatArray[0] = light.Direction.x;
		floatArray[1] = light.Direction.y;
		floatArray[2] = light.Direction.z;
		effect->SetFloatArray(lightDirectionHandle, floatArray, 3);

		effect->SetFloat(lightRangeHandle, light.Range);
		effect->SetFloat(lightFalloffHandle, light.Falloff);

		effect->SetFloat(lightAttenuation0Handle, light.Attenuation0);
		effect->SetFloat(lightAttenuation1Handle, light.Attenuation1);
		effect->SetFloat(lightAttenuation2Handle, light.Attenuation2);

		effect->SetFloat(lightThetaHandle, light.Theta);
		effect->SetFloat(lightPhiHandle, light.Phi);


		D3DXHANDLE normalHandle = effect->GetParameterByName(0, "normalTex");
		D3DXHANDLE depthHandle = effect->GetParameterByName(0, "depthTex");
		D3DXHANDLE diffuseHandle = effect->GetParameterByName(0, "diffuseTex");
		D3DXHANDLE specularHandle = effect->GetParameterByName(0, "specularTex");
		D3DXHANDLE stashHandle = effect->GetParameterByName(0, "stashTex");

		effect->SetTexture(normalHandle, normalTex);
		effect->SetTexture(depthHandle, depthTex);
		effect->SetTexture(diffuseHandle, diffuseTex);
		effect->SetTexture(specularHandle, specularTex);
		effect->SetTexture(stashHandle, stashTex);

#ifdef STENCIL_CULLING
		hTech = effect->GetTechniqueByName("StencilCulling");
#else
		hTech = effect->GetTechniqueByName("Plain");
#endif

		effect->SetTechnique(hTech);

		Device->Clear(0, 0, D3DCLEAR_STENCIL, 0x00000000, 1.0f, 0);

		numPasses = 0;
		effect->Begin(&numPasses, 0);

		for (int i = 0; i < numPasses; i++)
		{
			effect->BeginPass(i);

#ifdef STENCIL_CULLING
			drawBall();
#else
			drawScreenQuad();
#endif

			effect->EndPass();
		}

		effect->End();

		// copy origin output to stash surface
	    // https://docs.microsoft.com/en-us/previous-versions/windows/desktop/bb324163(v=vs.85)
		Device->StretchRect(originRenderTarget, NULL, stashSurface, NULL, D3DTEXF_NONE);
	}


	Device->EndScene();
	Device->Present(0, 0, 0, 0);

}

bool Display(float timeDelta)
{
	if (Device)
	{
		static float angle = 0;
		static float height = 8.f;
		static float radius = 5.0f;

		if (::GetAsyncKeyState(VK_LEFT) & 0x8000f)
			angle += timeDelta;

		if (::GetAsyncKeyState(VK_RIGHT) & 0x8000f)
			angle -= timeDelta;

		if (::GetAsyncKeyState(VK_UP) & 0x8000f)
		{
			if (::GetAsyncKeyState(VK_CONTROL) & 0x8000f) {
				height -= 2.0f * timeDelta;
			}
			else {
				radius -= 2.0f * timeDelta;
			}
		}

		if (::GetAsyncKeyState(VK_DOWN) & 0x8000f)
		{
			if (::GetAsyncKeyState(VK_CONTROL) & 0x8000f) {
				height += 2.0f * timeDelta;
			}
			else {
				radius += 2.0f * timeDelta;
			}
		}

		D3DXMatrixTranslation(&world, 0, 0, 0);

		D3DXVECTOR3 position(cosf(angle) * radius, sinf(angle) * radius, height);
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

		// update lights positions
		for (int i = 0; i < LIGHT_NUM; i++) {
			D3DVECTOR p = lights[i].Position;

			// move in x-y plane
			p.x = p.x + timeDelta * 3;
			p.y = p.y + timeDelta * 3;

			if (p.x > LIGHT_MOVEMENT_LIMIT) {
				p.x = -LIGHT_MOVEMENT_LIMIT;
			}
			else if (p.x < -LIGHT_MOVEMENT_LIMIT) {
				p.x = LIGHT_MOVEMENT_LIMIT;
			}

			if (p.y > LIGHT_MOVEMENT_LIMIT) {
				p.y = -LIGHT_MOVEMENT_LIMIT;
			}
			else if (p.y < -LIGHT_MOVEMENT_LIMIT) {
				p.y = LIGHT_MOVEMENT_LIMIT;
			}

			lights[i].Position = p;
		}

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