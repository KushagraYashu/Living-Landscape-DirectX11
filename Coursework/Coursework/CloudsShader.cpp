#include "CloudsShader.h"

// Constructor: Initializes the shader with vertex and pixel shader filenames.
CloudsShader::CloudsShader(ID3D11Device* device, HWND hwnd, const wchar_t* vsFileName, const wchar_t* psFileName) : BaseShader(device, hwnd)
{
    initShader(vsFileName, psFileName);
}

// Destructor: Releases resources such as sampler state, constant buffers, and layout.
CloudsShader::~CloudsShader()
{
    if (sampleState) {
        sampleState->Release();
        sampleState = 0;
    }

    if (matrixBuffer) {
        matrixBuffer->Release();
        matrixBuffer = 0;
    }

    if (layout) {
        layout->Release();
        layout = 0;
    }

    if (scrollDataBuffer) {
        scrollDataBuffer->Release();
        scrollDataBuffer = 0;
    }

    // Release base shader components.
    BaseShader::~BaseShader();
}

// Initialize the shader: loads the shader files, sets up buffers, sampler state, and blend state.
void CloudsShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
    D3D11_BUFFER_DESC matrixBufferDesc;
    D3D11_SAMPLER_DESC samplerDesc;
    D3D11_BUFFER_DESC scrollDataBufferDesc;

    // Load and compile the vertex and pixel shader files.
    loadVertexShader(vsFilename);
    loadPixelShader(psFilename);

    // Set up blend state for transparency.
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    renderer->CreateBlendState(&blendDesc, &blendState);

    // Set up the matrix buffer description for the vertex shader.
    matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
    matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    matrixBufferDesc.MiscFlags = 0;
    matrixBufferDesc.StructureByteStride = 0;
    renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

    // Create a texture sampler state description for texture filtering and addressing.
    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    renderer->CreateSamplerState(&samplerDesc, &sampleState);

    // Set up the scroll data constant buffer.
    scrollDataBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    scrollDataBufferDesc.ByteWidth = sizeof(ScrollData);
    scrollDataBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    scrollDataBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    scrollDataBufferDesc.MiscFlags = 0;
    scrollDataBufferDesc.StructureByteStride = 0;
    auto result = renderer->CreateBuffer(&scrollDataBufferDesc, NULL, &scrollDataBuffer);
}

// Set the shader parameters for the pixel and vertex shaders, including the scroll speed and time.
void CloudsShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX& world, const XMMATRIX& view, const XMMATRIX& projection, ID3D11ShaderResourceView* texture, float scrollSpeed, float time)
{
    HRESULT result;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    MatrixBufferType* dataPtr;
    XMMATRIX tworld, tview, tproj;

    // Transpose the matrices to prepare them for the shader.
    tworld = XMMatrixTranspose(world);
    tview = XMMatrixTranspose(view);
    tproj = XMMatrixTranspose(projection);

    // Send matrix data to the vertex shader.
    result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    dataPtr = (MatrixBufferType*)mappedResource.pData;
    dataPtr->world = tworld;
    dataPtr->view = tview;
    dataPtr->projection = tproj;
    deviceContext->Unmap(matrixBuffer, 0);
    deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

    // Send scroll data to the pixel shader (for cloud scrolling effect).
    ScrollData* scrollPtr;
    deviceContext->Map(scrollDataBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    scrollPtr = (ScrollData*)mappedResource.pData;
    scrollPtr->scrollSpeed = XMFLOAT4(scrollSpeed, time, 0, 0);
    deviceContext->Unmap(scrollDataBuffer, 0);
    deviceContext->PSSetConstantBuffers(0, 1, &scrollDataBuffer);

    // Set blend state for proper transparency blending.
    float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    deviceContext->OMSetBlendState(blendState, blendFactor, 0xFFFFFFFF);

    // Set shader texture and sampler resources in the pixel shader.
    deviceContext->PSSetShaderResources(0, 1, &texture);
    deviceContext->PSSetSamplers(0, 1, &sampleState);
}