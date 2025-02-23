// CloudsShader.h
// Shader class for rendering animated clouds with scrolling effects.

#pragma once

#include "DXF.h" // Include the DirectX Framework base header.

using namespace std;
using namespace DirectX;

// CloudsShader class definition for handling cloud rendering with scrolling effects
// Rastertek (2013) DirectX 11 Terrain Tutorial(Lesson 11) (code version 1)[online tutorial]. Adapted from: https://rastertek.com/tertut11.html.
class CloudsShader : public BaseShader {
private:
    // Structure to hold scrolling data
    struct ScrollData {
        XMFLOAT4 scrollSpeed; // Scrolling speed for UV coordinates (.x) and related time data.
    };

public:
    // Constructor: Initializes the shader with vertex and pixel shader files
    CloudsShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName);

    // Destructor: Cleans up allocated resources
    ~CloudsShader();

    // Method to set parameters for the shader
    void setShaderParameters(ID3D11DeviceContext* deviceContext,
        const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection,
        ID3D11ShaderResourceView* texture, float scrollSpeed, float time);

private:
    // Initializes the shader and its resources
    void initShader(const wchar_t* vs, const wchar_t* ps);

    // Shader resources
    ID3D11Buffer* matrixBuffer;         // Buffer for transformation matrices
    ID3D11SamplerState* sampleState;    // Sampler state for texture sampling
    ID3D11BlendState* blendState = nullptr; // Optional blend state for blending effects
    ID3D11Buffer* scrollDataBuffer;     // Buffer for scrolling speed data
};